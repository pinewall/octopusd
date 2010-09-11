/*=============================================================================
��������JTree��
���ʱ�䣺2005/12/5
���ߣ�xlinFairy
E-mail��1fairy1@163.com
�뱣��������Ϣ
=============================================================================*/
function treeNode(){
	var self		=this;
	this.obj		=null;	//ָcaption���ڵı�ǩ��<span>����</span>
	this.caption	=null;	//��ʾ������
	this.level		=null;	//�ڵ�Ĳ��
	this.value		=null;	//���ֵ��ʱû���õ���Ԥ�е������ã���Ϊ��Delphi����ʱ������Ϊȱ����صĶ��������ò��������İ취��ȡ��
	//----------------------------------
	this.treeNodes		=new Array();	//��������
	
	this.parentTreeNode	=null;			//��ǰ����֦������֦��������Ҷ����֦�Ĺ�ϵһ����
	this.expand=function(pFlag){		//�������֦����������չ����Ҫ�ض�λһ��Ҫչ���������Ķ���
		try{
			self.obj.parentNode.expand(pFlag);//pFlagֻ��Ϊfalse��true
		}catch(e){};
	}
	this.click=function(){
		self.obj.onclick();
	}
}

function JTree(pParent,xmlFile){
	this.PICPATH	=	"/images/tree"	//ͼƬ�ļ����ڵ��ļ��У��ɼ�public���ɸı䡣
	
	var self		=this;	//�൱��һ�����ã�ָ���Լ���JTree.
	//-----------------------------------------------------------------------------
	//���ɼ�private��
	//����
	var JOIN		=	this.PICPATH +	"join.gif";
	var JOINBOTTOM	=	this.PICPATH +	"joinbottom.gif";
	var MINUS		=	this.PICPATH +	"minus.gif";
	var MINUSBOTTOM	=	this.PICPATH +	"minusbottom.gif";
	var PLUS		=	this.PICPATH +	"plus.gif";
	var PLUSBOTTOM	=	this.PICPATH +	"plusbottom.gif";
	var EMPTY		=	this.PICPATH +	"empty.gif";
	var LINE		=	this.PICPATH +	"line.gif";
	
	var LEAFICON	=	this.PICPATH +	"page.gif";
	var NODEICON	=	this.PICPATH + 	"folder.gif";
	
	var OPEN		=new Array();
		OPEN[true]	=MINUS;
		OPEN[false]	=PLUS;
	
	var OPENBOTTOM			=new Array();
		OPENBOTTOM[true]	=MINUSBOTTOM;
		OPENBOTTOM[false]	=PLUSBOTTOM;
	
	this.setPicPath=function(pPath){
		self.PICPATH=pPath;
		
		JOIN		=	self.PICPATH +	"join.gif";
		JOINBOTTOM	=	self.PICPATH +	"joinbottom.gif";
		MINUS		=	self.PICPATH +	"minus.gif";
		MINUSBOTTOM	=	self.PICPATH +	"minusbottom.gif";
		PLUS		=	self.PICPATH +	"plus.gif";
		PLUSBOTTOM	=	self.PICPATH +	"plusbottom.gif";
		EMPTY		=	self.PICPATH +	"empty.gif";
		LINE		=	self.PICPATH +	"line.gif";
			
		OPEN[true]	=MINUS;
		OPEN[false]	=PLUS;
		
		OPENBOTTOM[true]	=MINUSBOTTOM;
		OPENBOTTOM[false]	=PLUSBOTTOM;
		
		LEAFICON	=	self.PICPATH +	"page.gif";
		NODEICON	=	self.PICPATH + 	"folder.gif";
	}
	
	this.CAPTIONATT	=	"caption";//������������һ������
	this.ICONATT	=	"icon";//ͼ������
	this.EXPANDALL	=	true;//�Ƿ�ȫ����չ��
	
	this.clickItem	=new treeNode;//���ڵ��ʱ������ֵ��
	this.selectNode	=null;//ͬ��
	//----------------------------------------------------
	this.treeNodes	=new Array();//���ڵ�ļ��ϡ�
	this.treeNodes.push(null);
	this.root=this.treeNodes[0]=new treeNode;//���ĸ�
	
	
	this.onclick=null;
	this.onmouseover=null;
	this.onmouseout=null;
	//-----------------------------------------------------------------------------
	//����ID�õ�OBJ
	var $=function(pObjID){
		return document.getElementById(pObjID)
	}			
	//-----------------------------------------------------------------------------
	this.body=$(pParent) || document.body;
	//-----------------------------------------------------------------------------
	//��XML����
	var createXMLDom=function(){
		if (window.ActiveXObject) 
			var xmldoc=new ActiveXObject("Microsoft.XMLDOM");
		else 
			if (document.implementation&&document.implementation.createDocument)
				var xmldoc=document.implementation.createDocument("","doc",null);
		xmldoc.async = false;
		//Ϊ�˺�FireFoxһ�������ﲻ�ܸ�ΪFalse;
		xmldoc.preserveWhiteSpace=true;
		return xmldoc;
	}
	
	var createXMLHttp=function(){
		var xmlHttp;
		if (window.ActiveXObject){
			xmlHttp=new ActiveXObject("Microsoft.XMLHTTP");
		}else{
			xmlHttp=new XMLHttpRequest();
		}
		return xmlHttp;
	}
	
	
	//-----------------------------------------------------------------------------
	//����XML�ļ���
	var xmlDom=createXMLDom();
	try{
		xmlDom.load(xmlFile);
	}catch(e){
		var xmlHttp=createXMLHttp();
			xmlHttp.onreadystatechange = function(){
				if(xmlHttp.readyState == 4){
					xmlDom=xmlHttp.responseXML;
				}else{
					//window.state="XML�ļ�������...";
				}
			}		
			xmlHttp.open("GET",xmlFile,false);
			xmlHttp.send(null);
	}
	//ע��FF��֧��xml
	//alert(xmlDom.documentElement.childNodes[1].xml);
	var DOMRoot=xmlDom.documentElement;
	//for(o in DOMRoot){//ֻ������FireFox��opera�£���������IE�¡�
	//	document.write(o+" = " + DOMRoot.eval(o)+"<br>");
	//}
	//FF��IE��֧�������������ȡ���Ե����ݡ�
	//alert(root.attributes.getNamedItem("caption").nodeValue);
	//-----------------------------------------------------------------------------
	//ȡ��ָ���ڵ�����ԡ�
	var getDOMAtt=function(pNode,pAttribute){
		try{
			return pNode.attributes.getNamedItem(pAttribute).nodeValue;
		}catch(e){
			//alert("ָ���ڵ㲻���ڣ���ָ�����ԣ�"+pAttribute+" ������!")
			return false;
		}
		
	}
	//alert(getAttribute(root,"caption"));
	//-----------------------------------------------------------------------------
	//�½�HTML��ǩ��
	var createTag=function(pTagName){
		return document.createElement(pTagName)
	}
	var createImg=function(pSrc){
		var tmp=createTag("IMG");
		tmp.align	="absmiddle";
		tmp.src		=pSrc;
		tmp.onerror=function(){
			try{this.parentNode.removeChild(this);}catch(e){}
		}
		return tmp;
	}


	var createCaption=function(pNode,pLevel){
		var tmp=createTag("SPAN");
		tmp.innerHTML=getDOMAtt(pNode,self.CAPTIONATT);
		tmp.className="caption";
		tmp.onmouseover=function(){
			if(this.className!="captionHighLight")
				this.className="captionActive";
			try{self.onmouseover()}catch(e){}//�������
		}
		tmp.onmouseout=function(){
			if(this.className!="captionHighLight")
				this.className="caption";
			try{self.onmouseout()}catch(e){}//�������
		}
		tmp.onclick=function(){
			try{
				self.clickItem.obj.className="caption";
			}catch(e){
				//
			}
			this.className="captionHighLight";
			
			//alert(self);
			var clickItem=new treeNode;
			
			clickItem.obj		=tmp;
			clickItem.caption	=getDOMAtt(pNode,self.CAPTIONATT);
			clickItem.level		=pLevel
			
			try{
				clickItem.treeNodes		=this.parentNode.tree.treeNodes;
				clickItem.parentTreeNode=this.parentNode.tree.parentTreeNode;
			}catch(e){}
			
			self.clickItem=clickItem;
			self.selectNode=pNode;
			try{self.onclick();}catch(e){}//������ϣ����selfû�ж�onclick��ֵ�Ļ�������������
		}
		return tmp;
	}

	var createTreeLine=function(pNode,pParentArea){
		var hasChildren=pNode.hasChildNodes();//�Ƿ��к��ӡ�		
		for(var i=0;i<pParentArea.level;i++){
			var tmpArea=pParentArea;		
			for(var j=pParentArea.level;j>i;j--){
				//tmpArea=tmpArea.parentNode;
				tmpArea=tmpArea.parentNode.parentNode;
			}
			
			if(tmpArea.isLastChild)
				appendTo(createImg(EMPTY),pParentArea);
			else
				appendTo(createImg(LINE),pParentArea);
		}
				
		if(hasChildren){//�к���
			var childShowBtn;
			if(!pParentArea.isLastChild){	
				childShowBtn=createImg(OPEN[true]);
				appendTo(childShowBtn,pParentArea);
			}else{
				childShowBtn=createImg(OPENBOTTOM[true]);
				appendTo(childShowBtn,pParentArea);
			}
			childShowBtn.onclick=function(){
				var isExpand=this.parentNode.expand();
								
				if(!pParentArea.isLastChild){
					this.src=OPEN[isExpand];
				}else{
					this.src=OPENBOTTOM[isExpand];
				}
				
			}
			pParentArea.expandBtn=childShowBtn;//������
		}else{//�޺��ӡ�
			if(!pParentArea.isLastChild)	
				appendTo(createImg(JOIN),pParentArea);
			else
				appendTo(createImg(JOINBOTTOM),pParentArea);			
		}
	}
	
	var createIcon=function(pNode,pParentArea){
		var hasChildren=pNode.hasChildNodes();//�Ƿ��к���
		var tmpIcon=getDOMAtt(pNode,self.ICONATT);
		if(tmpIcon==false){
			if(hasChildren)
				appendTo(createImg(NODEICON),pParentArea);
			else
				appendTo(createImg(LEAFICON),pParentArea);
		}else{
			appendTo(createImg(tmpIcon),pParentArea);
		}		
	}
	//-----------------------------------------------------------------------------
	//��ָ��OBJ׷�ӵ�ĳ��OBJ������档
	var appendTo=function(pObj,pTargetObj){
		try{
			pTargetObj.appendChild(pObj);
		}catch(e){
			alert(e.message);
		}
	}
	//-----------------------------------------------------------------------------
	var isFirstChild=function(pNode){
		//���˿հ׽ڵ�֮�⣬�Ƿ��ǵ�һ���ڵ�
		var tmpNode=pNode.previousSibling;
		try{
			while(tmpNode.previousSibling!=null && tmpNode.nodeType!=1)
				tmpNode=tmpNode.previousSibling;
			if(tmpNode.nodeType==3)//�ǿսڵ�
				return true;
			else
				return false;
		}catch(e){
			return true;
		}
	}
	var isLastChild=function(pNode){
		var tmpNode=pNode.nextSibling;
		try{
			while(tmpNode.nextSibling!=null && tmpNode.nodeType!=1)
				tmpNode=tmpNode.nextSibling;
			if(tmpNode.nodeType==3)//�ǿսڵ�
				return true;
			else
				return false;
		}catch(e){
			return true;	
		}
	}
	//-----------------------------------------------------------------------------
	//ѭ�����Ƹ��ڵ㡣��������Щ����Щ�ڵ�����������ܣ����ԣ��������Щ��Ӧ�ñ�oRoot����������Ӧ����oOutLine�ĺ��ӡ�
	var createSubTree=function(pNode,pLevel,pNodeArea,pTreeNode){
		var subNode;
		for(var i=0;subNode=pNode.childNodes[i];i++){			
			if(subNode.nodeType!=1) continue;//����Ĭ���˰ѿհ�Ҳ����һ���ڵ����������ԣ�����Ҫ�ж�һ�¡�
			
			var subNodeItem		=createTag("DIV")
			
			if(subNode.hasChildNodes())
				var subNodeSubArea	=createTag("DIV");
				
				subNodeItem.level			=pLevel+1;
				subNodeItem.isFirstChild	=isFirstChild(subNode);
				subNodeItem.isLastChild		=isLastChild(subNode);
				//subNodeItem.parentTreeNode	=pTreeNode;//��������
				
			//��������λ�ò��ܱ䶯����ΪcreateTreeLine���õ�������parentNode
			appendTo(subNodeItem,pNodeArea);
			
			createTreeLine(subNode,subNodeItem);
			createIcon(subNode,subNodeItem);
			var subNodeCaption		=createCaption(subNode,pLevel+1);
				subNodeItem.caption	=subNodeCaption.innerHTML;
				
				subNodeItem.tree				=new treeNode();
				subNodeItem.tree.obj			=subNodeCaption;
				subNodeItem.tree.caption		=subNodeItem.caption;
				subNodeItem.tree.level			=subNodeItem.level;
				subNodeItem.tree.parentTreeNode	=pTreeNode;

			pTreeNode.treeNodes.push(subNodeItem.tree);
			
			appendTo(subNodeCaption,subNodeItem);
			
			if(subNode.hasChildNodes()){
				//createSubTree(subNode,pLevel+1,subNodeItem);
				appendTo(subNodeSubArea,subNodeItem);
				createSubTree(subNode,pLevel+1,subNodeSubArea,pTreeNode.treeNodes[pTreeNode.treeNodes.length-1]);
				subNodeItem.subNodeSubArea=subNodeSubArea;
				
				subNodeItem.expand=function(pFlag){
					//���״̬��չ���������棬���򷵻ؼ١�
					//this.subNodeSubArea.style.display=="" ? this.subNodeSubArea.style.display="none" : this.subNodeSubArea.style.display="";
					
					if(pFlag==null){
						if(this.subNodeSubArea.style.display==""){
							this.subNodeSubArea.style.display="none";
							return false;
						}else{
							this.subNodeSubArea.style.display="";
							return true;	
						}
					}else{
						//alert(this.expandBtn.tagName);
						if(pFlag)
							this.subNodeSubArea.style.display="";
						else this.subNodeSubArea.style.display="none";
												
						if(!this.isLastChild)
							this.expandBtn.src=OPEN[pFlag];
						else
							this.expandBtn.src=OPENBOTTOM[pFlag];					
						
					}
					
				};
			}
		}
	}
	
	
	this.expandByLevel=function(pLevel){
			
	}
	
	this.create=function(){
		//-----------------------------------------------------------------------------
		//��������
		var oOutLine=createTag("DIV");
		oOutLine.className="outLine";
		appendTo(oOutLine,this.body);
		//oOutLine.onclick=this.onclick;
		//-----------------------------------------------------------------------------
		//���Ƹ�����������߱������Ĺ��ܡ�
		var oRoot		=createTag("DIV");
			
			oRoot.level	=-1;//���𡣸��ļ���Ϊ-1;
	
		var oRootIcon	=createImg(getDOMAtt(DOMRoot,self.ICONATT));	
		//var oRootCaption=createCaption(getDOMAtt(DOMRoot,self.CAPTIONATT),-1);
		var oRootCaption	=createCaption(DOMRoot,-1);
			oRoot.caption	=oRootCaption.innerHTML;	
		
		//================================================
		//����
		//================================================
			oRoot.tree					=new treeNode();
			oRoot.tree.obj				=oRootCaption;
			oRoot.tree.caption			=oRoot.caption;
			oRoot.tree.level			=oRoot.level;
			oRoot.tree.parentTreeNode	=self.treeNodes[0];
		
		self.root=self.treeNodes[0]		=oRoot.tree;
		
		appendTo(oRootIcon,oRoot);
		appendTo(oRootCaption,oRoot);
		appendTo(oRoot,oOutLine);
		//------------------------------------------------------------------------------		
		createSubTree(DOMRoot,-1,oOutLine,self.treeNodes[0]);
	}
}

//�ṩ2���Ƚ�ͨ�õĺ�������ʹ��ҳ�����
//����ĳ���ĵ�ID��ֵ
function setValueById(pObjID,pValue){
	var obj=document.getElementById(pObjID);
	try{
		obj.value=pValue;
	}catch(e){
	}
}
//��ȡĳ���ڵ��ֵ
function getNodeAtt(pNode,pAtt){
	try{
		return pNode.attributes.getNamedItem(pAtt).nodeValue;
	}catch(e){
		return "��";
	}
}
