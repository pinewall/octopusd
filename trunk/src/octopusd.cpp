
#include "octopusd.h"

//程序的主目录
string g_sHomeDir = "";

//程序的名称
string g_sProgramName = "";

//Server Socket的监听类
CTCPSocket g_insServerSocket;

//日志文件文件类，注意debu功能必须在maintain和config类完成初始化后才能使用
CLogMng g_insLogMng;

//全局配置管理类
CConfigMng g_insConfigMng;

//全局维护管理类
CMaintain g_insMaintain;

//全局的队列管理类
CQueueMng g_insQueueMng;

//连接池
CThreadMutex g_insConCtrl;  //锁控制
list<CTCPSocket> g_lstConnectPool;

//用于维护进程退出的状态
map<int, int> g_mapChildExitStat;

//记录信息，由于linux上的pthread_self和getpid不同，所以记录一个这样的对应信息，供察看之用
map<int, int> g_mapThread2pid;


//octopusd的主程序
int main(int argc, char* argv[])
{
        // [1]版本检查
        if( argc == 2 && strcmp( argv[1], "-v" ) == 0 )
        {
                printf( "Version:V2.3Beta01Build020, BUILD DATE: %s %s\n",   __DATE__, __TIME__ );
                exit(0);
        }

        //程序名称 newoctopusd
        g_sProgramName = "newoctopusd";

        // [2]octopusd的主目录
        if (NULL == getenv("NEWOCTOPUSD_HOME"))
        {
                g_sHomeDir = "/usr/local/newoctopusd";
        }
        else
        {
                g_sHomeDir =  getenv("NEWOCTOPUSD_HOME");
        }
        //检查目录是否存在
        struct stat st1;
        if(!((stat(g_sHomeDir.c_str(), &st1)== 0) && S_ISDIR(st1.st_mode)))
        {
                printf("home dir %s does not exist, %s start failed.\n", g_sHomeDir.c_str(), g_sProgramName.c_str());
                exit(0);
        }

        //[3] 守护进程, 同时只有一个实例运行
        daemon_init();
        string sPidFileName = g_sHomeDir + "/dat/" + g_sProgramName + ".pid" ;
        run_single( (char *)sPidFileName.c_str(), (char*)g_sProgramName.c_str(), 1);

        // [4]信号量处理
        signal( SIGIO, SIG_IGN);
        signal( SIGPIPE, SIG_IGN);
        signal( SIGTERM, sighandler);
        signal( SIGHUP, sighandler);
        signal( SIGCHLD, sighandler);
        signal( SIGUSR1, sighandler);
        signal( SIGINT, sighandler);
        signal(SIGALRM, sighandler);        

        // [5]日志模块
        if (0 != g_insLogMng.init(g_sProgramName))
        {
                printf( "Create log file failed, %s start failed.\n", g_sProgramName.c_str());
                exit(0);
        }
        g_insLogMng.normal("%s begin to running......", g_sProgramName.c_str());
        g_insLogMng.normal("===Init logmng module successfully===");

        
        // [6]配置文件
        if(0 !=  g_insConfigMng.init())
        {
                g_insLogMng.error("Init configmng module failed.");
                exit(0);
        }
        
        g_insLogMng.normal("===Init configmng module successfully===");


        // [7]维护模块
        if(0 !=  g_insMaintain.init())
        {
                g_insLogMng.error("Init maintain module failed.");
                exit(0);
        }
        g_insLogMng.normal("===Init maintain module successfully===");


        // [8]提供服务的监听
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

        // [9]队列管理类
        if (0 != g_insQueueMng.init())
        {
                g_insLogMng.error("Init queuemng module failed.");
                exit(0);
        }
        g_insLogMng.normal("===Init queuemng module successfully===");

        // [10]线程的建立
        int iLoop = 0;
        int piD;

        g_insLogMng.normal("Begin to create ConnectThread(%d)......", 1);
        for(iLoop = 0; iLoop < 1; iLoop++)  //only一个
        {
                //[10.0]建立连接线程
                pthread_create((pthread_t*)&piD, NULL,  pthread_connect, (void *)NULL);
                g_mapChildExitStat[piD] = 0;  //置孩子状态
                g_mapThread2pid[piD] = 0;  //threadid 2 pid
        }

        g_insLogMng.normal("Begin to create RecvThread(%d)......",  g_insConfigMng.m_tComCfg.m_iRecvNrOfThread);
        for(iLoop = 0; iLoop < g_insConfigMng.m_tComCfg.m_iRecvNrOfThread; iLoop++)
        {
                //[10.1]建立接收线程
                pthread_create((pthread_t*)&piD, NULL,  pthread_recv,  (void *)NULL);       
                g_mapChildExitStat[piD] = 0;  //置孩子状态
                g_mapThread2pid[piD] = 0;  //threadid 2 pid
        }

        g_insLogMng.normal("Begin to create P_MaintainThread(%d)......", 1);
        //[10.2.1]建立维护线程P
         pthread_create((pthread_t*)&piD, NULL, pthread_maintain_p,  (void *)NULL);        
        g_mapChildExitStat[piD] = 0;  //置孩子状态
        g_mapThread2pid[piD] = 0;  //threadid 2 pid

         g_insLogMng.normal("Begin to create R_MaintainThread(%d)......", 1);
        //[10.2.2]建立维护线程R
         pthread_create((pthread_t*)&piD, NULL, pthread_maintain_r,  (void *)NULL);        
        g_mapChildExitStat[piD] = 0;  //置孩子状态
        g_mapThread2pid[piD] = 0;  //threadid 2 pid    

        //****** BEGIN 这一块，只有当有分发队列的时候才建立，没有的时候不处理 ****** 
        if (0 != g_insConfigMng.m_tComCfg.m_iDistributeFlag)
        {
                //[10.2.3]建立维护线程F，这个属于维护线程系列，但是这个线程当没有分发任务时,无需工作
                pthread_create((pthread_t*)&piD, NULL, pthread_maintain_f,  (void *)NULL);        
                g_mapChildExitStat[piD] = 0;  //置孩子状态
                g_mapThread2pid[piD] = 0;  //threadid 2 pid    
        
                g_insLogMng.normal("Begin to create SendThread(%d)......",  g_insConfigMng.m_tComCfg.m_iSendNrOfThread);
                for(iLoop = 0; iLoop < g_insConfigMng.m_tComCfg.m_iSendNrOfThread; iLoop++)
                {
                        //[10.3]建立发送线程
                        pthread_create((pthread_t*)&piD, NULL, pthread_send,  (void *)NULL);      
                        g_mapChildExitStat[piD] = 0;  //置孩子状态
                        g_mapThread2pid[piD] = 0;  //threadid 2 pid
                }

                g_insLogMng.normal("Begin to create SendErrorThread(%d)......",  g_insConfigMng.m_tComCfg.m_iSendErrorNrOfThread);
                for(iLoop = 0; iLoop < g_insConfigMng.m_tComCfg.m_iSendErrorNrOfThread; iLoop++)
                {
                        //[10.3.2]建立发送失败线程
                        pthread_create((pthread_t*)&piD, NULL, pthread_sendError,  (void *)NULL);      
                        g_mapChildExitStat[piD] = 0;  //置孩子状态
                        g_mapThread2pid[piD] = 0;  //threadid 2 pid
                }

                g_insLogMng.normal("Begin to create CheckThread(%d)......",  1);
                 //[10.3.3]建立验证倒换线程
                pthread_create((pthread_t*)&piD, NULL, pthread_check,  (void *)NULL);      
                g_mapChildExitStat[piD] = 0;  //置孩子状态
                g_mapThread2pid[piD] = 0;  //threadid 2 pid
                
        }
        // ****** END ******       

        g_insLogMng.normal("===Init all child thread successfully===");

        //[11]主线程
        g_insLogMng.normal("%s(%u) starts successfully and is running......", g_sProgramName.c_str(), pthread_self());

        //kevinliu print screen
        printf("\n\n%s starts successfully and is running......\n", g_sProgramName.c_str());

        //重新启动时,发送一下手机短信,以防止异常重启不可知
        g_insMaintain.sendMobileMsg("系统启动成功，运行中......");

        while(1)
        {
                //判断程序是否退出
                if (1 == g_insMaintain.isExitProgram())
                {
                        g_insLogMng.normal("MainProcess recv a exit notice and forward it to its %d children.", 
                                g_mapChildExitStat.size());
                        setChildExitStat();
                        while(1)
                        {
                                //孩子们是否都已经exit
                                if(0 != checkChildExitStat())
                                {                                        
                                        CComFun::sleep_msec(0, 100);                                         
                                        continue;
                                }
                                 g_insLogMng.normal("all children exited, begin to save duty queue.");

                                //将任务队列中尚未发送的任务写入失败文件
                                if (1 == g_insMaintain.getFlag(CMaintain::SaveDutyQueueWhenExit))
                                {

                                        g_insQueueMng.saveDutyQueue2File();
                                }

                                //都完成后退出
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
 *  \brief 使进程后台以damon方式运行
 *  \return <0 错误 >=0 成功
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
 *  \brief 调用系统函数fcntl(2)给文件设置读写锁
 *
 *  fcntl(2)基本调用格式:<br>
 *      int fcntl( int fd, int cmd, struct * flock );
 *  <ul>
 *  <li> 命令:
 *      <ol>
 *      <li> F_SETLK    获取或释放flcok结构指定的锁，如果不成功立即返回EACCES或EAGAIN
 *      <li> F_SETLKW   获取或释放flock结构指定的锁，如果不成功则调用进程被阻塞
 *      <li> F_GETLK    获取flock结构指定的锁是否存在，以及相关进程pid，锁类型的信息
 *      </ol>
 *  <li> struct flock           //文件锁参数结构
 *      <ul>
 *      <li> short l_type;      //F_RDLCK, F_WRLCK, F_UNLCK 分别是读锁，写锁，无锁
 *      <li> short l_whence;    //SEEK_SET, SEEK_CUR, SEEK_END 文件偏移起始计算位置，文件头，当前，文件尾
 *      <li> off_t l_start;     //relative starting offset in bytes,偏移字节数
 *      <li> off_t l_len;       //#bytes; 0 means until end-of-file, 锁的长度，0为整个文件
 *      <li> pid_t l_pid;       //pid returned by F_GETLK, 在使用F_GETLK命令时返回相关锁进程的pid
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
 *  \brief 控制进程仅有一个实例运行的函数,使用一个文件写锁，进程退出，锁自然释放
 *  \param filepath 由于建立文件锁的文件名,该文件将被截断,并被填入进程id
 *  \param appname  运行程序的名称,一般由argv[0]得到
 *  \return 0 执行加锁成功 如果加锁不成功则, 当前进程被强制中止
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
 *  \brief 信号处理函数
 *  \brief 进程收到的信号
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
                /*屏蔽所有可能的信号*/
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
        //小睡一会,等待主进程初始化所有线程,100ms足够
        CComFun::sleep_msec(0, 100);
        
        //用来处理客户端的连接
        char sClientIP[LenOfIPAddr];
        int iFD;
        int iMyID = (int)pthread_self();
        g_mapChildExitStat[iMyID] = 0;  //置自己状态
        g_mapThread2pid[iMyID] = (int)getpid();  //threadid 2 pid
        g_insLogMng.normal("ConnectthreadID = %u PID = %d" , (unsigned int)iMyID, getpid());
        int iWaitTime = 1; // 1s
         
        //主循环
        while(1)
        {
                // [1]是否该退出系统了
                if (0 != checkMyExitStat(iMyID))
                {
                        g_insLogMng.normal("ConnectThread(%u) recv an exit notice, so exit.",  (unsigned int)iMyID);
                        setMyExitStat(iMyID, 0);
                        break;
                }

                // [2]接收客户端的连接 
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
                 else if (900 < iFD)  //基本限制是fd最大为1024，这个判断一下，如果fd超过了900，则报一下警，以防止fd泄露无法警觉
                 {
                        if (0 != access((g_sHomeDir + "/cfg/fd.if").c_str(),  F_OK))  //不存在就报警，如果确认系统可以支持到1000，可天添加此文件不来报警
                        {
                                g_insMaintain.sendMobileMsg("建立连接文件描述符FD>900,系统已经自动清理前500个，请检查系统是否有FD泄露。");
                                //for(int ixx = 10; ixx < 500; ixx++)
                                //{
                                //        close(ixx); //为了防止系统导致的故障，这儿强行加入这个逻辑，如果确认不需要，请在cfg添加fd.if文件
                                //}
                        }
                        //写alarm跟踪文件 [ALARM:4]
                        CComFun::writeWholeFileContent((g_sHomeDir + "/dat/maxfd.alm"), "fd>900");                        
                 }
                 //else 不再在这里删除，而是监控脚本根据文件的modify是否已经更新来判断
                 //{
                 //       CComFun::delFile(g_sHomeDir + "/dat/maxfd.alm"); 
                 //}
                
                CTCPSocket insClientSocket(iFD, g_insServerSocket.m_sIP, g_insServerSocket.m_iPort);

                // [3] 判断sClientIP是否在LoginIP之内,不在则返回，不做很严格的判断，只做一个简单字符查找
                char *pTmp = strstr(g_insConfigMng.m_tComCfg.m_sLoginIP,  sClientIP);
                if (NULL == pTmp)
                {
                        g_insLogMng.normal("Recv a connection from IP(%s), it isn't in common.cfg(LoginIP), so refuse it.",
                                sClientIP);
                        insClientSocket.closeSocket();  //关闭
                        CComFun::sleep_msec(0, 1);
                        continue;
                }
                //else  //连接成功，打印一下日志信息，一遍跟踪
                //{
                //        g_insLogMng.debug("all", CLogMng::LogMode1, 
                //                "Accept a connection from IP(%s) successfully.", sClientIP);
                //}

                // [4]向队列中加连接
                g_insConCtrl.setLock();  //加锁
                if (0 != g_insConfigMng.m_tComCfg.m_iMaxNrOfCnt
                        && g_lstConnectPool.size() >= (unsigned int)g_insConfigMng.m_tComCfg.m_iMaxNrOfCnt)
                {
                        g_insConCtrl.unLock(); //解锁
                        g_insLogMng.normal("NrOfConnection has reached max = %d,  so refuse a new connection.",
                                g_insConfigMng.m_tComCfg.m_iMaxNrOfCnt);
                        insClientSocket.closeSocket();  //关闭
                        CComFun::sleep_msec(0, 1);
                        continue;
                }
                g_lstConnectPool.push_back(insClientSocket);
                g_insConCtrl.unLock(); //解锁

                //sleep等待下一次
                CComFun::sleep_msec(0, 1);
        }
        return (void*)NULL;
}

void* pthread_recv(void *p)
{
        //小睡一会,等待主进程初始化所有线程,100ms足够
        CComFun::sleep_msec(0, 100);
        
        //用来处理客户端的连接
        string sClientIP;
        int iMyID = pthread_self();
        g_mapChildExitStat[iMyID] = 0;  //置自己状态
        g_insQueueMng.initThreadStatInfo(0, iMyID);
        g_mapThread2pid[iMyID] = (int)getpid();  //threadid 2 pid
        g_insLogMng.normal("RecvthreadID = %u PID = %d" , (unsigned int)iMyID, getpid());
        
        //主循环
        while(1)
        {
                // [1]是否该退出系统了
                if (0 != checkMyExitStat(iMyID))
                {
                        g_insLogMng.normal("RecvThread(%u) recv an exit notice, so exit.",  (unsigned int)iMyID);
                        setMyExitStat(iMyID, 0);
                        break;
                }

                // [2]取连接
                g_insConCtrl.setLock(); //加锁
                if (0 == g_lstConnectPool.size())
                {
                        g_insConCtrl.unLock();
                        CComFun::sleep_msec(0, 1);
                        continue;
                }
                list<CTCPSocket>::iterator it = g_lstConnectPool.begin();
                CTCPSocket insClientSocket = *it;
                g_lstConnectPool.erase(it);
                g_insConCtrl.unLock();   //解锁

                // [3] 有消息进入协议处理
                CProtocolMng::processRecv(insClientSocket);

                insClientSocket.closeSocket();

                //sleep等待下一次
                CComFun::sleep_msec(0, 1);
        }
        return (void*)NULL;
}

void* pthread_send(void *p)
{
        //小睡一会,等待主进程初始化所有线程,100ms足够
        CComFun::sleep_msec(0, 100);
        
        int iMyID = pthread_self();
        g_mapChildExitStat[iMyID] = 0;  //置自己状态
        g_insQueueMng.initThreadStatInfo(1, iMyID);
        g_mapThread2pid[iMyID] = (int)getpid();  //threadid 2 pid
        g_insLogMng.normal("SendthreadID = %u PID = %d" , (unsigned int)iMyID, getpid());
                
        while(1)
        {
                //是否该退出系统了
                if (0 != checkMyExitStat(iMyID))
                {
                        g_insLogMng.normal("SendThread(%u) recv an exit notice, so exit.",  (unsigned int)iMyID);
                        setMyExitStat(iMyID, 0);
                        break;
                }

                //处理发送任务
                CProtocolMng::processSend(0);

                //sleep等待下一次
                CComFun::sleep_msec(0, 1);
        }
        return (void*)NULL;
}


void* pthread_sendError(void *p)
{
        //小睡一会,等待主进程初始化所有线程,100ms足够
        CComFun::sleep_msec(0, 100);
        
        int iMyID = pthread_self();
        g_mapChildExitStat[iMyID] = 0;  //置自己状态
        g_insQueueMng.initThreadStatInfo(1, iMyID);
        g_mapThread2pid[iMyID] = (int)getpid();  //threadid 2 pid
        g_insLogMng.normal("SendErrorthreadID = %u PID = %d" , (unsigned int)iMyID, getpid());
                
        while(1)
        {
                //是否该退出系统了
                if (0 != checkMyExitStat(iMyID))
                {
                        g_insLogMng.normal("ErrorSendThread(%u) recv an exit notice, so exit.",  (unsigned int)iMyID);
                        setMyExitStat(iMyID, 0);
                        break;
                }

                //处理发送任务
                CProtocolMng::processSend(1);

                //sleep等待下一次
                CComFun::sleep_msec(0, 1);
        }
        return (void*)NULL;
}

void* pthread_maintain_p(void *p)
{
        //小睡一会,等待主进程初始化所有线程,100ms足够
        CComFun::sleep_msec(0, 100);
        
        int iMyID = pthread_self();
        g_mapChildExitStat[iMyID] = 0;  //置自己状态
        g_insQueueMng.initThreadStatInfo(0, iMyID);  //失败任务重新加载的时候，算作接受任务
        g_mapThread2pid[iMyID] = (int)getpid();  //threadid 2 pid
        g_insLogMng.normal("P_MaintainthreadID = %u PID = %d" , (unsigned int)iMyID, getpid());
        
        while(1)
        {
                //是否该退出系统了
                if (0 != checkMyExitStat(iMyID))
                {
                        g_insLogMng.normal("P_MaintainThread(%u) recv an exit notice, so exit.",  (unsigned int)iMyID);
                        setMyExitStat(iMyID, 0);
                        break;
                }

                //功能处理
                g_insMaintain.process();

                //sleep等待下一次
                CComFun::sleep_msec(1, 0);
        }
        return (void*)NULL;
}

void* pthread_maintain_r(void *p)
{
        //小睡一会,等待主进程初始化所有线程,100ms足够
        CComFun::sleep_msec(0, 100);
        
        int iMyID = pthread_self();
        g_mapChildExitStat[iMyID] = 0;  //置自己状态
        g_insQueueMng.initThreadStatInfo(0, iMyID);  //失败任务重新加载的时候，算作接受任务
        g_mapThread2pid[iMyID] = (int)getpid();  //threadid 2 pid
        g_insLogMng.normal("R_MaintainthreadID = %u PID = %d" , (unsigned int)iMyID, getpid());
        
        while(1)
        {
                //是否该退出系统了
                if (0 != checkMyExitStat(iMyID))
                {
                        g_insLogMng.normal("R_MaintainThread(%u) recv an exit notice, so exit.",  (unsigned int)iMyID);
                        setMyExitStat(iMyID, 0);
                        break;
                }

                 //例行处理
                g_insMaintain.routine();

                //sleep等待下一次
                CComFun::sleep_msec(1, 0);
        }
        return (void*)NULL;
}

void* pthread_maintain_f(void *p)
{
        //小睡一会,等待主进程初始化所有线程,100ms足够
        CComFun::sleep_msec(0, 100);
        
        int iMyID = pthread_self();
        g_mapChildExitStat[iMyID] = 0;  //置自己状态
        g_insQueueMng.initThreadStatInfo(0, iMyID);  //失败任务重新加载的时候，算作接受任务
        g_mapThread2pid[iMyID] = (int)getpid();  //threadid 2 pid
        g_insLogMng.normal("F_MaintainthreadID = %u PID = %d" , (unsigned int)iMyID, getpid());
        
        while(1)
        {
                //是否该退出系统了
                if (0 != checkMyExitStat(iMyID))
                {
                        g_insLogMng.normal("F_MaintainThread(%u) recv an exit notice, so exit.",  (unsigned int)iMyID);
                        setMyExitStat(iMyID, 0);
                        break;
                }

                //例行处理,也是为了防止处理失败任务时，出现并发冲突
                g_insMaintain.routineFailDuty();

                //sleep等待下一次
                CComFun::sleep_msec(1, 0);
        }
        return (void*)NULL;
}

void* pthread_check(void *p)
{
        //小睡一会,等待主进程初始化所有线程,100ms足够
        CComFun::sleep_msec(0, 100);
        
        int iMyID = pthread_self();
        g_mapChildExitStat[iMyID] = 0;  //置自己状态
        g_insQueueMng.initThreadStatInfo(0, iMyID);  //失败任务重新加载的时候，算作接受任务
        g_mapThread2pid[iMyID] = (int)getpid();  //threadid 2 pid
        g_insLogMng.normal("CheckthreadID = %u PID = %d" , (unsigned int)iMyID, getpid());
        
        while(1)
        {
                //是否该退出系统了
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
                        //队列转移
                        g_insQueueMng.mvErrorQ2SendQ();
                }

                //清理失效的任务，可配置
                if(1 == iClearD)
                {
                        g_insQueueMng.clearDisabledDuty(); 
                }

                //等待时间，正常情况下修改足够长的时间                
                if (iSafe == 0 && iClearD == 0)
                {
                        CComFun::sleep_msec(20, 0);  // 20s，其实还可以更长，修改之只是为了响应退出信号更快一些
                }
                else
                {
                        CComFun::sleep_msec(0, 10); 
                }
        }
        return (void*)NULL;
}

