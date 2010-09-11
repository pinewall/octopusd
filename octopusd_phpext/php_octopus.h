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

#ifndef PHP_OCTOPUS_H
#define PHP_OCTOPUS_H

extern zend_module_entry octopus_module_entry;
#define phpext_octopus_ptr &octopus_module_entry

#ifdef PHP_WIN32
#define PHP_OCTOPUS_API __declspec(dllexport)
#else
#define PHP_OCTOPUS_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(octopus);
PHP_MSHUTDOWN_FUNCTION(octopus);
PHP_RINIT_FUNCTION(octopus);
PHP_RSHUTDOWN_FUNCTION(octopus);
PHP_MINFO_FUNCTION(octopus);

/*
 * �ϴ��ļ�
 * ���ø�ʽ oct_upload_file(string server, int port, int sign, string site, string remote, string local)
 * server ������������ip
 * port �������˿�
 * sign �ϴ����� 0x1�ļ��� 0x2�ļ���
 * site վ������
 * remote �ļ���Զ�˵������վĿ¼�����Ŀ¼���ļ�����
 * local �ļ��ڱ��صľ���Ŀ¼������
 */
PHP_FUNCTION(oct_upload_file);

/*
 * �ϴ�����ļ�
 * ���ø�ʽ oct_upload_pack_file(string server, int port, int sign, string site, string remote, string local)
 * server ������������ip
 * port �������˿�
 * sign �ϴ����� 0x1�ļ��� 0x2�ļ���
 * site վ������
 * remote �ļ���Զ�˵������վĿ¼�����Ŀ¼
 * local �ļ��ڱ��صľ���Ŀ¼������
 */
PHP_FUNCTION(oct_upload_pack_file);

/*
 * ɾ���ļ�
 * ���ø�ʽ oct_delete_file(string server, int port, string site, string remote)
 * server ������������ip
 * port �������˿�
 * site վ������
 * remote �ļ���Զ�˵������վĿ¼�����Ŀ¼���ļ�����
 */
PHP_FUNCTION(oct_delete_file);

/*
 * �ƶ��ļ�
 * ���ø�ʽ oct_move_file(string server, int port, string dest, string dest_path, string src, string src_path)
 * server ������������ip
 * port �������˿�
 * dest Ŀ��վ������
 * dest_path Ŀ��Ŀ¼�ļ�,�����վ�����Ŀ¼
 * src Դվ������
 * src_path ԴĿ¼�ļ�,�����վ�����Ŀ¼
 */
PHP_FUNCTION(oct_move_file);

/*
 * �����ļ�
 * ���ø�ʽ oct_copy_file(string server, int port, string dest, string dest_path, string src, string src_path)
 * server ������������ip
 * port �������˿�
 * dest Ŀ��վ������
 * dest_path Ŀ��Ŀ¼�ļ�,�����վ�����Ŀ¼
 * src Դվ������
 * src_path ԴĿ¼�ļ�,�����վ�����Ŀ¼
 */
PHP_FUNCTION(oct_copy_file);

/*
 * ��ȡ���һ�η����Ĵ���
 * ���ø�ʽ oct_get_error()
 */
PHP_FUNCTION(oct_get_error);


/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     

ZEND_BEGIN_MODULE_GLOBALS(octopus)
	long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(octopus)
*/

/* In every utility function you add that needs to use variables 
   in php_octopus_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as OCTOPUS_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define OCTOPUS_G(v) TSRMG(octopus_globals_id, zend_octopus_globals *, v)
#else
#define OCTOPUS_G(v) (octopus_globals.v)
#endif

#endif	/* PHP_OCTOPUS_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
