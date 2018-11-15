#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/json/php_json.h"
#include "php_fcgicli.h"
#include "main/SAPI.h"

#include "zend_exceptions.h"
#include "zend_interfaces.h"
#include "SAPI.h"

ZEND_DECLARE_MODULE_GLOBALS(fcgicli)

/* {{{ PHP_INI
*/
PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("fcgicli.model_dir",  NULL, PHP_INI_SYSTEM, OnUpdateString, model_dir, zend_fcgicli_globals, fcgicli_globals)
PHP_INI_END()
/* }}} */

/* Handlers */
static zend_object_handlers fcgicli_object_handlers;

/* Class entries */
zend_class_entry *php_fcgicli_sc_entry;


/* {{{ proto void fcgicli::__construct()
 */
PHP_METHOD(fcgicli, __construct)
{
	php_fcgicli_object *fcgi_obj;
	zval *object = getThis();

	fcgi_obj = Z_FCGICLI_P(object);

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	fcgi_obj->fcgicli = MultiRequestCreate();
}
/* }}} */

/* {{{ proto long fcgicli::connect(string listen, int port)
 */
PHP_METHOD(fcgicli, connect)
{
	php_fcgicli_object *fcgi_obj;
	zval *object = getThis();
	char *listen;
	size_t listen_len;
	zend_long port = 0;
	zend_long res;

	ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_STRING(listen, listen_len)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(port)
    ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

	fcgi_obj = Z_FCGICLI_P(object);

	if (0 == port) {
		res = MultiRequestUnixDomein(fcgi_obj->fcgicli, listen);
	} else {
		res = MultiRequestConnect(fcgi_obj->fcgicli, listen, port);
	}

	RETURN_LONG(res);
}
/* }}} */

/* {{{ proto void fcgicli::setParams(array params, string contents)
 */
PHP_METHOD(fcgicli, setParams)
{
	php_fcgicli_object *fcgi_obj;
	zval *object = getThis();
	zval *params = NULL;
	char *contents;
	size_t contents_len;

	zend_string *attribute;
	zend_ulong index;
	HashTable *ht;
	int idx, num_params;

	ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_ARRAY(params)
		Z_PARAM_STRING(contents, contents_len)
    ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

	fcgi_obj = Z_FCGICLI_P(object);

	ht = Z_ARRVAL_P(params);
	num_params = zend_hash_num_elements(ht);

	for (idx=0; idx<num_params; idx++) {
		if (zend_hash_get_current_key(ht, &attribute, &index) == HASH_KEY_IS_STRING) {
			zend_string *buf;
			zval *value = zend_hash_get_current_data(ht);
			buf = zval_get_string(value);

			MultiRequestSetParam(fcgi_obj->fcgicli, ZSTR_VAL(attribute), ZSTR_VAL(buf));
			zend_string_release(buf);
		}
		zend_hash_move_forward(ht);
	}

	MultiRequestSetContents(fcgi_obj->fcgicli, contents);
}
/* }}} */


/* {{{ proto mixed fcgicli::exec()
 */
PHP_METHOD(fcgicli, exec)
{
	php_fcgicli_object *fcgi_obj;
	zval *object = getThis();
	char *resutl;

	fcgi_obj = Z_FCGICLI_P(object);

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	resutl = MultiRequestExec(fcgi_obj->fcgicli);

	php_json_decode(return_value, resutl, strlen(resutl), 1, PHP_JSON_PARSER_DEFAULT_DEPTH);
	MultiRequestFreeText(resutl);
}
/* }}} */


/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO(arginfo_fcgicli_void, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fcgicli_one_opt, 0, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, arg1)
	ZEND_ARG_INFO(0, arg2)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fcgicli_two, 0, 0, 1)
	ZEND_ARG_INFO(0, arg1)
	ZEND_ARG_INFO(0, arg2)
ZEND_END_ARG_INFO()
/* }}} */


/* {{{ php_sfcgicli_class_methods */
static zend_function_entry php_fcgicli_class_methods[] = {
	PHP_ME(fcgicli, __construct, arginfo_fcgicli_void,    ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(fcgicli, connect,     arginfo_fcgicli_one_opt, ZEND_ACC_PUBLIC)
	PHP_ME(fcgicli, setParams, 	 arginfo_fcgicli_two,     ZEND_ACC_PUBLIC)
	PHP_ME(fcgicli, exec,        arginfo_fcgicli_void,    ZEND_ACC_PUBLIC)

	PHP_FE_END
};
/* }}} */

static void php_fcgicli_object_free_storage(zend_object *object) /* {{{ */
{
	php_fcgicli_object *intern = php_fcgicli_from_obj(object);

	if (!intern) {
		return;
	}

	if (intern->fcgicli) {
		MultiRequestFree(intern->fcgicli);
		intern->fcgicli = NULL;
	}

	zend_object_std_dtor(&intern->zo);
}
/* }}} */

static zend_object *php_fcgicli_object_new(zend_class_entry *class_type) /* {{{ */
{
	php_fcgicli_object *intern;

	/* Allocate memory for it */
	int mrsize = MultiRequestSize();
	intern = ecalloc(1, sizeof(php_fcgicli_object) + zend_object_properties_size(class_type) + mrsize);

	zend_object_std_init(&intern->zo, class_type);
	object_properties_init(&intern->zo, class_type);

	intern->zo.handlers = &fcgicli_object_handlers;

	return &intern->zo;
}
/* }}} */


/* {{{ PHP_MINIT_FUNCTION
*/
PHP_MINIT_FUNCTION(fcgicli)
{
	zend_class_entry ce;

	memcpy(&fcgicli_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

	/* Register Fcgicli Class */
	INIT_CLASS_ENTRY(ce, "fcgicli", php_fcgicli_class_methods);
	ce.create_object = php_fcgicli_object_new;
	fcgicli_object_handlers.offset = XtOffsetOf(php_fcgicli_object, zo);
	fcgicli_object_handlers.clone_obj = NULL;
	fcgicli_object_handlers.free_obj = php_fcgicli_object_free_storage;
	php_fcgicli_sc_entry = zend_register_internal_class(&ce);

	REGISTER_INI_ENTRIES();

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
*/
PHP_MSHUTDOWN_FUNCTION(fcgicli)
{
	UNREGISTER_INI_ENTRIES();

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
*/
PHP_MINFO_FUNCTION(fcgicli)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "FastCGI-cli support", "enabled");
	php_info_print_table_row(2, "FastCGI-cli module version", PHP_FCGICLI_VERSION);
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ PHP_GINIT_FUNCTION
*/
static PHP_GINIT_FUNCTION(fcgicli)
{
	memset(fcgicli_globals, 0, sizeof(*fcgicli_globals));
}
/* }}} */

/* {{{ fcgicli_module_entry
*/
zend_module_entry fcgicli_module_entry = {
	STANDARD_MODULE_HEADER,
	"fcgicli",
	NULL,
	PHP_MINIT(fcgicli),
	PHP_MSHUTDOWN(fcgicli),
	NULL,
	NULL,
	PHP_MINFO(fcgicli),
	PHP_FCGICLI_VERSION,
	PHP_MODULE_GLOBALS(fcgicli),
	PHP_GINIT(fcgicli),
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_FCGICLI
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(fcgicli)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
