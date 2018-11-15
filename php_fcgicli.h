#ifndef PHP_FCGICLI_H
#define PHP_FCGICLI_H

#include "fcgicli_api.h"

#define PHP_FCGICLI_VERSION	"0.1.0"

extern zend_module_entry fcgicli_module_entry;
#define phpext_fcgicli_ptr &fcgicli_module_entry

ZEND_BEGIN_MODULE_GLOBALS(fcgicli)
	char *model_dir;
ZEND_END_MODULE_GLOBALS(fcgicli)

#ifdef ZTS
# define FCGICLI_G(v) TSRMG(fcgicli_globals_id, zend_fcgicli_globals *, v)
# ifdef COMPILE_DL_FCGICLI
ZEND_TSRMLS_CACHE_EXTERN()
# endif
#else
# define FCGICLI_G(v) (fcgicli_globals.v)
#endif

typedef struct {
    zend_object zo;
	zval error;
    MultiRequestHandle fcgicli;
} php_fcgicli_object;

static inline php_fcgicli_object *php_fcgicli_from_obj(zend_object *obj) {
	return (php_fcgicli_object*)((char*)(obj) - XtOffsetOf(php_fcgicli_object, zo));
}

#define Z_FCGICLI_P(zv) php_fcgicli_from_obj(Z_OBJ_P((zv)))


#endif  /* PHP_FCGICLI_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
