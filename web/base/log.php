<?php
/****日志****/
require_once ("define.php");
require_once ("exception.php");

//日志的路径，暂时用相对路径，保证所有的代码都在一级目录且和log目录并列
define('LOG_SIZE', 5000000);
define('LOG_CUR_PATH', '/home/kevinliu/mysite/oct/log/');
define('LOG_OLD_PATH', '/home/kevinliu/mysite/oct/log/backup/');

function writelog($info, $type)
{
	// 考察当前日志存放路径是否存在
    if (!@dir(LOG_CUR_PATH))
    {
    	if (!@mkdir(LOG_CUR_PATH))
        {
            // 目录创建失败
            return false;
        }
    }

    //根据类型生成文件路径名称
    $file_name = LOG_CUR_PATH . OCT_PROC_NAME;
    if ($type == 'error')
    {
        $file_name = $file_name . ".error";
    }
    else if ($type == 'log')
    {
         $file_name = $file_name . ".log";
    }
    else if ($type == 'debug')
    {
         $file_name = $file_name . ".debug";
    }

    //写文件
    if (@file_exists($file_name))
    {
        $size = filesize($file_name);
        // 准备更换存放地址
        if ($size > LOG_SIZE)
        {
            // 考察老的日志存放路径是否存在
            if (!@dir(LOG_OLD_PATH))
            {
                if (!@mkdir(LOG_OLD_PATH))
                {
                    // 目录创建失败
                    return false;
                }
            }
            $temp_name = LOG_OLD_PATH + LOG_OLD_PATH;
            if ($type == 'error')
            {
                $temp_name = $temp_name . ".error" . date("YmdHms");
            }
            else if ($type == 'log')
            {
                $temp_name = $temp_name . ".log" . date("YmdHms");
            }
            else if ($type == 'debug')
		    {
		         $temp_name = $temp_name . ".debug" . date("YmdHms");
		    }
            copy($file_name, $temp_name);
            unlink($file_name);
        }
    }

    //开始写文件
    $fp = fopen($file_name, 'a+');
    if(null == $fp)
    {
    	return false;
    }
    fwrite($fp, '[ ' . date('Y-m-d H:m:s') . ' ] ' . $info . "\n");
	fclose($fp);
	return true;
}

//供外部调用的日志函数
function DEBUG_LOG($info)
{
	if (_TEST_ == true) 
	{
		writelog($info, 'debug');
	}
}
function DEBUG_CODE($info)
{
	if (_TEST_ == true) 
	{
		$tmp = bin2hex($info);
		$prt = "";
		for($i = 0; $i < strlen($tmp);)
		{
			$prt = $prt . substr($tmp, $i, 2) . " ";
			$i = $i + 2;
		}
		writelog($prt, 'debug');
	}
}
function NORMAL_LOG($info)
{
	writelog($info, 'log');
	DEBUG_LOG($info);
}
function ERR_LOG($info)
{
	writelog($info, 'error');
	NORMAL_LOG($info);
}
?>