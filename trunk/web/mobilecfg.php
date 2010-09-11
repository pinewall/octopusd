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
$subcheck = $_POST['subcheck'];
$mobilecnt = $_POST["mobilecnt"];

//������Ϣ����ı�����
function doMobileCfg()
{
	global $serverip;
	global  $serverport;
	global $optype;
	global $mobilecnt;
	global $subcheck;
	
	if ($serverip == "" || $serverport == "" )
	{
		ERR_LOG("IP($serverip) or Port($serverport) invalid.");
		return "";
	}
	
	$type = 0x23;
	// tlv-type = 0 �����ͣ� tlv-value = ���� ��0 log��1 error 2 debug��
	$array_send = array();
	$array_send[] = $optype;
	if ($optype == 1)
	{
		$array_send[] = $mobilecnt;
		$array_send[] = $subcheck;
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
	if ($optype == 1)
	{
		return $mobilecnt;  //�����ύ�ļ�¼
	}
	else 
	{
		return $array_recv[2]; //���ػ�ü�¼
	}
}
$result = "���������Ϣ�ɹ�";
if($optype == 1)
{
	$result = "����������Ϣ�ɹ�";
}
$mobilecnt = doMobileCfg();
if ($mobilecnt == "")
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
	<div class="topt">�޸ĸ澯������Ϣ</div>	
	<div class="cmt"> 
		<dd class="lf">			    
			<input id=optype name=optype type="hidden"></input>	
			<input id=subcheck name=subcheck type="hidden"></input>	
			<div class="aline">��������IP��ַ��
			<input id=serverip name=serverip value="<?php echo $serverip; ?>" class=inputLC type=text></input>
			</div>
			<div class="aline">�������Ķ˿ڣ�
			<input id=serverport name=serverport value="<?php echo $serverport; ?>" class=inputLC type=text></input>
			</div>		
			<div class="aline">����������ͬ���޸�:
			<input id=ifcheck name=ifcheck type="checkbox" <?php if ($subcheck == 1 || $subcheck == "") echo "checked"; ?>></input>
			</div>
			<br>	
			<div align="center">
				<input type="button" class="btn_normal" value="�鿴����" onclick="cfgmodify(3);">	
				<input type="button" class="btn_normal" value="�ַ�����" onclick="cfgmodify(0);">	<p></p>
			</div> 			
		</dd>
	</div>	
</div>
<div class="rightf">
    <span class="normalTitle"><?php echo $result;?></span>
	<textarea class="textareaC" id = "mobilecnt" name="mobilecnt"><?php echo $mobilecnt;?></textarea>
	<div align="center">
    	<input type="button" class="btn_normal" value="�ύ�޸�" onclick="cfgmodify(33);">
    	<input type="button" class="btn_normal" value="��������" onclick="cfgmodify(3);">	
    	<br><br><br>
	</div> 	
</div>
</form>
</body>
</html>