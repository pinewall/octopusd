#ifndef _OCT_CLI_H_
#define _OCT_CLI_H_

/**
 *  \brief 设置要发送的服务器的IP和端口，主要由于服务端要鉴权，因此需要把本机IP在服务器端配置
 *  \param sIP 服务器端IP
 *  \param iPort 服务端端口
 *  \return void
 */
void oct_init_(const char* sIP, int iPort);

/**
 *  \brief 设置上传消息的内容
 *  \param cType 上传文件的类型
 *  \param sSiteName 站点名称
 *  \param sRemotePathName 文件在远端的相对主站目录的相对目录和文件名称
 *  \param sLocalPathName 文件在本地的绝对目录和名称
 *  \return void
 */
void oct_set_upload_(int cType, const char* siteName, const char* remotePathName, const char* localPathName);

/**
 *  \brief 设置打包上传消息的内容
 *  \param cType 上传文件的类型
 *  \param sSiteName 站点名称
 *  \param sRemotePath 文件在远端的相对主站目录的相对目录
 *  \param sLocalPathName 文件在本地的绝对目录和名称
 *  \return -100表示失败,0成功
 */
int oct_set_upload_pack_(int cType, const char* siteName, const char* remotePathName, const char* localPathName);

/**
 *  \brief 设置上传消息的内容
 *  \param sSiteName 站点名称
 *  \param sRemotePathName 文件在远端的相对主站目录的相对目录和文件名称
 *  \return void
 */
void oct_set_delete_(const char* siteName, const char* remotePathName);

/**
 *  \brief 移动和拷贝文件
 *  \param sdestSiteName 目的站点名称
 *  \param sDestPathName 目的目录文件,相对于站点的主目录
 *  \param sSrcSiteName 源站点名称
 *  \param sSrcPathName 源目录文件,相对于站点的主目录
 *  \return void
 */
void oct_set_mv_(const char* destSiteName, const char* destPathName, const char* srcSiteName, const char* srcPathName);
void oct_set_cp_(const char* destSiteName, const char* destPathName, const char* srcSiteName, const char* srcPathName);

/**
 *  \brief 前面设置成功后，发送消息
 *  \return 成功相应码，0成功，负数失败
 */
int oct_execute_();

/**
 *  \brief 出口程序
 *  \return 成功相应码，0成功，负数失败
 */
int oct_upload_file_(const char* sIP, int iPort, int cType, const char* siteName, const char* remotePathName, const char* localPathName);
int oct_delete_file_(const char* sIP, int iPort,const char* siteName, const char* remotePathName);
int oct_move_file_(const char* sIP, int iPort, const char* destSiteName, const char* destPathName, const char* srcSiteName, const char* srcPathName);
int oct_copy_file_(const char* sIP, int iPort, const char* destSiteName, const char* destPathName, const char* srcSiteName, const char* srcPathName);

/**
 *  \brief 根据当前时间生成消息中的时间格式
 *  \return 消息中的时间格式
 */
const char* oct_get_time_();

/**
 *  \brief 输出错误信息
 *  \return 错误信息
 */
const char* oct_get_error_();


//获得文件名中的后缀(规则就是取第一个.后面的所有东西)
int getTypeOfUploadPackFile(char * sFile);

void divPathFile(char *sPathFile, char *sPath, char *sFileName);


#endif /* _OCT_CLI_H_ */
