<?php
//====���������һ��ԭ��:��һ�����ô�д��define��ʵ�֣�����"����"���ñ�����ʵ�֣���Сд����======
// [1.0] ���Ի�������ʵ������������Ϣ,TRUE��ʾ���Ի�����FALSE��ʾʵ�ʻ���
define ("_DEBUG_", true);
//[1.1] ���Կ��أ�TRUE ��ʾ���ԣ�������ӡdebug��־��FALSE��ʾ�ر�debug��־��ֻ��ʾ��������־(normal, error) 
define('_TEST_', true); 
//[1.2]�������Ϣ�����ݰ汾���Ͳ�ͬ���������ͬ������
// WEBDEV��DEBUG�汾����ʾ���ڷ����Ͳ��԰汾��������Ϣ�����ϸ����Ϣ
// QQCOM�汾����ʾ���ⷢ���İ汾������ֱ��д������Ϣ��ֻ���һЩί�����Ϣ
define ("_VERSIONTYPE_", 'WEBDEV'); 

//[2]���������Ķ��峣��
if (_DEBUG_ == false) 
{
	//[2.1] ���ݿ����������
    //define ("DB_HOST", "172.16.19.25");
    define ("DB_HOST", "localhost");
    define ("DB_USER", "root");
    define ("DB_PASSWD", "");
    define ("DB_NAME", "db_octopusd");
}
else
{
    //[2.1] ���ݿ����������
    define ("DB_HOST", "localhost");
    define ("DB_USER", "root");
    define ("DB_PASSWD", "");
    define ("DB_NAME", "db_octopusd"); 
}

// [3] ȫ��ʹ�ñ���
define('OCT_PROC_NAME', 'octadmin');

?>