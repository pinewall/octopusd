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
error_reporting(E_ERROR | E_WARNING | E_PARSE);
$topserverip = $_POST['topserverip'];
$topserverport = $_POST['topserverport'];
$topsitename = $_POST['topsitename'];
?>
</head>
<body>
<form name="form_searchtree" action="" method="post" class="main"> 
		&nbsp;<span class="stfont">请输入分发服务器信息：</span>服务器IP:
			<input id=topserverip name=topserverip value="<?php echo $topserverip; ?>" class=inputAC type=text></input>			
		服务器Port:
		<input id=topserverport name=topserverport value="<?php echo $topserverport; ?>" class=inputAC type=text></input>
		分发站点:
		<input id=topsitename name=topsitename value="<?php echo $topsitename; ?>" class=inputAC type=text></input>
		<input class=btn_normal id=searchtreesubmit value="搜 索" type="button" onclick="searchtree();"></input> 
		<input class=btn_normal id=searchtreereset value="重 置" type="button" onclick="searchtree_reset();"></input> 
</form>
<div class="leftf">
	<div class="topt">修改对应的属性</div>	
	<div class="cmt"> 
		<dd class="lf">	
		<form name="form_edit" action="" method="post"> 
		  <input id=oprtype name=optype type="hidden"></input>
		  <input id=statdate name=statdate type="hidden"></input>
			<div class="aline">上层节点IP和端口：
			<input id=parent name=parent value="-" class=inputLC type=text readonly></input>
			</div>
			<div class="aline">所属于站点：
			<input id=sitename name=sitename value="<?php echo $topsitename; ?>" class=inputLC type=text></input>
			</div>
			<div class="aline">服务器的IP地址：
			<input id=serverip name=serverip value="<?php echo $topserverip; ?>" class=inputLC type=text></input>
			</div>
			<div class="aline">服务器的端口：
			<input id=serverport name=serverport value="<?php echo $topserverport; ?>" class=inputLC type=text></input>
			</div>
			<div class="aline">分发树上的层次号：
			<input id=level name=level value="-" class=inputLC type=text readonly></input>
			</div>
			<div align="center">
				<input type="button" class="btn_normal" value="下属分发信息" onclick="cfgmodify(0);">
				<input type="button" class="btn_normal" value="修改基础配置" onclick="cfgmodify(1);">
				<input type="button" class="btn_normal" value="修改分发配置" onclick="cfgmodify(2);">
				<input type="button" class="btn_normal" value="修改告警配置" onclick="cfgmodify(3);">
				<input type="button" class="btn_normal" value="查看日志信息" onclick="cfgmodify(4);">
				<input type="button" class="btn_normal" value="查看队列信息" onclick="cfgmodify(5);">
				<input type="button" class="btn_normal" value="查看统计信息" onclick="cfgmodify(8);">
			</div> 				
			<div align="left">
				<div class="aline">新站点名称：<input id=newsitename name=newsitename value="" class=inputLC type=text></input></div>
				<div align="center">
					<input type="button" class="btn_normal" value="克隆站点配置" onclick="cfgmodify(222);">	
				</div>
			</div>
			</form>
		</dd>
	</div>	
</div>
<div class="rightf">  
	<table width="100%">
	<tr>
	<td>
	 <div class="cmt2"> 
		<dd class="lf">	
		<!--在这儿开始添加自定义的快捷方式-->
		<a class=b0 href="#" onclick="quicktree('10.1.43.137', '21811', '');">·发布系统(全站)</a>
		<a class=b0 href="#" onclick="quicktree('10.1.43.137', '21811', 'news');">·发布系统(新闻)</a>
		<a class=b0 href="#" onclick="quicktree('10.1.43.137', '21811', 'ent');">·发布系统(娱乐)</a>
		<!--在这儿结束添加自定义的快捷方式-->
		</dd>
	</div>
	</td>
	</tr>
	
	<tr>
	<td>
	<div class="cmt2" id="showTree"></div>
	</td>
	</tr>
	</table>
</div>
</body>
</html>
<script language="javascript" type="text/javascript">
var url = "";
<?php 
    if ($topserverip == '')
    {
    	echo "var url = \"/initxml.php\";";
    }
    else 
    {
		echo "var url = \"/treexml.php?topserverip=$topserverip&topserverport=$topserverport&topsitename=$topsitename\";";
    }	
?>
var myTree=new JTree("showTree", url);
myTree.setPicPath("/images/tree/")
myTree.onclick=function()
{
	if ("&nbsp;&nbsp;no result" == getNodeAtt(myTree.selectNode,"caption"))
	{
		return;
	}
	if('site' == getNodeAtt(myTree.selectNode,"type"))
	{
		setValueById("parent", getNodeAtt(myTree.selectNode.parentNode,"caption"));
		var st = getNodeAtt(myTree.selectNode,"caption");
		var pos = st.indexOf("(");
		if (pos == -1)
		{
			setValueById("sitename", st); //站点名称
		}
		else
		{
			setValueById("sitename", st.substring(0, pos)); //站点名称
		}
		setValueById("serverip",'');
		setValueById("serverport",'');
		setValueById("level",'');
		return; //如果是站点节点,则不显示到左侧窗口中
	}
	var p;
	if (myTree.clickItem.level == 1) // -1,0,1,2... 属于第1层节点时，由于0是站点信息,因此略去显示,显示更上一级
	{
		p = getNodeAtt(myTree.selectNode.parentNode.parentNode,"caption");
	}	
	else
	{
		p = getNodeAtt(myTree.selectNode.parentNode,"caption");
	}
	if (myTree.clickItem.level == -1)
	{
		setValueById("parent", "无");
	}
	else
	{
		setValueById("parent", p);
	}
	
	//归属站点,向上递归直到碰到第一个site站点
	var tmpNode = myTree.selectNode;
	setValueById("sitename", ""); ///先放置成空，以防止找不到用上次的值
	while(tmpNode)
	{
		//alert(getNodeAtt(tmpNode,"type"));
		if ("site" == getNodeAtt(tmpNode,"type"))
		{
			setValueById("sitename", getNodeAtt(tmpNode,"caption"));
			break;
		}
		tmpNode = tmpNode.parentNode;
	}	
	var title = getNodeAtt(myTree.selectNode,"caption");
	//分析成2个字段 IP_Port
	if (title == 'none')
	{
		setValueById("serverip", "无");
		setValueById("serverport","无");
	}
	else
	{
		var args = title.split("_");
		setValueById("serverip", args[0]);
		var pos = args[1].indexOf("(");
		var port ;
		if (pos == -1)
		{
			port = args[1];
		}
		else
		{
			port = args[1].substring(0, pos);
		}
		setValueById("serverport", port);
	}	
	if (myTree.clickItem.level == -1)
	{
		setValueById("level",'0');
	}
	else
	{
		setValueById("level",myTree.clickItem.level);
	}	
}
myTree.CAPTIONATT="caption";
myTree.create();
</script>