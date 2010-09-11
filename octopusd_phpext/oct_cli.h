#ifndef _OCT_CLI_H_
#define _OCT_CLI_H_

/**
 *  \brief ����Ҫ���͵ķ�������IP�Ͷ˿ڣ���Ҫ���ڷ����Ҫ��Ȩ�������Ҫ�ѱ���IP�ڷ�����������
 *  \param sIP ��������IP
 *  \param iPort ����˶˿�
 *  \return void
 */
void oct_init_(const char* sIP, int iPort);

/**
 *  \brief �����ϴ���Ϣ������
 *  \param cType �ϴ��ļ�������
 *  \param sSiteName վ������
 *  \param sRemotePathName �ļ���Զ�˵������վĿ¼�����Ŀ¼���ļ�����
 *  \param sLocalPathName �ļ��ڱ��صľ���Ŀ¼������
 *  \return void
 */
void oct_set_upload_(int cType, const char* siteName, const char* remotePathName, const char* localPathName);

/**
 *  \brief ���ô���ϴ���Ϣ������
 *  \param cType �ϴ��ļ�������
 *  \param sSiteName վ������
 *  \param sRemotePath �ļ���Զ�˵������վĿ¼�����Ŀ¼
 *  \param sLocalPathName �ļ��ڱ��صľ���Ŀ¼������
 *  \return -100��ʾʧ��,0�ɹ�
 */
int oct_set_upload_pack_(int cType, const char* siteName, const char* remotePathName, const char* localPathName);

/**
 *  \brief �����ϴ���Ϣ������
 *  \param sSiteName վ������
 *  \param sRemotePathName �ļ���Զ�˵������վĿ¼�����Ŀ¼���ļ�����
 *  \return void
 */
void oct_set_delete_(const char* siteName, const char* remotePathName);

/**
 *  \brief �ƶ��Ϳ����ļ�
 *  \param sdestSiteName Ŀ��վ������
 *  \param sDestPathName Ŀ��Ŀ¼�ļ�,�����վ�����Ŀ¼
 *  \param sSrcSiteName Դվ������
 *  \param sSrcPathName ԴĿ¼�ļ�,�����վ�����Ŀ¼
 *  \return void
 */
void oct_set_mv_(const char* destSiteName, const char* destPathName, const char* srcSiteName, const char* srcPathName);
void oct_set_cp_(const char* destSiteName, const char* destPathName, const char* srcSiteName, const char* srcPathName);

/**
 *  \brief ǰ�����óɹ��󣬷�����Ϣ
 *  \return �ɹ���Ӧ�룬0�ɹ�������ʧ��
 */
int oct_execute_();

/**
 *  \brief ���ڳ���
 *  \return �ɹ���Ӧ�룬0�ɹ�������ʧ��
 */
int oct_upload_file_(const char* sIP, int iPort, int cType, const char* siteName, const char* remotePathName, const char* localPathName);
int oct_delete_file_(const char* sIP, int iPort,const char* siteName, const char* remotePathName);
int oct_move_file_(const char* sIP, int iPort, const char* destSiteName, const char* destPathName, const char* srcSiteName, const char* srcPathName);
int oct_copy_file_(const char* sIP, int iPort, const char* destSiteName, const char* destPathName, const char* srcSiteName, const char* srcPathName);

/**
 *  \brief ���ݵ�ǰʱ��������Ϣ�е�ʱ���ʽ
 *  \return ��Ϣ�е�ʱ���ʽ
 */
const char* oct_get_time_();

/**
 *  \brief ���������Ϣ
 *  \return ������Ϣ
 */
const char* oct_get_error_();


//����ļ����еĺ�׺(�������ȡ��һ��.��������ж���)
int getTypeOfUploadPackFile(char * sFile);

void divPathFile(char *sPathFile, char *sPath, char *sFileName);


#endif /* _OCT_CLI_H_ */
