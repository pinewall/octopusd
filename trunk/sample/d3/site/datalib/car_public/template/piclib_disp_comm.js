//tree control
var setLeftFrame = function(flag){
	this.showFrame = function(){
		window.top.document.getElementById("FrameMain").cols='0,13,*'; 
		$("Control").onclick=function(){setLeftFrame(false)};
		$("Control").style.background="url(http://mat1.qq.com/auto/datalib/new_lib/close.gif) no-repeat left center;";
		$("Controlbg").style.background="url(http://mat1.qq.com/auto/datalib/new_lib/closebg.gif)";
		$("Control").title="显示左侧列表"; 
	};
	this.hiddenFrame = function(){
		window.top.document.getElementById("FrameMain").cols=this.save;
		$("Control").onclick=function(){setLeftFrame(true)};
		$("Control").style.background="url(http://mat1.qq.com/auto/datalib/new_lib/open.gif) no-repeat left center;"
		$("Controlbg").style.background="url(http://mat1.qq.com/auto/datalib/new_lib/openbg.gif)";
		$("Control").title="隐藏左侧列表";
	};
	if(flag){
		this.save = window.top.document.getElementById("FrameMain").cols;
		this.showFrame();
	} 
	else{
		this.hiddenFrame();
	}
};

function dispIndexUpload() {
	JsLoader.load("/car_public/1/upload_actp_json.js", function()
	{
		if(typeof oUploadACTP != "undefined" && oUploadACTP.arrArticle.length != 0)
		{
			var buffer = new StringBuffer();
			buffer.append('<table width="100%" border="0" cellpadding="0" cellspacing="0">');
			buffer.append('<tr>');
			buffer.append('<td>');
			buffer.append('<table width="100%" border="0" cellpadding="0" cellspacing="0" background="http://mat1.qq.com/auto/datalib/new_lib/bg_d12.gif">');
			buffer.append('<tr>');
			buffer.append('<td width="20" height="28" background="http://mat1.qq.com/auto/datalib/new_lib/bg_d11.gif">&nbsp;</td>');
			buffer.append('<td align="left" valign="bottom" class="fontbold fontl26">最新网友上传</td>');
			buffer.append('<td width="6" background="http://mat1.qq.com/auto/datalib/new_lib/bg_d13.gif">&nbsp;</td>');
			buffer.append('</tr>');
			buffer.append('</table></td>');
			buffer.append('</tr>');
			buffer.append('<tr>');
			buffer.append('<td align="center"><table width="96%" border="0" cellpadding="0" cellspacing="0" id="right_picslist">');
			var nUserfulIndex=0;
			oUploadACTP.arrArticle.any(function(item, index){
				if(item.sKeyword.indexOf("http") != -1) {
					if(nUserfulIndex % 5 == 0) {
						buffer.append('<tr align="center">');
					}				
					buffer.append('<td valign="top">');
					buffer.append('<a href="' + item.sUrl + '" target="_blank">');
					buffer.append('<img src="' + item.sKeyword + '" width="120" height="90" onError="this.src=\'http://mat1.qq.com/auto/datalib/new_lib/none.gif\'" title="' + item.sTitle + '"/>');
					buffer.append('</a><br />');
					buffer.append('<div style="width:120px;overflow:hidden;text-overflow:ellipsis;white-space:nowrap;">');												  
					buffer.append('<a href="' + item.sUrl + '" target="_blank" title="' + item.sTitle + '">');
					buffer.append(item.sTitle);
					buffer.append('</a>');
					buffer.append('</div>');
					buffer.append('</td>');
					if(nUserfulIndex % 5 == 4) {
						buffer.append('</tr>');
					}
					nUserfulIndex++;
					if(nUserfulIndex == 10) {
						return true;
					}
				}
			});
			for(;nUserfulIndex % 5 != 0; nUserfulIndex++) {
				buffer.append('<td width="20%">&nbsp;</td>');
				if(nUserfulIndex % 5 == 4) {
					buffer.append('</tr>');
				}
			}
			buffer.append('</table>');
			buffer.append('</td>');
			buffer.append('</tr>');
			buffer.append('</table>');			
			$("disp_upload").innerHTML = buffer.toString();			
		}
	});
}

function initIndexPicLib() {
	addLoadEvent(dispIndexUpload);
}


ShowBoxb.prototype = new Page();

