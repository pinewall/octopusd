<?php
/* 引用一下原来的异常类的处理
class Exception
{
   protected $message = 'Unknown exception';  // 异常信息
   protected $code = 0;                        // 用户自定义异常代码
   protected $file;                            // 发生异常的文件名
   protected $line;                            // 发生异常的代码行号

   function __construct($message = null, $code = 0);

   final function getMessage();                // 返回异常信息
   final function getCode();                  // 返回异常代码
   final function getFile();                  // 返回发生异常的文件名
   final function getLine();                  // 返回发生异常的代码行号
   final function getTrace();                  // backtrace() 数组
   final function getTraceAsString();          // 已格成化成字符串的 getTrace() 信息

   // 可重载的方法
   function __toString();                      // 可输出的字符串
   
   //如果使用自定义的类来扩展内置异常处理类，并且要重新定义构造函数的话，建议同时调用 parent::__construct() 来检查所有的变量是否已被赋值。当对象要输出字符串的时候，可以重载 __toString() 并自定义输出的样式。 
}
*/

class My_Exception extends Exception 
{
	//为了便于定位，添加上函数的行数和文件名称
	function __construct($message, $code, $file, $line)
	{
		$this->message = $message;
		$this->code = $code;
		$tihs->file = $file;
		$this->line = $line;
		parent::__construct();
	}
	//输出打印格式信息
	function __toString()
	{
		if(_VERSIONTYPE_ == 'QQCOM')
		{
			return "很抱歉，系统繁忙，请稍后再试。";
		}
		else 
		{
			return __CLASS__ . " , code is {$this->code},desc is {$this->message}, position is  [{$this->file}, {$this->line}]\n";
		}
	}
}
?>