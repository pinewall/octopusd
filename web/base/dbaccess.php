<?php

require_once ("exception.php");
require_once ("log.php");

/******************
说明:暂时采用mysql的方式，mysqli的方式,可能还是因为不太稳定地方式,老是出现模型奇妙的错误,期待高版本解决吧；
mysql的方式没有采用抛出异常的方式,将之改造成抛出异常的形式
********************/

//[1.1]自定义的一个quote函数
function mysql_quote($value)
{
	$value = str_replace('"', "'", $value);
	return $value;
}
//[1.2]字段信息的转义，想代替mysql_escape_string() ,待测试是否安全
function mysql_escape($value)
{
	if (!get_magic_quotes_gpc()) 
	{	
		return addslashes($value);		
	} 
	else
	{
   		return $value;
	}
}
//[2.0]数据库操作类
class CDBAccess
{
	protected $m_link = null; //数据库连接
	public $m_rows = array();
	function __construct()
	{
	}
	function __destruct ()
	{
		//关闭数据库链接在析构函数进行
		$this->disconnect();
	}
	//[0.1]连接数据库 这儿耦合define.php函数，如果单独作为代码库使用，请将信息拷贝过来，之所以不遵守编成规范这样做，便于维护人员修改数据库信息，而不影响实现代码
	public function connect()
	{
		if ($this->m_link != null) // 非空
		{
			return;
		}
		//连接数据库
		$this->m_link = mysql_connect(DB_HOST, DB_USER, DB_PASSWD);
		if ($this->m_link == null)
		{
			$err = mysql_error();
			throw new My_Exception("Connect db failed, reason is $err.", mysql_errno(), __FILE__, __LINE__);
		}
		//切到对应的数据库上
		if (false == mysql_select_db(DB_NAME))
		{
			$err = mysql_error();
			throw new My_Exception("select db failed, reason is $err.", mysql_errno(), __FILE__, __LINE__);
		}
		return;
	}
	//[0.2] 关闭数据连接
	function disconnect()
	{
		if ($this->m_link != null)
		{
			mysql_close($this->m_link);
			$this->m_link = null;
		}
	}

