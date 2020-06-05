#include <stdio.h>
#include <string.h>
#include "TraceLog.h"
#include "Utils.h"
#include <sys/timeb.h>
#ifdef ANDROID
#include <android/log.h>
#endif

#ifdef IOS
int GetIOSDocDirectory(char* data, const int data_len);
#endif

TraceLog &TraceLog::GetInstance()
{
    static TraceLog trace_log;
    return trace_log;
}

TraceLog::~TraceLog()
{
	CloseLog();
}

bool TraceLog::OpenLog(const char* filename, int min_loglevel, bool is_stdout)
{
    print_stdout_ = is_stdout;
    min_log_level_ = min_loglevel;
    memset(file_name_, 0, sizeof(file_name_));
    if (NULL != filename)
    {
        file_able_ = true;
        memcpy(file_name_, filename, strlen(filename));
    }
    
    return true;
}

void TraceLog::EnabledFile(bool able)
{
    file_able_ = able;
}

void TraceLog::EnabledStdout(bool able)
{
    print_stdout_ = able;
}

void TraceLog::EnabledTotalLog(bool able)
{
    total_log_enable_ = able;
}

void TraceLog::SetLogLevel(int level)
{
    min_log_level_ = level;
}

void TraceLog::CloseLog()
{
    if (log_file_)
    {
        fclose(log_file_);
        log_file_ = NULL;
    }
}

void TraceLog::AppCloseLog()
{
	CCriticalAutoLock nAuto(file_cs_);
	file_able_ = false;
	if (log_file_)
	{
		fclose(log_file_);
		log_file_ = NULL;
	}
	memset((void*)&tm_log_date_, 0, sizeof(tm_log_date_));
	memset(file_name_, 0, sizeof(file_name_));
	
}
void TraceLog::TraceLogging(const char *curr_file, int line_no, int loglevel, const char *format, ...)
{
    if (NULL == curr_file || NULL == format)
    {
        return;
    }

    if (loglevel < min_log_level_)
    {
        return;
    }

    char log_head[100] = { 0 };
    const char* curr_file_name = GetFileName(curr_file);
    struct timeb tp;
    struct tm * tm;
    ftime(&tp);
    tm = localtime(&(tp.time));
    if (NULL == curr_file_name)
    {
        sprintf(log_head, "%02d:%02d:%02d:%03d [%s] ",
            tm->tm_hour, tm->tm_min, tm->tm_sec, tp.millitm,
            GetLogleveInfo(loglevel));
    }
    else
    {
#if defined(WIN32)
        DWORD tid = GetCurrentThreadId();
        sprintf(log_head, "%02d:%02d:%02d:%03d [%u] [%s] [%s:%d] ",
            tm->tm_hour, tm->tm_min, tm->tm_sec, tp.millitm, tid,
            GetLogleveInfo(loglevel), curr_file_name, line_no);
#elif defined(LINUX)
       sprintf(log_head, "%02d:%02d:%02d:%03d [%lx] [%s] [%s:%d] ",
           tm->tm_hour, tm->tm_min, tm->tm_sec, tp.millitm, pthread_self(),
            GetLogleveInfo(loglevel), curr_file_name, line_no);
#else
        sprintf(log_head, "%02d:%02d:%02d:%03d [%s] [%s:%d] ",
            tm->tm_hour, tm->tm_min, tm->tm_sec, tp.millitm,
            GetLogleveInfo(loglevel), curr_file_name, line_no);
#endif
    }

    if (file_able_)
    {
        CheckReopenFile(tm);
    }

    if (file_able_)
    {
        if (log_file_)
        {
            CCriticalAutoLock nAuto(file_cs_);
            fprintf(log_file_, "%s", log_head);
            va_list file_aplist;
            va_start(file_aplist, format);
            vfprintf(log_file_, format, file_aplist);
            fprintf(log_file_, "\n");
            fflush(log_file_);
            va_end(file_aplist);
        }
    }

    if (print_stdout_)
    {
        va_list stdout_aplist;
        va_start(stdout_aplist, format);
#ifdef ANDROID
        __android_log_vprint(ANDROID_LOG_DEBUG, "TraceLog", format, stdout_aplist);
#else
        vprintf(log_head, stdout_aplist);
        vprintf(format, stdout_aplist);
        printf("\n");
#endif
        va_end(stdout_aplist);
    }
}