function ShowBoxb(sObjName, nTotalNum, nNumPerPage)
{
	Page.call(this, sObjName, nTotalNum, nNumPerPage);	
}

ShowBoxb.prototype.initobj = function(obj)
{
    ShowBoxb.item=obj;   
}


ShowBoxb.prototype.goPage = function(nGotoPage) 
{
    
/*    
    if(this.item.length == 1)//如果事件下只有一个明细，直接跳转到该明细，不然页面太空，体验不好。
    {
        window.location.href = '/car_public/1/piclib_c_disp.shtml?cid=' + this.item[0].CID;
        return;
    }
*/
    
    var buffer = new StringBuffer();
    var inum=(nGotoPage-1)*this._nNumPerPage;
    var nUserfulIndex=0;
    var CName="";
	var Aid=0;
	var Bid=0;
	var AName="";
	var BName="";
    this._nCurPage = parseInt(nGotoPage,10);
    buffer.append('<table width="96%" border="0" cellpadding="0" cellspacing="0" id="right_picslist">');
    for(inum=(nGotoPage-1)*this._nNumPerPage;(nUserfulIndex<this._nNumPerPage)&&(inum<this.item.length);inum++)
    {
       if(CName == "") 
       {
			CName = this.item[inum].CName;
			Aid=this.item[inum].AID;
			AName=this.item[inum].AName;
			Bid=this.item[inum].BID;
			BName=this.item[inum].BName;
$("classtitle").innerHTML=BName+"列表";
        }
		if(nUserfulIndex % 5 == 0) 
		{
    		buffer.append('<tr align="center">');
    	}
    	buffer.append('<td valign="top" style="padding-bottom:20px">');
    	buffer.append('<a href="/car_public/1/piclib_c_disp.shtml?cid=' + this.item[inum].CID + '" >');

         var tmppicurl=this.item[inum].PicUrl;
    	tmppicurl=tmppicurl.slice(0,tmppicurl.indexOf(".jpg")-1);
    	tmppicurl=tmppicurl+".jpg";

    	buffer.append('<img width="120" src="' + tmppicurl + '" onError="this.src=\'http://mat1.qq.com/auto/datalib/new_lib/none.gif\'" title="' + (this.item[inum].PicNum=="0" ? this.item[inum].CName : "本图集共有" + this.item[inum].PicNum + "张图片 点击进入") +  '" style="margin-bottom:10px;"/>');
    	buffer.append('</a><br />');
    	buffer.append('<a href="/car_public/1/piclib_c_disp.shtml?cid=' + this.item[inum].CID + '" title="' + (this.item[inum].PicNum=="0" ? this.item[inum].CName : "本图集共有" + this.item[inum].PicNum + "张图片 点击进入") +  '">');
    	buffer.append(this.item[inum].CName + "(" + (this.item[inum].PicNum=="0" ? "暂无" :this.item[inum].PicNum) + ")");
    	buffer.append('</a>');
    	buffer.append('</td>');					
    	if(nUserfulIndex % 5 == 4) 
    	{
    		buffer.append('</tr>');
    	}
        nUserfulIndex++;									
    }

   for(;nUserfulIndex % 5 != 0; nUserfulIndex++) 
   {
		buffer.append('<td width="20%">&nbsp;</td>');
		if(nUserfulIndex % 5 == 4) 
		{
			buffer.append('</tr>');
		}
	}
	buffer.append('</table>');
	$("disp_top_links").innerHTML = '<a href="/car_brand/index.shtml" target="_parent">汽车库</a> &gt; <a href="/car_public/1/piclib_frame_main.shtml">汽车图库</a> &gt; ' + BName;
    if(nUserfulIndex > 0) 
    {
		$("disp_serial_list").innerHTML = buffer.toString();	
	} 
	else 
    {

/*      JsLoader.load("/techevent/" + oUrlParas1.bid + "/info.js", function()
        {
        $("disp_top_links").innerHTML = '<a href="http://tech.qq.com" target="_blank" class="underline">科技首页</a> &gt; <a href="/car_public/1/piclib_frame_main.shtml" class="underline">科技图库</a> &gt; <a href="/car_public/1/piclib_frame_a.shtml?aid='+oDetailInf.TechId+'" class="underline">'+oDetailInf.TechName+'</a> &gt; '+oDetailInf.Name;
        });
*/
        $("disp_top_links").innerHTML = '<a href="/car_brand/index.shtml" target="_parent">汽车库</a> &gt; <a href="/car_public/1/piclib_frame_main.shtml">汽车图库</a>';
		$("disp_serial_list").innerHTML = "暂无记录";
	}

    var tmpstr=this.getPageInfo();
    $("page_info1").innerHTML = tmpstr;
    $("page_info2").innerHTML = tmpstr;

}


