/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2007 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id: header,v 1.16.2.1.2.1 2007/01/01 19:32:09 iliaa Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_octopus.h"
#include "oct_cli.h"

/* If you declare any globals in php_octopus.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(octopus)
*/

/* True global resources - no need for thread safety here */
static int le_octopus;

/* {{{ octopus_functions[]
 *
 * Every user visible function must have an entry in octopus_functions[].
 */
zend_function_entry octopus_functions[] = {
        PHP_FE(oct_upload_file, NULL)
        PHP_FE(oct_upload_pack_file, NULL)
        PHP_FE(oct_delete_file, NULL)
        PHP_FE(oct_move_file, NULL)
        PHP_FE(oct_copy_file, NULL)
        PHP_FE(oct_get_error, NULL)
	{NULL, NULL, NULL}	/* Must be the last line in octopus_functions[] */
};
/* }}} */

/* {{{ octopus_module_entry
 */
zend_module_entry octopus_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"octopus",
	octopus_functions,
	PHP_MINIT(octopus),
	PHP_MSHUTDOWN(octopus),
	PHP_RINIT(octopus),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(octopus),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(octopus),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_OCTOPUS
ZEND_GET_MODULE(octopus)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("octopus.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_octopus_globals, octopus_globals)
    STD_PHP_INI_ENTRY("octopus.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_octopus_globals, octopus_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_octopus_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_octopus_init_globals(zend_octopus_globals *octopus_globals)
{
	octopus_globals->global_value = 0;
	octopus_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(octopus)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(octopus)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(octopus)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(octopus)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(octopus)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "octopus support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


/* Remove the following function when you have succesfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */


PHP_FUNCTION(oct_upload_file)
{
        char *ip    = NULL;
        char *site  = NULL;
        char *remote= NULL;
        char *local = NULL;

        long ip_len     = 0;
        long site_len   = 0;
        long remote_len = 0;
        long local_len  = 0;
        long port       = 0;
        long type       = 0;
        long ret        = 0;

        if ( 6 == ZEND_NUM_ARGS() ) {
                ret = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sllsss",
                        &ip,    &ip_len,
                        &port,
                        &type,
                        &site,  &site_len,
                        &remote,&remote_len,
                        &local, &local_len);
        } else {
                RETURN_LONG (-1);
        }

        ret = oct_upload_file_(ip, port, type, site, remote, local);

        RETURN_LONG(ret);
}

PHP_FUNCTION(oct_upload_pack_file)
{
        char *ip    = NULL;
        char *site  = NULL;
        char *remote= NULL;
        char *local = NULL;

        long ip_len     = 0;
        long site_len   = 0;
        long remote_len = 0;
        long local_len  = 0;
        long port       = 0;
        long type       = 0;
        long ret        = 0;

        if ( 6 == ZEND_NUM_ARGS() ) {
                ret = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sllsss",
                        &ip,    &ip_len,
                        &port,  &type,
                        &site,  &site_len,
                        &remote,&remote_len,
                        &local, &local_len);
        } else {
                RETURN_LONG (-1);
        }

        ret = oct_upload_pack_file_(ip, port, type, site, remote, local);

        RETURN_LONG(ret);
}

PHP_FUNCTION(oct_delete_file)
{
        char *ip    = NULL;
        char *site  = NULL;
        char *remote= NULL;

        long ip_len     = 0;
        long site_len   = 0;
        long remote_len = 0;
        long port       = 0;
        long ret        = 0;

        if ( 4 == ZEND_NUM_ARGS() ) {
                ret = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "slss",
                        &ip,    &ip_len,
                        &port,
                        &site,  &site_len,
                        &remote,&remote_len);
        } else {
                RETURN_LONG (-1);
        }

        ret = oct_delete_file_(ip, port, site, remote);

        RETURN_LONG(ret);
}

PHP_FUNCTION(oct_move_file)
{
        int result;
        char *ip = NULL;
        char *targetSite = NULL;
        char *targetPath = NULL;
        char *srcSite = NULL;
        char *srcPath = NULL;
        long ip_len = 0;
        long targetSite_len = 0;
        long targetPath_len = 0;
        long srcSite_len = 0;
        long srcPath_len = 0;
        long port = 0;

        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "slssss",
                &ip, &ip_len,
                &port,
                &targetSite, &targetSite_len,
                &targetPath,  &targetPath_len,
                &srcSite,  &srcSite_len,
                &srcPath,  &srcPath_len) == FAILURE) {
                return;
        }

        result = oct_move_file_(ip, port, targetSite, targetPath, srcSite, srcPath);

        RETURN_LONG(result);
}

PHP_FUNCTION(oct_copy_file)
{
        int result;
        char *ip = NULL;
        char *targetSite = NULL;
        char *targetPath = NULL;
        char *srcSite = NULL;
        char *srcPath = NULL;
        long ip_len = 0;
        long targetSite_len = 0;
        long targetPath_len = 0;
        long srcSite_len = 0;
        long srcPath_len = 0;
        long port = 0;

        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "slssss",
                &ip, &ip_len,
                &port,
                &targetSite,  &targetSite_len,
                &targetPath,  &targetPath_len,
                &srcSite,  &srcSite_len,
                &srcPath,  &srcPath_len) == FAILURE) {
                return;
        }

        result = oct_copy_file_(ip, port, targetSite, targetPath, srcSite, srcPath);

        RETURN_LONG(result);
}

PHP_FUNCTION(oct_get_error)
{
        char result[500];
        sprintf(result, "%s", oct_get_error_());

        RETURN_STRING(result, 1);
}


/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
