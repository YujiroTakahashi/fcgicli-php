#include <string>
#include <unordered_map>
#include <uv.h>
#include "fcgicli_api.h"
#include "MultiRequest.h"

/**
 * get MultiRequest size
 *
 * @access public
 * @return int
 */
int MultiRequestSize()
{
    return sizeof(croco::MultiRequest);
}

/**
 * create a MultiRequest handle
 *
 * @access public
 * @return MultiRequestHandle
 */
MultiRequestHandle MultiRequestCreate()
{
    MultiRequestHandle handle = new croco::MultiRequest;
    return handle;
}

/**
 * free a JumanPP handle
 *
 * @access public
 * @param  MultiRequestHandle handle
 * @return void
 */
void MultiRequestFree(MultiRequestHandle handle)
{
    delete static_cast<croco::MultiRequest*>(handle);
}

/**
 * free a char handle
 *
 * @access public
 * @param  char *text
 * @return void
 */
void MultiRequestFreeText(char *text)
{
    delete[] text;
}

/**
 * set connection
 *
 * @access public
 * @param  MultiRequestHandle handle
 * @param  const char* listen
 * @param  const int port
 * @return int
 */
int MultiRequestConnect(MultiRequestHandle handle, const char* listen, const int port)
{
    croco::MultiRequest *mreq = static_cast<croco::MultiRequest*>(handle);
    mreq->setListen(listen, port);
    return FCGICLI_TRUE;
}

/**
 * set connection
 *
 * @access public
 * @param  MultiRequestHandle handle
 * @param  const char* listen
 * @return int
 */
int MultiRequestUnixDomein(MultiRequestHandle handle, const char* listen)
{
    croco::MultiRequest *mreq = static_cast<croco::MultiRequest*>(handle);
    mreq->setListen(listen);
    return FCGICLI_TRUE;
}

/**
 * analyze
 *
 * @access public
 * @param  MultiRequestHandle handle
 * @param  const char* word
 * @return JPStr
 */
int MultiRequestSetParam(MultiRequestHandle handle, const char* key, const char* value)
{
    croco::MultiRequest *mreq = static_cast<croco::MultiRequest*>(handle);
    mreq->setParam(key, value);
}

/**
 * analyze
 *
 * @access public
 * @param  MultiRequestHandle handle
 * @param  const char* word
 * @return JPStr
 */
int MultiRequestSetContents(MultiRequestHandle handle, const char* contents)
{
    croco::MultiRequest *mreq = static_cast<croco::MultiRequest*>(handle);
    mreq->setContents(contents);
}


/**
 * analyze
 *
 * @access public
 * @param  MultiRequestHandle handle
 * @return JPStr
 */
char *MultiRequestExec(MultiRequestHandle handle)
{
    croco::MultiRequest *mreq = static_cast<croco::MultiRequest*>(handle);
    std::string result = mreq->exec();

    char *text = new char[result.size() + 1];
    strcpy(text, result.c_str());

    return text;
}
