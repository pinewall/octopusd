/************************************************************
����: cclientMsg���ʵ��
����: Hermanzhao
��������: 2006-11-13
�޶�����:  ����     �޸���      �޸�����
            1       Hermanzhao  ����
************************************************************/
#include "ClientMsg.h"
#include <netinet/in.h>
#include "Trace.h"

cClientMsg::cClientMsg()
{
        m_iTotalLen             =       0;                //��Ϣ�ܳ���        
        m_pszMessage    =       NULL;                     //��Ϣָ��        
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
        // �ȼ���һ����Ҫ���Ŀռ�
        // run()����������ɺ�,�Ѿ�����˿ռ����ݵ�������
        int iContentLen = 0;	
	
        unsigned int iLoop = 1;        

        for(iLoop = 1; iLoop < m_vecContent.size(); iLoop++)
        {	
                iContentLen += m_vecContent[iLoop].size();
        }
       	        
        // head + TLV(total -1 = getContentSize()) * num + contentlen
        m_iTotalLen = nMsgHeadLen + getContentSize() * (sizeof(char) + sizeof(int))  + iContentLen;
        
        //����ռ�
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

        //�����ݴ��
        int iCurLen = 0;

        //����汾��Ϣ
        msg_putChar(m_pszMessage, cMsgVersion, iCurLen);
        iCurLen += sizeof(char);        

        //����ȫ��
        msg_putInt(m_pszMessage, m_iTotalLen, iCurLen);
        iCurLen += sizeof(int);
                
        //����vector����vector�е�ֵ���뵽�ַ�����
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
         //����
        int iRet = 0;

        //������Ϣ������
        m_vecContent.clear();
        m_vecContent.push_back("1");  // msgversion
        
        int iCurLen = 0;
        //cType ���������ź�
        while(iCurLen < (m_iTotalLen - nMsgHeadLen))
        {
                //Type
                char cType;
                msg_getChar(m_pszMessage, iCurLen, cType);		
                iCurLen += sizeof(char);
                //������մ�����˴���
                if ((unsigned int)cType != m_vecContent.size())  // ���յ�ʱ��,���Ӧ���ǵ��ڵ�ʱ�ĳ��ȵ�
                {
                        MAGIC_LOG(("TLV's type is %d != %d, invalid, break decode.", cType, m_vecContent.size()));
                        iRet = -1;
                        break;
                }

                //Length
                int iLength;
                msg_getInt(m_pszMessage, iCurLen, iLength);
                iCurLen += sizeof(int);
		
                //����ֵ���Ϸ�
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
		
                //�洢����Ϣ������
                m_vecContent .push_back(sValue);

                MAGIC_LOG(("decode: Type=%d ,Value = %s",  cType, sValue.c_str()));
        }
        
        //ɾ������Ϣ�Ŀռ�
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
                value = "";  //��
                return 0;  //����Ϊ0
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
        delete[] pValue;  //ɾ������Ŀռ�
        return iValueLen;
}
