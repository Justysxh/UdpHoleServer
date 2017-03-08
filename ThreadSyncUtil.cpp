//
// Created by sxh on 2017/1/23.
//

#include "ThreadSyncUtil.h"

CThreadCond::CThreadCond()
{

    pthread_mutexattr_init(&mAttr);
    pthread_mutexattr_settype(&mAttr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&mMutex, &mAttr);
    pthread_cond_init(&mCond,0);
}

CThreadCond::~CThreadCond()
{
    broadcast();
    destroy();
}

int CThreadCond::signal()
{
    pthread_mutex_lock(&mMutex);
    int ret = pthread_cond_signal(&mCond);
    pthread_mutex_unlock(&mMutex);
    return ret;
}

int CThreadCond::wait(int timeoutMS )
{
    int ret = 0;
    pthread_mutex_lock(&mMutex);
    if(timeoutMS == -1)
    {
        ret = pthread_cond_wait(&mCond, &mMutex);
    }
    else
    {
        struct timeval curTime = {0};
        gettimeofday(&curTime, 0);
        struct timespec tv = {0};
        tv.tv_sec = curTime.tv_sec+timeoutMS/1000;
        tv.tv_nsec = (curTime.tv_usec + (timeoutMS%1000))*1000;
        ret = pthread_cond_timedwait(&mCond, &mMutex, &tv);
    }
    pthread_mutex_unlock(&mMutex);
    return ret;
}

int CThreadCond::broadcast()
{

    pthread_mutex_lock(&mMutex);
    int ret = pthread_cond_broadcast(&mCond);
    pthread_mutex_unlock(&mMutex);
    return ret;

}

void CThreadCond::destroy()
{
    broadcast();
    pthread_cond_destroy(&mCond);
    pthread_mutex_destroy(&mMutex);
    pthread_mutexattr_destroy(&mAttr);
}
////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
CSemaphore::CSemaphore()
{
    sem_init(&mSem,0,0);
    mIsCreate = true;
}
CSemaphore::CSemaphore(const CSemaphore &other)
{
    mIsCreate = false;
    mSem = other.mSem;
}

CSemaphore::~CSemaphore()
{
    if(mIsCreate)
    {
        sem_post(&mSem);
        sem_destroy(&mSem);
    }
}

CSemaphore& CSemaphore::operator=(const CSemaphore &other)
{
    if(mIsCreate )
    {
        sem_destroy(&mSem);
    }
    mIsCreate = false;
    mSem = other.mSem;
    return *this;
}

int CSemaphore::wait(int timeoutMS)
{
#ifndef __ANDROID__
    return sem_wait(&mSem);
#else
    if(timeoutMS == -1)
    {
        return sem_wait(&mSem);
    }
    else
    {
        struct timeval curTime = {0};
        gettimeofday(&curTime, 0);
        struct timespec tv = {0};
        tv.tv_sec = curTime.tv_sec+timeoutMS/1000;
        tv.tv_nsec = (curTime.tv_usec + (timeoutMS%1000))*1000;
        int ret = 0;
        ret = sem_timedwait(&mSem, &tv);
        if(ret != 0)
        {
            ret = errno;
        }
        return ret;
    }
#endif

}

int CSemaphore::tryWait()
{
    int ret= sem_trywait(&mSem);
    if(ret != 0)
    {
        ret = errno;
    }
    return ret;
}

int CSemaphore::signal()
{
    int ret = sem_post(&mSem);
    if(ret != 0)
    {
        ret = errno;
    }
    return ret;
}



//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
CThreadMutex::CThreadMutex()
{
    mIsCreate = true;
    pthread_mutexattr_init(&mAttr);
    pthread_mutexattr_settype(&mAttr, PTHREAD_MUTEX_RECURSIVE);
    int ret = pthread_mutex_init(&mMutex, &mAttr);
    mpMutex = &mMutex;
}


CThreadMutex::CThreadMutex(const CThreadMutex &other)
{
    mIsCreate = false;
    mpMutex = other.mpMutex;
}

CThreadMutex &CThreadMutex::operator=(const CThreadMutex &other)
{
    mpMutex = other.mpMutex;
    return *this;
}

CThreadMutex::~CThreadMutex()
{
    if(mIsCreate)
    {
        pthread_mutex_unlock(&mMutex);
        destroy();
    }
}

int CThreadMutex::lock()
{
    return pthread_mutex_lock(mpMutex);
}

int CThreadMutex::unlock()
{
    return pthread_mutex_unlock(mpMutex);
}

int CThreadMutex::trylock()
{
    return pthread_mutex_trylock(mpMutex);
}

void CThreadMutex::destroy()
{
    pthread_mutexattr_destroy(&mAttr);
    pthread_mutex_destroy(&mMutex);
}


/////////////////////////////////////
//////////////////////////////////////
CThread::CThread(THRED_RUTINE fun, void *pParam)
{
    mCreateRet = pthread_create(&mThread, 0, fun, pParam);
}

CThread::~CThread()
{
    join();
}
int CThread::join()
{
    int ret = 0;
    if(mCreateRet==0)
    {
        ret = pthread_join(mThread, 0);
        mCreateRet = -1;
    }
    return ret;
}

void *CWorkThreadBase::ThreadBaseFun(void *pVoid)
{
    CWorkThreadBase *pThread = (CWorkThreadBase*)pVoid;
    pThread->beforeRun();
    pThread->run();
    pThread->afterRun();
    if(pThread->mIsSelfFree)
    {
       
        delete pThread;
    }
    return 0;
}

