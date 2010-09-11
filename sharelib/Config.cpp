//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: Config.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#include "Config.h"

CConfig::CConfig()
{
}

CConfig::~CConfig()
{
}

CConfig::CConfig(string sPathName)
{
        m_sPathName = sPathName;
}

int CConfig::Init(void)
{
        FILE*fp = fopen(m_sPathName.c_str(), "r");
        if (NULL == fp)
        {
                return -1;
        }
        char sTmpStr[1024 + 1];
        char sTmpStr1[1024 + 1];
        char sTmpStr2[1024 + 1];
        char *pNext;
        string sCurSecName = "";
        int iLine = 0;
        ConfigType insConfigType;

        //��ȡ
        while(!feof(fp))
        {                
                if (NULL == fgets(sTmpStr, 1024, fp))
                {
                        break;
                }
                iLine++; //������1

                //���˵������ַ�
                CComFun::trimString(sTmpStr, sTmpStr, 0); //�ո�
                if (0 == strlen(sTmpStr))
                {
                        fclose(fp);
                        return iLine;
                }
                while ('\n' == sTmpStr[strlen(sTmpStr) -1] || '\r' == sTmpStr[strlen(sTmpStr) -1] )
                {
                        sTmpStr[strlen(sTmpStr) -1] = '\0';  // ȥ��ĩβ�Ļس�����
                        if (0 == strlen(sTmpStr))
                                break;
                }                
                if (strlen(sTmpStr) == 0)
                {
                        continue;
                }
                
                //ע��
                if ( '#' == sTmpStr[0] || ';' == sTmpStr[0])
                {
                        continue;
                }

                //����
                if ( '[' == sTmpStr[0] && ']' == sTmpStr[strlen(sTmpStr) -1])
                {
                        sTmpStr[strlen(sTmpStr) -1] = '\0';
                        strcpy(sTmpStr1, sTmpStr+1);
                        if ( 0 == strlen(sTmpStr1))
                        {
                                fclose(fp);
                                return iLine;
                        }
                        sCurSecName = string(sTmpStr1);                        
                }
                else
                {
                        //��û���κ�����,����������
                        if ("" == sCurSecName)
                        {
                                fclose(fp);
                                return iLine;
                        }
                        //��ȡ����ֵ
                        if (0 != CComFun::getFlagStr(sTmpStr1, sTmpStr, &pNext, '='))
                        {
                                fclose(fp);
                                return iLine;
                        }
                        CComFun::trimString(sTmpStr1, sTmpStr1, 0);  // ����VarName
                        if (0 == strlen(sTmpStr1))
                        {
                                fclose(fp);
                                return iLine;
                        }                        
                        strcpy(sTmpStr2, pNext);
                        CComFun::trimString(sTmpStr2, sTmpStr2, 0);  // ����value

                        //��Ӽ�¼
                        m_mapSection[sCurSecName][sTmpStr1] = sTmpStr2;                         
                }
        }

        fclose(fp);
        return 0;  
}

 int CConfig::ReadItem(const char* section, const char*  key,
       const char* defaultvalue, char* itemvalue)
{
        map<string, ConfigType>::iterator it = m_mapSection.find(section);
        if( it != m_mapSection.end())
        {
                ConfigType::iterator itx = it->second.find(key);
                if (itx != it->second.end())
                {
                        strcpy(itemvalue, itx->second.c_str());
                        return 0;
                }
                else
                {
                       strcpy(itemvalue, defaultvalue);
                        return -1;
                }
        }
        else
        {
                strcpy(itemvalue, defaultvalue);
                return -1;
        }
}