	//[0.3]切换数据库
	public function changedb($dbname)
	{
		//切到对应的数据库上
		if (false == mysql_select_db($dbname))
		{
			$err = mysql_error();
			throw new My_Exception("select db($dbname) failed, reason is $err.", mysql_errno(), __FILE__, __LINE__);
		}
		return;
	}
	//[1.1]查询数据库
	public function q_select($fields, $tablename, $where, $order, $no_page, $every_limit)
	{	
		try
		{
			//字段列表
			$field_str = implode(',', $fields);
			//条件
			$q_where = '';
			if ($where != '')
			{
				$q_where = " where " . $where;
			}
			//排序
			$q_order = '';
			if ($order != '')
			{
				$q_order = " order by " . $order;
			}
			//条数
			$q_limit = '';
			if ($every_limit != 0)
			{
				$q_limit = " limit " . $no_page . "," . $every_limit;
			}		
			//开始拼写SQL语句了
			$sql = "select " . $field_str . " from " . $tablename . $q_where . $q_order . $q_limit;
			//开始查询
			$this->select($sql);
		}
		catch (My_Exception $e)
		{
			throw $e;
		}
	}
	//[1.2]查询数据库(直接的SQL语句)
	public function select($sql)
	{	
		try
		{
			DEBUG_LOG($sql);
			$this->connect();
			$result = mysql_query($sql);
			if($result == null) //空
			{
				$err = mysql_error();
				throw new My_Exception("Execute($sql) failed, reason is $err.", mysql_errno(), __FILE__, __LINE__);
			}
			//如果上次查询中，还有内容，请清空
			if (count($this->m_rows) != 0)
			{
				$this->m_rows = array();				
			}
			while($row = mysql_fetch_assoc($result))
			{
				$this->m_rows[] = $row;
			}	
			mysql_free_result($result);	
			return;
		}
		catch (My_Exception  $e)
		{
			throw $e;
		}	
	}
	// [2.1] 插入数据
	public function q_insert($tablename, $fields, $values)
	{
		try
		{
			//字段列表
			$field_str = implode(',', $fields);
			//字段值列表
			$value_str = "";
			for($i = 0; $i < count($values); $i++)
			{
				$value_str = $value_str . "'" . mysql_escape_string( $values[$i]) . "'" ;
				if ($i != count($values) - 1)
				{
					 $value_str = $value_str . ',';
				}
			}				
			//开始拼写SQL语句了
			$sql = "insert into " . $tablename ."(" . $field_str . ")values(" . $value_str . ")";
			return $this->insert($sql);
		}
		catch (My_Exception  $e)
		{
			throw $e;
		}
	}
	// [2.2] 插入数据（直接SQL语句）
	public function insert($sql)
	{		
		try
		{
			DEBUG_LOG($sql);
			$this->connect();	
			if (false == mysql_query($sql))
			{
				$err = mysql_error();
				throw new My_Exception("insert($sql) sql failed, reason is $err.", mysql_errno(), __FILE__, __LINE__);
			}
			return mysql_insert_id();
		}
		catch (My_Exception  $e)
		{
			throw $e;
		}
	}
	// [3.1]更新数据
	public function q_update($tablename, $fields, $values, $where)
	{		
		try
		{
			//字段值列表
			$update_str = "";
			for($i = 0; $i < count($fields); $i++)
			{				
				$update_str = $update_str . $fields[$i] . "='" . mysql_escape_string($values[$i]) . "'" ;
				if ($i != count($values) - 1)
				{
					 $update_str = $update_str . ',';
				}
			}
			//条件(kevinliu:为了防止误修改，采用必须输入条件才能修改的方法)
			$q_where = '';
			if ($where != '')
			{
				$q_where = " where " . $where;;
			}
			else  //(kevinliu:为了防止误修改，采用必须输入条件才能修改的方法，确实有必要考虑修改此处或者直接用sql语句的方式)
			{
				throw new My_Exception("where = null, break update.", -100, __FILE__, __LINE__);
			}
			//执行
			$sql = 'update ' . $tablename . " set " . $update_str . $q_where;
			return $this->update($sql);
		}
		catch (My_Exception  $e)
		{
			throw $e;
		}
	}
	// [3.2] 更新数据（直接SQL语句）
	public function update($sql)
	{
		try
		{
			DEBUG_LOG($sql);
			$this->connect();	
			if (false == mysql_query($sql))
			{
				$err = mysql_error();
				throw new My_Exception("update($sql) sql failed, reason is $err.", mysql_errno(), __FILE__, __LINE__);
			}
			return mysql_affected_rows();
		}
		catch (My_Exception  $e)
		{
			throw $e;
		}
	}
	// [4.1]删除数据
	public function q_delete($tablename, $where)
	{
		try
		{
			//条件
			$q_where = '';
			if ($where != '')
			{
				$q_where = " where " . $where;;
			}
			else  //(kevinliu:为了防止误删除，采用必须输入条件才能修改的方法，确实有必要考虑修改此处或者直接用sql语句的方式)
			{
				throw new My_Exception("where = null, break delete.", -100, __FILE__, __LINE__);
			}
			$sql = 'delete from ' . $tablename . $q_where;
			return $this->delete($sql);
		}
		catch (My_Exception  $e)
		{
			throw $e;
		}	
	}
	// [4.2] 删除数据（直接SQL语句）
	public function delete($sql)
	{	
		try
		{
			DEBUG_LOG($sql);
			$this->connect();	
			if (false == mysql_query($sql))
			{
				$err = mysql_error();
				throw new My_Exception("delete($sql) sql failed, reason is $err.", mysql_errno(), __FILE__, __LINE__);
			}
			return mysql_affected_rows();
		}
		catch (My_Exception  $e)
		{
			throw $e;
		}	
	}
}
?>