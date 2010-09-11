/************************************************************
描述: cclientMsg类的实现
作者: Hermanzhao
创建日期: 2006-11-13
修订日期:  次数     修改人      修改内容
            1       Hermanzhao  创建
************************************************************/
#include "ClientMsg.h"
#include <netinet/in.h>
#include "Trace.h"

cClientMsg::cClientMsg()
{
        m_iTotalLen             =       0;                //消息总长度        
        m_pszMessage    =       NULL;                     //消息指针        
}

cClientMsg::~cClientMsg()
{
        if(m_pszMessage)
        {
                delete[] m_pszMessage;
        }
}

cClientMsg::cClientMsg(const cClientMsg& ClientMsg)
{
        m_iTotalLen = ClientMsg.m_iTotalLen;
        
        if(ClientMsg.m_pszMessage)
        {
                m_pszMessage = new char[m_iTotalLen];
                memset(m_pszMessage, 0, m_iTotalLen);
                memcpy(m_pszMessage, ClientMsg.m_pszMessage, m_iTotalLen);
        }

        vector<string>::const_iterator iterBegin = ClientMsg.m_vecContent.begin();
        vector<string>::const_iterator iterEnd = ClientMsg.m_vecContent.end();
        for(; iterBegin != iterEnd; ++iterBegin)
        {
                m_vecContent.push_back(*iterBegin);
        }
}

cClientMsg& cClientMsg::operator = (const cClientMsg& ClientMsg)
{
        if(this == &ClientMsg)
        {
                return *this;
        }

        if(m_pszMessage)
        {
                delete[] m_pszMessage;
                m_pszMessage = NULL;
        }
        m_vecContent.clear();

        m_iTotalLen = ClientMsg.m_iTotalLen;        
        if(ClientMsg.m_pszMessage)
        {
                m_pszMessage = new char[m_iTotalLen];
                memset(m_pszMessage, 0, m_iTotalLen);
                memcpy(m_pszMessage, ClientMsg.m_pszMessage, m_iTotalLen);
        }

        vector<string>::const_iterator iterBegin = ClientMsg.m_vecContent.begin();
        vector<string>::const_iterator iterEnd = ClientMsg.m_vecContent.end();
        for(; iterBegin != iterEnd; ++iterBegin)
        {
                m_vecContent.push_back(*iterBegin);
        }

        return *this;
}

int cClientMsg::encode()
{
        // 先计算一下需要多大的空间
        // run()函数运行完成后,已经完成了空间内容的设置了
        int iContentLen = 0;	
	
        unsigned int iLoop = 1;        

        for(iLoop = 1; iLoop < m_vecContent.size(); iLoop++)
        {	
                iContentLen += m_vecContent[iLoop].size();
        }
       	        
        // head + TLV(total -1 = getContentSize()) * num + contentlen
        m_iTotalLen = nMsgHeadLen + getContentSize() * (sizeof(char) + sizeof(int))  + iContentLen;
        
        //申请空间
        if(m_pszMessage)
        {
                delete[] m_pszMessage;
                m_pszMessage = NULL;
        }

        m_pszMessage = new char[m_iTotalLen];

        if (NULL == m_pszMessage)
        {
                MAGIC_LOG(("Malloc Memory fail when decode Message"));
                return -1;
        }

        //将内容打包
        int iCurLen = 0;

        //插入版本信息
        msg_putChar(m_pszMessage, cMsgVersion, iCurLen);
        iCurLen += sizeof(char);        

        //插入全长
        msg_putInt(m_pszMessage, m_iTotalLen, iCurLen);
        iCurLen += sizeof(int);
                
        //便利vector，将vector中的值加入到字符串中
        for(iLoop = 1; iLoop < m_vecContent.size(); iLoop++)
        {
                MAGIC_LOG(("encode : Type=%d , Value = %s", iLoop, m_vecContent[iLoop].c_str()));
                msg_putChar(m_pszMessage, iLoop, iCurLen);
                iCurLen += sizeof(char);                

                int iTmpLen = m_vecContent[iLoop].size();
                msg_putInt(m_pszMessage, iTmpLen, iCurLen);
                iCurLen += sizeof(int);                

                msg_putString(m_pszMessage, m_vecContent[iLoop].c_str(), iTmpLen, iCurLen);
                iCurLen += iTmpLen;                
        }	
        
        MAGIC_LOG(("encode Message Success!"));
        return 0;
}
        
