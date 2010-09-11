<?php
require_once("log.php");
error_reporting(E_ERROR | E_PARSE);
class Msg
{
	//���͵���Ϣ����
	var $msgVersion = 0x02;  //������ǰ��Э����ƫ�����ģ������޸�ΪTLV���汾���޸�Ϊ0x02
	//ProtocolSearchTree = 0x20, //��÷ַ����ṹ
    //ProtocolCommonCfg = 0x21,  //�޸Ļ���������Ϣ
    //ProtocolSiteCfg = 0x22,      //�޸�վ��������Ϣ
    //ProtocolMobileCfg = 0x23,  //�޸ĸ澯�ļ�
    //ProtocolLogTrace = 0x24    //�쿴��־���
	var $type = '';
	var $msgLen = 0;
	var $msgBody;

	//TLV����������
	var $array_send;
	var $array_recv = array();
	
	//������Ϣ
	var $fp;
	var $serverip;
	var $serverport;

	//��Ϣ�Ĺ��캯��
	function Msg($serverip, $serverport, $type, $array_send)
	{		
		DEBUG_LOG("IP=" . $serverip . ":PORT=" . $serverport . ":TYPE=" . $type);
		$this->serverip = $serverip;
		$this->serverport = $serverport;
		$this->type = $type;
		$this->array_send = $array_send;
		$this->encode();
		$this->send();
		$this->close();  //�ر�
	}
	
	//��Ϣ����
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
	
	//���ӷ�����
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

	//��Ϣ�ķ��ͺͽ���
	function send()
	{
		DEBUG_LOG("BEGIN to send message");
		if  (0 != $this->connect())
		{
			return -1;
		}
		//��Ϣ����
		$s_len = $this->msgLen + 6;
		$bianary = chr($this->msgVersion) . chr($this->type) . pack('N*', $s_len) . $this->msgBody ;
		DEBUG_LOG("TOTALLEN:" . $s_len);
		DEBUG_CODE($bianary);
		fputs($this->fp, $bianary);
		DEBUG_LOG("END to send message");
		//��Ϣ����
		DEBUG_LOG("BEGIN to receive message");
		$buf = fread($this->fp, 1);				// �汾��
		$buf = ord($buf);
		DEBUG_LOG("version=" . $buf);	
		if($buf != 0x02)
		{
			ERR_LOG("Recv an invalid version($buf) message!"); 
			return;
		}
		$buf = fread($this->fp, 1);				// ��Ϣ����
		$buf = ord($buf);
		DEBUG_LOG("type=" . $buf);		
		$buf = fread($this->fp, 4);				// ��Ϣ����
		$total_length = unpack('N*', $buf);		// �������ֽ����Ϊ�����ֽ���
		DEBUG_LOG("totallen=" . $total_length[1]);
		$msg_length = $total_length[1] - 6;		// �汾��+��Ϣ����+��Ϣ����λ6���ֽ�
		if ($msg_length <= 0)
		{
			ERR_LOG("Recv an invalid (<0) message!");
			return;
		}
		//�������������߹ܵ���ȡʱ�������ڶ�ȡ��Զ���ļ��� popen() �Լ� proc_open() �ķ���ʱ����ȡ����һ��������֮��ֹͣ������ζ����Ӧ�ý������ռ������ϲ��ɴ��
		//$buf = fread($this->fp, $msg_length);  	//��ȡʵ����Ϣ��С
		$buf = "";
		while (!feof($this->fp)) 
		{
			$buf .= fread($this->fp, 4096);
		}		
		$index = 0;
		while ($index < $msg_length)
		{
			// ��Ϣ������� Type
			$id = unpack('C', $buf[$index]);
			$index += 1;
			// ��Ϣ��ĳ��� Length
			$length = substr($buf, $index, 4);
			$index += 4;
			$item_length = unpack('N*', $length);
			// ��Ϣ������� Value
			$item_buf = substr($buf, $index, $item_length[1]);
			$index += $item_length[1];
			$this->array_recv[$id[1]] = $item_buf;
			DEBUG_LOG("item:".$item_buf);
		}
		DEBUG_LOG("END to receive message");
	}
	//��ý����Ϣ�����������ʽ����
	function getRecvMsg()
	{
		return $this->array_recv;
	}
	//�ر�����
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