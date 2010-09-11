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

//发送消息获得文本内容
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
	// tlv-type = 0 （类型） tlv-value = 类型 （0查看，1修改）
	$array_send = array();
	$array_send[] = $optype;
	if($optype == 4)
	{
		$array_send[] = $filename;
	}

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
$result = "获取日志信息成功！";

$logcnt = doCommonCfg();
if ($logcnt == "")
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
else if ($logcnt == "none")
{
	$result = "日志文件不存在！";
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
	<div class="topt">查看日志记录信息</div>	
	<div class="cmt"> 
		<dd class="lf">	
			<div class="aline">服务器的IP地址：
			<input id=serverip name=serverip value="<?php echo $serverip; ?>" class=inputLC type=text></input>
			</div>
			<div class="aline">服务器的端口：
			<input id=serverport name=serverport value="<?php echo $serverport; ?>" class=inputLC type=text></input>
			</div>
			<div class="aline">日志类型：<br>
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
				<input type="button" class="btn_normal" value="查看信息" onclick="cfgmodify(4);">
				<input type="button" class="btn_normal" value="分发网络" onclick="cfgmodify(0);">		<p></p>
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