int cClientMsg::decode(char* pszMessage, int nLen)
{
        if(!pszMessage)
        {
                return -1;
        }

        if(m_pszMessage)
        {
                delete[] m_pszMessage;
                m_pszMessage = NULL;
        }
        
        m_pszMessage = new char[nLen];
        memcpy(m_pszMessage, pszMessage, nLen);
        m_iTotalLen = nLen;
        return decode();
}

int cClientMsg::decode()
{	
         //返回
        int iRet = 0;

        //解析消息的内容
        m_vecContent.clear();
        m_vecContent.push_back("1");  // msgversion
        
        int iCurLen = 0;
        //cType 必须依次排号
        while(iCurLen < (m_iTotalLen - nMsgHeadLen))
        {
                //Type
                char cType;
                msg_getChar(m_pszMessage, iCurLen, cType);		
                iCurLen += sizeof(char);
                //如果接收次序出了错误
                if ((unsigned int)cType != m_vecContent.size())  // 接收的时候,序号应该是等于当时的长度的
                {
                        MAGIC_LOG(("TLV's type is %d != %d, invalid, break decode.", cType, m_vecContent.size()));
                        iRet = -1;
                        break;
                }

                //Length
                int iLength;
                msg_getInt(m_pszMessage, iCurLen, iLength);
                iCurLen += sizeof(int);
		
                //长度值不合法
                if (iLength < 0)
                {
                        MAGIC_LOG(("TLV's length is %d < 0, invalid, break decode.", iLength));
                        iRet = -1;
                        break;
                }

                //Value
                string sValue;
                msg_getString(m_pszMessage, iCurLen, sValue, iLength);
                iCurLen += iLength;
		
                //存储起消息内容来
                m_vecContent .push_back(sValue);

                MAGIC_LOG(("decode: Type=%d ,Value = %s",  cType, sValue.c_str()));
        }
        
        //删除掉消息的空间
        delete[] m_pszMessage;
        m_pszMessage = NULL;

        MAGIC_LOG(("decode Message Success"));
        return iRet;        
}

unsigned int cClientMsg::msg_putChar(char* pCur, char value, int offset)
{
        memcpy(pCur + offset, &value, sizeof(value));
        return sizeof(value);
}

unsigned int cClientMsg::msg_putInt(char* pCur, int value, int offset)
{
        int tmpValue = htonl(value);
        memcpy(pCur + offset, &tmpValue, sizeof(value));
        return sizeof(value);
}

unsigned int cClientMsg::msg_putString(char* pCur, const char* value, int iValueLen, int offset)
{
        if ( 0 == iValueLen)
        {
                return 0;
        }
        else if (0 > iValueLen)
        {
                return 0;
        }
        memcpy(pCur + offset, value, iValueLen);
        return iValueLen;
}

unsigned int cClientMsg::msg_getChar(const char* pCur, int offset, char& value)
{
        memcpy(&value, pCur + offset, sizeof(value));
        return sizeof(value);
}

unsigned int cClientMsg::msg_getInt(const char* pCur, int offset, int& value)
{
        memcpy(&value, pCur + offset, sizeof(value));
        value = ntohl(value);
        return sizeof(value);
}
 
unsigned int cClientMsg::msg_getString(const char* pCur, int offset, string& value, int iValueLen)
{
        if (0 == iValueLen)
        {
                value = "";  //空
                return 0;  //长度为0
        }
        else if (0 > iValueLen)
        {
                value = "";
                return 0;
        }

        char* pValue = new char[iValueLen + 1];
        memset(pValue, 0, iValueLen + 1);
        memcpy(pValue, pCur + offset, iValueLen);
        value = pValue;
        delete[] pValue;  //删除申请的空间
        return iValueLen;
}
