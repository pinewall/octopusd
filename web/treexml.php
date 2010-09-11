<?php 
error_reporting(E_ERROR | E_WARNING | E_PARSE);

define("OLDVERSION" , "Disconnected or old version");
define("NOSITE" , "This site does not exist");
define("NONE", "none");
define("BLANK", "null");

require_once("base/log.php");
require_once("base/msg.php");

$topserverip = $_GET['topserverip'];
$topserverport = $_GET['topserverport'];
$topsitename = $_GET['topsitename'];

//ÿ̨��������IP_port:������������IP_port
global $ipport_subipport;
$ipport_subipport = array();

global $checkipport_map;
$checkipport_map = array();

//��ȡһ���������ӷ������б�
function getsubserver($topserverip, $topserverport, $topsitename)
{
	global  $ipport_subipport;
	$type = 0x20;
	// tlv-type = 0 ��վ�㣩 tlv-value = վ���� ��Ϊ�ձ�ʾ����վ�㣩
	$array_send = array();
	$array_send[] = $topsitename;
	
	//���ͺͽ���
	$msg = new Msg($topserverip, $topserverport, $type, $array_send);
	//type = 0 ������� value = 0���ɹ���1ʧ��
	//type = 1 �������������value = ����ʧ��ԭ�򣨼�¼��־������ʾ��ʹ�ã�������λ���⣩
	//type = 2 ��������Ϣ��һ���ַ���������  new:ip_port:ip_port;ent:ip_port:ip_port)
	$array_recv = $msg->getRecvMsg();
	if (count($array_recv) <= 0)
	{
		ERR_LOG("Receive message failed, maybe newoctopusd is old version.");
		$ipport_subipport[$topserverip."_".$topserverport] = OLDVERSION;
		return "";
	}
	if ($array_recv[0] != 0)
	{
		ERR_LOG($array_recv[1]);
		$ipport_subipport[$topserverip."_".$topserverport] = OLDVERSION;
		return "";
	}	
	//�ɹ��յ���Ӧ��Ϣ���������¼����
	$ipport_subipport[$topserverip."_".$topserverport] =  $array_recv[2];
	return $array_recv[2];
}

//��ȡ������
function gettree($topserverip, $topserverport, $topsitename)
{
	global $checkipport_map;
	//��Ҫ������Ҫ�ж��Ƿ��Ѿ���ȡ������ֹ�ظ�����Ϣ
	$key = $topserverip . "_" . $topserverport . "_" . $topsitename;
	DEBUG_LOG("BEGIN to handle " . $key .", " . count($checkipport_map) . " has been finnished.");
	if ($checkipport_map[$key] == 'ok')
	{
		DEBUG_LOG($key . " has been handled, ignore it.");
		return "";
	}
	$checkipport_map[$key] = 'ok';
	//��ȡ��һ���������Ϣ new:ip_port:ip_port;ent:ip_port:ip_port;sport:none
	$ret = getsubserver($topserverip, $topserverport, $topsitename);
	if ("" == $ret)
	{
		return "";
	}
	$result1 = explode(";", $ret);
	foreach ($result1 as $value) 
	{
		$result2 = explode(":", $value);
		if (count($result2) < 2)
		{
			continue; //С�ڣ��Ļ���֤��������������
		}		
		for($i = 1; $i < count($result2); $i++) //ȥ����һ��վ����Ϣ
		{
			if ($result2[$i] == NONE)
			{
				break; // �����NONE�Ļ�����һС�ھͽ�����
			}
			else if ($result2[$i] == BLANK)
			{
				break;
			}
			$result3 = explode("_", $result2[$i]);
			//վ�㻹������sitename, ���Ϊ"",����ȫ�������Ϊ�����վ�㣬���������Ҳֻ����һ��վ����Ϣ�����Ի�������ȫ�ֲ������Լ��ٷ�����Ϣ�������������
			gettree($result3[0], $result3[1], $topsitename); //�Ϸ���IP,�ݹ�����
		}
	}
}

