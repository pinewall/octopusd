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

//发送消息获得文本内容
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
	// tlv-type = 0 （类型） tlv-value = 类型 （0查看，1修改）
	$array_send = array();
	$array_send[] = $optype;
	$array_send[] = $statdate;

	//发送和接收
	$msg = new Msg($serverip, $serverport, $type, $array_send);
	//type = 0 （结果） value = 0，成功，1失败
	//type = 1 （结果描述）　value = 描述失败原因（记录日志或者显示等使用，用来定位问题）
	//type = 2 （内容）
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
	//成功收到响应消息，将结果记录下来
	return $array_recv[2]; //返回获得记录
}
$result = "获取信息成功！";

$statcnt = doStat();
if ($statcnt == "")
{
	if ($serverip == "")
	{
		$result = "请输入章鱼服务器的IP和端口";	
	}
	else 
	{
		$result = "服务器故障或者章鱼为老版本";
	}	
}
else if ($statcnt == "<stat>\nnone</stat>\n")
{
	$result = "当日（" . $statdate ."）统计信息不存在！";
	$statcnt = "";
}
//有数据的话，解析这个xml
$arrayrecvnum = array();
$arraysendnum = array();
if ($statcnt != "") 
{	
   	$dom = new DOMDocument();
   	$dom->preserveWhiteSpace = FALSE;  //不加载空行和空白
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
	<div class="topt">查看统计信息</div>	
	<div class="cmt"> 
		<dd class="lf">	
			<div class="aline">服务器的IP地址：
			<input id=serverip name=serverip value="<?php echo $serverip; ?>" class=inputLC type=text></input>
			</div>
			<div class="aline">服务器的端口：
			<input id=serverport name=serverport value="<?php echo $serverport; ?>" class=inputLC type=text></input>
			</div>			
			<div class="aline">统计日期（YYYYMMDD）：
			<input id=statdate name=statdate value="<?php echo $statdate; ?>" class=inputLC type=text></input>
			</div>			
			<div align="center">
				<input type="button" class="btn_normal" value="查看信息" onclick="cfgmodify(8);">
				<input type="button" class="btn_normal" value="分发网络" onclick="cfgmodify(0);">	<p></p>
			</div> 	
			<div class="aline"><span style="color:red">红色：接收的文件数目</span><br><span style="color:blue">蓝色：发送的文件数目</span><br><br></div>		
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

//红色曲线，接收到的文件
var jg = new jsGraphics("myCanvas");
jg.setColor("red"); // red
jg.setStroke(2);

//蓝色曲线，发送的文件
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