/**
 * fcgi_MultiRequest.cpp
 *
 * C++ versions 4.4.5
 *
 *      fcgi_MultiRequest : https://github.com/Yujiro3/MultiRequest
 *      Copyright (c) 2011-2013 sheeps.me All Rights Reserved.
 *
 * @package         fcgi_MultiRequest
 * @copyright       Copyright (c) 2011-2013 sheeps.me
 * @author          Yujiro Takahashi <yujiro3@gmail.com>
 * @filesource
 */
#include <iostream>
#include "fastcgi_cli.h"
#include "json.hpp"
#include "MultiRequest.h"

namespace croco {

/**
 * デストラクタ
 *
 * @access public
 */
MultiRequest::~MultiRequest()
{
    uv_loop_delete(_loop);
}

/**
 * 接続情報の設定
 *
 * @access public
 * @param string listen
 * @param int port
 */
void MultiRequest::setListen(std::string listen, int port)
{
    _listen = listen;
    _port = port;
}

/**
 * 接続情報の設定
 *
 * @access public
 * @param string listen
 */
void MultiRequest::setListen(std::string listen)
{
    _listen = listen;
    _port = 0;
}

/**
 * パラメータの設定：FastCGI引数
 *
 * @access public
 * @return String
 */
void MultiRequest::setParam(const char* key, const char* value)
{
    _params[std::string(key)] = std::string(value);
}

/**
 * パラメータの設定：コンテンツ
 *
 * @access public
 * @return String
 */
void MultiRequest::setContents(const char* contents)
{
    request_t request;

    request.listen = _listen;
    request.port = _port;
    request.contents = std::string(contents);
    request.params = _params;

    _requests.push_back(request);
    _params.clear();
}

/**
 * Execute a multi request to the FastCGI application
 *
 * @access public
 * @return String
 */
std::string MultiRequest::exec()
{
    _loop = uv_default_loop();

    for (auto &request : _requests) {
        request.req.data = (void*)&request;

        uv_queue_work(_loop, &request.req, _worker, _workerAfter);
    }

    uv_run(_loop, UV_RUN_DEFAULT);

    nlohmann::json retval;
    for (auto &request : _requests) {
        retval[request.params["REQUEST_URI"]] = request.response;
    }
    _requests.clear();

    return retval.dump();
}

/**
 * Execute a request to the FastCGI application
 *
 * @access private
 * @return void
 */
void MultiRequest::_worker(uv_work_t *req)
{
    request_t *request = static_cast<request_t *>(req->data);

    if (0 == request->port) {
        FastCGICli fcli(request->listen);
        request->response = fcli.request(request->params, request->contents);
    } else {
        FastCGICli fcli(request->listen, request->port);
        request->response = fcli.request(request->params, request->contents);
    }

    return;
}

/**
 * After Execute
 *
 * @access private
 * @return void
 */
void MultiRequest::_workerAfter(uv_work_t *req, int status)
{
    request_t *request = static_cast<request_t *>(req->data);

    return ;
}

} // namespace croco