ShowBoxc.prototype = new Page();

function ShowBoxc(sObjName, nTotalNum, nNumPerPage)
{
	Page.call(this, sObjName, nTotalNum, nNumPerPage);	
}

ShowBoxc.prototype.initobj = function(obj)
{
 ShowBoxc.item=obj;   
}


ShowBoxc.prototype.goPage = function(nGotoPage)
{
    var buffer = new StringBuffer();
    var inum=(nGotoPage-1)*this._nNumPerPage;
    var nUserfulIndex=0;
    this._nCurPage = parseInt(nGotoPage,10);
    g_objSerial.CID=this.SerialID;

	dispTopLinks = function() 
	{
		$("disp_top_links").innerHTML = '<a href="/car_brand/index.shtml" target="_parent">汽车库</a> &gt; <a href="/car_public/1/piclib_frame_main.shtml">汽车图库</a> &gt; <a href="/car_public/1/piclib_b_disp.shtml?bid='+g_objSerial.BID+'">'+g_objSerial.BName+'</a> &gt;' + g_objSerial.CName;
    };
    
    //加载明细信息
	JsLoader.load("/autopic/" +g_objSerial.CID+ "/info.js", function()
	{
		g_objSerial.BID = oDetailInf.BId;
		g_objSerial.BName = oDetailInf.BName;
		g_objSerial.AID = oDetailInf.AId;
		g_objSerial.AName = oDetailInf.AName;
		g_objSerial.CName = oDetailInf.CName;
		
		$("classtitle").innerHTML=g_objSerial.BName+"列表";
		
		dispTopLinks();
	});
    
    
    
    buffer.append('<table width="100%" border="0" cellpadding="0" cellspacing="0">');
    buffer.append('<tr>');
	buffer.append('<td align="center"><table width="96%" border="0" cellpadding="0" cellspacing="0" id="right_picslist">');
	
    for(inum=(nGotoPage-1)*this._nNumPerPage;(nUserfulIndex<this._nNumPerPage)&&(inum<this.item.length);inum++)
    {
			if(nUserfulIndex % 5 == 0) 
			{
				buffer.append('<tr align="center">');
			}				
			buffer.append('<td valign="top" style="padding-bottom:20px">');
			buffer.append('<a href="/car_public/1/piclib_disp.shtml?cid=' + g_objSerial.CID + '&pid=' + this.item[inum].PicID + '" target="_blank">');
			buffer.append('<img src="/autopic' + this.item[inum].sZoomImgUrl + '"  onError="this.src=\'http://mat1.qq.com/auto/datalib/new_lib/none.gif\'" title="' + this.item[inum].sDesc + ' 点击进入查看大图" style="margin-bottom:10px;"/>');
			buffer.append('</a><br />');
//			buffer.append('<div style="width:120px;overflow:hidden;text-overflow:ellipsis;white-space:nowrap;">');
//			buffer.append('<a href="/car_public/1/piclib_disp.shtml?cid=' + g_objSerial.CID + '&pid=' + this.item[inum].PicID + '" target="_blank" title="' + this.item[inum].sDesc + ' 点击进入查看大图">');
//			buffer.append(this.item[inum].sDesc);
//			buffer.append('</a>');
//			buffer.append('</div>');
			buffer.append('</td>');
			if(nUserfulIndex % 5 == 4) 
			{
				buffer.append('</tr>');
			}
			nUserfulIndex++;					
    }

   for(;nUserfulIndex % 5 != 0; nUserfulIndex++) 
   {
		buffer.append('<td width="20%">&nbsp;</td>');
		if(nUserfulIndex % 5 == 4) 
		{
			buffer.append('</tr>');
		}
	}
	
	
	buffer.append('</table>');
	buffer.append('</td>');
	buffer.append('</tr>');
	buffer.append('</table>');		

	if(nUserfulIndex > 0) 
	{
		$("disp_serial_list").innerHTML = buffer.toString();	
	} 
	else 
    {
		$("disp_serial_list").innerHTML = "暂无记录";
	}

    var tmpstr=this.getPageInfo();
    $("page_info1").innerHTML = tmpstr;
    $("page_info2").innerHTML = tmpstr;

}


