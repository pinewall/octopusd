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

$result = "未知";

//发送消息获得文本内容
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
	// tlv-type = 0 （类型） tlv-value = 类型 （0查看，1修改，2克隆）
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
		//从parent中提炼IP和端口
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
	//发送和接收
	$msg = new Msg($serverip, $serverport, $type, $array_send);
	//type = 0 （结果） value = 0，成功，1失败
	//type = 1 （结果描述）　value = 描述失败原因（记录日志或者显示等使用，用来定位问题）
	//type = 2 （sitecfg的内容）
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
	if ($optype == 0)
	{
		$result = "获取配置信息成功！";
		return $array_recv[2]; //返回获得记录
	}
	else if ($optype == 1)
	{		
		$result = "更新配置信息成功！";
		return $sitecfgcnt;  //返回提交的记录
	}
	else if ($optype == 2)
	{		
		$result = "根据" . $sitename . "复制出新站点" .$newsitename . "完成, 请检查整个网络是否全部同步成功";
		return "ok";  //返回提交的记录
	}
}
$sitecfgcnt = doSiteCfg();
if ($sitecfgcnt == "")
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
	<div class="topt">修改站点分发信息</div>	
	<div class="cmt"> 
		<dd class="lf">			
			<input id=optype name=optype type="hidden"></input>	
			<div class="aline">服务器的IP地址：
			<input id=serverip name=serverip value="<?php echo $serverip;?>" class=inputLC type=text></input>
			</div>
			<div class="aline">服务器的端口：
			<input id=serverport name=serverport value="<?php echo $serverport;?>" class=inputLC type=text></input>
			</div>
			<div align="center">
				<input type="button" class="btn_normal" value="查看配置" onclick="cfgmodify(2);">	
				<input type="button" class="btn_normal" value="分发网络" onclick="cfgmodify(0);">	<p></p>
			</div> 			
		</dd>
	</div>	
</div>
<div class="rightf">
    <span class="normalTitle"><?php echo $result;?></span>
	<textarea class="textareaC" id = "sitecfgcnt" name="sitecfgcnt"><?php echo $sitecfgcnt;?></textarea>
	<div align="center">
    	<input type="button" class="btn_normal" value="提交修改" onclick="cfgmodify(22);">
    	<input type="button" class="btn_normal" value="重新载入" onclick="cfgmodify(2);">	
    	<br><br><br>
	</div> 	
</div>
</form>
</body>
</html>