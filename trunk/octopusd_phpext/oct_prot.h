#ifndef _OCT_PROT_H_
#define _OCT_PROT_H_

#define MsgBufferSize 1024 * 10   //10K
#define WriteToFileSize 1024 * 1000   //500K 开始写一次文件,大于MsgBufferSize
#define LenOfSiteName 32   //站点名称长度
#define LenOfMsgTime 20   //时间格式长度
#define LenOfPathFileName 256  //路径和文件名称的长度
#define LenOfError 500  //错误信息的长度

//上传文件的类型
#define FileName  0x1 //文件名称
#define FileData 0x2  //文件流
//支持的消息类别
#define ProtocolUploadFile 0x01 //上传文件
#define ProtocolDelFile  0x02  //删除文件
#define ProtocolMvFile 0x15  //移动
#define ProtocolCpFile 0x16   //拷贝

#define ProtocolPackFile 0x17   //打包上传 2008-07-22新增

//文件目录 2008-07-22新增
#define FILE_ERROR  0     //错误
#define FILE_DIR    1       //目录
#define FILE_TARGZ  2     // *.tar.gz
#define FILE_ZIP    3       // *.zip
#define FILE_NORMAL 4    //普通文件

//消息版本
#define Version1 0x01

//消息类型
static int eProtocolType;
//文件类型
static int eFileType;
//站点名
static char sSiteName[LenOfSiteName];
//远端目录文件和名称
static char sRemotePathName[LenOfPathFileName];
//本地目录和名称
static char sLocalPathName[LenOfPathFileName]; 
//发送时间
static char sSendTime[LenOfMsgTime];
//错误原因
static char lastError[LenOfError];
static char s[50];

static struct in_addr server_addr_;
static short server_port_;

#endif
