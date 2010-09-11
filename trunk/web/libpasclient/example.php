<?php
/************************************************************
描述：Pasclient API Example
作者: Colinguo
创建日期：2006-11-25
************************************************************/
require_once('PasClient.php');

// 实例化客户端实例
$pasclient = new PasClient();

// [1] 获得用户是否登录，如果登录则得到用户名
echo "===== PasClient::checkUser =====<br>";
$loginedUser = '';
$ret = $pasclient->checkUser();
if ($ret == SUCCESS)
{
	$loginedUser = $pasclient->getUser();
}
echo "当前登录用户为：" . $loginedUser . "<br><br><br>";


echo "===== PasClient::verifyTicket =====<br>";
$ret = $pasclient->verifyTicket('vote', '', 1);

echo "调用verifyTicket返回值：" . $ret . "<br>";
echo "当前登录的用户：" . $pasclient->getUser() . "<br>";
echo "当前登录的组：" . $pasclient->getGroupEn() . "(" . $pasclient->getGroupCn() . ")<br>";
echo "当前的组权限：" . $pasclient->getPrivilege() . "<br><br><br>";


echo "===== PasClient::verifyTicketEx =====<br>";
$ret = $pasclient->verifyTicketEx('vote', 'news');
echo "调用verifyTicket返回值：" . $ret . "<br>";
echo "当前登录的用户：" . $pasclient->getUser() . "<br>";
echo "当前登录的组：" . $pasclient->getGroupEn() . "(" . $pasclient->getGroupCn() . ")<br>";
echo "当前的组权限：";
print_r($pasclient->getPrivilegeList());
echo "<br><br><br>";

echo "===== PasClient::queryUserInfo =====<br>";
$ret = $pasclient->queryUserInfo('colinguo');
if ($ret == SUCCESS)
{
	echo "用户信息：";
	print_r($pasclient->getUserInfo());
	echo "<br><br><br>";
}


echo "===== PasClient::queryUsersByPrivilege =====<br>";
$ret = $pasclient->queryUsersByPrivilege('vote', 'news', 'view');
if ($ret == SUCCESS)
{
	echo "用户列表：";
	print_r($pasclient->getUserList());
	echo "<br>";
}
?>

<a href="http://passport.webdev.com/cgi-bin/login?project=vote">切换项目</a>
<a href="http://passport.webdev.com/cgi-bin/logout?project=vote">退出</a>