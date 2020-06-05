#pragma once

#ifdef WIN32
#include <process.h>    /* _beginthread, _endthread */
#else
#include <pthread.h>	
#include <errno.h> 
#include <algorithm>
#endif
#include <string>
#include "TypeDef.h"
#include "CriticalSection.h"

#if defined(ANDROID) || defined(LINUX)
#include <sys/prctl.h>
#endif

class CBaseThread
{
public:
  CBaseThread();
  ~CBaseThread();
public:
    typedef enum tagThreadPriority 
    {
        kLowPriority = 1,
        kNormalPriority = 2,
        kHighPriority = 3,
        kHighestPriority = 4,
        kRealtimePriority = 5
    }ThreadPriority;

public:
  //Start Thread
    uint8_t BeginThread(uint32_t(*apStartFunc)(void *), void *apParam, unsigned int& uiThreadID,char *thread_name,ThreadPriority priority = kNormalPriority);

  //Thread Sleep
  void static Sleep(uint32_t dwMilliseconds);

  //Get system millisecond
  uint64_t static GetSystemTime();

  //Close Thread
  //BOOL Close(WORD awWaitMilliSecond);

  bool IsRunning();

#ifndef _WIN32
  void Join();
#endif

  CCriticalSection &getCriticalSection() { return m_critical_section; }
  void SetThreadName(std::string name)
  {
	  thread_name = name;
  }
  std::string GetThreadName()
  {
	  return thread_name;
  }

public:

	
private:
  struct STRU_THREAD_INFO
  {
    void * mpUserParam;
    uint32_t(*mpThreadFunc)(void *);
  };

  STRU_THREAD_INFO thread_info_;
  bool running_;

  CCriticalSection m_critical_section;

#ifdef _WIN32
  uintptr_t m_hThreadHandle;
  static unsigned int __stdcall Win32ThreadFunc(void * apParam);
#else
  pthread_t m_thread;
  pthread_cond_t m_cond_t;

  static void * LinuxThreadFunc(void * lpParam);
  int ConvertToLinuxSystemPriority(ThreadPriority priority, int min_prio, int max_prio);
#endif

  //目前只测试了Linux 和 windows
  bool SetPriority(ThreadPriority priority);
  std::string GetPriorityInfo(ThreadPriority priority);
  std::string thread_name;
};


