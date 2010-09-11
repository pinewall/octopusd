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

//每台服务器的IP_port:下属服务器的IP_port
global $ipport_subipport;
$ipport_subipport = array();

global $checkipport_map;
$checkipport_map = array();

//获取一个服务器子服务器列表
function getsubserver($topserverip, $topserverport, $topsitename)
{
	global  $ipport_subipport;
	$type = 0x20;
	// tlv-type = 0 （站点） tlv-value = 站点名 （为空表示所有站点）
	$array_send = array();
	$array_send[] = $topsitename;
	
	//发送和接收
	$msg = new Msg($topserverip, $topserverport, $type, $array_send);
	//type = 0 （结果） value = 0，成功，1失败
	//type = 1 （结果描述）　value = 描述失败原因（记录日志或者显示等使用，用来定位问题）
	//type = 2 （下属信息，一个字符串来传输  new:ip_port:ip_port;ent:ip_port:ip_port)
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
	//成功收到响应消息，将结果记录下来
	$ipport_subipport[$topserverip."_".$topserverport] =  $array_recv[2];
	return $array_recv[2];
}

//获取整个树
function gettree($topserverip, $topserverport, $topsitename)
{
	global $checkipport_map;
	//重要：得需要判断是否已经获取过，防止重复发消息
	$key = $topserverip . "_" . $topserverport . "_" . $topsitename;
	DEBUG_LOG("BEGIN to handle " . $key .", " . count($checkipport_map) . " has been finnished.");
	if ($checkipport_map[$key] == 'ok')
	{
		DEBUG_LOG($key . " has been handled, ignore it.");
		return "";
	}
	$checkipport_map[$key] = 'ok';
	//获取第一层服务器信息 new:ip_port:ip_port;ent:ip_port:ip_port;sport:none
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
			continue; //小于２的话，证明不完整，跳过
		}		
		for($i = 1; $i < count($result2); $i++) //去掉第一个站点信息
		{
			if ($result2[$i] == NONE)
			{
				break; // 如果是NONE的话，这一小节就结束了
			}
			else if ($result2[$i] == BLANK)
			{
				break;
			}
			$result3 = explode("_", $result2[$i]);
			//站点还是依据sitename, 如果为"",就是全部，如果为具体的站点，结果集合中也只会有一个站点信息，所以还是依据全局参数，以减少发送消息数量来提高性能
			gettree($result3[0], $result3[1], $topsitename); //合法的IP,递归向下
		}
	}
}

//根据结果字符串，获得其中站点和站点对应的ip列表
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

//拼装生成整个树状结构
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
	if ($topsitename != "")  // 单一站点
	{
		$site_ipportarray[$topsitename] = $all_site_ipportarray[$topsitename];
	}
	else 
	{
		$site_ipportarray = $all_site_ipportarray;
	}
	foreach($site_ipportarray as $key => $value)
	{
		//站点
		DEBUG_LOG("Create site(" . $key . ") node.");
		$element2 = $dom->createElement('level2', '');
		$element2->setAttribute("caption", $key);
		$element2->setAttribute("type", "site");		
		createsubtreexml(&$dom, &$element2, $key, $value, $key);
		$element->appendChild($element2);
	}
	$dom->appendChild($element);
	echo $dom->saveXML(); // 显示
	return;
}
//建立子树
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
			//none节点不再添加
			//$element->appendChild($element2);
		}
		else if ($value == BLANK)
		{
			//null，标注站点并不存在
			$element->setAttribute("caption", $caption . "(" . NOSITE . ")");
		}
		else 
		{
			$root2 = $ipport_subipport[$value]; //下面对应的数组
			if ($root2 == OLDVERSION)
			{
				$element2->setAttribute("caption", $value . "(" . OLDVERSION . ")");
				$element->appendChild($element2);
				continue;
			}
			$site_ipportarray = array();
			getsite8server($root2, &$site_ipportarray);
			if (count($site_ipportarray[$sitename]) == 0) //相当于不存在BLANK
			{
				$element2->setAttribute("caption", $value . "(" . NOSITE . ")");
			}
			else 
			{
				createsubtreexml(&$dom, &$element2, $value,$site_ipportarray[$sitename], $sitename); //递归
			}
			$element->appendChild($element2);
		}		
	}
}

//输出正常的文件头
header('Content-Type: text/xml');
gettree($topserverip, $topserverport, $topsitename);
createxml($topserverip, $topserverport, $topsitename); //显示xml
?>