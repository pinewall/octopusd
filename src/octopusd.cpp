
#include "octopusd.h"

//�������Ŀ¼
string g_sHomeDir = "";

//���������
string g_sProgramName = "";

//Server Socket�ļ�����
CTCPSocket g_insServerSocket;

//��־�ļ��ļ��࣬ע��debu���ܱ�����maintain��config����ɳ�ʼ�������ʹ��
CLogMng g_insLogMng;

//ȫ�����ù�����
CConfigMng g_insConfigMng;

//ȫ��ά��������
CMaintain g_insMaintain;

//ȫ�ֵĶ��й�����
CQueueMng g_insQueueMng;

//���ӳ�
CThreadMutex g_insConCtrl;  //������
list<CTCPSocket> g_lstConnectPool;

//����ά�������˳���״̬
map<int, int> g_mapChildExitStat;

//��¼��Ϣ������linux�ϵ�pthread_self��getpid��ͬ�����Լ�¼һ�������Ķ�Ӧ��Ϣ�����쿴֮��
map<int, int> g_mapThread2pid;


//octopusd��������
int main(int argc, char* argv[])
{
        // [1]�汾���
        if( argc == 2 && strcmp( argv[1], "-v" ) == 0 )
        {
                printf( "Version:V2.3Beta01Build020, BUILD DATE: %s %s\n",   __DATE__, __TIME__ );
                exit(0);
        }

        //�������� newoctopusd
        g_sProgramName = "newoctopusd";

        // [2]octopusd����Ŀ¼
        if (NULL == getenv("NEWOCTOPUSD_HOME"))
        {
                g_sHomeDir = "/usr/local/newoctopusd";
        }
        else
        {
                g_sHomeDir =  getenv("NEWOCTOPUSD_HOME");
        }
        //���Ŀ¼�Ƿ����
        struct stat st1;
        if(!((stat(g_sHomeDir.c_str(), &st1)== 0) && S_ISDIR(st1.st_mode)))
        {
                printf("home dir %s does not exist, %s start failed.\n", g_sHomeDir.c_str(), g_sProgramName.c_str());
                exit(0);
        }

        //[3] �ػ�����, ͬʱֻ��һ��ʵ������
        daemon_init();
        string sPidFileName = g_sHomeDir + "/dat/" + g_sProgramName + ".pid" ;
        run_single( (char *)sPidFileName.c_str(), (char*)g_sProgramName.c_str(), 1);

        // [4]�ź�������
        signal( SIGIO, SIG_IGN);
        signal( SIGPIPE, SIG_IGN);
        signal( SIGTERM, sighandler);
        signal( SIGHUP, sighandler);
        signal( SIGCHLD, sighandler);
        signal( SIGUSR1, sighandler);
        signal( SIGINT, sighandler);
        signal(SIGALRM, sighandler);        

        // [5]��־ģ��
        if (0 != g_insLogMng.init(g_sProgramName))
        {
                printf( "Create log file failed, %s start failed.\n", g_sProgramName.c_str());
                exit(0);
        }
        g_insLogMng.normal("%s begin to running......", g_sProgramName.c_str());
        g_insLogMng.normal("===Init logmng module successfully===");

        
        // [6]�����ļ�
        if(0 !=  g_insConfigMng.init())
        {
                g_insLogMng.error("Init configmng module failed.");
                exit(0);
        }
        
        g_insLogMng.normal("===Init configmng module successfully===");


        // [7]ά��ģ��
        if(0 !=  g_insMaintain.init())
        {
                g_insLogMng.error("Init maintain module failed.");
                exit(0);
        }
        g_insLogMng.normal("===Init maintain module successfully===");


        // [8]�ṩ����ļ���
        g_insServerSocket.init(g_insConfigMng.m_tComCfg.m_sServerIP,
                g_insConfigMng.m_tComCfg.m_iServerPort);
        if(0 != g_insServerSocket.createSocket())
        {
                 g_insLogMng.error("Create server(%s,%d) socket listen failed.",
                        g_insConfigMng.m_tComCfg.m_sServerIP,
                        g_insConfigMng.m_tComCfg.m_iServerPort);
                 exit(0);
        }
        g_insLogMng.normal("===Create server(%s,%d) socket listen successfully===",
                g_insConfigMng.m_tComCfg.m_sServerIP,
                g_insConfigMng.m_tComCfg.m_iServerPort);

        // [9]���й�����
        if (0 != g_insQueueMng.init())
        {
                g_insLogMng.error("Init queuemng module failed.");
                exit(0);
        }
        g_insLogMng.normal("===Init queuemng module successfully===");

        // [10]�̵߳Ľ���
        int iLoop = 0;
        int piD;

        g_insLogMng.normal("Begin to create ConnectThread(%d)......", 1);
        for(iLoop = 0; iLoop < 1; iLoop++)  //onlyһ��
        {
                //[10.0]���������߳�
                pthread_create((pthread_t*)&piD, NULL,  pthread_connect, (void *)NULL);
                g_mapChildExitStat[piD] = 0;  //�ú���״̬
                g_mapThread2pid[piD] = 0;  //threadid 2 pid
        }

        g_insLogMng.normal("Begin to create RecvThread(%d)......",  g_insConfigMng.m_tComCfg.m_iRecvNrOfThread);
        for(iLoop = 0; iLoop < g_insConfigMng.m_tComCfg.m_iRecvNrOfThread; iLoop++)
        {
                //[10.1]���������߳�
                pthread_create((pthread_t*)&piD, NULL,  pthread_recv,  (void *)NULL);       
                g_mapChildExitStat[piD] = 0;  //�ú���״̬
                g_mapThread2pid[piD] = 0;  //threadid 2 pid
        }

        g_insLogMng.normal("Begin to create P_MaintainThread(%d)......", 1);
        //[10.2.1]����ά���߳�P
         pthread_create((pthread_t*)&piD, NULL, pthread_maintain_p,  (void *)NULL);        
        g_mapChildExitStat[piD] = 0;  //�ú���״̬
        g_mapThread2pid[piD] = 0;  //threadid 2 pid

         g_insLogMng.normal("Begin to create R_MaintainThread(%d)......", 1);
        //[10.2.2]����ά���߳�R
         pthread_create((pthread_t*)&piD, NULL, pthread_maintain_r,  (void *)NULL);        
        g_mapChildExitStat[piD] = 0;  //�ú���״̬
        g_mapThread2pid[piD] = 0;  //threadid 2 pid    

        //****** BEGIN ��һ�飬ֻ�е��зַ����е�ʱ��Ž�����û�е�ʱ�򲻴��� ****** 
        if (0 != g_insConfigMng.m_tComCfg.m_iDistributeFlag)
        {
                //[10.2.3]����ά���߳�F���������ά���߳�ϵ�У���������̵߳�û�зַ�����ʱ,���蹤��
                pthread_create((pthread_t*)&piD, NULL, pthread_maintain_f,  (void *)NULL);        
                g_mapChildExitStat[piD] = 0;  //�ú���״̬
                g_mapThread2pid[piD] = 0;  //threadid 2 pid    
        
                g_insLogMng.normal("Begin to create SendThread(%d)......",  g_insConfigMng.m_tComCfg.m_iSendNrOfThread);
                for(iLoop = 0; iLoop < g_insConfigMng.m_tComCfg.m_iSendNrOfThread; iLoop++)
                {
                        //[10.3]���������߳�
                        pthread_create((pthread_t*)&piD, NULL, pthread_send,  (void *)NULL);      
                        g_mapChildExitStat[piD] = 0;  //�ú���״̬
                        g_mapThread2pid[piD] = 0;  //threadid 2 pid
                }

                g_insLogMng.normal("Begin to create SendErrorThread(%d)......",  g_insConfigMng.m_tComCfg.m_iSendErrorNrOfThread);
                for(iLoop = 0; iLoop < g_insConfigMng.m_tComCfg.m_iSendErrorNrOfThread; iLoop++)
                {
                        //[10.3.2]��������ʧ���߳�
                        pthread_create((pthread_t*)&piD, NULL, pthread_sendError,  (void *)NULL);      
                        g_mapChildExitStat[piD] = 0;  //�ú���״̬
                        g_mapThread2pid[piD] = 0;  //threadid 2 pid
                }

                g_insLogMng.normal("Begin to create CheckThread(%d)......",  1);
                 //[10.3.3]������֤�����߳�
                pthread_create((pthread_t*)&piD, NULL, pthread_check,  (void *)NULL);      
                g_mapChildExitStat[piD] = 0;  //�ú���״̬
                g_mapThread2pid[piD] = 0;  //threadid 2 pid
                
        }
        // ****** END ******       

        g_insLogMng.normal("===Init all child thread successfully===");

        //[11]���߳�
        g_insLogMng.normal("%s(%u) starts successfully and is running......", g_sProgramName.c_str(), pthread_self());

        //kevinliu print screen
        printf("\n\n%s starts successfully and is running......\n", g_sProgramName.c_str());

        //��������ʱ,����һ���ֻ�����,�Է�ֹ�쳣��������֪
        g_insMaintain.sendMobileMsg("ϵͳ�����ɹ���������......");

        while(1)
        {
                //�жϳ����Ƿ��˳�
                if (1 == g_insMaintain.isExitProgram())
                {
                        g_insLogMng.normal("MainProcess recv a exit notice and forward it to its %d children.", 
                                g_mapChildExitStat.size());
                        setChildExitStat();
                        while(1)
                        {
                                //�������Ƿ��Ѿ�exit
                                if(0 != checkChildExitStat())
                                {                                        
                                        CComFun::sleep_msec(0, 100);                                         
                                        continue;
                                }
                                 g_insLogMng.normal("all children exited, begin to save duty queue.");

                                //�������������δ���͵�����д��ʧ���ļ�
                                if (1 == g_insMaintain.getFlag(CMaintain::SaveDutyQueueWhenExit))
                                {

                                        g_insQueueMng.saveDutyQueue2File();
                                }

                                //����ɺ��˳�
                                g_insLogMng.normal("MainProcess confirm all children exited and exit itself.");
                                printf("\n\nMainProcess's all children thread safely exit, so MainProcess exit\n");
                                exit(0);
                        }
                }

		CComFun::sleep_msec(1, 0);
        }

        return 0;
}

