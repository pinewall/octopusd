<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
<title>��������</title>
<link href="/css/common.css" rel="stylesheet" type="text/css" />
</head>
<body>
<!--����-->
<div id="ChannelNavTop">
		<p><a href="http://passport.webdev.com/cgi-bin/logout?project=oct" target="_top">ע���˳�</a></p>
	        <p><a href="http://passport.webdev.com/cgi-bin/login?project=oct" target="_top">�л���Ŀ</a></p>
                <p><a href="/stat.php" target="f_bottom">ͳ����Ϣ</a></p>            
	        <p><a href="/queueinfo.php" target="f_bottom">������Ϣ</a></p>		    
		<p><a href="/logtrace.php" target="f_bottom">��־�鿴</a></p>			
		<p><a href="/mobilecfg.php" target="f_bottom">�澯����</a></p>			
		<p><a href="/sitecfg.php" target="f_bottom">�ַ�����</a></p>		
		<p><a href="/commoncfg.php" target="f_bottom">��������</a></p>	
		<p><a href="/tree.php" target="f_bottom">��״�ַ�</a></p>	
		<span>�����̨����ϵͳ2.2 </span>  ����ӭ�㣬
		<?php
		    //===Ȩ�޲���BEGIN===
			require_once("libpasclient/PasClient.php");
			// ʵ�����ͻ���ʵ��
			$pasclient = new PasClient();
			$ret = $pasclient->verifyTicket('oct', '', 0);
			if ($ret == SUCCESS)
			{	
				echo $pasclient->getUser();
				//echo "��ǰ��¼���飺" . $pasclient->getGroupEn() . "(" . $pasclient->getGroupCn() . ")<br>";
				//echo "��ǰ����Ȩ�ޣ�" . $pasclient->getPrivilege() . "<br><br><br>";
			}
			else
			{
				//���û�е�¼��������ҳ��¼
				echo "<script language=\"javascript\">window.top.location.href=\"/index.htm\";</script>";
			}
			//===Ȩ�޲���END===					
		 ?>
		��
</div>

</body>