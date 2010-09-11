<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
<title>main</title>
<link href="/css/common.css" rel="stylesheet" type="text/css" />
<link href="/css/JTree.css" rel="stylesheet" type="text/css" />
<script src="/js/wz_jsgraphics.js" type="text/javascript"></script>
<script src="/js/common.js" type="text/javascript"></script>
<script src="/js/JTree.js" type="text/javascript"></script>
<?php 
require_once("base/msg.php");

error_reporting(E_ERROR | E_PARSE);

$serverip = $_POST['serverip'];
$serverport = $_POST['serverport'];
$statdate = $_POST['statdate'];
$optype = 3; //$_POST['optype'];
$statcnt = "";

//������Ϣ����ı�����
function doStat()
{
	global $serverip;
	global  $serverport;
	global $statdate;
	global $optype;
	global $statcnt;
	
	if ($serverip == "" || $serverport == "" )
	{
		ERR_LOG("IP($serverip) or Port($serverport) invalid.");
		return "";
	}
	
	$type = 0x24;
	// tlv-type = 0 �����ͣ� tlv-value = ���� ��0�鿴��1�޸ģ�
	$array_send = array();
	$array_send[] = $optype;
	$array_send[] = $statdate;

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
$result = "��ȡ��Ϣ�ɹ���";

$statcnt = doStat();
if ($statcnt == "")
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
else if ($statcnt == "<stat>\nnone</stat>\n")
{
	$result = "���գ�" . $statdate ."��ͳ����Ϣ�����ڣ�";
	$statcnt = "";
}
//�����ݵĻ����������xml
$arrayrecvnum = array();
$arraysendnum = array();
if ($statcnt != "") 
{	
   	$dom = new DOMDocument();
   	$dom->preserveWhiteSpace = FALSE;  //�����ؿ��кͿհ�
	$dom->loadXML($statcnt);
	//echo $dom->saveXML();	
	$rootNode = $dom->documentElement;
	foreach ($rootNode->childNodes as $node) 
	{
		if ($node->nodeType != XML_ELEMENT_NODE) //Element
		{
			continue;
		}
		foreach ($node->childNodes as $subnode) //
		{
			if($subnode->localName == "recvnum")
			{
				$arrayrecvnum[] = $subnode->textContent;
			}
			else if ($subnode->localName == "sendnum")
			{
				$arraysendnum[] =  $subnode->textContent;;
			}
		}
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
	<div class="topt">�鿴ͳ����Ϣ</div>	
	<div class="cmt"> 
		<dd class="lf">	
			<div class="aline">��������IP��ַ��
			<input id=serverip name=serverip value="<?php echo $serverip; ?>" class=inputLC type=text></input>
			</div>
			<div class="aline">�������Ķ˿ڣ�
			<input id=serverport name=serverport value="<?php echo $serverport; ?>" class=inputLC type=text></input>
			</div>			
			<div class="aline">ͳ�����ڣ�YYYYMMDD����
			<input id=statdate name=statdate value="<?php echo $statdate; ?>" class=inputLC type=text></input>
			</div>			
			<div align="center">
				<input type="button" class="btn_normal" value="�鿴��Ϣ" onclick="cfgmodify(8);">
				<input type="button" class="btn_normal" value="�ַ�����" onclick="cfgmodify(0);">	<p></p>
			</div> 	
			<div class="aline"><span style="color:red">��ɫ�����յ��ļ���Ŀ</span><br><span style="color:blue">��ɫ�����͵��ļ���Ŀ</span><br><br></div>		
		</dd>
	</div>	
</div>
<div class="rightf">
    <span class="normalTitle"><?php echo $result;?></span>
	<div id=main>	
	<table>
		<tr>
			<td width="600px" height="400px" background="/images/bstat.jpg">			
			<div id="myCanvas" style="position:relative;height:366px;width:557px;margin-left:43px;margin-bottom:34px"></div>
			</td>
		</tr>
	</table>
	</div>
</div>
</form>
</body>


<script type="text/javascript">
<!--

//��ɫ���ߣ����յ����ļ�
var jg = new jsGraphics("myCanvas");
jg.setColor("red"); // red
jg.setStroke(2);

//��ɫ���ߣ����͵��ļ�
var jg2 = new jsGraphics("myCanvas");
jg2.setColor("blue"); // blue
jg2.setStroke(1);

//535 * 365
// 24*60=1440  9500
// i*535/1440=x    365- num*365/9500=y
<?php
$x1 = 0; 
$y1 = 365;
for($i = 0; $i < count($arrayrecvnum); $i++)
{
	$x2 = $i*535/1440;
	$y2 = 365 - ($arrayrecvnum[$i]*365/9500);
	if ($y2 < 0)
	{
		$y2 = 0;
	}
	echo "jg.drawLine(" . $x1 . ", " . $y1. ", " . $x2. ", " . $y2. "); \n";	
	$x1 = $x2;
	$y1 = $y2;	
}

$x1 = 0; 
$y1 = 365;
for($i = 0; $i < count($arraysendnum); $i++)
{
	$x2 = $i*535/1440;
	$y2 = 365 - ($arraysendnum[$i]*365/9500);
	if ($y2 < 0)
	{
		$y2 = 0;
	}
	echo "jg2.drawLine(" . $x1 . ", " . $y1. ", " . $x2. ", " . $y2. "); \n";	
	$x1 = $x2;
	$y1 = $y2;	
}
?>
jg.paint();
jg2.paint();

//-->
</script> 

</html>