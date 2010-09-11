//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: Config.h,v $
//  *    $Revision: 1.0 $
//  *    $Author: kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#ifndef CONFIG_H_HEADER_INCLUDED_BCE1B2D4
#define CONFIG_H_HEADER_INCLUDED_BCE1B2D4

#include "ComFun.h"
typedef map<string, string>  ConfigType;

class CConfig
{
public:
        CConfig();
        ~CConfig();
        CConfig(string sPathName);

        /**
      * This function is used to load
      * config file to buf.
      * @return 0 on success.
      *         -1 on failure.
      */
        int Init();

        /**
       *Used to Read config Item.
       *@param section indicates config section.
       *@param key indicates config item key.
       *@param defaultvalue indicates value which are
       *      are supplied by user as default value, if
       *      fail to get value.
       *@param itemvalue  indicates value which get from 
       *       config file.
       *@return 0 on success. 
       *        -1 on failure.
       */
        int ReadItem(const char* section, const char*  key,
                        const char* defaultvalue, char* itemvalue);
private:       
        //config file name.
        std::string m_sPathName;  
  
        // config section defination.
        std::map<string, ConfigType> m_mapSection;  
};

#endif


