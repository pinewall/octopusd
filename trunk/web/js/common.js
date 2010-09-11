// 0. 公共区域
function rhis()
{
	history.back();
	return;
}
function geturlarg()
{
	var url = unescape(window.location.href);
	var allargs = url.split("?")[1];  //?后半部分，注意务必保证其中只有１个问号
	var args = allargs.split("&");    //再次分割成按照每个参数分开
	for(var i = 0; i < args.length; i++)  //作为对象取出来
	{
		var arg = args[i].split("="); //分割关键字和值
		eval('this.' + arg[0] + '="' + arg[1] + '";'); // 作为对象存储
	}
} 
//var urlargs = new geturlarg();

function checkipport(ip, port)
{
	var s = ip;
	patrn=/^([0-9]|[.]){7,15}$/;
	if (!patrn.exec(s)) 
	{
		alert("请输入正确的IP地址的形式！");
		return false;
	}
	s = port;
	patrn=/^[0-9]{1,6}$/;
	if (!patrn.exec(s)) 
	{
		alert("请输入正常的端口范围1-6位整数");
		return false;
	}
	return true;
}

//根据主机IP和端口，搜索数据库
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
		alert("站点名称必须为0-30位的可带数字、“_”、“.”“:”的字符串！");
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
	//下属分发网络
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
			document.form_edit.optype.value = 0;  // 0 查看
		}
		else // 11
		{
			if (document.form_edit.comoncfgcnt.value == "")
			{
				alert("配置文件的内容不能为空！");
				return false;
			}
			document.form_edit.optype.value = 1;  // 1修改
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
			document.form_edit.optype.value = 0;  // 0 查看
		}
		else if(type == 22)// 22
		{
			if (document.form_edit.sitecfgcnt.value == "")
			{
				alert("配置文件的内容不能为空！");
				return false;
			}
			document.form_edit.optype.value = 1;  // 1修改
		}		
		document.form_edit.action = "/sitecfg.php";
		document.form_edit.submit();
		return;
	}
	else  if (type == 222)
	{
		if (document.form_edit.level.value != "") //必须没有层次字段
		{
			alert("请选中站点名称再操作！");
			return false;
		}		
		var s = document.form_edit.parent.value;
		patrn=/^([a-zA-Z0-9]|[._]){9,30}$/;
		if (!patrn.exec(s)) 
		{
			alert("上层节点IP和端口必须为1-30位的可带数字、“_”、“.”“:”的字符串！");
			return false;
		}
		var a = document.form_edit.sitename.value;
		patrn=/^([a-zA-Z0-9]|[._:]){1,30}$/;
		if (!patrn.exec(a)) 
		{
			alert("站点名称必须为1-30位的可带数字、“_”、“.”“:”的字符串！");
			return false;
		}
		var b = document.form_edit.newsitename.value;
		patrn=/^([a-zA-Z0-9]|[._:]){1,30}$/;
		if (!patrn.exec(b)) 
		{
			alert("新站点名称必须为1-30位的可带数字、“_”、“.”“:”的字符串！");
			return false;
		}
		if (a == b)
		{
			alert("站点名称相同！");
			return false;
		}
		if (!confirm("克隆站点会发生在所有的下属服务器上，是否确认增加此新的站点？"))
        {
            return false;
        }
		document.form_edit.optype.value = 2;  // 2克隆
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
			document.form_edit.optype.value = 0;  // 0 查看
		}
		else if(type == 33)// 33
		{
			if (document.form_edit.mobilecnt.value == "")
			{
				alert("配置文件的内容不能为空！");
				return false;
			}
			document.form_edit.optype.value = 1;  // 1修改
			document.form_edit.subcheck.value = 0; //初始值
			var objs = document.getElementsByTagName("input");
		    for(var i=0; i<objs.length; i++) 
		    {
		    	if(objs[i].type.toLowerCase() == "checkbox")
		    	{
		    		if(objs[i].checked == true)
		    		{
		    			document.form_edit.subcheck.value = 1;  // 需要下属
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
		//对日期进行校验
		var s = document.form_edit.statdate.value;
		if("" != s)
		{
			patrn=/^[0-9]{4}[0-9]{2}[0-9]{2}$/;
			if (!patrn.exec(s)) 
			{
				alert("请输入正确的日期形式！");
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