/*
function dispSerialList(arrSerial, sType)
{
	var buffer = new StringBuffer();
	var nUserfulIndex = 0;
	var sBrandName="";
	var classbtitle="";
	buffer.append('<table width="96%" border="0" cellpadding="0" cellspacing="0" id="right_picslist">');
	arrSerial.each(function(item, index){
		if(sBrandName == "") 
		{
			sBrandName = item.BName;
		}
		
		if(classbtitle == "")
		{
		    classbtitle = item.AName;
		}
		
		
		if(sType == "search" || item.PicNum != "0") 
		{
			if(nUserfulIndex % 5 == 0) 
			{
				buffer.append('<tr align="center">');
			}
			buffer.append('<td valign="top">');
			buffer.append('<a href="/car_public/1/piclib_c_disp.shtml?cid=' + item.CID + '" >');
			buffer.append('<img src="' + item.PicUrl + '" width="120" height="90" onError="this.src=\'http://mat1.qq.com/auto/datalib/new_lib/none.gif\'" title="' + item.CName + '"/>');
			buffer.append('</a><br />');
			
/--*
			if(sType == "search") {
				buffer.append('<a href="/car_public/1/piclib_frame_b.shtml?bid=' + item.BrandID + '" >');
				buffer.append(item.Brand);
				buffer.append('</a><br />');
			}
*--/
			
			buffer.append('<a href="/car_public/1/piclib_c_disp.shtml?cid=' + item.CID + '" >');
			buffer.append(item.CName + "(" + (item.PicNum=="0" ? "暂无" :item.PicNum) + ")");
			buffer.append('</a>');
			
/--*
			if(sType == "brand") {
				buffer.append('<br /><a href="/car_serial/' + item.ID + '/index.shtml" target="_blank" class="blue">');
				buffer.append('详细参数');
				buffer.append('</a>');
				buffer.append(' <a href="/car_public/1/bar.shtml?sid=' + item.ID + '&bid=' + item.BarID  + '" target="_blank" class="blue">');
				buffer.append("论坛");
				buffer.append('</a>');
			}
*--/

			buffer.append('</td>');					
			if(nUserfulIndex % 5 == 4) 
			{
				buffer.append('</tr>');
			}
			nUserfulIndex++;					
		}
	});
	
	for(;nUserfulIndex % 5 != 0; nUserfulIndex++) 
	{
		    buffer.append('<td width="20%">&nbsp;</td>');
		    if(nUserfulIndex % 5 == 4) 
		    {
			    buffer.append('</tr>');
	        }
	}
	
	buffer.append('</table>');
	$("classtitle").innerHTML = classbtitle;
	
	if(sType == "brand") 
	{
		$("disp_top_links").innerHTML = '<a href="/car_brand/index.shtml" target="_parent">汽车库</a> &gt; <a href="/car_public/1/piclib_frame_main.shtml">汽车图库</a> &gt; ' + sBrandName;
	}
	
	if(nUserfulIndex > 0) 
	{
		$("disp_serial_list").innerHTML = buffer.toString();	
	} 
	else 
	{
		$("disp_serial_list").innerHTML = "暂无记录";
	}	
	
}

*/

/*
var BRAND_ID="";
function initBPicLib() {
	var sUrl = window.location.href;
	var oUrlParas = sUrl.toQueryParams();
	if(typeof oUrlParas.bid == "undefined" || oUrlParas.bid == "")
	{
		alert("传入参数错误");
		return;
	}
	BRAND_ID = oUrlParas.bid;
	JsLoader.load("/autopics/" + BRAND_ID + "/c_class_data.js", function()
	{
		if(typeof oClassAData != "undefined" && oClassAData.arrClassA.length != 0) {
			dispSerialList(oClassAData.arrClassA, "brand");
		}
	});
}
*/


