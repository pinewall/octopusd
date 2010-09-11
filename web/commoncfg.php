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
$comoncfgcnt = $_POST['comoncfgcnt'];


//发送消息获得文本内容
function doCommonCfg()
{
	global $serverip;
	global  $serverport;
	global $optype;
	global  $comoncfgcnt;
	
	if ($serverip == "" || $serverport == "" )
	{
		ERR_LOG("IP($serverip) or Port($serverport) invalid.");
		return "";
	}
	
	$type = 0x21;
	// tlv-type = 0 （类型） tlv-value = 类型 （0查看，1修改）
	$array_send = array();
	$array_send[] = $optype;
	if ($optype == 1)
	{
		$array_send[] = $comoncfgcnt;
	}	
	//发送和接收
	$msg = new Msg($serverip, $serverport, $type, $array_send);
	//type = 0 （结果） value = 0，成功，1失败
	//type = 1 （结果描述）　value = 描述失败原因（记录日志或者显示等使用，用来定位问题）
	//type = 2 （commoncfg的内容）
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
	if ($optype == 1)
	{
		return $comoncfgcnt;  //返回提交的记录
	}
	else 
	{
		return $array_recv[2]; //返回获得记录
	}
}
$result = "";
if($optype == 0)
{
	$result = "获取配置信息成功！";
}
else 
{
	$result = "更新配置信息成功！";
}
$comoncfgcnt = doCommonCfg();
if ($comoncfgcnt == "")
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
	<div class="topt">修改基础配置信息</div>	
	<div class="cmt"> 
		<dd class="lf">			
			<input id=optype name=optype type="hidden"></input>	
			<div class="aline">服务器的IP地址：
			<input id=serverip name=serverip value="<?php echo $serverip; ?>" class=inputLC type=text></input>
			</div>
			<div class="aline">服务器的端口：
			<input id=serverport name=serverport value="<?php echo $serverport; ?>" class=inputLC type=text></input>
			</div>
			<div align="center">
				<input type="button" class="btn_normal" value="查看配置" onclick="cfgmodify(1);">	
				<input type="button" class="btn_normal" value="分发网络" onclick="cfgmodify(0);">	<p></p>
			</div> 			
		</dd>
	</div>	
</div>
<div class="rightf">
    <span class="normalTitle"><?php echo $result;?></span>
	<textarea class="textareaC" id = "comoncfgcnt" name="comoncfgcnt"><?php echo $comoncfgcnt;?></textarea>
	<div align="center">
    	<input type="button" class="btn_normal" value="提交修改" onclick="cfgmodify(11);">
    	<input type="button" class="btn_normal" value="重新载入" onclick="cfgmodify(1);">	
    	<br><br><br>
	</div> 	
</div>
</form>
</body>
</html>