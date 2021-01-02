#ifndef MY_QUEUE_H
#define MY_QUEUE_H

#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include "../locker.h"
using namespace std;


template<typename T>
class my_queue{
    my_queue(int max_size = 10e5){
        if(max_size <= 0){
            exit(-1);
        }

        m_max_size = max_size;
        m_array = new T[max_size];
        m_size = 0;
        m_front = -1;
        m_back = -1;
    }

    ~my_queue(){
        m_mutex.lock();
        m_size = 0;
        m_front = -1;
        m_back = -1;
    }

    void clear();

    bool full();

    bool empty();

    bool front();

    bool back();

    int size();

    int max_size();

    bool push(const T &item);

    bool pop(T &item);

private:
    locker m_mutex;
    cond m_cond;

    T *m_array;
    int m_size;
    int m_max_size;
    int m_front;
    int m_back;

}

#endif