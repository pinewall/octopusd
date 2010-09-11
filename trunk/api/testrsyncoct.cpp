#include "RsyncOct.h"
#include <unistd.h>
using namespace std;
using namespace CGIMAGIC;
void
exiting (void)
{
  MAGIC_LOG_CLOSE ();
} int

main ()
{
  MAGIC_LOG_INIT (("/tmp/testRsyncOct"));
  atexit (exiting);
  CRsyncOct insRsyncOct;
  string sServerIP = "127.0.0.1";
  int iPort = 8123;

  //测试4种打包情况
  //(1) 本地普通文件
#if 0
  insRsyncOct.uploadPackFile (sServerIP, iPort, CRsyncOct::FileName,
			      "news", "/test_d/",
			      "/home/kevinliu/test_d/a.txt");

#endif /*  */
#if 0
  //(2) 本地目录
  insRsyncOct.uploadPackFile (sServerIP, iPort, CRsyncOct::FileName,
			      "news", "/test_d/", "/home/kevinliu/test_d/");

#endif /*  */
/*
        #if 1
        //(3) 本地tar.gz
        insRsyncOct.uploadPackFile(sServerIP, iPort, CRsyncOct::FileName,
                "news", "/test_d/", "/home/kevinliu/test_d/b.tar.gz");
        #endif
        #if 1
        //(4) 本地 .zip
        insRsyncOct.uploadPackFile(sServerIP, iPort, CRsyncOct::FileName,
                "news",  "/test_d/", "/home/kevinliu/test_d/c.zip");
         #endif
 */
  for (int i = 0; i < 1; ++i)
    {

      //(2) Upload File
      insRsyncOct.uploadFile (sServerIP.c_str (), iPort,
			      CRsyncOct::FileName, "news",
			      "/pw_test_20060425_2.txt",
			      "/home/bud/SiteWizard/newoctopusd/api/a.txt");
      insRsyncOct.uploadFile (sServerIP.c_str (), iPort,
			      CRsyncOct::FileName, "sports",
			      "/pw_test_20060425_3.txt",
			      "/home/bud/SiteWizard/newoctopusd/api/a.txt");
      insRsyncOct.uploadFile (sServerIP.c_str (), iPort,
			      CRsyncOct::FileName, "sports",
			      "/pw_test_20060425_4.txt",
			      "/home/bud/SiteWizard/newoctopusd/api/a.txt");

      //(3) Delete File
      insRsyncOct.deleteFile (sServerIP.c_str (), iPort, "sports",
			      "/pw_test_20060425_3.txt");

      //(4) Move File
      insRsyncOct.mvFile (sServerIP.c_str (), iPort, "news",
			  "/pw_test_20060425_2.txt", "sports",
			  "/pw_test_20060425_2.txt");

      //(5) Copy File
      insRsyncOct.cpFile (sServerIP.c_str (), iPort, "sports",
			  "/pw_test_20060425_4.txt", "news",
			  "/pw_test_20060425_6.txt");

      //usleep(1000); // sleep 10e-3s
    } return 0;
}