/*
function dispPicClassList()
{
	var nCSWGNum = 0;
	var nCXZYNum = 0;
	var nZKFXPNum = 0;
	var nQTGZNum = 0;
	var nUploadNum = 0;
	var nLoadedNum = 0;
	//显示分类图片
	this.dispClassPic = function(sTypeID, sTypeTitle, arrPic) {
		var buffer = new StringBuffer();
		buffer.append('<table width="100%" border="0" cellpadding="0" cellspacing="0">');
		buffer.append('<tr>');
		buffer.append('<td>');
		buffer.append('<table width="100%" border="0" cellpadding="0" cellspacing="0" background="http://mat1.qq.com/auto/datalib/new_lib/bg_d12.gif">');
		buffer.append('<tr>');
		buffer.append('<td width="20" height="28" background="http://mat1.qq.com/auto/datalib/new_lib/bg_d11.gif">&nbsp;</td>');
		buffer.append('<td align="left" valign="bottom" class="fontbold fontl26">');
		buffer.append('<a href="/car_public/1/disp_pic.shtml?sid='+g_objSerial.nSerialID+'&tid=' + sTypeID + '" target="_blank">');
		buffer.append(sTypeTitle + '('+arrPic.length+')');
		buffer.append('</a></td>');
		buffer.append('<td width="6" background="http://mat1.qq.com/auto/datalib/new_lib/bg_d13.gif">&nbsp;</td>');
		buffer.append('</tr>');
		buffer.append('</table></td>');
		buffer.append('</tr>');
		buffer.append('<tr>');
		buffer.append('<td align="center"><table width="96%" border="0" cellpadding="0" cellspacing="0" id="right_picslist">');
		var nUserfulIndex=0;
		arrPic.any(function(item, index){
			if(nUserfulIndex % 5 == 0) {
				buffer.append('<tr align="center">');
			}				
			buffer.append('<td valign="top">');
			buffer.append('<a href="/car_public/1/disp_pic.shtml?sid=' + g_objSerial.nSerialID + '&tid=' +sTypeID + '&pid=' + item.nID + '" target="_blank">');
			buffer.append('<img src="http://mat1.qq.com/datalib_img/car_serial' + item.sZoomImgUrl + '" width="120" height="90" onError="this.src=\'http://mat1.qq.com/auto/datalib/new_lib/none.gif\'" title="' + item.sDesc + '"/>');
			buffer.append('</a><br />');
			buffer.append('<div style="width:120px;overflow:hidden;text-overflow:ellipsis;white-space:nowrap;">');												  
			buffer.append('<a href="/car_public/1/disp_pic.shtml?sid=' + g_objSerial.nSerialID + '&tid=' +sTypeID + '&pid=' + item.nID + '" target="_blank" title="' + item.sDesc + '">');
			buffer.append(item.sDesc);
			buffer.append('</a>');
			buffer.append('</div>');
			buffer.append('</td>');
			if(nUserfulIndex % 5 == 4) {
				buffer.append('</tr>');
			}
			nUserfulIndex++;
			if(nUserfulIndex == 10) {
				return true;
			}
		});
		for(;nUserfulIndex % 5 != 0; nUserfulIndex++) {
			buffer.append('<td width="20%">&nbsp;</td>');
			if(nUserfulIndex % 5 == 4) {
				buffer.append('</tr>');
			}
		}
		buffer.append('</table>');
		buffer.append('</td>');
		buffer.append('</tr>');
		buffer.append('</table>');		
		if(nUserfulIndex > 0) {		
			$("disp_"+sTypeID).innerHTML = buffer.toString();
		}
	};
	//显示上传图片
	this.dispUploadPic = function(arrPic) {
		var buffer = new StringBuffer();
		buffer.append('<table width="100%" border="0" cellpadding="0" cellspacing="0">');
		buffer.append('<tr>');
		buffer.append('<td>');
		buffer.append('<table width="100%" border="0" cellpadding="0" cellspacing="0" background="http://mat1.qq.com/auto/datalib/new_lib/bg_d12.gif">');
		buffer.append('<tr>');
		buffer.append('<td width="20" height="28" background="http://mat1.qq.com/auto/datalib/new_lib/bg_d11.gif">&nbsp;</td>');
		buffer.append('<td align="left" valign="bottom" class="fontbold fontl26">最新网友上传</td>');
		buffer.append('<td width="6" background="http://mat1.qq.com/auto/datalib/new_lib/bg_d13.gif">&nbsp;</td>');
		buffer.append('</tr>');
		buffer.append('</table></td>');
		buffer.append('</tr>');
		buffer.append('<tr>');
		buffer.append('<td align="center"><table width="96%" border="0" cellpadding="0" cellspacing="0" id="right_picslist">');
		var nUserfulIndex=0;
		arrPic.any(function(item, index){
			if(item.sKeyword.indexOf("http") != -1) {
				if(nUserfulIndex % 5 == 0) {
					buffer.append('<tr align="center">');
				}				
				buffer.append('<td valign="top">');
				buffer.append('<a href="' + item.sUrl + '" target="_blank">');
				buffer.append('<img src="' + item.sKeyword + '" width="120" height="90" onError="this.src=\'http://mat1.qq.com/auto/datalib/new_lib/none.gif\'" title="' + item.sTitle + '"/>');
				buffer.append('</a><br />');
				buffer.append('<div style="width:120px;overflow:hidden;text-overflow:ellipsis;white-space:nowrap;">');												  
				buffer.append('<a href="' + item.sUrl + '" target="_blank" title="' + item.sTitle + '">');
				buffer.append(item.sTitle);
				buffer.append('</a>');
				buffer.append('</div>');
				buffer.append('</td>');
				if(nUserfulIndex % 5 == 4) {
					buffer.append('</tr>');
				}
				nUserfulIndex++;
				if(nUserfulIndex == 10) {
					return true;
				}
			}
		});
		for(;nUserfulIndex % 5 != 0; nUserfulIndex++) {
			buffer.append('<td width="20%">&nbsp;</td>');
			if(nUserfulIndex % 5 == 4) {
				buffer.append('</tr>');
			}
		}
		buffer.append('</table>');
		buffer.append('</td>');
		buffer.append('</tr>');
		buffer.append('</table>');			
		if(nUserfulIndex > 0) {
			$("disp_upload").innerHTML = buffer.toString();
		}
	};
	//显示图库导航
	this.dispTopLinks = function() {
		$("disp_top_links").innerHTML = '<a href="/car_brand/index.shtml" target="_parent">汽车库</a> &gt; '
									+ '<a href="/car_public/1/piclib_frame_main.shtml">汽车图库</a> &gt; '
									+ '<a href="/car_public/1/piclib_frame_b.shtml?bid=' + g_objSerial.nBrandID + '">' + g_objSerial.sBrand + '</a> &gt; '
									+ g_objSerial.sSerial + (g_objSerial.sArea == '进口' ? '(' + g_objSerial.sArea + ')' : '');
	};
	//显示图片分类导航
	this.dispClassLinks = function() {
		var buffer = new StringBuffer();
		buffer.append('<span class="fontbold">图片分类：</span>');
		buffer.append('<span class="fontblue1">');
		buffer.append('<a target="_blank" href="/car_public/1/disp_pic.shtml?sid='+g_objSerial.nSerialID+'&tid=37">车身外观('+nCSWGNum+'张)</a>');
		buffer.append(' | <a target="_blank" href="/car_public/1/disp_pic.shtml?sid='+g_objSerial.nSerialID+'&tid=38">车厢座椅('+nCXZYNum+'张)</a>');
		buffer.append(' | <a target="_blank" href="/car_public/1/disp_pic.shtml?sid='+g_objSerial.nSerialID+'&tid=39">中控方向盘('+nZKFXPNum+'张)</a>');
		buffer.append(' | <a target="_blank" href="/car_public/1/disp_pic.shtml?sid='+g_objSerial.nSerialID+'&tid=40">改装与其他('+nQTGZNum+'张)</a>');		
		buffer.append(' <a target="_blank" href="/car_public/1/disp_pic.shtml?sid='+g_objSerial.nSerialID+'&tid=37">更多图片&gt;&gt;</a>');		
		buffer.append('</span>');
		$("disp_class_links").innerHTML = buffer.toString();
	};
	
	//加载车系信息
	new Ajax.Request("/car_serial/"+g_objSerial.nSerialID+"/info.xml", {
		method: 'get',
		onSuccess: function(transport) {			
			var oXmlDom = transport.responseXML;
			g_objSerial.nBrandID = getNodeValue(oXmlDom.selectSingleNode("//LibData/FBrandID"));
			g_objSerial.sBrand = getNodeValue(oXmlDom.selectSingleNode("//LibData/FBrand"));
			g_objSerial.sSerial = getNodeValue(oXmlDom.selectSingleNode("//LibData/FName"));
			g_objSerial.sArea = getNodeValue(oXmlDom.selectSingleNode("//LibData/FArea"));
			this.dispTopLinks();
		}
	});
	
	//车身外观
	JsLoader.load("/car_serial/" + g_objSerial.nSerialID + "/cswg_piclib_josn.js", function() {
		nCSWGNum = oCswgPiclib.arrPic.length;
		this.dispClassPic("37", "车身外观", oCswgPiclib.arrPic);
		if(++nLoadedNum >= 4) {
			this.dispClassLinks();
		}
	});	
	//车厢座椅
	JsLoader.load("/car_serial/" + g_objSerial.nSerialID + "/cxzy_piclib_josn.js", function() {
		nCXZYNum = oCxzyPiclib.arrPic.length;
		this.dispClassPic("38", "车厢座椅", oCxzyPiclib.arrPic);
		if(++nLoadedNum >= 4) {
			this.dispClassLinks();
		}
	});
	//中控方向盘
	JsLoader.load("/car_serial/" + g_objSerial.nSerialID + "/zkfxp_piclib_josn.js", function() {
		nZKFXPNum = oZkfxpPiclib.arrPic.length;
		this.dispClassPic("39", "中控方向盘", oZkfxpPiclib.arrPic);
		if(++nLoadedNum >= 4) {
			this.dispClassLinks();
		}
	});
	//其它与改装
	JsLoader.load("/car_serial/" + g_objSerial.nSerialID + "/qtgz_piclib_josn.js", function() {
		nQTGZNum = oQtgzPiclib.arrPic.length;
		this.dispClassPic("40", "其它与改装", oQtgzPiclib.arrPic);
		if(++nLoadedNum >= 4) {
			this.dispClassLinks();
		}
	});	
	//网友上传
	JsLoader.load("/car_serial/" + g_objSerial.nSerialID + "/upload_actp_json.js", function() {
		this.dispUploadPic(oUploadACTP.arrArticle);		
	});	
}

*/

