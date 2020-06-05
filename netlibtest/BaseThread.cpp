#include "BaseThread.h"
#include <sys/types.h> 
#include <sys/timeb.h>
#include <assert.h>
#include <signal.h>

#if defined(ANDROID)
#include "ftime_android_ndk_r14.h"
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBaseThread::CBaseThread()
  :running_(false)
  , thread_name("")
{
#ifndef _WIN32
  pthread_cond_init(&m_cond_t, NULL);
#endif
}

CBaseThread::~CBaseThread()
{
#ifndef _WIN32
  pthread_cond_destroy(&m_cond_t);
#endif
}

uint8_t CBaseThread::BeginThread(uint32_t(*apStartFunc)(void *), void *apParam, unsigned int& uiThreadID, char *thread_name,ThreadPriority priority)
{
  //     STRU_THREAD_INFO *lpThreadInfo = NULL;
  //     lpThreadInfo = new struct STRU_THREAD_INFO;
  //     if (NULL == lpThreadInfo)
  //     {
  //         return 0;
  //     }
  if (running_) {
    return 0;
  }

  thread_info_.mpUserParam = apParam;
  thread_info_.mpThreadFunc = apStartFunc;
  thread_name = thread_name;

#ifdef WIN32

  SetThreadName(thread_name);

  uiThreadID = 0;
  m_hThreadHandle = _beginthreadex(NULL, 0, Win32ThreadFunc, this, 0, &uiThreadID);
  if (-1 == m_hThreadHandle) {
    return 0;
  }

  SetPriority(priority);

  //TRACE4("CBaseThread::BeginThread, luThreadID = %u !\n", uiThreadID);
  CloseHandle((HANDLE)m_hThreadHandle);
  return 1;

#else
  sigset_t signal_mask;
  sigemptyset(&signal_mask);
  sigaddset(&signal_mask, SIGPIPE);
  int rc = pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);
  if (rc != 0) {
    //LogERR("block sigpipe error\n");
  }
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  SetThreadName(thread_name);

  if (0 != pthread_create(&m_thread, &attr, LinuxThreadFunc, this)) {
    return 0;
  }

  SetPriority(priority);
  //uiThreadID = (unsigned int)m_thread;
  //wait for create thread finish
  {
    CCriticalAutoLock l(m_critical_section);
    while (!running_) {
      pthread_cond_wait(&m_cond_t, m_critical_section.getCriticalSection());
    }
  }
  return 1;

#endif
}

#ifdef WIN32
unsigned int CBaseThread::Win32ThreadFunc(void * lpParam)
{
  CBaseThread *th = (CBaseThread *)lpParam;
  STRU_THREAD_INFO *lpThreadInfo = &th->thread_info_;

  //ASSERT(lpThreadInfo != NULL);
  //ASSERT(lpThreadInfo->mpThreadFunc != NULL);

  th->running_ = true;

  lpThreadInfo->mpThreadFunc(lpThreadInfo->mpUserParam);

  //if (lpThreadInfo != NULL)
  //{
  //	delete lpThreadInfo;
  //	lpThreadInfo = NULL;
  //}
  th->running_ = false;
  _endthreadex(0);
  return 1;
}

#else

void * CBaseThread::LinuxThreadFunc(void * lpParam)
{
  CBaseThread *th = (CBaseThread *)lpParam;
  struct STRU_THREAD_INFO *lpThreadInfo = &th->thread_info_;//(struct STRU_THREAD_INFO *)lpParam;

  assert(lpThreadInfo != NULL);
  assert(lpThreadInfo->mpThreadFunc != NULL);
#if defined(ANDROID) || defined(LINUX)

  std::string name = th->GetThreadName();
  prctl(PR_SET_NAME, (unsigned long)name.c_str()); 
  //prctl(PR_SET_NAME, (unsigned long)"sr-basethread"); 
#endif
  {
    CCriticalAutoLock l(th->m_critical_section);
    th->running_ = true;
    pthread_cond_signal(&th->m_cond_t);
  }

  lpThreadInfo->mpThreadFunc(lpThreadInfo->mpUserParam);

  //if (lpThreadInfo != NULL)
  //{
  //delete lpThreadInfo;
  //lpThreadInfo = NULL;
  //}

  th->running_ = false;

  return NULL;
}
#endif

