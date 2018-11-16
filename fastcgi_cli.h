/**
 * fastcgi_cli.h
 *
 * C++ versions 4.4.5
 *
 *      fcgicli-php : https://github.com/YujiroTakahashi/fcgicli-php
 *      Copyright (c) 2018 CROCO All Rights Reserved.
 *
 * @package         fcgicli-php
 * @copyright       Copyright (c) 2018 CROCO
 * @author          Yujiro Takahashi <yujiro@cro-co.co.jp>
 * @filesource
 */

#ifndef __CROCO_FASTCGI_CLI_H__
#define __CROCO_FASTCGI_CLI_H__

#include <string>
#include <unordered_map>

#include <fcgiapp.h>
#include "fastcgi.h"

namespace croco {
/**
 * FastCGICliクラス
 *
 * @package     FastCGICli
 * @author      Yujiro Takahashi <yujiro3@gmail.com>
 */
class FastCGICli {
public:
    /**
     * 2次元配列コンテナの型定義
     * @typedef std::map<std::string, std::string>
     */
    typedef std::unordered_map<std::string, std::string> param_t;

    /**
     * ヘッダーサイズ
     * @const integer
     */
    const int HEADER_LEN = 8;

    /**
     * 書込サイズ
     * @const integer
     */
     const int WRITER_LEN = 8192;
private:
    int _requestId;
    int _sock;
    struct sockaddr *_address;
    int _addrlen;

public:
    FastCGICli(std::string listen, int port);
    FastCGICli(std::string listen);
    ~FastCGICli();
    void send(param_t &params, std::string &stdin);
    std::string request(param_t &params, std::string &stdin);

private:
    void _send(param_t &params, std::string &stdin);
    void _pair(FCGX_Stream *paramsStream, const char *key, const char *value);
    int _write(unsigned char type, const char *content, int length);
    int _read(std::string &response);
    bool _connect();
}; // class FastCGICli

}  // namespace croco

#endif // #ifndef __CROCO_FASTCGI_CLI_H__
