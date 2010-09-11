<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
<title>main</title>
<link href="/css/common.css" rel="stylesheet" type="text/css" />
<link href="/css/JTree.css" rel="stylesheet" type="text/css" />
<script src="/js/common.js" type="text/javascript"></script>
<script src="/js/JTree.js" type="text/javascript"></script>
<?php 
require_once("base/msg.php");

error_reporting(E_ERROR | E_PARSE);

$serverip = $_POST['serverip'];
$serverport = $_POST['serverport'];
$filename = $_POST['filename'];
$optype = $_POST['optype'];
$logcnt = "";
function showCheck($input)
{
	global $optype;
	if ($input == $optype)
	{
		return "checked";
	}
	return "";
}

//������Ϣ����ı�����
function doCommonCfg()
{
	global $serverip;
	global  $serverport;
	global $filename;
	global $optype;
	global $logcnt;
	
	if ($serverip == "" || $serverport == "" )
	{
		ERR_LOG("IP($serverip) or Port($serverport) invalid.");
		return "";
	}
	
	$type = 0x24;
	// tlv-type = 0 �����ͣ� tlv-value = ���� ��0�鿴��1�޸ģ�
	$array_send = array();
	$array_send[] = $optype;
	if($optype == 4)
	{
		$array_send[] = $filename;
	}

	//���ͺͽ���
	$msg = new Msg($serverip, $serverport, $type, $array_send);
	//type = 0 ������� value = 0���ɹ���1ʧ��
	//type = 1 �������������value = ����ʧ��ԭ�򣨼�¼��־������ʾ��ʹ�ã�������λ���⣩
	//type = 2 �����ݣ�
	$array_recv = $msg->getRecvMsg();
	if (count($array_recv) <= 0)
	{
		ERR_LOG("Receive message failed, maybe newoctopusd is old version.");
		return "";
	}
	if ($array_recv[0] != 0)
	{
		ERR_LOG($array_recv[1]);
		return "";
	}	
	//�ɹ��յ���Ӧ��Ϣ���������¼����
	return $array_recv[2]; //���ػ�ü�¼
}
$result = "��ȡ��־��Ϣ�ɹ���";

$logcnt = doCommonCfg();
if ($logcnt == "")
{
	if ($serverip == "")
	{
		$result = "�����������������IP�Ͷ˿�";	
	}
	else 
	{
		$result = "���������ϻ�������Ϊ�ϰ汾";
	}	
}
else if ($logcnt == "none")
{
	$result = "��־�ļ������ڣ�";
	$logcnt = "";
}
?>
</head>
<body>
<form name="form_searchtree" action="" method="post">
<input id=topserverip name=topserverip type="hidden"></input>	
<input id=topserverport name=topserverport type="hidden"></input>	
<input id=topsitename name=topsitename type="hidden"></input>	
</form>
<form name="form_edit" action="" method="post">
<input id=sitename name=sitename type="hidden" value=<?php echo $_POST['sitename'];?>></input>
<div class="leftf">
	<div class="topt">�鿴��־��¼��Ϣ</div>	
	<div class="cmt"> 
		<dd class="lf">	
			<div class="aline">��������IP��ַ��
			<input id=serverip name=serverip value="<?php echo $serverip; ?>" class=inputLC type=text></input>
			</div>
			<div class="aline">�������Ķ˿ڣ�
			<input id=serverport name=serverport value="<?php echo $serverport; ?>" class=inputLC type=text></input>
			</div>
			<div class="aline">��־���ͣ�<br>
			<input type="radio" name="optype" value="0" <?php echo showCheck(0); ?>>log</input>
			<input type="radio" name="optype" value="1" <?php echo showCheck(1); ?>>err</input>
			<input type="radio" name="optype" value="2" <?php echo showCheck(2); ?>>run</input>
			<br>
			<input type="radio" name="optype" value="4" <?php echo showCheck(4); ?>>file:(Ex:/log/a.txt)</input>
			<br>
			<input id=filename name=filename value="<?php echo $filename; ?>" class=inputLC type=text></input>
			</div>		
			<br>	
			<div align="center">
				<input type="button" class="btn_normal" value="�鿴��Ϣ" onclick="cfgmodify(4);">
				<input type="button" class="btn_normal" value="�ַ�����" onclick="cfgmodify(0);">		<p></p>
			</div> 			
		</dd>
	</div>	
</div>
<div class="rightf">
    <span class="normalTitle"><?php echo $result;?></span>
	<div id=main><textarea class="textareaC" id = "logcnt" name="logcnt"><?php echo $logcnt;?></textarea>	</div>
</div>
</form>
</body>
</html>