//���ݽ���ַ������������վ���վ���Ӧ��ip�б�
function getsite8server($string, $site_ipportarray)
{
	$result = explode(";", $string);	
	for($i = 0; $i < count($result); $i++)
	{
		$tmp = array();
		$result2 = explode(":", $result[$i]);
		for($j = 1; $j < count($result2); $j++)
		{
			$tmp[] = $result2[$j];
		}
		$site_ipportarray[$result2[0]] = $tmp; // site=>iparray
	}	
	return;
}

//ƴװ����������״�ṹ
function createxml($topserverip, $topserverport, $topsitename)
{
	global  $ipport_subipport;
	$dom = new DOMDocument('1.0', 'gb2312');
	DEBUG_LOG("Create root(" . $topserverip."_".$topserverport . ") node.");
	$element = $dom->createElement('level', '');
	$element->setAttribute("caption", $topserverip."_".$topserverport);
	$element->setAttribute("type", "server");
	$element->setAttribute("icon", "/images/tree/base.gif");	
	$root = $ipport_subipport[$topserverip."_".$topserverport];
	if ($root == OLDVERSION)
	{
		$element->setAttribute("caption", $topserverip."_".$topserverport . "(" . OLDVERSION . ")");
		$dom->appendChild($element);
		echo $dom->saveXML();
		return;
	}
	$all_site_ipportarray = array();	
	$site_ipportarray = array();
	getsite8server($root, &$all_site_ipportarray);
	if ($topsitename != "")  // ��һվ��
	{
		$site_ipportarray[$topsitename] = $all_site_ipportarray[$topsitename];
	}
	else 
	{
		$site_ipportarray = $all_site_ipportarray;
	}
	foreach($site_ipportarray as $key => $value)
	{
		//վ��
		DEBUG_LOG("Create site(" . $key . ") node.");
		$element2 = $dom->createElement('level2', '');
		$element2->setAttribute("caption", $key);
		$element2->setAttribute("type", "site");		
		createsubtreexml(&$dom, &$element2, $key, $value, $key);
		$element->appendChild($element2);
	}
	$dom->appendChild($element);
	echo $dom->saveXML(); // ��ʾ
	return;
}
//��������
function createsubtreexml($dom, $element, $caption, $ipportarray, $sitename)
{
	global  $ipport_subipport;
	for ($i = 0; $i < count($ipportarray); $i++) 
	{
		$value = $ipportarray[$i];
		DEBUG_LOG("Create server(" . $value . ") node.");
		$element2 = $dom->createElement('level3', '');		
		$element2->setAttribute("caption", $value);
		$element2->setAttribute("type", "server");
		if ($value == NONE)
		{
			//none�ڵ㲻�����
			//$element->appendChild($element2);
		}
		else if ($value == BLANK)
		{
			//null����עվ�㲢������
			$element->setAttribute("caption", $caption . "(" . NOSITE . ")");
		}
		else 
		{
			$root2 = $ipport_subipport[$value]; //�����Ӧ������
			if ($root2 == OLDVERSION)
			{
				$element2->setAttribute("caption", $value . "(" . OLDVERSION . ")");
				$element->appendChild($element2);
				continue;
			}
			$site_ipportarray = array();
			getsite8server($root2, &$site_ipportarray);
			if (count($site_ipportarray[$sitename]) == 0) //�൱�ڲ�����BLANK
			{
				$element2->setAttribute("caption", $value . "(" . NOSITE . ")");
			}
			else 
			{
				createsubtreexml(&$dom, &$element2, $value,$site_ipportarray[$sitename], $sitename); //�ݹ�
			}
			$element->appendChild($element2);
		}		
	}
}

//����������ļ�ͷ
header('Content-Type: text/xml');
gettree($topserverip, $topserverport, $topsitename);
createxml($topserverip, $topserverport, $topsitename); //��ʾxml
?>