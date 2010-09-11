<?php
/************************************************************
������Pasclient API Example
����: Colinguo
�������ڣ�2006-11-25
************************************************************/
require_once('PasClient.php');

// ʵ�����ͻ���ʵ��
$pasclient = new PasClient();

// [1] ����û��Ƿ��¼�������¼��õ��û���
echo "===== PasClient::checkUser =====<br>";
$loginedUser = '';
$ret = $pasclient->checkUser();
if ($ret == SUCCESS)
{
	$loginedUser = $pasclient->getUser();
}
echo "��ǰ��¼�û�Ϊ��" . $loginedUser . "<br><br><br>";


echo "===== PasClient::verifyTicket =====<br>";
$ret = $pasclient->verifyTicket('vote', '', 1);

echo "����verifyTicket����ֵ��" . $ret . "<br>";
echo "��ǰ��¼���û���" . $pasclient->getUser() . "<br>";
echo "��ǰ��¼���飺" . $pasclient->getGroupEn() . "(" . $pasclient->getGroupCn() . ")<br>";
echo "��ǰ����Ȩ�ޣ�" . $pasclient->getPrivilege() . "<br><br><br>";


echo "===== PasClient::verifyTicketEx =====<br>";
$ret = $pasclient->verifyTicketEx('vote', 'news');
echo "����verifyTicket����ֵ��" . $ret . "<br>";
echo "��ǰ��¼���û���" . $pasclient->getUser() . "<br>";
echo "��ǰ��¼���飺" . $pasclient->getGroupEn() . "(" . $pasclient->getGroupCn() . ")<br>";
echo "��ǰ����Ȩ�ޣ�";
print_r($pasclient->getPrivilegeList());
echo "<br><br><br>";

echo "===== PasClient::queryUserInfo =====<br>";
$ret = $pasclient->queryUserInfo('colinguo');
if ($ret == SUCCESS)
{
	echo "�û���Ϣ��";
	print_r($pasclient->getUserInfo());
	echo "<br><br><br>";
}


echo "===== PasClient::queryUsersByPrivilege =====<br>";
$ret = $pasclient->queryUsersByPrivilege('vote', 'news', 'view');
if ($ret == SUCCESS)
{
	echo "�û��б�";
	print_r($pasclient->getUserList());
	echo "<br>";
}
?>

<a href="http://passport.webdev.com/cgi-bin/login?project=vote">�л���Ŀ</a>
<a href="http://passport.webdev.com/cgi-bin/logout?project=vote">�˳�</a>