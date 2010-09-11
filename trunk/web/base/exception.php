<?php
/* ����һ��ԭ�����쳣��Ĵ���
class Exception
{
   protected $message = 'Unknown exception';  // �쳣��Ϣ
   protected $code = 0;                        // �û��Զ����쳣����
   protected $file;                            // �����쳣���ļ���
   protected $line;                            // �����쳣�Ĵ����к�

   function __construct($message = null, $code = 0);

   final function getMessage();                // �����쳣��Ϣ
   final function getCode();                  // �����쳣����
   final function getFile();                  // ���ط����쳣���ļ���
   final function getLine();                  // ���ط����쳣�Ĵ����к�
   final function getTrace();                  // backtrace() ����
   final function getTraceAsString();          // �Ѹ�ɻ����ַ����� getTrace() ��Ϣ

   // �����صķ���
   function __toString();                      // ��������ַ���
   
   //���ʹ���Զ����������չ�����쳣�����࣬����Ҫ���¶��幹�캯���Ļ�������ͬʱ���� parent::__construct() ��������еı����Ƿ��ѱ���ֵ��������Ҫ����ַ�����ʱ�򣬿������� __toString() ���Զ����������ʽ�� 
}
*/

class My_Exception extends Exception 
{
	//Ϊ�˱��ڶ�λ������Ϻ������������ļ�����
	function __construct($message, $code, $file, $line)
	{
		$this->message = $message;
		$this->code = $code;
		$tihs->file = $file;
		$this->line = $line;
		parent::__construct();
	}
	//�����ӡ��ʽ��Ϣ
	function __toString()
	{
		if(_VERSIONTYPE_ == 'QQCOM')
		{
			return "�ܱ�Ǹ��ϵͳ��æ�����Ժ����ԡ�";
		}
		else 
		{
			return __CLASS__ . " , code is {$this->code},desc is {$this->message}, position is  [{$this->file}, {$this->line}]\n";
		}
	}
}
?>