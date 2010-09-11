//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *
//  *    $RCSfile: ComCfg.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#ifndef COMCFG_H_HEADER_INCLUDED_BCE1EA59
#define COMCFG_H_HEADER_INCLUDED_BCE1EA59

#include "ComFun.h"

//¹«¹²µÄÅäÖÃÎÄ¼şĞÅÏ¢
class CComCfg
{
      public:
        CComCfg();
        CComCfg(const CComCfg& right);
        virtual ~CComCfg();
        CComCfg& operator=(const CComCfg& right);

        //ÔÊĞíÁ¬½ÓµÄIPÅäÖÃÁĞ±í£¬¶à¸öÒÔ | ¸ô¿ª
        char m_sLoginIP[1024];
        char m_sServerFlag[1024];

        //Æô¶¯Ê±ÈÕÖÁÄ£Ê½£¬0 µÍ¼¶±ğ£¬³£¹æÈÕÖ¾  1 ³öÏÖÒ»¶¨µÄÂß¼­Ö´ĞĞÈÕÖ¾
        // 2 °üº¬ÏêÏ¸µÄ²Ù×÷ÈÕÖÁ 3 °üº¬ÏûÏ¢ÀàÂëÁ÷
        int m_iLogMode;

        //½ÓÊÜµÄÎÄ¼şµÄÈ¨ÏŞ£¬Êı×ÖĞÎÊ½£¬Çë×ÔĞĞ±£Ö¤ÆäÕıÈ·ĞÔ£¬ÏµÍ³ÓÃchmodÖ´ĞĞ£¬±ÈÈçchmod 755 /a/xyz.exe
        //Îª¿ÕÄ¬ÈÏÈ¨ÏŞÎª666£¨-rw-rw-rw-)£¬Ö÷ÒªÓÃÀ´²¿ÊğCGIÓÃ£¬±£Ö¤Æä¾ßÓĞ¿É¶ÁĞÔÈ¨ÏŞ£¬´ó²¿·ÖÎÄ¼şÎŞĞèĞŞ¸Ä´ËÖ¸
        char m_sFileModeOption[16];

        //ÎÄ¼şÉÏ´©³É¹¦ºóÒª²»ÒªÉ¾³ıÎÄ¼ş, 0 ±íÊ¾²»ÒªÉ¾³ı, 1±íÊ¾ÒªÉ¾³ı
        //ÓÉÓÚÊµÏÖÆğÀ´±È½Ï¸´ÔÓ,ËùÒÔµ±Ç°¿ª¹ØÖ»ÄÜÔÚ·Ö·¢ÏÂÊô»úÆ÷Ö»ÓĞ1Ì¨»úÆ÷Ê±²ÅÆğ×÷ÓÃ
        int m_iDelFileAfterUpload;

        //ÎÄ¼şÎª0×Ö½ÚÊ±£¬ÉÏ´«Ê±»¹Òª²»Òª·Ö·¢£¬0²»ÒªÔÙ·Ö·¢ÁË£¬1Òª·Ö·¢
        int m_iIfUploadZeroFile;

        // ÔÊĞíÉÏ´«µÄ×î´óµÄÎÄ¼ş´óĞ¡£¬³¬¹ı´Ë´óĞ¡£¬²»Óè·Ö·¢£¬Ä¬ÈÏÎª0£¬²»ÏŞÖÆ,µ¥Î»ÎªKbyte£¬Èç10M,Ğ´³É10000
        int m_iMaxLimitOfUploadFile;

        //Ğ­ÒéÀàĞÍ 1±íÊ¾Ã¿¸öÏûÏ¢¶Î¶¼ÒªÈ·ÈÏÏûÏ¢ 0±íÊ¾Õû¸öÏûÏ¢Ò»È·ÈÏ
        int m_iProtocolType;

        //Ê§°ÜÊ±µÄÖØÊÔ´ÎÊı£¬0±íÊ¾ÎŞÏŞÖÆ
        int m_iMaxNrOfRetry;

