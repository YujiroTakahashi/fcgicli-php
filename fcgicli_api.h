#ifndef PHP_FCGICLI_API_H
#define PHP_FCGICLI_API_H

#include <string.h>
#include <stdint.h>

#ifdef __cplusplus

#include "fastcgi_cli.h"
#include "json.hpp"

extern "C" {

#endif /* __cplusplus */

#ifndef FCGICLI_API
#   if defined(_WIN32) || defined(_WIN64)
#       define FCGICLI_API __declspec(dllimport)
#   else
#       define FCGICLI_API extern
#   endif /* defined(_WIN32) || defined(_WIN64) */
#endif /* FCGICLI_API */

#ifndef FCGICLI_VERSION
#    define FCGICLI_VERSION 12
#endif /* FCGICLI_VERSION */

#define FCGICLI_TRUE           (1)
#define FCGICLI_FALSE          (0)

typedef void *MultiRequestHandle;

FCGICLI_API int MultiRequestSize();
FCGICLI_API MultiRequestHandle MultiRequestCreate();
FCGICLI_API void MultiRequestFree(MultiRequestHandle handle);
FCGICLI_API void MultiRequestFreeText(char *text);
FCGICLI_API int MultiRequestConnect(MultiRequestHandle handle, const char* listen, const int port);
FCGICLI_API int MultiRequestUnixDomein(MultiRequestHandle handle, const char* listen);
FCGICLI_API int MultiRequestSetParam(MultiRequestHandle handle, const char* key, const char* value);
FCGICLI_API int MultiRequestSetContents(MultiRequestHandle handle, const char* contents);
FCGICLI_API char *MultiRequestExec(MultiRequestHandle handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PHP_FCGICLI_API_H */
