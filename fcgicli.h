/**
 * fcgi_FCgiCli.h
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

#ifndef __CROCO_FCGCLI_H__
#define __CROCO_FCGCLI_H__

#include <string>
#include <map>

namespace croco {
/**
 * FCGIプロトコル開始リクエスト本文クラス
 *
 * @package     FCGIFCgiCli
 * @author      Yujiro Takahashi <yujiro3@gmail.com>
 */
class FCgiCli {
public:
    /**
     * 2次元配列コンテナの型定義
     * @typedef std::map<std::string, std::string>
     */
    typedef std::map<std::string, std::string> param_t;

    /**
     * 文字の ASCII 値を返す
     */
    inline unsigned int ord(int val) {
        return ((unsigned int) val & 0xff);
    }

    /**
     * ヘッダー
     * @typedef struct header_t
     */
    typedef struct {
        unsigned int version;
        unsigned int type;
        unsigned int requestId;
        unsigned int contentLength;
        unsigned int paddingLength;
        unsigned int reserved;
        unsigned int flag;
    } header_t;

    /**
     * バージョン
     * @const unsigned int
     */
    const unsigned int VERSION = 1;

    /**
     * タイプ：開始リクエスト
     * @const unsigned int
     */
    const unsigned int BEGIN_REQUEST = 1;

    /**
     * タイプ：中断リクエスト
     * @const unsigned int
     */
    const unsigned int ABORT_REQUEST = 2;

    /**
     * タイプ：終了リクエスト
     * @const unsigned int
     */
    const unsigned int END_REQUEST = 3;

    /**
     * タイプ：引数
     * @const unsigned int
     */
    const unsigned int PARAMS = 4;

    /**
     * タイプ：標準入力
     * @const unsigned int
     */
    const unsigned int STDIN = 5;

    /**
     * タイプ：標準出力
     * @const unsigned int
     */
    const unsigned int STDOUT = 6;

    /**
     * タイプ：エラー
     * @const unsigned int
     */
    const unsigned int STDERR = 7;

    /**
     * タイプ：データ
     * @const unsigned char
     */
    const int DATA                 = 8;
    const int GET_VALUES           = 9;
    const int GET_VALUES_RESULT    = 10;
    const int UNKNOWN_TYPE         = 11;
    const int MAXTYPE              = 11;

    const int RESPONDER            = 1;
    const int AUTHORIZER           = 2;
    const int FILTER               = 3;

    /**
     * フラグ：コンプリート
     * @const unsigned char
     */
    const unsigned char REQUEST_COMPLETE = 0;
    const unsigned char CANT_MPX_CONN    = 1;
    const unsigned char OVERLOADED       = 2;
    const unsigned char UNKNOWN_ROLE     = 3;

    /**
     * ヘッダーサイズ
     * @const integer
     */
    const int HEADER_LEN = 8;

    /**
     * 読込サイズ
     * @const integer
     */
    const int READ_LEN = 1024;

    /**
     * 最大値
     * @const integer
     */
    const int MAX_LENGTH = 0xffff;

private:
    int _sock;
    struct sockaddr *_address;
    int _addrlen;

public:

    FCgiCli(std::string listen, int port);
    FCgiCli(std::string listen);
    ~FCgiCli();
    bool send(param_t &params, std::string &stdin);
    std::string request(param_t &params, std::string &stdin);
    std::string multiRequest(param_t &params, std::string &stdin);

private:
    void _buildRecord(std::string &record, param_t &params, std::string &stdin);
    void _buildPacket(std::string &record, int type, std::string &content, int requestId);
    std::string _buildNvpair(std::string name, std::string value);
    void _readPacketHeader(header_t &header);
    void _readPacket(header_t &header, std::string &response);
    bool _connect();
};

}  // namespace croco

#endif // #ifndef __CROCO_FCGCLI_H__