        //´ïµ½ÖØÊÔ´ÎÊı£¬ÈÔÈ»Ê§°ÜµÄÈÎÎñ£¬ÊÇ·ñĞèÒªĞ´ÈÕÖ¾£¬0·ñ 1ÊÇ
        int m_iIfFailLogFlag;

        //¶ÓÁĞÈÎÎñÖĞ×î´óµÄÏûÏ¢¸öÊı£¬0±íÊ¾ÎŞÏŞÖÆ
        int m_iMaxNrOfQueueDuty;

        //±£´æ¶àÉÙ¸ö×î½ü·¢ËÍ³É¹¦µÄÈÎÎñ£¬0±íÊ¾²»¼ÇÂ¼ºÍ±£´æ×î½ü³É¹¦µÄÈÎÎñ
        int m_iMaxNrOfSaveLastSucDuty;

        //ÀıĞĞÈÎÎñ£ºÃ¿¸ô¶à¾Ã´òÓ¡Ò»´ÎÏß³Ì¹¤×÷µÄÍ³¼ÆĞÅÏ¢,µ¥Î»s, 0²»´òÓ¡
        int m_iTimeOfThreadStat;

        //ÀıĞĞÈÎÎñ£ºÃ¿¸ô¶à¾Ã¼ì²é²¢×Ô¶¯»Ö¸´Ê§°ÜµÄÈÎÎñ,µ¥Î»s£¬0²»Ö´ĞĞ
        int m_iTimeOfFailDutyRetry;

        //ÀıĞĞÈÎÎñ£ºÃ¿¸ô¶à¾Ã¼ì²é¼ì²éÒ»ÏÂÔËĞĞ×´¿öÒÔ¼°ÏÂÊô»úÆ÷µÄ×´¿ö, 0±íÊ¾²»¼ì²é
        int m_iTimeOfCheckRunStatus;

        //¶ÓÁĞÈÎÎñ£¬´æÔÚÁË¶àÉÙÈÎÎñÈÏÎªÓĞÒì³££¬¾Í»á±¨¾¯£¬±¨¾¯¼ä¸ôÒÀÀµTimeOfCheckRunStatus
        int m_iMaxNrOfDutyToAlarm;

        //·Ö·¢Ñ¡ÏîÅäÖÃ 0²»ÔÙÏòÏÂ·Ö·¢ÎÄ¼ş; 1·Ö·¢·şÎñÆ÷ĞÅÏ¢À´Ô´ÓÚÊı¾İ¿â; 2À´Ô´ÓÚÅäÖÃÎÄ¼ş
        int m_iDistributeFlag;

        //µ±DistributeFlag=1ÓĞĞ§£¬Êı¾İ¿â·şÎñµÄÅäÖÃ£¬Èç¹ûÃ»ÓĞÃÜÂë£¬Çë×¢Óµô
        char m_sDBHost[LenOfNormal];
        char m_sDBUser[LenOfNormal];
        char m_sDBPassword[LenOfNormal];

        //Ìá¹©·şÎñµØÖ·ºÍ¶Ë¿Ú
        char m_sServerIP[LenOfIPAddr];
        int m_iServerPort;

        //Ïß³ÌÊı¿ØÖÆ£¬µ¥Î»¸öÊı
        int m_iRecvNrOfThread;
        int m_iSendNrOfThread;
        int m_iSendErrorNrOfThread;

        //¿É½ÓÊÕµÄ×î´óÁ¬½ÓÊı£¬0±íÊ¾ÎŞÏŞÖÆ
        int m_iMaxNrOfCnt;

        //½ÓÊÕÏûÏ¢µÄ³¬Ê±Ê±¼ä£¬µ¥Î»Ãë
        int m_iRecvMsgTimeout;
        int m_iRecvFileTimeout;

        //Á÷Á¿¿ØÖÆ£¬±íÊ¾Ã¿ÃëÖÓÖ»ÄÜ·¢ËÍ¶àÉÙK×Ö½Ú£¬0±íÊ¾²»ÏŞÖÆ£¬×¢Òâµ¥Î»Îª KB,×îĞ¡Îª10K
        int m_iSendFlowCtrl;
};

#endif /* COMCFG_H_HEADER_INCLUDED_BCE1EA59 */

