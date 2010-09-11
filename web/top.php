<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
<title>顶部导航</title>
<link href="/css/common.css" rel="stylesheet" type="text/css" />
</head>
<body>
<!--导航-->
<div id="ChannelNavTop">
		<p><a href="http://passport.webdev.com/cgi-bin/logout?project=oct" target="_top">注销退出</a></p>
	        <p><a href="http://passport.webdev.com/cgi-bin/login?project=oct" target="_top">切换项目</a></p>
                <p><a href="/stat.php" target="f_bottom">统计信息</a></p>            
	        <p><a href="/queueinfo.php" target="f_bottom">队列信息</a></p>		    
		<p><a href="/logtrace.php" target="f_bottom">日志查看</a></p>			
		<p><a href="/mobilecfg.php" target="f_bottom">告警配置</a></p>			
		<p><a href="/sitecfg.php" target="f_bottom">分发配置</a></p>		
		<p><a href="/commoncfg.php" target="f_bottom">基础配置</a></p>	
		<p><a href="/tree.php" target="f_bottom">树状分发</a></p>	
		<span>章鱼后台管理系统2.2 </span>  （欢迎你，
		<?php
		    //===权限部分BEGIN===
			require_once("libpasclient/PasClient.php");
			// 实例化客户端实例
			$pasclient = new PasClient();
			$ret = $pasclient->verifyTicket('oct', '', 0);
			if ($ret == SUCCESS)
			{	
				echo $pasclient->getUser();
				//echo "当前登录的组：" . $pasclient->getGroupEn() . "(" . $pasclient->getGroupCn() . ")<br>";
				//echo "当前的组权限：" . $pasclient->getPrivilege() . "<br><br><br>";
			}
			else
			{
				//如果没有登录，借助首页登录
				echo "<script language=\"javascript\">window.top.location.href=\"/index.htm\";</script>";
			}
			//===权限部分END===					
		 ?>
		）
</div>

</body>