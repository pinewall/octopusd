<?php
/************************************************************
������Pasclient API
����: Colinguo
�������ڣ�2006-11-25
************************************************************/

require_once('MsgDefine.php');

class Msg
{
	var $msgVersion = 0x01;
	var $msgLen = 0;
	var $msgBody;
	var $array_send;
	var $array_recv = array();
	var $fp;

	function Msg($array_send)
	{
		$this->array_send = $array_send;
		$this->encode();
		$this->send();
	}
	
	function encode()
	{
		for ($i = 0; $i < count($this->array_send); $i++)
		{
			$this->msgBody .= chr($i + 1) . pack('N*', strlen($this->array_send[$i])) . $this->array_send[$i];
		}
		$this->msgLen = strlen($this->msgBody);
	}

	function connect()
	{
		$this->fp = fsockopen(SESSION_SERVER, SESSION_PORT , $errno, $errstr, 10);
		if ($this->fp == false)
		{
			return -1;
		}
		return 0;
	}

	function send()
	{
		if  (0 != $this->connect())
		{
			return -1;
		}

		$bianary = chr($this->msgVersion) . pack('N*', $this->msgLen + 5) . $this->msgBody ;
		fputs($this->fp, $bianary);

		$buf = fread($this->fp, 1);				// �汾��
		$buf = fread($this->fp, 4);				// ��Ϣ����
		$total_length = unpack('N*', $buf);		// �������ֽ����Ϊ�����ֽ���
		$msg_length = $total_length[1] - 5;		// �汾��+��Ϣ����λ5���ֽ�
		if ($msg_length == 0)
		{
			return;
		}
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
		}
	}
	
	function getRecvMsg()
	{
		return $this->array_recv;
	}

	function close()
	{
		fclose($this->fp);
	}
}
?>