<?php
//====常量定义的一个原则:单一常量用大写的define来实现；数组"常量"，用变量来实现，大小写混排======
// [1.0] 测试环境和真实环境的配置信息,TRUE表示测试环境，FALSE表示实际环境
define ("_DEBUG_", true);
//[1.1] 测试开关，TRUE 表示调试，程序会打印debug日志，FALSE表示关闭debug日志，只显示正常的日志(normal, error) 
define('_TEST_', true); 
//[1.2]输出的信息，根据版本类型不同，而输出不同的内容
// WEBDEV和DEBUG版本，表示对内发布和测试版本，错误信息输出详细的信息
// QQCOM版本，表示对外发布的版本，不能直接写错误信息，只输出一些委婉的信息
define ("_VERSIONTYPE_", 'WEBDEV'); 

//[2]依赖环境的定义常量
if (_DEBUG_ == false) 
{
	//[2.1] 数据库服务器配置
    //define ("DB_HOST", "172.16.19.25");
    define ("DB_HOST", "localhost");
    define ("DB_USER", "root");
    define ("DB_PASSWD", "");
    define ("DB_NAME", "db_octopusd");
}
else
{
    //[2.1] 数据库服务器配置
    define ("DB_HOST", "localhost");
    define ("DB_USER", "root");
    define ("DB_PASSWD", "");
    define ("DB_NAME", "db_octopusd"); 
}

// [3] 全局使用变量
define('OCT_PROC_NAME', 'octadmin');

?>