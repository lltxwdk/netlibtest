#ifndef _TRACE_LOGE__H_
#define _TRACE_LOGE__H_
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include "TypeDef.h"
#include "CriticalSection.h"
#define TL_EMERG      7
#define TL_ALERT      6
#define TL_CRIT       5
#define TL_ERR        4
#define TL_WARNING    3
#define TL_NOTICE     2
#define TL_INFO       1
#define TL_DEBUG      0

class TraceLog
{
public:
    static TraceLog& GetInstance();
    bool OpenLog ( const char* filename, int min_loglevel, bool is_stdout );
    void EnabledFile(bool able);
    void EnabledStdout(bool able);
    void EnabledTotalLog(bool able);
    void SetLogLevel(int level);
    void CloseLog();
	void AppCloseLog();
    void TraceLogging (const char* curr_file, int line_no, int loglevel, const char * format, ...);
	void TraceLogging_withoutnewline ( const char *curr_file, int line_no, int loglevel, const char *format, ... );

    //other way to logging
    void TotalLog(const char* file_name, const char * format, ...);

    const char *GetFileName(const char *full_name);

private:
    FILE* log_file_;
    int min_log_level_;
    bool print_stdout_;
    bool file_able_;
    char file_name_[1024];
    struct tm tm_log_date_;
    CCriticalSection file_cs_;
    bool total_log_enable_;
private:
    TraceLog() 
    {
        min_log_level_ = 0;
        log_file_ = NULL;
        print_stdout_ =false;
        file_able_ = false;
        total_log_enable_ = false;
        memset(file_name_, 0, sizeof(file_name_));
        memset((void*)&tm_log_date_, 0, sizeof(tm_log_date_));
    }
    TraceLog ( const TraceLog& );

	~TraceLog();

    const char* GetLogleveInfo(int loglevel);
    void CheckReopenFile(struct tm * tm);
};
#define LOG(level, format, ...) TraceLog::GetInstance().TraceLogging(__FILE__, __LINE__, level, format, ##__VA_ARGS__)
#define LogEMERG(format, ...) TraceLog::GetInstance().TraceLogging(__FILE__, __LINE__, TL_EMERG, format, ##__VA_ARGS__)
#define LogALERT(format, ...) TraceLog::GetInstance().TraceLogging(__FILE__, __LINE__, TL_ALERT, format, ##__VA_ARGS__)
#define LogCRIT(format, ...) TraceLog::GetInstance().TraceLogging(__FILE__, __LINE__, TL_CRIT, format, ##__VA_ARGS__)
#define LogERR(format, ...) TraceLog::GetInstance().TraceLogging(__FILE__, __LINE__, TL_ERR, format, ##__VA_ARGS__)
#define LogWARNING(format, ...) TraceLog::GetInstance().TraceLogging(__FILE__, __LINE__, TL_WARNING, format, ##__VA_ARGS__)
#define LogNOTICE(format, ...) TraceLog::GetInstance().TraceLogging(__FILE__, __LINE__, TL_NOTICE, format, ##__VA_ARGS__)
#define LogINFO(format, ...) TraceLog::GetInstance().TraceLogging(__FILE__, __LINE__, TL_INFO, format, ##__VA_ARGS__)
#define LogDEBUG(format, ...) TraceLog::GetInstance().TraceLogging(__FILE__, __LINE__, TL_DEBUG, format, ##__VA_ARGS__)

#endif
