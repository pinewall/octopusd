<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
<title>main</title>
<link href="/css/common.css" rel="stylesheet" type="text/css" />
<link href="/css/JTree.css" rel="stylesheet" type="text/css" />
<script src="/js/common.js" type="text/javascript"></script>
<script src="/js/JTree.js" type="text/javascript"></script>
<?php 
error_reporting(E_ERROR | E_WARNING | E_PARSE);
$topserverip = $_POST['topserverip'];
$topserverport = $_POST['topserverport'];
$topsitename = $_POST['topsitename'];
?>
</head>
<body>
<form name="form_searchtree" action="" method="post" class="main"> 
		&nbsp;<span class="stfont">������ַ���������Ϣ��</span>������IP:
			<input id=topserverip name=topserverip value="<?php echo $topserverip; ?>" class=inputAC type=text></input>			
		������Port:
		<input id=topserverport name=topserverport value="<?php echo $topserverport; ?>" class=inputAC type=text></input>
		�ַ�վ��:
		<input id=topsitename name=topsitename value="<?php echo $topsitename; ?>" class=inputAC type=text></input>
		<input class=btn_normal id=searchtreesubmit value="�� ��" type="button" onclick="searchtree();"></input> 
		<input class=btn_normal id=searchtreereset value="�� ��" type="button" onclick="searchtree_reset();"></input> 
</form>
<div class="leftf">
	<div class="topt">�޸Ķ�Ӧ������</div>	
	<div class="cmt"> 
		<dd class="lf">	
		<form name="form_edit" action="" method="post"> 
		  <input id=oprtype name=optype type="hidden"></input>
		  <input id=statdate name=statdate type="hidden"></input>
			<div class="aline">�ϲ�ڵ�IP�Ͷ˿ڣ�
			<input id=parent name=parent value="-" class=inputLC type=text readonly></input>
			</div>
			<div class="aline">������վ�㣺
			<input id=sitename name=sitename value="<?php echo $topsitename; ?>" class=inputLC type=text></input>
			</div>
			<div class="aline">��������IP��ַ��
			<input id=serverip name=serverip value="<?php echo $topserverip; ?>" class=inputLC type=text></input>
			</div>
			<div class="aline">�������Ķ˿ڣ�
			<input id=serverport name=serverport value="<?php echo $topserverport; ?>" class=inputLC type=text></input>
			</div>
			<div class="aline">�ַ����ϵĲ�κţ�
			<input id=level name=level value="-" class=inputLC type=text readonly></input>
			</div>
			<div align="center">
				<input type="button" class="btn_normal" value="�����ַ���Ϣ" onclick="cfgmodify(0);">
				<input type="button" class="btn_normal" value="�޸Ļ�������" onclick="cfgmodify(1);">
				<input type="button" class="btn_normal" value="�޸ķַ�����" onclick="cfgmodify(2);">
				<input type="button" class="btn_normal" value="�޸ĸ澯����" onclick="cfgmodify(3);">
				<input type="button" class="btn_normal" value="�鿴��־��Ϣ" onclick="cfgmodify(4);">
				<input type="button" class="btn_normal" value="�鿴������Ϣ" onclick="cfgmodify(5);">
				<input type="button" class="btn_normal" value="�鿴ͳ����Ϣ" onclick="cfgmodify(8);">
			</div> 				
			<div align="left">
				<div class="aline">��վ�����ƣ�<input id=newsitename name=newsitename value="" class=inputLC type=text></input></div>
				<div align="center">
					<input type="button" class="btn_normal" value="��¡վ������" onclick="cfgmodify(222);">	
				</div>
			</div>
			</form>
		</dd>
	</div>	
</div>
<div class="rightf">  
	<table width="100%">
	<tr>
	<td>
	 <div class="cmt2"> 
		<dd class="lf">	
		<!--�������ʼ����Զ���Ŀ�ݷ�ʽ-->
		<a class=b0 href="#" onclick="quicktree('10.1.43.137', '21811', '');">������ϵͳ(ȫվ)</a>
		<a class=b0 href="#" onclick="quicktree('10.1.43.137', '21811', 'news');">������ϵͳ(����)</a>
		<a class=b0 href="#" onclick="quicktree('10.1.43.137', '21811', 'ent');">������ϵͳ(����)</a>
		<!--�������������Զ���Ŀ�ݷ�ʽ-->
		</dd>
	</div>
	</td>
	</tr>
	
	<tr>
	<td>
	<div class="cmt2" id="showTree"></div>
	</td>
	</tr>
	</table>
</div>
</body>
</html>
<script language="javascript" type="text/javascript">
var url = "";
<?php 
    if ($topserverip == '')
    {
    	echo "var url = \"/initxml.php\";";
    }
    else 
    {
		echo "var url = \"/treexml.php?topserverip=$topserverip&topserverport=$topserverport&topsitename=$topsitename\";";
    }	
?>
var myTree=new JTree("showTree", url);
myTree.setPicPath("/images/tree/")
myTree.onclick=function()
{
	if ("&nbsp;&nbsp;no result" == getNodeAtt(myTree.selectNode,"caption"))
	{
		return;
	}
	if('site' == getNodeAtt(myTree.selectNode,"type"))
	{
		setValueById("parent", getNodeAtt(myTree.selectNode.parentNode,"caption"));
		var st = getNodeAtt(myTree.selectNode,"caption");
		var pos = st.indexOf("(");
		if (pos == -1)
		{
			setValueById("sitename", st); //վ������
		}
		else
		{
			setValueById("sitename", st.substring(0, pos)); //վ������
		}
		setValueById("serverip",'');
		setValueById("serverport",'');
		setValueById("level",'');
		return; //�����վ��ڵ�,����ʾ����ര����
	}
	var p;
	if (myTree.clickItem.level == 1) // -1,0,1,2... ���ڵ�1��ڵ�ʱ������0��վ����Ϣ,�����ȥ��ʾ,��ʾ����һ��
	{
		p = getNodeAtt(myTree.selectNode.parentNode.parentNode,"caption");
	}	
	else
	{
		p = getNodeAtt(myTree.selectNode.parentNode,"caption");
	}
	if (myTree.clickItem.level == -1)
	{
		setValueById("parent", "��");
	}
	else
	{
		setValueById("parent", p);
	}
	
	//����վ��,���ϵݹ�ֱ��������һ��siteվ��
	var tmpNode = myTree.selectNode;
	setValueById("sitename", ""); ///�ȷ��óɿգ��Է�ֹ�Ҳ������ϴε�ֵ
	while(tmpNode)
	{
		//alert(getNodeAtt(tmpNode,"type"));
		if ("site" == getNodeAtt(tmpNode,"type"))
		{
			setValueById("sitename", getNodeAtt(tmpNode,"caption"));
			break;
		}
		tmpNode = tmpNode.parentNode;
	}	
	var title = getNodeAtt(myTree.selectNode,"caption");
	//������2���ֶ� IP_Port
	if (title == 'none')
	{
		setValueById("serverip", "��");
		setValueById("serverport","��");
	}
	else
	{
		var args = title.split("_");
		setValueById("serverip", args[0]);
		var pos = args[1].indexOf("(");
		var port ;
		if (pos == -1)
		{
			port = args[1];
		}
		else
		{
			port = args[1].substring(0, pos);
		}
		setValueById("serverport", port);
	}	
	if (myTree.clickItem.level == -1)
	{
		setValueById("level",'0');
	}
	else
	{
		setValueById("level",myTree.clickItem.level);
	}	
}
myTree.CAPTIONATT="caption";
myTree.create();
</script>