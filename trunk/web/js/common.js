// 0. ��������
function rhis()
{
	history.back();
	return;
}
function geturlarg()
{
	var url = unescape(window.location.href);
	var allargs = url.split("?")[1];  //?��벿�֣�ע����ر�֤����ֻ�У����ʺ�
	var args = allargs.split("&");    //�ٴηָ�ɰ���ÿ�������ֿ�
	for(var i = 0; i < args.length; i++)  //��Ϊ����ȡ����
	{
		var arg = args[i].split("="); //�ָ�ؼ��ֺ�ֵ
		eval('this.' + arg[0] + '="' + arg[1] + '";'); // ��Ϊ����洢
	}
} 
//var urlargs = new geturlarg();

function checkipport(ip, port)
{
	var s = ip;
	patrn=/^([0-9]|[.]){7,15}$/;
	if (!patrn.exec(s)) 
	{
		alert("��������ȷ��IP��ַ����ʽ��");
		return false;
	}
	s = port;
	patrn=/^[0-9]{1,6}$/;
	if (!patrn.exec(s)) 
	{
		alert("�����������Ķ˿ڷ�Χ1-6λ����");
		return false;
	}
	return true;
}

//��������IP�Ͷ˿ڣ��������ݿ�
function searchtree()
{
	if (false == checkipport(document.form_searchtree.topserverip.value, document.form_searchtree.topserverport.value))
	{
		return false;
	}
	var s = document.form_searchtree.topsitename.value;
	patrn=/^([a-zA-Z0-9]|[._:]){0,30}$/;
	if (!patrn.exec(s)) 
	{
		alert("վ�����Ʊ���Ϊ0-30λ�Ŀɴ����֡���_������.����:�����ַ�����");
		return false;
	}
	document.form_searchtree.action = "/tree.php";
    document.form_searchtree.submit();
}
function searchtree_reset()
{
	document.form_searchtree.topserverip.value = '';
	document.form_searchtree.topserverport.value = '';
	document.form_searchtree.topsitename.value = '';
}
function cfgmodify(type)
{
	//�����ַ�����
	if (type == 0)
	{
		document.form_searchtree.topserverip.value = document.form_edit.serverip.value;
		document.form_searchtree.topserverport.value = document.form_edit.serverport.value;
		/*
		var ifexist = 0;		
		var objs = document.getElementsByTagName("input");
	    for(var i=0; i<objs.length; i++) 
	    {
	    	if(objs[i].type.toLowerCase() == "text")
	    	{
	    		if(objs[i].name == "sitename")
	    		{
	    			ifexist = 1;
	    			break;
	    		}
	    	}
	    }
	    */
		document.form_searchtree.topsitename.value = document.form_edit.sitename.value;
		return searchtree();
	}
	else if (type == 1 || type == 11)
	{
		if (false == checkipport(document.form_edit.serverip.value, document.form_edit.serverport.value))
		{
			return false;
		}
		if(type == 1)
		{
			document.form_edit.optype.value = 0;  // 0 �鿴
		}
		else // 11
		{
			if (document.form_edit.comoncfgcnt.value == "")
			{
				alert("�����ļ������ݲ���Ϊ�գ�");
				return false;
			}
			document.form_edit.optype.value = 1;  // 1�޸�
		}
		document.form_edit.action = "/commoncfg.php";
		document.form_edit.submit();
		return;
	}
	else if (type == 2 || type == 22)
	{		
		if (false == checkipport(document.form_edit.serverip.value, document.form_edit.serverport.value))
		{
			return false;
		}
		if(type == 2)
		{
			document.form_edit.optype.value = 0;  // 0 �鿴
		}
		else if(type == 22)// 22
		{
			if (document.form_edit.sitecfgcnt.value == "")
			{
				alert("�����ļ������ݲ���Ϊ�գ�");
				return false;
			}
			document.form_edit.optype.value = 1;  // 1�޸�
		}		
		document.form_edit.action = "/sitecfg.php";
		document.form_edit.submit();
		return;
	}
	else  if (type == 222)
	{
		if (document.form_edit.level.value != "") //����û�в���ֶ�
		{
			alert("��ѡ��վ�������ٲ�����");
			return false;
		}		
		var s = document.form_edit.parent.value;
		patrn=/^([a-zA-Z0-9]|[._]){9,30}$/;
		if (!patrn.exec(s)) 
		{
			alert("�ϲ�ڵ�IP�Ͷ˿ڱ���Ϊ1-30λ�Ŀɴ����֡���_������.����:�����ַ�����");
			return false;
		}
		var a = document.form_edit.sitename.value;
		patrn=/^([a-zA-Z0-9]|[._:]){1,30}$/;
		if (!patrn.exec(a)) 
		{
			alert("վ�����Ʊ���Ϊ1-30λ�Ŀɴ����֡���_������.����:�����ַ�����");
			return false;
		}
		var b = document.form_edit.newsitename.value;
		patrn=/^([a-zA-Z0-9]|[._:]){1,30}$/;
		if (!patrn.exec(b)) 
		{
			alert("��վ�����Ʊ���Ϊ1-30λ�Ŀɴ����֡���_������.����:�����ַ�����");
			return false;
		}
		if (a == b)
		{
			alert("վ��������ͬ��");
			return false;
		}
		if (!confirm("��¡վ��ᷢ�������е������������ϣ��Ƿ�ȷ�����Ӵ��µ�վ�㣿"))
        {
            return false;
        }
		document.form_edit.optype.value = 2;  // 2��¡
		document.form_edit.action = "/sitecfg.php";
		document.form_edit.submit();
		return true;
	}
	else if (type == 3 || type == 33)
	{
		if (false == checkipport(document.form_edit.serverip.value, document.form_edit.serverport.value))
		{
			return false;
		}
		if(type == 3)
		{
			document.form_edit.optype.value = 0;  // 0 �鿴
		}
		else if(type == 33)// 33
		{
			if (document.form_edit.mobilecnt.value == "")
			{
				alert("�����ļ������ݲ���Ϊ�գ�");
				return false;
			}
			document.form_edit.optype.value = 1;  // 1�޸�
			document.form_edit.subcheck.value = 0; //��ʼֵ
			var objs = document.getElementsByTagName("input");
		    for(var i=0; i<objs.length; i++) 
		    {
		    	if(objs[i].type.toLowerCase() == "checkbox")
		    	{
		    		if(objs[i].checked == true)
		    		{
		    			document.form_edit.subcheck.value = 1;  // ��Ҫ����
		    			break;
		    		}
		    	}
		   }
		}		
		document.form_edit.action = "/mobilecfg.php";
		document.form_edit.submit();
		return;
	}
	else if (type == 4)
	{
		if (false == checkipport(document.form_edit.serverip.value, document.form_edit.serverport.value))
		{
			return false;
		}		
		document.form_edit.action = "/logtrace.php";
		document.form_edit.submit();
	}
	else if (type == 8)  //the same to type=4, only action
	{
		if (false == checkipport(document.form_edit.serverip.value, document.form_edit.serverport.value))
		{
			return false;
		}
		//�����ڽ���У��
		var s = document.form_edit.statdate.value;
		if("" != s)
		{
			patrn=/^[0-9]{4}[0-9]{2}[0-9]{2}$/;
			if (!patrn.exec(s)) 
			{
				alert("��������ȷ��������ʽ��");
				return false;
			}		
		}
		document.form_edit.action = "/stat.php";
		document.form_edit.submit();
	}
	else if (type == 5)
	{
		if (false == checkipport(document.form_edit.serverip.value, document.form_edit.serverport.value))
		{
			return false;
		}
		document.form_edit.optype.value = 0;
		document.form_edit.action = "/queueinfo.php";
		document.form_edit.submit();
	}
	else if (type == 6)
	{
		if (false == checkipport(document.form_edit.serverip.value, document.form_edit.serverport.value))
		{
			return false;
		}		
		document.form_edit.optype.value = 1;
		document.form_edit.action = "/queueinfo.php";
		document.form_edit.submit();
	}
	else if (type == 7)
	{
		if (false == checkipport(document.form_edit.serverip.value, document.form_edit.serverport.value))
		{
			return false;
		}		
		document.form_edit.optype.value = 2;
		document.form_edit.action = "/queueinfo.php";
		document.form_edit.submit();
	}
}
function quicktree(ip, port, site)
{
	document.form_searchtree.topserverip.value = ip;
	document.form_searchtree.topserverport.value = port;
	document.form_searchtree.topsitename.value = site;
	return searchtree();
}