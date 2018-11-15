/**
 * fcgi_FCgiCli.cpp
 *
 * C++ versions 4.4.5
 *
 *      fcgi_FCgiCli : https://github.com/Yujiro3/fcgicli
 *      Copyright (c) 2011-2013 sheeps.me All Rights Reserved.
 *
 * @package         fcgi_FCgiCli
 * @copyright       Copyright (c) 2011-2013 sheeps.me
 * @author          Yujiro Takahashi <yujiro3@gmail.com>
 * @filesource
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <cstring>
#include <cassert>
#include <cstdlib>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <uv.h>

#include "fcgicli.h"

namespace croco {

/**
 * コンストラクタ
 *
 * @access public
 * @param string listen
 * @param int port
 */
FCgiCli::FCgiCli(std::string listen, int port) {
    struct sockaddr_in *sin;
    sin = new struct sockaddr_in;
    bzero(sin, sizeof(struct sockaddr_in));
    sin->sin_family = AF_INET;
    sin->sin_addr.s_addr = inet_addr(listen.c_str());
    sin->sin_port = htons(port);

    _sock = socket(AF_INET, SOCK_STREAM, 0);
    _address = (struct sockaddr *)sin;
    _addrlen = sizeof(*sin);
}

/**
 * コンストラクタ
 *
 * @access public
 * @param string listen
 */
FCgiCli::FCgiCli(std::string listen) {
    struct sockaddr_un *sun;
    sun = new struct sockaddr_un;

    bzero(sun, sizeof(struct sockaddr_un));
    sun->sun_family = AF_LOCAL;
    strcpy(sun->sun_path, listen.c_str());
    _sock = socket(PF_UNIX, SOCK_STREAM, 0);
    _address = (struct sockaddr *)sun;
    _addrlen = sizeof(*sun);
}

/**
 * デストラクタ
 *
 * @access public
 */
FCgiCli::~FCgiCli() {
    delete _address;
}

/**
 * Execute a send to the FastCGI application
 *
 * @access public
 * @param String stdin Content
 * @return String
 */
bool FCgiCli::send(param_t &params, std::string &stdin) {
    std::string record("");

    _buildRecord(record, params, stdin);

    if (!_connect()) {
        return false;
    }

    write(_sock, record.data(), record.size());

    if (_sock > 0) {
        close(_sock);
    }

    return true;
}

/**
 * Execute a request to the FastCGI application
 *
 * @access public
 * @param String stdin Content
 * @return String
 */
std::string FCgiCli::request(param_t &params, std::string &stdin) {
    std::string record("");
    _buildRecord(record, params, stdin);

    if (!_connect()) {
        return std::string("Unable to connect to FastCGI application.");
    }
    write(_sock, record.data(), record.size());

    header_t header = {0, 0, 0, 0, 0, 0, 0};
    std::string response("");
    do {
        _readPacket(header, response);
    } while (header.type != END_REQUEST);

    if (_sock > 0) {
        close(_sock);
    }

    /* 終了チェック */
    if (header.flag == REQUEST_COMPLETE) {
        return response;
    } else if (header.flag == CANT_MPX_CONN) {
        return std::string("This app can't multiplex [CANT_MPX_CONN]");
    } else if (header.flag == OVERLOADED) {
        return std::string("New request rejected; too busy [OVERLOADED]");
    } else if (header.flag == UNKNOWN_ROLE) {
        return std::string("Role value not known [UNKNOWN_ROLE]");
    }
    return response;
}

/**
 * レコードの作成
 *
 * @access public
 * @param String stdin Content
 * @return void
 */
void FCgiCli::_buildRecord(std::string &record, param_t &params, std::string &stdin)
{
    std::stringstream conlength;
    conlength << stdin.length();

    params["CONTENT_LENGTH"] = conlength.str();

    std::string head("");
    head += char(0);
    head += char(1);    // Responder
    head += char(0);    // Keep alive
    head += char(0); head += char(0); head += char(0); head += char(0); head += char(0);
    _buildPacket(record, BEGIN_REQUEST, head, 1);

    std::string paramsRequest("");
    for (auto &param : params) {
        paramsRequest += _buildNvpair(param.first, param.second);
    }

    std::string empty("");
    if (paramsRequest.size() > 0) {
        _buildPacket(record, PARAMS, paramsRequest, 1);
    }
    _buildPacket(record, PARAMS, empty, 1);

    if (stdin.size() > 0) {
        _buildPacket(record, STDIN, stdin, 1);
    }
    _buildPacket(record, STDIN, empty, 1);
}