void TraceLog::TraceLogging_withoutnewline(const char *curr_file, int line_no, int loglevel, const char *format, ...)
{
    if (NULL == curr_file || NULL == format)
    {
        return;
    }

    if (loglevel < min_log_level_)
    {
        return;
    }

    char log_head[128] = { 0 };
    struct tm * tm;
#ifndef LINUX
    struct timeb tp;
    ftime(&tp);
    tm = localtime(&(tp.time));
    sprintf(log_head, "%02d:%02d:%02d:%03d [%s] ",
        tm->tm_hour, tm->tm_min, tm->tm_sec, tp.millitm,
        GetLogleveInfo(loglevel));
#else
    time_t nowtime;
    struct timespec nowspec;   
    ::clock_gettime(CLOCK_MONOTONIC, &nowspec);
    nowtime = ::time(NULL);
    tm = ::localtime(&nowtime);	
    snprintf(log_head,sizeof(log_head),"%02d:%02d:%02d(%ld.%ld)[%s] ",
        tm->tm_hour,tm->tm_min,tm->tm_sec,nowspec.tv_sec,nowspec.tv_nsec,
        GetLogleveInfo(loglevel));
#endif

    if (file_able_)
    {
        CheckReopenFile(tm);
    }

    if (log_file_ && file_able_)
    {
        CCriticalAutoLock nAuto(file_cs_);
        fprintf(log_file_, "%s", log_head);
        va_list file_aplist;
        va_start(file_aplist, format);
        vfprintf(log_file_, format, file_aplist);
        fflush(log_file_);
        va_end(file_aplist);
    }

    if (print_stdout_)
    {
        va_list stdout_aplist;
        va_start(stdout_aplist, format);
#ifdef ANDROID
        __android_log_vprint(ANDROID_LOG_DEBUG, "TraceLog", format, stdout_aplist);
#else
        vprintf(log_head, stdout_aplist);
        vprintf(format, stdout_aplist);
#endif
        va_end(stdout_aplist);
    }
}


void TraceLog::TotalLog(const char* file_name, const char * format, ...)
{
    if (file_name == NULL || format == NULL)
    {
        return;
    }
    if (!total_log_enable_)
    {
        return;
    }
#ifdef WIN32
    FILE *fp = NULL;
    fp = fopen(file_name, "a+");
    if (NULL != fp) {
        time_t now = time(NULL);
        struct tm tm_now;
        localtime_s(&tm_now, &now);
        fprintf(fp, "%d-%d-%d_%d:%d:%d ",
            tm_now.tm_year + 1900, tm_now.tm_mon + 1, tm_now.tm_mday,
            tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec);
        va_list apptr;
        va_start(apptr, format);
        vfprintf(fp, format, apptr);
        fclose(fp);
    }
#elif defined(OSX) || defined(IOS)
    time_t now = time(NULL);
    struct tm tm_now;
    localtime_r(&now, &tm_now);
    fprintf(stdout, "[%s]%d-%d-%d_%d:%d:%d ", file_name,
        tm_now.tm_year + 1900, tm_now.tm_mon + 1, tm_now.tm_mday,
        tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec);
    va_list apptr;
    va_start(apptr, format);
    vfprintf(stdout, format, apptr);
    fflush(stdout);
    va_end(apptr);
#elif defined(LINUX)
    time_t now = time(NULL);
    struct tm tm_now;
    localtime_r(&now, &tm_now);
    fprintf(stdout, "[%s]%d-%d-%d_%d:%d:%d ", file_name,
        tm_now.tm_year + 1900, tm_now.tm_mon + 1, tm_now.tm_mday,
        tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec);
    va_list apptr;
    va_start(apptr, format);
    vfprintf(stdout, format, apptr);
    fflush(stdout);
    va_end(apptr);
#elif defined (ANDROID)
    va_list apptr;
    va_start(apptr, format);
    __android_log_vprint(ANDROID_LOG_DEBUG, file_name, format, apptr);
    va_end(apptr);
#endif
}

const char* TraceLog::GetLogleveInfo(int loglevel)
{
    switch (loglevel)
    {
    case 0: return "DBG";
    case 1: return "INF";
    case 2: return "NTC";
    case 3: return "WAR";
    case 4: return "ERR";
    case 5: return "CRT";
    case 6: return "ALT";
    case 7: return "ERG";
    default:return "UKN";
    }
}

void TraceLog::CheckReopenFile(struct tm * tm)
{
    if (strlen(file_name_) <= 0)
    {
        return;
    }

    if (tm_log_date_.tm_mday == tm->tm_mday)
    {
        return;
    }
    tm_log_date_ = *tm;
    char now_date[50] = { 0 };
    sprintf(now_date, "%d-%02d-%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
    CCriticalAutoLock nAuto(file_cs_);
    CloseLog();
    char temp_file[1024] = { 0 };
#ifdef ANDROID
    sprintf(temp_file, "%s-%s", file_name_, now_date);
    log_file_ = fopen(temp_file, "a+");
#elif defined IOS
    char work_dir[900] = { 0 };
    GetIOSDocDirectory(work_dir, sizeof(work_dir));
    sprintf(temp_file, "%s/%s-%s", work_dir, file_name_, now_date);
    log_file_ = fopen(temp_file, "a+");
#else
    sprintf(temp_file, "%s-%s", file_name_, now_date);
    log_file_ = fopen(temp_file, "a+");
#endif
}

const char *TraceLog::GetFileName(const char *full_name)
{
    const char *target_file_name = NULL;
#ifdef _WIN32
    target_file_name = strrchr(full_name, '\\');
#else 
    target_file_name = strrchr(full_name, '/');
#endif

    if (target_file_name != NULL && strlen(target_file_name) > 1)
    {
        return target_file_name + 1;
    }

    return target_file_name;
}

