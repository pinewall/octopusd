/************************************************************
描述: cClientMsg类申明,用于PAS应用系统和服务器之间进行收发消息
作者: Hermanzhao
创建日期: 2006-11-13
修订日期:  次数     修改人      修改内容
            1       Hermanzhao  创建
************************************************************/
#ifndef _CLIENT_MSG_H_
#define _CLIENT_MSG_H_
#include <string>
#include <vector>
#include <stdio.h>
using namespace std;

const int nMsgHeadLen = 5;
const int nMaxMsgLen = 65535 << 2;
const char cMsgVersion = 0x01;

class cClientMsg
{
public:
        cClientMsg();
        ~cClientMsg();
        cClientMsg(const cClientMsg& ClientMsg);
        cClientMsg& operator = (const cClientMsg& ClientMsg);

        int encode();
        int decode();
        int decode(char* pszMessage, int nLen);

        char* getMessage()
        {
                return m_pszMessage;
        }

        void putValue(const string& szContent)
        {
                if (m_vecContent.size() == 0)
                {                
                        m_vecContent.push_back("1"); //MsgVersion
                }
                
                m_vecContent.push_back(szContent);                
        }
        void putValue(int iContent)
        {
                if (m_vecContent.size() == 0)
                {                
                        m_vecContent.push_back("1"); //MsgVersion
                }
                char szContent[16];
                memset(szContent, 0, 16);
                snprintf(szContent, 15, "%d", iContent);                
                m_vecContent.push_back(szContent);
        }
        void putValue(unsigned int iContent)
        {
                if (m_vecContent.size() == 0)
                {                
                        m_vecContent.push_back("1"); //MsgVersion
                }
                char szContent[16];
                memset(szContent, 0, 16);
                snprintf(szContent, 15, "%ud", iContent);                
                m_vecContent.push_back(szContent);
        }
        string getValue(int iType)
        {
                if (iType >= (int)m_vecContent.size())
                {
                        return "";
                }
                return m_vecContent[iType];
        }

        void clearContent()
        {
                m_vecContent.clear();
        }

        int getContentSize()
        {
                return m_vecContent.size() - 1;
        }

        //消息的拷贝和黏贴系列
        //三个参数pCur 当前缓冲区的位置   value 需要加入的值  offset加入的偏移地址
        static unsigned int msg_putChar(char* pCur, char value, int offset);                
        static unsigned int msg_putInt(char* pCur, int value, int offset);
        static unsigned int msg_putString(char* pCur, const char* value, int iValueLen, int offset);

        static  unsigned int msg_getChar(const char* pCur, int offset, char& value);                
        static  unsigned int msg_getInt(const char* pCur, int offset, int& value);
        static  unsigned int msg_getString(const char* pCur, int offset, string& value, int iValueLen);

public:
        int m_iTotalLen;                        //消息总长度        
        char* m_pszMessage;                     //消息指针
        vector<string> m_vecContent;            //保存消息内容的vector
};
#endif
