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
$optype = $_POST['optype'];
$sitecfgcnt = $_POST['sitecfgcnt'];
$parentinfo = $_POST['parent'];
$sitename =  $_POST['sitename'];
$newsitename =  $_POST['newsitename'];

$result = "δ֪";

//������Ϣ����ı�����
function doSiteCfg()
{
	global $serverip;
	global  $serverport;
	global $optype;
	global  $sitecfgcnt;
	global  $result;
	global  $parentinfo;
	global  $sitename;
	global  $newsitename;
	
	$type = 0x22;
	// tlv-type = 0 �����ͣ� tlv-value = ���� ��0�鿴��1�޸ģ�2��¡��
	$array_send = array();
	$array_send[] = $optype;
	if ($optype == 0)
	{		
	}
	else if ($optype == 1)
	{
		$array_send[] = $sitecfgcnt;
	}
	else if ($optype == 2)
	{
		//��parent������IP�Ͷ˿�
		$tmparray = explode("_", $parentinfo);
		$serverip = $tmparray[0];
		$serverport = $tmparray[1];
		$array_send[] = $sitename;
		$array_send[] = $newsitename;
	}	
	if ($serverip == "" || $serverport == "" )
	{
		ERR_LOG("IP($serverip) or Port($serverport) invalid.");
		return "";
	}
	//���ͺͽ���
	$msg = new Msg($serverip, $serverport, $type, $array_send);
	//type = 0 ������� value = 0���ɹ���1ʧ��
	//type = 1 �������������value = ����ʧ��ԭ�򣨼�¼��־������ʾ��ʹ�ã�������λ���⣩
	//type = 2 ��sitecfg�����ݣ�
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
	if ($optype == 0)
	{
		$result = "��ȡ������Ϣ�ɹ���";
		return $array_recv[2]; //���ػ�ü�¼
	}
	else if ($optype == 1)
	{		
		$result = "����������Ϣ�ɹ���";
		return $sitecfgcnt;  //�����ύ�ļ�¼
	}
	else if ($optype == 2)
	{		
		$result = "����" . $sitename . "���Ƴ���վ��" .$newsitename . "���, �������������Ƿ�ȫ��ͬ���ɹ�";
		return "ok";  //�����ύ�ļ�¼
	}
}
$sitecfgcnt = doSiteCfg();
if ($sitecfgcnt == "")
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
else if ($sitecfgcnt == "ok")
{
	$sitecfgcnt = "";
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
	<div class="topt">�޸�վ��ַ���Ϣ</div>	
	<div class="cmt"> 
		<dd class="lf">			
			<input id=optype name=optype type="hidden"></input>	
			<div class="aline">��������IP��ַ��
			<input id=serverip name=serverip value="<?php echo $serverip;?>" class=inputLC type=text></input>
			</div>
			<div class="aline">�������Ķ˿ڣ�
			<input id=serverport name=serverport value="<?php echo $serverport;?>" class=inputLC type=text></input>
			</div>
			<div align="center">
				<input type="button" class="btn_normal" value="�鿴����" onclick="cfgmodify(2);">	
				<input type="button" class="btn_normal" value="�ַ�����" onclick="cfgmodify(0);">	<p></p>
			</div> 			
		</dd>
	</div>	
</div>
<div class="rightf">
    <span class="normalTitle"><?php echo $result;?></span>
	<textarea class="textareaC" id = "sitecfgcnt" name="sitecfgcnt"><?php echo $sitecfgcnt;?></textarea>
	<div align="center">
    	<input type="button" class="btn_normal" value="�ύ�޸�" onclick="cfgmodify(22);">
    	<input type="button" class="btn_normal" value="��������" onclick="cfgmodify(2);">	
    	<br><br><br>
	</div> 	
</div>
</form>
</body>
</html>