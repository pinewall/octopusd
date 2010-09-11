<?php
/************************************************************
������Pasclient API
����: Colinguo
�������ڣ�2006-11-25
************************************************************/

require_once('Msg.php');
require_once('PasCookie.php');

// ����ֵ
define ("SUCCESS", 0);				//��֤�ɹ�
define ("TICKETNOTEXIST", 1);		//���ݿ��в�����Ticket
define ("TICKETEXPIRED", 2);		//Ticket����
define ("WITHOUTPRIVILEGE", 3);	//�û�û��Ȩ�޷�����Ŀ����
define ("OTHERERROR", 4);			//��������
define ("USERNOTEXIST", 5);		//�û�������
define ("PROJECTNOTEXIST", 6);		//ָ������Ŀ������
define ("GROUPNOTEXIST", 7);		//ָ�����鲻����

// ��Ϣ����ID
define ("VERIFYTICKET", 1);			//��֤Ticket����Ȩ��
define ("QUERYUSERINFO", 2);		//��ѯ�û���Ϣ
define ("QUERYPRIVILEGE", 3);		//��ѯȨ��
define ("QUERYPRIVILEGELIST", 4);	//��ѯȨ������
define ("QUERYALLPRIVILEGE", 5);	//��ѯӵ��Ȩ�޵��û�
define ("VERIFYANDQUERYPRIVILEGE", 6);	//��֤Ticket���һ�ȡ���鵽����Ȩ���б�
define ("QUERYUSERDETAILBYPRIVILEGE", 7);	//��ѯӵ��Ȩ�޵��û�
define ("GETUSERNAMEBYTICKET", 8);			//��ѯ�û��Ƿ��¼

class PasClient
{
	var $ticket;
	var $project;
	var $group;
	var $loginedUser;
	var $loginedGroupEn;
	var $loginedGroupCn;
	var $privilege;
	var $array_privilege = array();
	var $array_userInfo = array();
	var $array_userList = array();
	
	function checkTicket()
	{
		$pasCookie = new PasCookie();
		$this->ticket = $pasCookie->getTicket();
		return $this->ticket ? true : false;
	}
	
	function checkGroup()
	{
		$pasCookie = new PasCookie();
		$pasCookie->getLastGroupByProject($this->project, 
			$this->loginedGroupEn, 
			$this->loginedGroupCn);
		if ($this->group == '')
		{
			$this->group = $this->loginedGroupEn;
		}
	}
	
	// ����û��Ƿ��¼
	function checkUser()
	{
		if (!$this->checkTicket())
		{
			return TICKETNOTEXIST;
		}
		
		$array_send = array(GETUSERNAMEBYTICKET,
			$this->ticket);
		$msg = new Msg($array_send);
		$array_recv = $msg->getRecvMsg();
		$ret = $array_recv[1];
		$this->loginedUser = $array_recv[2];
		switch ($ret)
		{
			case '0':
				return SUCCESS;
			case '1':
				return TICKETEXPIRED;
		}
	}
	
	// ����û��Ƿ��¼�����ҽ�����Ȩ�޵���֤�����$groupΪ�գ����ʾ��֤��ǰ��¼����
	// ���Ի�õ�ǰ��¼���û�������ǰ��¼����Ӣ����������������ǰ��¼�û���$group�е�Ȩ������
	function verifyTicket($project, $group, $type)
	{
		if (!$this->checkTicket())
		{
			return TICKETNOTEXIST;
		}
		$this->project = $project;
		$this->group = $group;
		$this->checkGroup();
		
		$array_send = array(VERIFYTICKET,
			$this->ticket,
			$this->project,
			$this->group,
			$type);
		$msg = new Msg($array_send);
		$array_recv = $msg->getRecvMsg();
		$ret = $array_recv[1];
		switch ($ret)
		{
			case '0':
				$this->loginedUser = $array_recv[2];
				$this->privilege = $array_recv[3];
				return SUCCESS;
			case '1':
				return TICKETNOTEXIST;
			case '2':
				return TICKETEXPIRED;
			case '3':
				return WITHOUTPRIVILEGE;
		}
	}
	
	// ����û��Ƿ��¼�����ҽ�����Ȩ�޵���֤�����$groupΪ�գ����ʾ��֤��ǰ��¼����
	// ���Ի�õ�ǰ��¼���û�������ǰ��¼����Ӣ����������������ǰ��¼�û���$group��������Ȩ�������б�
	function verifyTicketEx($project, $group)
	{
		if (!$this->checkTicket())
		{
			return TICKETNOTEXIST;
		}
		$this->project = $project;
		$this->group = $group;
		$this->checkGroup();
		
		$array_send = array(VERIFYANDQUERYPRIVILEGE,
			$this->ticket,
			$this->project,
			$this->group);
		$msg = new Msg($array_send);
		$array_recv = $msg->getRecvMsg();
		$ret = $array_recv[1];
		switch ($ret)
		{
			case '0':
				$this->loginedUser = $array_recv[2];
				$total = count($array_recv) + 1;
				for ($i = 3; $i < $total; $i += 2)
				{
					$this->array_privilege[$array_recv[$i]] = $array_recv[$i + 1];
				}
				return SUCCESS;
			case '1':
				return TICKETNOTEXIST;
			case '2':
				return TICKETEXPIRED;
			case '3':
				return WITHOUTPRIVILEGE;
		}
	}
	
	// ���ָ���û�����ϸ����
	function queryUserInfo($userId)
	{
		if ($userId == '')
		{
			return NULL;
		}
		$array_send = array(QUERYUSERINFO, $userId);
		$msg = new Msg($array_send);
		$array_recv = $msg->getRecvMsg();
		$ret = $array_recv[1];
		switch ($ret)
		{
			case '0':
				$this->array_userInfo = explode("|", $array_recv[2]);
				return SUCCESS;
			case '1':
				return USERNOTEXIST;
		}
	}
	
	// ���ӵ��ĳ��Ŀ��ĳ��ĳ��Ȩ�޵��û���ϸ��Ϣ�б�
	function queryUsersByPrivilege($project, $group, $privilege)
	{
		$array_send = array(QUERYUSERDETAILBYPRIVILEGE, $project, $group, $privilege);
		$msg = new Msg($array_send);
		$array_recv = $msg->getRecvMsg();
		$ret = $array_recv[1];
		switch ($ret)
		{
			case '0':
				$total = count($array_recv) + 1;
				for ($i = 2; $i < $total; $i++)
				{
					$array_userInfo = explode("|", $array_recv[$i]);
					$this->array_userList[$array_userInfo[0]] = $array_userInfo;
				}
				return SUCCESS;
			case '1':
				return PROJECTNOTEXIST;
			case '2':
				return GROUPNOTEXIST;
		}
	}
	
	function getUser()
	{
		return $this->loginedUser;
	}
	function getGroupEn()
	{
		return $this->loginedGroupEn;
	}
	function getGroupCn()
	{
		return $this->loginedGroupCn;
	}
	function getPrivilege()
	{
		return $this->privilege;
	}
	function getPrivilegeList()
	{
		return $this->array_privilege;
	}
	function getUserInfo()
	{
		return $this->array_userInfo;
	}
	function getUserList()
	{
		return $this->array_userList;
	}
}

?>
