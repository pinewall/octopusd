<?php
/****��־****/
require_once ("define.php");
require_once ("exception.php");

//��־��·������ʱ�����·������֤���еĴ��붼��һ��Ŀ¼�Һ�logĿ¼����
define('LOG_SIZE', 5000000);
define('LOG_CUR_PATH', '/home/kevinliu/mysite/oct/log/');
define('LOG_OLD_PATH', '/home/kevinliu/mysite/oct/log/backup/');

function writelog($info, $type)
{
	// ���쵱ǰ��־���·���Ƿ����
    if (!@dir(LOG_CUR_PATH))
    {
    	if (!@mkdir(LOG_CUR_PATH))
        {
            // Ŀ¼����ʧ��
            return false;
        }
    }

    //�������������ļ�·������
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

    //д�ļ�
    if (@file_exists($file_name))
    {
        $size = filesize($file_name);
        // ׼��������ŵ�ַ
        if ($size > LOG_SIZE)
        {
            // �����ϵ���־���·���Ƿ����
            if (!@dir(LOG_OLD_PATH))
            {
                if (!@mkdir(LOG_OLD_PATH))
                {
                    // Ŀ¼����ʧ��
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

    //��ʼд�ļ�
    $fp = fopen($file_name, 'a+');
    if(null == $fp)
    {
    	return false;
    }
    fwrite($fp, '[ ' . date('Y-m-d H:m:s') . ' ] ' . $info . "\n");
	fclose($fp);
	return true;
}

//���ⲿ���õ���־����
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