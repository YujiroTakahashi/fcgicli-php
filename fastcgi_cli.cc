/**
 * fastcgi_cli.cc
 *
 * C++ versions 4.4.5
 *
 *      FastCGICli-php : https://github.com/YujiroTakahashi/FastCGICli-php
 *      Copyright (c) 2018 CROCO All Rights Reserved.
 *
 * @package         FastCGICli-php
 * @copyright       Copyright (c) 2018 CROCO
 * @author          Yujiro Takahashi <yujiro@cro-co.co.jp>
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

#include "fastcgi_cli.h"

namespace croco {

/**
 * コンストラクタ
 *
 * @access public
 * @param string listen
 * @param int port
 */
FastCGICli::FastCGICli(std::string listen, int port)
{
    struct sockaddr_in *sin;
    sin = new struct sockaddr_in;

    bzero(sin, sizeof(struct sockaddr_in));
    sin->sin_family      = AF_INET;
    sin->sin_addr.s_addr = inet_addr(listen.c_str());
    sin->sin_port        = htons(port);

    _sock    = socket(AF_INET, SOCK_STREAM, 0);
    _address = (struct sockaddr *)sin;
    _addrlen = sizeof(*sin);
}

/**
 * コンストラクタ
 *
 * @access public
 * @param string listen
 */
FastCGICli::FastCGICli(std::string listen)
{
    struct sockaddr_un *sun;
    sun = new struct sockaddr_un;

    bzero(sun, sizeof(struct sockaddr_un));
    sun->sun_family = AF_LOCAL;
    strcpy(sun->sun_path, listen.c_str());

    _sock    = socket(PF_UNIX, SOCK_STREAM, 0);
    _address = (struct sockaddr *)sun;
    _addrlen = sizeof(*sun);
}

/**
 * デストラクタ
 *
 * @access public
 */
FastCGICli::~FastCGICli()
{
    delete _address;
}

/**
 * FastCGIアプリへ送信
 *
 * @access public
 * @param param_t params Param
 * @param string stdin Content
 * @return void
 */
void FastCGICli::send(param_t &params, std::string &stdin)
{
    _connect();
    _send(params, stdin);

    if (_sock > 0) {
        close(_sock);
    }
}

/**
 * FastCGIアプリへ送受信
 *
 * @access public
 * @param param_t params Param
 * @param string stdin Content
 * @return string
 */
std::string FastCGICli::request(param_t &params, std::string &stdin)
{
    _connect();
    _send(params, stdin);

    /* 末尾まで読み込み */
    int type;
    std::string response("");
    do {
        type = _read(response);
    } while (type != FCGI_END_REQUEST);

    if (_sock > 0) {
        close(_sock);
    }

    std::string::size_type pos = response.find("\r\n\r\n");
    return response.substr(pos + 4);
}

/**
 * ヘッダーパケットの送信
 *
 * @access private
 * @param param_t params Param
 * @param std::string stdin
 * @return void
 */
void FastCGICli::_send(param_t &params, std::string &stdin)
{
    _requestId = stdin.length() % 50;
    std::stringstream conlength;
    conlength << stdin.length();
    params["CONTENT_LENGTH"] = conlength.str();

    /* リクエストの開始準備 */
    FCGI_BeginRequestBody body;
    body.roleB1 = 0x00;
    body.roleB0 = FCGI_RESPONDER;
    body.flags  = 0x000;
    bzero(body.reserved, sizeof(body.reserved));
    _write(FCGI_BEGIN_REQUEST, (char*)&body, sizeof(body));

    /* FCGIパラメータの書き込み */
    FCGX_Stream *paramsStream = FCGX_CreateWriter(_sock, _requestId, WRITER_LEN, FCGI_PARAMS);

    for (auto &param : params) {
        _pair(paramsStream, param.first.c_str(), param.second.c_str());
    }
    FCGX_FClose(paramsStream);
    FCGX_FreeStream(&paramsStream);

    /* FastCGIコンテンツの送信 */
    _write(FCGI_STDIN, stdin.c_str(), stdin.length());
}

/**
 * ヘッダーパケットの読込
 *
 * @access private
 * @param FCGX_Stream *paramsStream
 * @param const char *key
 * @param const char *value
 * @return void
 */
