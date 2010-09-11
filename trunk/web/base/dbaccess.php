<?php

require_once ("exception.php");
require_once ("log.php");

/******************
˵��:��ʱ����mysql�ķ�ʽ��mysqli�ķ�ʽ,���ܻ�����Ϊ��̫�ȶ��ط�ʽ,���ǳ���ģ������Ĵ���,�ڴ��߰汾����ɣ�
mysql�ķ�ʽû�в����׳��쳣�ķ�ʽ,��֮������׳��쳣����ʽ
********************/

//[1.1]�Զ����һ��quote����
function mysql_quote($value)
{
	$value = str_replace('"', "'", $value);
	return $value;
}
//[1.2]�ֶ���Ϣ��ת�壬�����mysql_escape_string() ,�������Ƿ�ȫ
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
//[2.0]���ݿ������
class CDBAccess
{
	protected $m_link = null; //���ݿ�����
	public $m_rows = array();
	function __construct()
	{
	}
	function __destruct ()
	{
		//�ر����ݿ�������������������
		$this->disconnect();
	}
	//[0.1]�������ݿ� ������define.php���������������Ϊ�����ʹ�ã��뽫��Ϣ����������֮���Բ����ر�ɹ淶������������ά����Ա�޸����ݿ���Ϣ������Ӱ��ʵ�ִ���
	public function connect()
	{
		if ($this->m_link != null) // �ǿ�
		{
			return;
		}
		//�������ݿ�
		$this->m_link = mysql_connect(DB_HOST, DB_USER, DB_PASSWD);
		if ($this->m_link == null)
		{
			$err = mysql_error();
			throw new My_Exception("Connect db failed, reason is $err.", mysql_errno(), __FILE__, __LINE__);
		}
		//�е���Ӧ�����ݿ���
		if (false == mysql_select_db(DB_NAME))
		{
			$err = mysql_error();
			throw new My_Exception("select db failed, reason is $err.", mysql_errno(), __FILE__, __LINE__);
		}
		return;
	}
	//[0.2] �ر���������
	function disconnect()
	{
		if ($this->m_link != null)
		{
			mysql_close($this->m_link);
			$this->m_link = null;
		}
	}

	//[0.3]�л����ݿ�
	public function changedb($dbname)
	{
		//�е���Ӧ�����ݿ���
		if (false == mysql_select_db($dbname))
		{
			$err = mysql_error();
			throw new My_Exception("select db($dbname) failed, reason is $err.", mysql_errno(), __FILE__, __LINE__);
		}
		return;
	}
	//[1.1]��ѯ���ݿ�
	public function q_select($fields, $tablename, $where, $order, $no_page, $every_limit)
	{	
		try
		{
			//�ֶ��б�
			$field_str = implode(',', $fields);
			//����
			$q_where = '';
			if ($where != '')
			{
				$q_where = " where " . $where;
			}
			//����
			$q_order = '';
			if ($order != '')
			{
				$q_order = " order by " . $order;
			}
			//����
			$q_limit = '';
			if ($every_limit != 0)
			{
				$q_limit = " limit " . $no_page . "," . $every_limit;
			}		
			//��ʼƴдSQL�����
			$sql = "select " . $field_str . " from " . $tablename . $q_where . $q_order . $q_limit;
			//��ʼ��ѯ
			$this->select($sql);
		}
		catch (My_Exception $e)
		{
			throw $e;
		}
	}
	//[1.2]��ѯ���ݿ�(ֱ�ӵ�SQL���)
	public function select($sql)
	{	
		try
		{
			DEBUG_LOG($sql);
			$this->connect();
			$result = mysql_query($sql);
			if($result == null) //��
			{
				$err = mysql_error();
				throw new My_Exception("Execute($sql) failed, reason is $err.", mysql_errno(), __FILE__, __LINE__);
			}
			//����ϴβ�ѯ�У��������ݣ������
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
	// [2.1] ��������
	public function q_insert($tablename, $fields, $values)
	{
		try
		{
			//�ֶ��б�
			$field_str = implode(',', $fields);
			//�ֶ�ֵ�б�
			$value_str = "";
			for($i = 0; $i < count($values); $i++)
			{
				$value_str = $value_str . "'" . mysql_escape_string( $values[$i]) . "'" ;
				if ($i != count($values) - 1)
				{
					 $value_str = $value_str . ',';
				}
			}				
			//��ʼƴдSQL�����
			$sql = "insert into " . $tablename ."(" . $field_str . ")values(" . $value_str . ")";
			return $this->insert($sql);
		}
		catch (My_Exception  $e)
		{
			throw $e;
		}
	}
	// [2.2] �������ݣ�ֱ��SQL��䣩
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
	// [3.1]��������
	public function q_update($tablename, $fields, $values, $where)
	{		
		try
		{
			//�ֶ�ֵ�б�
			$update_str = "";
			for($i = 0; $i < count($fields); $i++)
			{				
				$update_str = $update_str . $fields[$i] . "='" . mysql_escape_string($values[$i]) . "'" ;
				if ($i != count($values) - 1)
				{
					 $update_str = $update_str . ',';
				}
			}
			//����(kevinliu:Ϊ�˷�ֹ���޸ģ����ñ����������������޸ĵķ���)
			$q_where = '';
			if ($where != '')
			{
				$q_where = " where " . $where;;
			}
			else  //(kevinliu:Ϊ�˷�ֹ���޸ģ����ñ����������������޸ĵķ�����ȷʵ�б�Ҫ�����޸Ĵ˴�����ֱ����sql���ķ�ʽ)
			{
				throw new My_Exception("where = null, break update.", -100, __FILE__, __LINE__);
			}
			//ִ��
			$sql = 'update ' . $tablename . " set " . $update_str . $q_where;
			return $this->update($sql);
		}
		catch (My_Exception  $e)
		{
			throw $e;
		}
	}
	// [3.2] �������ݣ�ֱ��SQL��䣩
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
	// [4.1]ɾ������
	public function q_delete($tablename, $where)
	{
		try
		{
			//����
			$q_where = '';
			if ($where != '')
			{
				$q_where = " where " . $where;;
			}
			else  //(kevinliu:Ϊ�˷�ֹ��ɾ�������ñ����������������޸ĵķ�����ȷʵ�б�Ҫ�����޸Ĵ˴�����ֱ����sql���ķ�ʽ)
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
	// [4.2] ɾ�����ݣ�ֱ��SQL��䣩
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