/**
 *  \brief ʹ���̺�̨��damon��ʽ����
 *  \return <0 ���� >=0 �ɹ�
 */
int daemon_init(void)
{
        pid_t childpid;
        childpid = fork();
        if( childpid < 0 )
        {
                printf("Fork error in daemon_init");
                exit(2);
        }
        else if( childpid != 0 )
                exit(0);
        setsid();               /*  become session leader */
        umask(0);               /*  clear our file mode creation mask */
        return(0);
}


/**
 *  \brief ����ϵͳ����fcntl(2)���ļ����ö�д��
 *
 *  fcntl(2)�������ø�ʽ:<br>
 *      int fcntl( int fd, int cmd, struct * flock );
 *  <ul>
 *  <li> ����:
 *      <ol>
 *      <li> F_SETLK    ��ȡ���ͷ�flcok�ṹָ��������������ɹ���������EACCES��EAGAIN
 *      <li> F_SETLKW   ��ȡ���ͷ�flock�ṹָ��������������ɹ�����ý��̱�����
 *      <li> F_GETLK    ��ȡflock�ṹָ�������Ƿ���ڣ��Լ���ؽ���pid�������͵���Ϣ
 *      </ol>
 *  <li> struct flock           //�ļ��������ṹ
 *      <ul>
 *      <li> short l_type;      //F_RDLCK, F_WRLCK, F_UNLCK �ֱ��Ƕ�����д��������
 *      <li> short l_whence;    //SEEK_SET, SEEK_CUR, SEEK_END �ļ�ƫ����ʼ����λ�ã��ļ�ͷ����ǰ���ļ�β
 *      <li> off_t l_start;     //relative starting offset in bytes,ƫ���ֽ���
 *      <li> off_t l_len;       //#bytes; 0 means until end-of-file, ���ĳ��ȣ�0Ϊ�����ļ�
 *      <li> pid_t l_pid;       //pid returned by F_GETLK, ��ʹ��F_GETLK����ʱ������������̵�pid
 *      </ul>
 *  </ul>
 */