void FastCGICli::_pair(FCGX_Stream *paramsStream, const char *key, const char *value)
{
    char header[HEADER_LEN];
    char *hedptr = &header[0];

    int keyLength = strlen(key);
    if (keyLength < 0x80) {
        *hedptr++ = (unsigned char) keyLength;
    } else {
        *hedptr++ = (unsigned char)((keyLength >> 24) | 0x80);
        *hedptr++ = (unsigned char)(unsigned char) (keyLength >> 16);
        *hedptr++ = (unsigned char)(unsigned char) (keyLength >> 8);
        *hedptr++ = (unsigned char)(unsigned char)  keyLength;
    }

    int valueLength = strlen(value);
    if (valueLength < 0x80) {
        *hedptr++ = (unsigned char) valueLength;
    } else {
        *hedptr++ = (unsigned char)((valueLength >> 24) | 0x80);
        *hedptr++ = (unsigned char)(unsigned char) (valueLength >> 16);
        *hedptr++ = (unsigned char)(unsigned char) (valueLength >> 8);
        *hedptr++ = (unsigned char)(unsigned char)  valueLength;
    }

    /* ヘッダーの書き込み */
    int result = FCGX_PutStr(&header[0], hedptr - &header[0], paramsStream);
    if (result != hedptr - &header[0]) {
        throw "Unable to put header buffer.";
    }

    /* キーの書き込み */
    result = FCGX_PutStr(key, keyLength, paramsStream);
    if (result !=  keyLength) {
        throw "Unable to put key buffer.";
    }

    /* 値の書き込み */
    result = FCGX_PutStr(value, valueLength, paramsStream);
    if (result !=  valueLength) {
        throw "Unable to putvalue buffer.";
    }
}

/**
 * FastCGIアプリへの書き込み
 *
 * @access private
 * @param unsigned char type
 * @param const char *content
 * @param int length
 * @return int
 */
int FastCGICli::_write(unsigned char type, const char *content, int length)
{
    FCGI_Header header;

    header.version         = (unsigned char) FCGI_VERSION_1;
    header.type            = (unsigned char) type;
    header.requestIdB1     = (unsigned char) ((_requestId >> 8) & 0xff);
    header.requestIdB0     = (unsigned char) ((_requestId     ) & 0xff);
    header.contentLengthB1 = (unsigned char) ((length >> 8    ) & 0xff);
    header.contentLengthB0 = (unsigned char) ((length         ) & 0xff);
    header.paddingLength   = (unsigned char) 0;
    header.reserved        = (unsigned char) 0;

    int count = -1;
    /* ヘッダーの送信 */
    count = write(_sock, (char *)&header, HEADER_LEN);
    if (count != HEADER_LEN) {
        throw "Unable to write fcgi header.";
    }

    /* コンテンツの送信 */
    count = write(_sock, content, length);
    if (count != length) {
        throw "Unable to write fcgi content.";
    }

    return HEADER_LEN + length;
}

/**
 * FastCGIアプリからの読み込み
 *
 * @access private
 * @param string response
 * @return int
 */
int FastCGICli::_read(std::string &response)
{
    FCGI_Header *header = new FCGI_Header();

    int count = -1;
    count = read(_sock, (char *)header, sizeof(header));
    if (count != HEADER_LEN) {
        throw "Unable to read fcgi header.";
    }
    int length = (header->contentLengthB1 << 8) + header->contentLengthB0;

    if (length > 0) {
        char *buf = new char[(length + header->paddingLength + 1)];
        count = read(_sock, buf, length + header->paddingLength);
        if (count != length + header->paddingLength) {
            throw "Unable to read fcgi content.";
        }
        if (header->type == FCGI_STDOUT) {
            response.append(buf);
        }
        delete[] buf;
    } // if (length > 0)

    count = header->type;
    delete header;

    return count;
}

/**
 * FastCGIサーバへ接続
 *
 * @access private
 * @return boolean
 */
bool FastCGICli::_connect()
{
    if (connect(_sock, _address, _addrlen)) {
        return false;
    }
    return true;
}

} // namespace croco
