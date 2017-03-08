//
// Created by sxh on 2017/1/23.
//

#ifndef UDTDEMO_THREADSYNCUTIL_H
#define UDTDEMO_THREADSYNCUTIL_H



#include <pthread.h>
#include <errno.h>
#include <sys/time.h>
#include <semaphore.h>
#include <errno.h>
#include <cstdio>



class CBaseSignal
{
public:
    CBaseSignal()
    {

    }
    virtual ~CBaseSignal()
    {

    }
    virtual int signal()=0;
    virtual int wait(int timeoutMS=-1)=0;
};


class CThreadCond :public CBaseSignal
{
private:
    pthread_cond_t mCond;
    pthread_mutex_t mMutex;
    pthread_mutexattr_t  mAttr;
public:
    CThreadCond();
    ~CThreadCond();
    int signal();

    //0:ok  ETIMEDOUT: timeout  EINVAL:
    int wait(int timeoutMS = -1);
    int broadcast();

    void destroy();
};



class CSemaphore: public CBaseSignal
{
private:
    sem_t mSem;
    bool  mIsCreate;
public:
    CSemaphore();
    CSemaphore(const CSemaphore &other) ;
    ~CSemaphore() ;
    CSemaphore&operator=(const CSemaphore &other);

    /**
     * method 等待信号
     * @param timeoutMS 超时时间 (毫秒)
     * @return int  0:成功 失败时返回 ETIMEOUT:超时 EINTER:系统中断
     */
    int wait(int timeoutMS=-1);

    /**
     * method 尝试等待信号, 会立即返回, 有无信号都马上返回.
     * @return int 0:有信号  EAGAIN:无信号 EINTER:系统中断
     */
    int tryWait();

    int signal() ;

};




class CBaseLocker
{
public:
    virtual  ~CBaseLocker()
    {

    }
    virtual int lock()=0;
    virtual int unlock()=0;
};


class CThreadMutex: public CBaseLocker
{
private:
    bool  mIsCreate;
    pthread_mutex_t mMutex;
    pthread_mutex_t *mpMutex;
    pthread_mutexattr_t mAttr;
public:
    CThreadMutex();
    CThreadMutex(const CThreadMutex &other);
    CThreadMutex &operator=(const CThreadMutex &other);
    ~CThreadMutex() ;

    pthread_mutex_t *getHandle()
    {
        return mpMutex;
    }

    int lock() ;

    int unlock() ;


    int trylock();


    void destroy();
};



typedef void *(THRED_RUTINE)(void*);

class CThread
{
private:
    pthread_t mThread;
    int mCreateRet;
public:
    CThread(THRED_RUTINE fun, void *pParam);
    ~CThread();
    int join();
};



class CWorkThreadBase
{
protected:
    pthread_t  mThread;
    bool mIsRun;
    bool mIsSelfFree; //允许自删除
    static void* ThreadBaseFun(void *pVoid);
public:
    int mThreadID;
    CWorkThreadBase()
    {
        mIsRun = false;
        mThreadID = 0;
        mThread = 0;
        mIsSelfFree = true;
    }
    virtual ~CWorkThreadBase()
    {
    }

    virtual int start()
    {
        mIsRun = true;
        return pthread_create(&mThread,NULL, ThreadBaseFun,this);
    }
    virtual int beforeRun()
    {
        return 0;
    }

    virtual int afterRun()
    {
        return 0;
    }


    virtual void stop()
    {
        mIsRun = false;
    }

    int join()
    {
        return pthread_join(mThread,NULL);
    }

    virtual void run()=0;

    void setSelfFree(bool bFree)
    {
        mIsSelfFree = bFree;
    }


};








#endif //UDTDEMO_THREADSYNCUTIL_H
