#ifndef __TRACE_HH__
#define __TRACE_HH__

#include <stdarg.h>
#include <string>

//default log file size 
#define MAX_LOG_SIZE  1000000 
//default backup file number.
#define MAX_LOG_NUM   5



#ifdef DEBUG 
//debug mode can log trace info. 
// for example :MAGIC_TRACE(("test trace."));
//           or:MAGIC_TRACE(("test trace:%d %s",10,"test"));

#define MAGIC_TRACE(X) \
   do { \
	Magic_Trace *magic__ = Magic_Trace::instance(); \
	magic__->setTraceInfo(__LINE__,__FILE__);  \
	magic__->WriteTrace  X; \
    }while(0)
#define  MAGIC_BIN_TRACE(X) \
   do {  \
	Magic_Trace *magic__ = Magic_Trace::instance(); \
	magic__->setTraceInfo(__LINE__,__FILE__);  \
	magic__->WriteBinTrace X;  \
    }while(0)

#else 
#define MAGIC_TRACE(X) \
    do {} while(0) 
#define MAGIC_BIN_TRACE(X) \
    do {} while(0) 
#endif

//when process exit ,you should invoke this macro 
// to cleanup Magic_Trace instance.
#define MAGIC_LOG_CLOSE() \
  do {   \
  	Magic_Trace *magic__ = Magic_Trace::instance(); \
  	magic__->close(); \
     }while(0)

//when you start your process,if you want to log and 
//record trace info,you should invoke this macro to 
//initialize your Magic_Trace instance.
#define MAGIC_LOG_INIT(X) \
    do { \
	Magic_Trace *magic__ = Magic_Trace::instance(); \
        magic__->Init X ; \
    } while(0)  
 
//use this macro to record log information.
#define  MAGIC_LOG(X) \
  do {  \
        Magic_Trace  *magic__ = Magic_Trace::instance(); \
	magic__->WriteLog  X;   \
    }while(0) 
          	

/**
 * Magic_Trace is used to write trace information.
 * using static mode write to only at one file.
 */
class Magic_Trace {

public:

   enum  {
	TRACE_SIZE = 10*1024  
   }; 
  /**
   *singleton instance.
   *return instance of Magic_Trace.
   */
   static Magic_Trace* instance(void);
   
  /**
   *shutdown method used to clean instance.
   */
   static void close(void);
  

   /**
    * Use to Initialize Magic_Trace param.
    * @param file indicates trace or log file include fullpath.
    * @param logsize trace or log file size.
    * @param lognum trace or log backup file number.
    * @return 0 on success.
    *      -1 on failure.
    */
   int Init(const std::string& file,
	long  logsize = MAX_LOG_SIZE,
        int   lognum  = MAX_LOG_NUM );
        
        
   //destructor
   virtual ~Magic_Trace();


  /** 
   * Used to write trace msg.
   * @param sFormat is the trace format.
   * @param ... trace information.
   */
   void WriteTrace(const char* sFormat,...);

  /**
   * Used to write msg using hex  format. 
   * @param sFormat indicates trace format. 
   */
  void WriteBinTrace(const char *buf, int size);

  /**
   *  Used to write log msg.
   * @param sFormat is the log format.
   * @param ... log information.
   */
   void WriteLog(const char* sFormat,...);


  /**
   * Used to set trace information.
   *@param lLine indicates the line trace logged.
   *@param sFile indicates the file trace logged.  
   */
   void setTraceInfo(long lLine,const char*sFile);
   
private:


  //contructor
   Magic_Trace(void);


   /**
    * Used to log message to file.
    */
  int WriteMsg( 
      char *sErrMsg, 
      const char *sFormat,
      va_list ap
     );


    /**
     * Used to change log/trace file.
     * @param sLogBaseName ,log or trace filename.
     * @param lMaxLogSize ,max log size.
     * @param iMaxLogNum, max log file number.for backup.
     * @param sErrMsg err message.
     * @return 0 on success,
     *          -1 on failure.
     */
    int ShiftFiles(
	char *sErrMsg
    );
   
   /**
    * Used to get current date time.
    */
   char*  GetCurDateTimeStr(void);
     
   std::string m_sfile;
   long m_lLogSize;
   int  m_iLogNum;

   //trace info 
   long m_lLine; 
   std::string m_tracefile; 
   
   static Magic_Trace *m_trace;
};

#endif // __Magic_TRACE_H__