/**
 * パケット作成
 *
 * @access private
 * @param  int type
 * @param  string content
 * @param  int requestId
 */
void FCgiCli::_buildPacket(std::string &record, int type, std::string &content, int requestId) {
    int contentLength = content.size();

    assert(contentLength >= 0 && contentLength <= MAX_LENGTH);

    record += char(VERSION);                            // version
    record += char(type);                               // type
    record += char((requestId     >> 8) & 0xff);        // requestIdB1
    record += char((requestId         ) & 0xff);        // requestIdB0
    record += char((contentLength >> 8) & 0xff);        // contentLengthB1
    record += char((contentLength     ) & 0xff);        // contentLengthB0
    record += char(0);                                  // paddingLength
    record += char(0);                                  // reserved
    record.append(content.data(), content.size());      // content
}

/**
 * FastCGIヘッダー部分の作製
 *
 * @access private
 * @param string name Name
 * @param string value Value
 * @return string FastCGI Name value pair
 */
std::string FCgiCli::_buildNvpair(std::string name, std::string value) {
    std::string nvpair("");

    int nlen = name.size();
    if (nlen < 128) {
        nvpair  = (unsigned char) nlen;                     // name LengthB0
    } else {
        nvpair  = (unsigned char) ((nlen >> 24) | 0x80);    // name LengthB3
        nvpair += (unsigned char) ((nlen >> 16) & 0xff);    // name LengthB2
        nvpair += (unsigned char) ((nlen >>  8) & 0xff);    // name LengthB1
        nvpair += (unsigned char) (nlen & 0xff);            // name LengthB0
    }

    int vlen = value.size();
    if (vlen < 128) {
        nvpair += (unsigned char) vlen;                     // value LengthB0
    } else {
        nvpair += (unsigned char) ((vlen >> 24) | 0x80);    // value LengthB3
        nvpair += (unsigned char) ((vlen >> 16) & 0xff);    // value LengthB2
        nvpair += (unsigned char) ((vlen >>  8) & 0xff);    // value LengthB1
        nvpair += (unsigned char) (vlen & 0xff);            // value LengthB0
    }
    nvpair += name + value;

    /* nameData & valueData */
    return nvpair;
}

/**
 * ヘッダーパケットの読込
 *
 * @access private
 * @return void
 */
void FCgiCli::_readPacketHeader(header_t &header) {
    char pack[HEADER_LEN];

    int len = read(_sock, pack, sizeof(pack));
    if (len < 0) {
        throw "Unable to read response header.";
    }
    header.version       = ord(pack[0]);
    header.type          = ord(pack[1]);
    header.requestId     = (ord(pack[2]) << 8) + ord(pack[3]);
    header.contentLength = (ord(pack[4]) << 8) + ord(pack[5]);
    header.paddingLength = ord(pack[6]);
    header.reserved      = ord(pack[7]);
}

/**
 * パケットの読み込み
 *
 * @access private
 * @return void
 */
void FCgiCli::_readPacket(header_t &header, std::string &response) {
    /* ヘッダーデータの読込 */
    _readPacketHeader(header);

    if (header.contentLength > 0) {
        char* buff = new char[header.contentLength];
        int length = read(_sock, buff, header.contentLength);
        if (length < 0) {
            throw "Unable to read content buffer.";
        }

        if (header.type == STDOUT || header.type == STDERR) {
            response.append(buff);
        } else if (header.type == END_REQUEST) {
            header.flag = buff[4];
        }
        delete [] buff;
    }

    /* Paddingデータの読込 */
    if (header.paddingLength > 0) {
        char* padding = new char[header.paddingLength];
        int len = read(_sock, padding, header.paddingLength);
        delete [] padding;
        if (len < 0) {
            throw "Unable to read padding buffer.";
        }
    } // if (header.paddingLength > 0)
}

/**
 * FastCGIサーバへ接続
 *
 * @access private
 * @return void
 */
bool FCgiCli::_connect() {
    if (connect(_sock, _address, _addrlen)) {
        return false;
    }
    return true;
}

} // namespace croco