var g_objSerial = {
	CID:"",
	CName:"",
	AID:"",
	AName:"",
	BID:"",
	BName:"",
	bLoaded:false
};

/*
function initSerialPicLib() {
	var sUrl = window.location.href;
	var oUrlParas = sUrl.toQueryParams();
	if(typeof oUrlParas.sid == "undefined" || oUrlParas.sid == "")
	{
		alert("传入参数错误");
		return;
	}
	g_objSerial.nSerialID = oUrlParas.sid;
	dispPicClassList();	
}
*/

function checkKeywordSearch() {
	var sKeyWord = document.frmSearch.keyword.value;
	if (sKeyWord == "" || sKeyWord == document.frmSearch.keyword.defaultValue) {
		alert("请输入车型关键字");
		return false;
	}	
	document.frmSearch.FilterValueOR.value = sKeyWord+"|"+sKeyWord;
	document.frmSearch.FilterAttrOR.value = "1819|1816";
	return true;
}

function SearchResult(sObjName, nTotalNum, nNumPerPage)
{
	Page.call(this, sObjName, nTotalNum, nNumPerPage);	
}

SearchResult.prototype = new Page();

SearchResult.prototype.goPage = function(nGotoPage) {
	if(checkKeywordSearch()){
		document.frmSearch.CurPage.value = nGotoPage;
		document.frmSearch.submit();
	}
}


/*
function initSearchPicLib() {
	if(document.frmSearch.FilterValueOR.value != "") {
		document.frmSearch.keyword.value = document.frmSearch.FilterValueOR.value.split("|")[0];
	}
	dispSerialList(oSearchPiclib.arrPic.slice(0, oSearchPiclib.arrPic.length-1), "search");	
}
*/

var LOADED=false;
function initRightFrame(frameRight) {
	if(!LOADED) {
		var sUrl = window.location.href;
		var oUrlParas = sUrl.toQueryParams();
		if(typeof oUrlParas.bid != "undefined" && oUrlParas.bid != "")
		{
			frameRight.src = "piclib_frame_b.shtml?bid=" + oUrlParas.bid ;
		}
		else if(typeof oUrlParas.sid != "undefined" && oUrlParas.sid != "")
		{
			frameRight.src = "piclib_frame_s.shtml?sid=" + oUrlParas.sid ;
		}
		else {
			frameRight.src = "piclib_frame_main.shtml";
		}	
		LOADED = true;
	}
}

