int lock_reg (int fd, int cmd, int type, off_t offset, int whence, off_t len)
{
    struct flock lock;
    lock.l_type = type;
    lock.l_start = offset;
    lock.l_whence = whence;
    lock.l_len = len;
    return (fcntl (fd, cmd, &lock));    /* -1 upon error */
}

/**
 *  \brief ���ƽ��̽���һ��ʵ�����еĺ���,ʹ��һ���ļ�д���������˳�������Ȼ�ͷ�
 *  \param filepath ���ڽ����ļ������ļ���,���ļ������ض�,�����������id
 *  \param appname  ���г��������,һ����argv[0]�õ�
 *  \return 0 ִ�м����ɹ� ����������ɹ���, ��ǰ���̱�ǿ����ֹ
 */
int run_single(char * filepath, char * appname,  int iLogFlag)
{
        int pidfd;
        char line[1024];
        pidfd = open( filepath, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        if( pidfd < 0 )
        {
                if (1 == iLogFlag)
                {
                        printf("Can't create pid file %s", filepath);
                }
                exit(4);
        }
        if(lock_reg( pidfd, F_SETLK, F_WRLCK, 0, SEEK_SET, 0 ) < 0 )
        {
                if( errno == EACCES || errno == EAGAIN )
                {
                        if (1 == iLogFlag)
                        {
                                printf("Unable to lock %s, maybe %s is already running.\n", filepath, appname);
                        }
                        exit(5);
                }
                else
                {
                        if (1 == iLogFlag)
                        {
                                printf("Unable to lock %s.", filepath);
                        }
                        exit(6);
                }
        }
        ftruncate( pidfd, 0 );  /*clear the file */
        snprintf(line, sizeof(line), "%d\n", getpid() );
        write( pidfd, line,strlen(line) );
        return 0;
}

/**
 *  \brief �źŴ�����
 *  \brief �����յ����ź�
 */
void sighandler(int signum)
{
        sigset_t block_mask;

        g_insLogMng.debug("all", CLogMng::LogMode1, "[pid %d got a signal %d]", getpid(), signum);

        if (signum == SIGCHLD)
        {
                int errno2, result;
                pid_t deadpid;
                errno2 = errno;
                while((deadpid = waitpid(0, &result, WNOHANG)) > 0)
                {
                }
                
                errno = errno2;
                signal(SIGCHLD, sighandler);
        }
        else if (signum == SIGUSR1)
        {
                signal(SIGUSR1, sighandler);
        }
        else if ( signum == SIGINT || signum == SIGTERM)
        {
                /*�������п��ܵ��ź�*/
                sigemptyset( &block_mask );
                sigaddset( &block_mask, SIGINT );
                sigaddset( &block_mask, SIGQUIT );
                sigaddset( &block_mask, SIGTERM );
                sigaddset( &block_mask, SIGCHLD );
                sigaddset( &block_mask, SIGIO );
                sigaddset( &block_mask, SIGPIPE );
                sigaddset( &block_mask, SIGHUP );
                sigprocmask( SIG_BLOCK, &block_mask, NULL );
                g_insLogMng.normal("Process is killed by signal %d", signum );
                exit(0);
        }
}

int checkChildExitStat()
{
        for(map<int, int>::iterator it = g_mapChildExitStat.begin();
                it != g_mapChildExitStat.end();it++)
        {
                if (0 != it->second)
                {
                        //g_insLogMng.debug("protocol", CLogMng::LogMode2,  "id=%d is not exit(%d).",
                        //        it->first, it->second);
                        return 1;
                }
        }
        return 0;
}

void setChildExitStat()
{
        for(map<int, int>::iterator it = g_mapChildExitStat.begin();
                it != g_mapChildExitStat.end();it++)
        {
                it->second = 1;
        }
        return;
}

void setMyExitStat(int iMyID, int iStatus)
{
        g_mapChildExitStat[iMyID] = iStatus;
}

int checkMyExitStat(int iMyID)
{
        return g_mapChildExitStat[iMyID];
}

void* pthread_connect(void *p)
{
        //С˯һ��,�ȴ������̳�ʼ�������߳�,100ms�㹻
        CComFun::sleep_msec(0, 100);
        
        //��������ͻ��˵�����
        char sClientIP[LenOfIPAddr];
        int iFD;
        int iMyID = (int)pthread_self();
        g_mapChildExitStat[iMyID] = 0;  //���Լ�״̬
        g_mapThread2pid[iMyID] = (int)getpid();  //threadid 2 pid
        g_insLogMng.normal("ConnectthreadID = %u PID = %d" , (unsigned int)iMyID, getpid());
        int iWaitTime = 1; // 1s
         
        //��ѭ��
        while(1)
        {
                // [1]�Ƿ���˳�ϵͳ��
                if (0 != checkMyExitStat(iMyID))
                {
                        g_insLogMng.normal("ConnectThread(%u) recv an exit notice, so exit.",  (unsigned int)iMyID);
                        setMyExitStat(iMyID, 0);
                        break;
                }

                // [2]���տͻ��˵����� 
                if ( 0 > g_insServerSocket.checkSocket(iWaitTime, CTCPSocket::ReadMode))
                {
                       CComFun::sleep_msec(0, 1);
                         continue;
                }
                
                 iFD = g_insServerSocket.acceptClient(sClientIP);
                 if (0 > iFD)
                 {
                         CComFun::sleep_msec(0, 1);
                         continue;
                 }
                 else if (900 < iFD)  //����������fd���Ϊ1024������ж�һ�£����fd������900����һ�¾����Է�ֹfdй¶�޷�����
                 {
                        if (0 != access((g_sHomeDir + "/cfg/fd.if").c_str(),  F_OK))  //�����ھͱ��������ȷ��ϵͳ����֧�ֵ�1000��������Ӵ��ļ���������
                        {
                                g_insMaintain.sendMobileMsg("���������ļ�������FD>900,ϵͳ�Ѿ��Զ�����ǰ500��������ϵͳ�Ƿ���FDй¶��");
                                //for(int ixx = 10; ixx < 500; ixx++)
                                //{
                                //        close(ixx); //Ϊ�˷�ֹϵͳ���µĹ��ϣ����ǿ�м�������߼������ȷ�ϲ���Ҫ������cfg���fd.if�ļ�
                                //}
                        }
                        //дalarm�����ļ� [ALARM:4]
                        CComFun::writeWholeFileContent((g_sHomeDir + "/dat/maxfd.alm"), "fd>900");                        
                 }
                 //else ����������ɾ�������Ǽ�ؽű������ļ���modify�Ƿ��Ѿ��������ж�
                 //{
                 //       CComFun::delFile(g_sHomeDir + "/dat/maxfd.alm"); 
                 //}
                
                CTCPSocket insClientSocket(iFD, g_insServerSocket.m_sIP, g_insServerSocket.m_iPort);

                // [3] �ж�sClientIP�Ƿ���LoginIP֮��,�����򷵻أ��������ϸ���жϣ�ֻ��һ�����ַ�����
                char *pTmp = strstr(g_insConfigMng.m_tComCfg.m_sLoginIP,  sClientIP);
                if (NULL == pTmp)
                {
                        g_insLogMng.normal("Recv a connection from IP(%s), it isn't in common.cfg(LoginIP), so refuse it.",
                                sClientIP);
                        insClientSocket.closeSocket();  //�ر�
                        CComFun::sleep_msec(0, 1);
                        continue;
                }
                //else  //���ӳɹ�����ӡһ����־��Ϣ��һ�����
                //{
                //        g_insLogMng.debug("all", CLogMng::LogMode1, 
                //                "Accept a connection from IP(%s) successfully.", sClientIP);
                //}

                // [4]������м�����
                g_insConCtrl.setLock();  //����
                if (0 != g_insConfigMng.m_tComCfg.m_iMaxNrOfCnt
                        && g_lstConnectPool.size() >= (unsigned int)g_insConfigMng.m_tComCfg.m_iMaxNrOfCnt)
                {
                        g_insConCtrl.unLock(); //����
                        g_insLogMng.normal("NrOfConnection has reached max = %d,  so refuse a new connection.",
                                g_insConfigMng.m_tComCfg.m_iMaxNrOfCnt);
                        insClientSocket.closeSocket();  //�ر�
                        CComFun::sleep_msec(0, 1);
                        continue;
                }
                g_lstConnectPool.push_back(insClientSocket);
                g_insConCtrl.unLock(); //����

                //sleep�ȴ���һ��
                CComFun::sleep_msec(0, 1);
        }
        return (void*)NULL;
}

void* pthread_recv(void *p)
{
        //С˯һ��,�ȴ������̳�ʼ�������߳�,100ms�㹻
        CComFun::sleep_msec(0, 100);
        
        //��������ͻ��˵�����
        string sClientIP;
        int iMyID = pthread_self();
        g_mapChildExitStat[iMyID] = 0;  //���Լ�״̬
        g_insQueueMng.initThreadStatInfo(0, iMyID);
        g_mapThread2pid[iMyID] = (int)getpid();  //threadid 2 pid
        g_insLogMng.normal("RecvthreadID = %u PID = %d" , (unsigned int)iMyID, getpid());
        
        //��ѭ��
        while(1)
        {
                // [1]�Ƿ���˳�ϵͳ��
                if (0 != checkMyExitStat(iMyID))
                {
                        g_insLogMng.normal("RecvThread(%u) recv an exit notice, so exit.",  (unsigned int)iMyID);
                        setMyExitStat(iMyID, 0);
                        break;
                }

                // [2]ȡ����
                g_insConCtrl.setLock(); //����
                if (0 == g_lstConnectPool.size())
                {
                        g_insConCtrl.unLock();
                        CComFun::sleep_msec(0, 1);
                        continue;
                }
                list<CTCPSocket>::iterator it = g_lstConnectPool.begin();
                CTCPSocket insClientSocket = *it;
                g_lstConnectPool.erase(it);
                g_insConCtrl.unLock();   //����

                // [3] ����Ϣ����Э�鴦��
                CProtocolMng::processRecv(insClientSocket);

                insClientSocket.closeSocket();

                //sleep�ȴ���һ��
                CComFun::sleep_msec(0, 1);
        }
        return (void*)NULL;
}

void* pthread_send(void *p)
{
        //С˯һ��,�ȴ������̳�ʼ�������߳�,100ms�㹻
        CComFun::sleep_msec(0, 100);
        
        int iMyID = pthread_self();
        g_mapChildExitStat[iMyID] = 0;  //���Լ�״̬
        g_insQueueMng.initThreadStatInfo(1, iMyID);
        g_mapThread2pid[iMyID] = (int)getpid();  //threadid 2 pid
        g_insLogMng.normal("SendthreadID = %u PID = %d" , (unsigned int)iMyID, getpid());
                
        while(1)
        {
                //�Ƿ���˳�ϵͳ��
                if (0 != checkMyExitStat(iMyID))
                {
                        g_insLogMng.normal("SendThread(%u) recv an exit notice, so exit.",  (unsigned int)iMyID);
                        setMyExitStat(iMyID, 0);
                        break;
                }

                //����������
                CProtocolMng::processSend(0);

                //sleep�ȴ���һ��
                CComFun::sleep_msec(0, 1);
        }
        return (void*)NULL;
}


void* pthread_sendError(void *p)
{
        //С˯һ��,�ȴ������̳�ʼ�������߳�,100ms�㹻
        CComFun::sleep_msec(0, 100);
        
        int iMyID = pthread_self();
        g_mapChildExitStat[iMyID] = 0;  //���Լ�״̬
        g_insQueueMng.initThreadStatInfo(1, iMyID);
        g_mapThread2pid[iMyID] = (int)getpid();  //threadid 2 pid
        g_insLogMng.normal("SendErrorthreadID = %u PID = %d" , (unsigned int)iMyID, getpid());
                
        while(1)
        {
                //�Ƿ���˳�ϵͳ��
                if (0 != checkMyExitStat(iMyID))
                {
                        g_insLogMng.normal("ErrorSendThread(%u) recv an exit notice, so exit.",  (unsigned int)iMyID);
                        setMyExitStat(iMyID, 0);
                        break;
                }

                //����������
                CProtocolMng::processSend(1);

                //sleep�ȴ���һ��
                CComFun::sleep_msec(0, 1);
        }
        return (void*)NULL;
}

void* pthread_maintain_p(void *p)
{
        //С˯һ��,�ȴ������̳�ʼ�������߳�,100ms�㹻
        CComFun::sleep_msec(0, 100);
        
        int iMyID = pthread_self();
        g_mapChildExitStat[iMyID] = 0;  //���Լ�״̬
        g_insQueueMng.initThreadStatInfo(0, iMyID);  //ʧ���������¼��ص�ʱ��������������
        g_mapThread2pid[iMyID] = (int)getpid();  //threadid 2 pid
        g_insLogMng.normal("P_MaintainthreadID = %u PID = %d" , (unsigned int)iMyID, getpid());
        
        while(1)
        {
                //�Ƿ���˳�ϵͳ��
                if (0 != checkMyExitStat(iMyID))
                {
                        g_insLogMng.normal("P_MaintainThread(%u) recv an exit notice, so exit.",  (unsigned int)iMyID);
                        setMyExitStat(iMyID, 0);
                        break;
                }

                //���ܴ���
                g_insMaintain.process();

                //sleep�ȴ���һ��
                CComFun::sleep_msec(1, 0);
        }
        return (void*)NULL;
}

void* pthread_maintain_r(void *p)
{
        //С˯һ��,�ȴ������̳�ʼ�������߳�,100ms�㹻
        CComFun::sleep_msec(0, 100);
        
        int iMyID = pthread_self();
        g_mapChildExitStat[iMyID] = 0;  //���Լ�״̬
        g_insQueueMng.initThreadStatInfo(0, iMyID);  //ʧ���������¼��ص�ʱ��������������
        g_mapThread2pid[iMyID] = (int)getpid();  //threadid 2 pid
        g_insLogMng.normal("R_MaintainthreadID = %u PID = %d" , (unsigned int)iMyID, getpid());
        
        while(1)
        {
                //�Ƿ���˳�ϵͳ��
                if (0 != checkMyExitStat(iMyID))
                {
                        g_insLogMng.normal("R_MaintainThread(%u) recv an exit notice, so exit.",  (unsigned int)iMyID);
                        setMyExitStat(iMyID, 0);
                        break;
                }

                 //���д���
                g_insMaintain.routine();

                //sleep�ȴ���һ��
                CComFun::sleep_msec(1, 0);
        }
        return (void*)NULL;
}

void* pthread_maintain_f(void *p)
{
        //С˯һ��,�ȴ������̳�ʼ�������߳�,100ms�㹻
        CComFun::sleep_msec(0, 100);
        
        int iMyID = pthread_self();
        g_mapChildExitStat[iMyID] = 0;  //���Լ�״̬
        g_insQueueMng.initThreadStatInfo(0, iMyID);  //ʧ���������¼��ص�ʱ��������������
        g_mapThread2pid[iMyID] = (int)getpid();  //threadid 2 pid
        g_insLogMng.normal("F_MaintainthreadID = %u PID = %d" , (unsigned int)iMyID, getpid());
        
        while(1)
        {
                //�Ƿ���˳�ϵͳ��
                if (0 != checkMyExitStat(iMyID))
                {
                        g_insLogMng.normal("F_MaintainThread(%u) recv an exit notice, so exit.",  (unsigned int)iMyID);
                        setMyExitStat(iMyID, 0);
                        break;
                }

                //���д���,Ҳ��Ϊ�˷�ֹ����ʧ������ʱ�����ֲ�����ͻ
                g_insMaintain.routineFailDuty();

                //sleep�ȴ���һ��
                CComFun::sleep_msec(1, 0);
        }
        return (void*)NULL;
}

void* pthread_check(void *p)
{
        //С˯һ��,�ȴ������̳�ʼ�������߳�,100ms�㹻
        CComFun::sleep_msec(0, 100);
        
        int iMyID = pthread_self();
        g_mapChildExitStat[iMyID] = 0;  //���Լ�״̬
        g_insQueueMng.initThreadStatInfo(0, iMyID);  //ʧ���������¼��ص�ʱ��������������
        g_mapThread2pid[iMyID] = (int)getpid();  //threadid 2 pid
        g_insLogMng.normal("CheckthreadID = %u PID = %d" , (unsigned int)iMyID, getpid());
        
        while(1)
        {
                //�Ƿ���˳�ϵͳ��
                if (0 != checkMyExitStat(iMyID))
                {
                        g_insLogMng.normal("CheckthreadID(%u) recv an exit notice, so exit.",  (unsigned int)iMyID);
                        setMyExitStat(iMyID, 0);
                        break;
                }

                int iSafe = g_insMaintain.getFlag(CMaintain::QueueSafeMode);
                int iClearD = g_insMaintain.getFlag(CMaintain::ClearDisabledDuty);
                if(1 == iSafe)
                {
                        //����ת��
                        g_insQueueMng.mvErrorQ2SendQ();
                }

                //����ʧЧ�����񣬿�����
                if(1 == iClearD)
                {
                        g_insQueueMng.clearDisabledDuty(); 
                }

                //�ȴ�ʱ�䣬����������޸��㹻����ʱ��                
                if (iSafe == 0 && iClearD == 0)
                {
                        CComFun::sleep_msec(20, 0);  // 20s����ʵ�����Ը������޸�ֻ֮��Ϊ����Ӧ�˳��źŸ���һЩ
                }
                else
                {
                        CComFun::sleep_msec(0, 10); 
                }
        }
        return (void*)NULL;
}

