/**
 * MultiRequest.h
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

#ifndef __CROCO_MULTI_REQUEST_H__
#define __CROCO_MULTI_REQUEST_H__

#include <string>
#include <unordered_map>
#include <vector>
#include <uv.h>

namespace croco {
/**
 * MultiRequestクラス
 *
 * @package     MultiRequest
 * @author      Yujiro Takahashi <yujiro@cro-co.co.jp>
 */
class MultiRequest {
public:
    /**
     * request data
     * @typedef struct request_t
     */
    typedef struct {
        uv_work_t req;
        std::string listen;
        int port;
        std::unordered_map<std::string, std::string> params;
        std::string contents;
        std::string response;
    } request_t;

private:
    std::string _listen;
    int _port;
    uv_loop_t *_loop;
    std::vector<request_t> _requests;
    std::unordered_map<std::string, std::string> _params;

public:
    ~MultiRequest();
    void connect(std::string listen, int port);
    void connect(std::string listen);
    void setParam(const char* key, const char* value);
    void setContents(const char* contents);
    std::string exec();

private:
    static void _worker(uv_work_t *req);
    static void _workerAfter(uv_work_t *req, int status);
}; // class MultiRequest

}  // namespace croco

#endif // #ifndef __CROCO_MULTI_REQUEST_H__
