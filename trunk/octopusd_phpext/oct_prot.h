#ifndef _OCT_PROT_H_
#define _OCT_PROT_H_

#define MsgBufferSize 1024 * 10   //10K
#define WriteToFileSize 1024 * 1000   //500K ��ʼдһ���ļ�,����MsgBufferSize
#define LenOfSiteName 32   //վ�����Ƴ���
#define LenOfMsgTime 20   //ʱ���ʽ����
#define LenOfPathFileName 256  //·�����ļ����Ƶĳ���
#define LenOfError 500  //������Ϣ�ĳ���

//�ϴ��ļ�������
#define FileName  0x1 //�ļ�����
#define FileData 0x2  //�ļ���
//֧�ֵ���Ϣ���
#define ProtocolUploadFile 0x01 //�ϴ��ļ�
#define ProtocolDelFile  0x02  //ɾ���ļ�
#define ProtocolMvFile 0x15  //�ƶ�
#define ProtocolCpFile 0x16   //����

#define ProtocolPackFile 0x17   //����ϴ� 2008-07-22����

//�ļ�Ŀ¼ 2008-07-22����
#define FILE_ERROR  0     //����
#define FILE_DIR    1       //Ŀ¼
#define FILE_TARGZ  2     // *.tar.gz
#define FILE_ZIP    3       // *.zip
#define FILE_NORMAL 4    //��ͨ�ļ�

//��Ϣ�汾
#define Version1 0x01

//��Ϣ����
static int eProtocolType;
//�ļ�����
static int eFileType;
//վ����
static char sSiteName[LenOfSiteName];
//Զ��Ŀ¼�ļ�������
static char sRemotePathName[LenOfPathFileName];
//����Ŀ¼������
static char sLocalPathName[LenOfPathFileName]; 
//����ʱ��
static char sSendTime[LenOfMsgTime];
//����ԭ��
static char lastError[LenOfError];
static char s[50];

static struct in_addr server_addr_;
static short server_port_;

#endif
