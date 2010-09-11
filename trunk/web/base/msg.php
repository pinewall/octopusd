<?php
require_once("log.php");
error_reporting(E_ERROR | E_PARSE);
class Msg
{
	//发送的消息内容
	var $msgVersion = 0x02;  //章鱼以前的协议是偏移量的，现在修改为TLV，版本号修改为0x02
	//ProtocolSearchTree = 0x20, //获得分发数结构
    //ProtocolCommonCfg = 0x21,  //修改基础配置信息
    //ProtocolSiteCfg = 0x22,      //修改站点配置信息
    //ProtocolMobileCfg = 0x23,  //修改告警文件
    //ProtocolLogTrace = 0x24    //察看日志情况
	var $type = '';
	var $msgLen = 0;
	var $msgBody;

	//TLV的内容数组
	var $array_send;
	var $array_recv = array();
	
	//连接信息
	var $fp;
	var $serverip;
	var $serverport;

	//消息的构造函数
	function Msg($serverip, $serverport, $type, $array_send)
	{		
		DEBUG_LOG("IP=" . $serverip . ":PORT=" . $serverport . ":TYPE=" . $type);
		$this->serverip = $serverip;
		$this->serverport = $serverport;
		$this->type = $type;
		$this->array_send = $array_send;
		$this->encode();
		$this->send();
		$this->close();  //关闭
	}
	
	//消息编码
	function encode()
	{
		DEBUG_LOG("BEGIN to encode message");
		for ($i = 0; $i < count($this->array_send); $i++)
		{
			DEBUG_LOG("ITEM:" . $this->array_send[$i]);
			$this->msgBody .= chr($i) . pack('N*', strlen($this->array_send[$i])) . $this->array_send[$i];
		}		
		$this->msgLen = strlen($this->msgBody);
		DEBUG_LOG("BODYLEN:" . $this->msgLen);
		DEBUG_LOG("END to encode message");
	}
	
	//连接服务器
	function connect()
	{
		$this->fp = fsockopen($this->serverip, $this->serverport, $errno, $errstr, 5);
		if ($this->fp == false)
		{
			ERR_LOG("Connect to ($this->serverip, $this->serverport) failed, reason is " . $errno . ":" . $errstr);
			return -1;
		}
		return 0;
	}

	//消息的发送和接收
	function send()
	{
		DEBUG_LOG("BEGIN to send message");
		if  (0 != $this->connect())
		{
			return -1;
		}
		//消息发送
		$s_len = $this->msgLen + 6;
		$bianary = chr($this->msgVersion) . chr($this->type) . pack('N*', $s_len) . $this->msgBody ;
		DEBUG_LOG("TOTALLEN:" . $s_len);
		DEBUG_CODE($bianary);
		fputs($this->fp, $bianary);
		DEBUG_LOG("END to send message");
		//消息接收
		DEBUG_LOG("BEGIN to receive message");
		$buf = fread($this->fp, 1);				// 版本号
		$buf = ord($buf);
		DEBUG_LOG("version=" . $buf);	
		if($buf != 0x02)
		{
			ERR_LOG("Recv an invalid version($buf) message!"); 
			return;
		}
		$buf = fread($this->fp, 1);				// 消息类型
		$buf = ord($buf);
		DEBUG_LOG("type=" . $buf);		
		$buf = fread($this->fp, 4);				// 消息长度
		$total_length = unpack('N*', $buf);		// 将网络字节序变为主机字节序
		DEBUG_LOG("totallen=" . $total_length[1]);
		$msg_length = $total_length[1] - 6;		// 版本号+消息类型+消息长度位6个字节
		if ($msg_length <= 0)
		{
			ERR_LOG("Recv an invalid (<0) message!");
			return;
		}
		//当从网络流或者管道读取时，例如在读取从远程文件或 popen() 以及 proc_open() 的返回时，读取会在一个包可用之后停止。这意味着你应该将数据收集起来合并成大块
		//$buf = fread($this->fp, $msg_length);  	//获取实际消息大小
		$buf = "";
		while (!feof($this->fp)) 
		{
			$buf .= fread($this->fp, 4096);
		}		
		$index = 0;
		while ($index < $msg_length)
		{
			// 消息项的类型 Type
			$id = unpack('C', $buf[$index]);
			$index += 1;
			// 消息项的长度 Length
			$length = substr($buf, $index, 4);
			$index += 4;
			$item_length = unpack('N*', $length);
			// 消息项的内容 Value
			$item_buf = substr($buf, $index, $item_length[1]);
			$index += $item_length[1];
			$this->array_recv[$id[1]] = $item_buf;
			DEBUG_LOG("item:".$item_buf);
		}
		DEBUG_LOG("END to receive message");
	}
	//获得结果消息，以数组的形式返回
	function getRecvMsg()
	{
		return $this->array_recv;
	}
	//关闭连接
	function close()
	{
		if ($this->fp == false)
		{
			return;
		}
		fclose($this->fp);
	}
}
?>