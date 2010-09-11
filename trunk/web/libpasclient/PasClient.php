<?php
/************************************************************
描述：Pasclient API
作者: Colinguo
创建日期：2006-11-25
************************************************************/

require_once('Msg.php');
require_once('PasCookie.php');

// 返回值
define ("SUCCESS", 0);				//验证成功
define ("TICKETNOTEXIST", 1);		//数据库中不存在Ticket
define ("TICKETEXPIRED", 2);		//Ticket过期
define ("WITHOUTPRIVILEGE", 3);	//用户没有权限访问项目、组
define ("OTHERERROR", 4);			//其它错误
define ("USERNOTEXIST", 5);		//用户不存在
define ("PROJECTNOTEXIST", 6);		//指定的项目不存在
define ("GROUPNOTEXIST", 7);		//指定的组不存在

// 消息任务ID
define ("VERIFYTICKET", 1);			//验证Ticket和组权限
define ("QUERYUSERINFO", 2);		//查询用户信息
define ("QUERYPRIVILEGE", 3);		//查询权限
define ("QUERYPRIVILEGELIST", 4);	//查询权限链表
define ("QUERYALLPRIVILEGE", 5);	//查询拥有权限的用户
define ("VERIFYANDQUERYPRIVILEGE", 6);	//验证Ticket并且获取从组到根的权限列表
define ("QUERYUSERDETAILBYPRIVILEGE", 7);	//查询拥有权限的用户
define ("GETUSERNAMEBYTICKET", 8);			//查询用户是否登录

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
	
	// 检查用户是否登录
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
	
	// 检查用户是否登录，并且进行组权限的验证，如果$group为空，则表示验证当前登录的组
	// 可以获得当前登录的用户名、当前登录的组英文名和中文名、当前登录用户在$group中的权限类型
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
	
	// 检查用户是否登录，并且进行组权限的验证，如果$group为空，则表示验证当前登录的组
	// 可以获得当前登录的用户名、当前登录的组英文名和中文名、当前登录用户从$group到最高组的权限类型列表
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
	
	// 获得指定用户的详细资料
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
	
	// 获得拥有某项目中某组某类权限的用户详细信息列表
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
