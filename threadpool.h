#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "locker.h"

template< typename T >
class threadpool
{
public:
    //线程池中线程数量、请求队列允许的最多请求数量
    threadpool( int thread_number = 8, int max_requests = 10000 );
    ~threadpool();
    bool append( T* request );

private:
    //工作线程运行的函数，它不断从工作队列中取出任务并执行
    static void* worker( void* arg );
    void run();

private:
    int m_thread_number;    //线程池中的线程数
    int m_max_requests;     //请求队列允许的最大请求数量
    pthread_t* m_threads;   //线程池数组
    std::list< T* > m_workqueue;    //请求队列
    locker m_queuelocker;   //保护请求队列的互斥锁
    sem m_queuestat;        //是否有任务需要处理
    bool m_stop;            //是否结束线程
};

template< typename T >
threadpool< T >::threadpool( int thread_number, int max_requests ) : 
        m_thread_number( thread_number ), m_max_requests( max_requests ), m_stop( false ), m_threads( NULL )
{
    if( ( thread_number <= 0 ) || ( max_requests <= 0 ) )
    {
        throw std::exception();
    }
    //创建线程池
    m_threads = new pthread_t[ m_thread_number ];
    if( ! m_threads )
    {
        throw std::exception();
    }

    for ( int i = 0; i < thread_number; ++i )
    {
        printf( "create the %dth thread\n", i );
        //this指针作为worker的参数
        if( pthread_create( m_threads + i, NULL, worker, this ) != 0 )
        {
            delete [] m_threads;
            throw std::exception();
        }
        //使得线程结束后自动释放资源，和pthread_join有点类似，但是pthread_join是一个线程阻塞地等待另一个线程释放资源
        if( pthread_detach( m_threads[i] ) )
        {
            delete [] m_threads;
            throw std::exception();
        }
    }
}

template< typename T >
threadpool< T >::~threadpool()
{
    delete [] m_threads;
    m_stop = true;
}

//把任务加入请求队列
template< typename T >
bool threadpool< T >::append( T* request )
{
    m_queuelocker.lock();
    if ( m_workqueue.size() > m_max_requests )
    {
        m_queuelocker.unlock();
        return false;
    }
    m_workqueue.push_back( request );
    m_queuelocker.unlock();
    //信号量+1，表示多了一个任务
    m_queuestat.post();
    return true;
}

template< typename T >
void* threadpool< T >::worker( void* arg )
{
    threadpool* pool = ( threadpool* )arg;
    pool->run();
    return pool;
}

//从请求队列中取出任务并处理
template< typename T >
void threadpool< T >::run()
{
    while ( ! m_stop )
    {
        //信号量-1，表示从队列中取出一个任务处理
        m_queuestat.wait();
        m_queuelocker.lock();
        if ( m_workqueue.empty() )
        {
            m_queuelocker.unlock();
            continue;
        }
        T* request = m_workqueue.front();
        //将取出的任务从队列里丢弃
        m_workqueue.pop_front();
        m_queuelocker.unlock();
        if ( ! request )
        {
            continue;
        }
        
        request->process();
    }
}

#endif