void CBaseThread::Sleep(uint32_t dwMilliseconds)
{
#ifdef _WIN32
  ::Sleep(dwMilliseconds);
#else
  uint32_t ldwSleep = 1000 * dwMilliseconds;
  usleep(ldwSleep);
#endif
}


uint64_t CBaseThread::GetSystemTime()
{
  struct timeb loTimeb;
  //memset(&loTimeb, 0 , sizeof(timeb));
  ftime(&loTimeb);
  return ((uint64_t)loTimeb.time * 1000) + loTimeb.millitm;
}

bool CBaseThread::IsRunning()
{
  return running_;
}

#ifndef _WIN32
void CBaseThread::Join()
{
  pthread_join(m_thread, NULL);
}

int CBaseThread::ConvertToLinuxSystemPriority(ThreadPriority priority, int min_prio, int max_prio)
{
    assert(max_prio - min_prio > 2);
    const int top_prio = max_prio - 1;
    const int low_prio = min_prio + 1;

    switch (priority) {
    case kLowPriority:
        return low_prio;
    case kNormalPriority:
        // The -1 ensures that the kHighPriority is always greater or equal to
        // kNormalPriority.
        return (low_prio + top_prio - 1) / 2;
    case kHighPriority:
        return std::max(top_prio - 2, low_prio);
    case kHighestPriority:
        return std::max(top_prio - 1, low_prio);
    case kRealtimePriority:
        return top_prio;
    }
    assert(false);
    return low_prio;
}
#endif

bool CBaseThread::SetPriority(ThreadPriority priority)
{
#ifdef _WIN32
    BOOL result = 1;
    switch (priority) 
    {
    case kLowPriority:
        result = SetThreadPriority((HANDLE)m_hThreadHandle, THREAD_PRIORITY_BELOW_NORMAL);
        break;
    case kNormalPriority:
        result = SetThreadPriority((HANDLE)m_hThreadHandle, THREAD_PRIORITY_NORMAL);
        break;
    case kHighPriority:
        result = SetThreadPriority((HANDLE)m_hThreadHandle, THREAD_PRIORITY_ABOVE_NORMAL);
        break;
    case kHighestPriority:
        result = SetThreadPriority((HANDLE)m_hThreadHandle, THREAD_PRIORITY_HIGHEST);
        break;
    case kRealtimePriority:
        result = SetThreadPriority((HANDLE)m_hThreadHandle, THREAD_PRIORITY_TIME_CRITICAL);
        break;
    default:
        assert(false);
        break;
    };

    //printf("[thread] set thread priority[%s] ok\n", GetPriorityInfo(priority).c_str());

    assert(result > 0);
    return (result > 0);
#else
#ifdef THREAD_FIFO
    const int policy = SCHED_FIFO;
#else
    const int policy = SCHED_RR;
#endif
    sched_param param;
    const int min_prio = sched_get_priority_min(policy);
    const int max_prio = sched_get_priority_max(policy);

    if ((min_prio == EINVAL) || (max_prio == EINVAL)) 
    {
        printf("[thread] unable to retreive min or max priority for threads\n");
        assert(false);
        return false;
    }
    if (max_prio - min_prio <= 2) 
    {
        printf("[thread] There is no room for setting priorities with any granularity\n");
        assert(false);
        return false;
    }

    param.sched_priority = ConvertToLinuxSystemPriority(priority, min_prio, max_prio);
    int result = pthread_setschedparam(m_thread, policy, &param);
    if (result == EINVAL) 
    {
        printf("[thread] unable to set thread priority\n");
        assert(false);
        return false;
    }
    //printf("[thread] set thread priority[%s] ok\n", GetPriorityInfo(priority).c_str());
    return true;
#endif 
}

std::string CBaseThread::GetPriorityInfo(ThreadPriority priority)
{
    switch (priority)
    {
    case kLowPriority:
        return "kLowPriority";
    case kNormalPriority:
        return "kNormalPriority";
    case kHighPriority:
        return "kHighPriority";
    case kHighestPriority:
        return "kHighestPriority";
    case kRealtimePriority:
        return "kRealtimePriority";
    default:
        assert(false);
    };

    assert(false);
    return "";
}


//#endif
