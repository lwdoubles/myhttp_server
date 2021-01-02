#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include "myqueue.h"
#include <pthread.h>

void my_queue::clear(){
    m_mutex.lock();
    m_size = 0;
    m_front = -1;
    m_back = -1;
    m_mutex.unlock();
}


bool my_queue::full(){
    m_mutex.lock();
    if(m_size >= m_max_size){
        m_mutex.unlock();
        return true;
    }
    m_mutex.unlock();
    return false;
} 

bool my_queue:empty(){
    m_mutex.lock();
    if(m_size == 0){
        m_mutex.unlock();
        return true;
    }
    m_mutex.unlock();
    return false;
}

bool my_queue::front(T &value){
    m_mutex.lock();
    if(m_size == 0){
        m_mutex.unlock();
        return false;
    }
    value = m_array[m_front];
    m_mutex.unlock();
    return true;
}


bool my_queue::back(T &value){
    m_mutex.lock();
    if(m_size == 0){
        m_mutex.unlock();
        return false;
    }
    value = m_array[m_back];
    m_mutex.unlock();
    return true;
}

int my_queue::size(){
    int tmp = 0;
    m_mutex.lock();
    tmp = m_size;
    m_mutex.unlock();
    return tmp;
}

bool my_queue::push(const T &item){
    m_mutex.lock();
    if(m_size >= m_max_size){
        m_cond.brocast();
        m_mutex.unlock();
        return false;
    }

    m_back = (m_back + 1) % m_max_size;
    m_array[m_back] = item;
    m_size++;

    m_cond.broadcast();
    m_mutex.unlock();
    return true;
}


bool my_queue::pop(T &item){
    m_mutex.lock();
    while(m_size <= 0){
        if(!m_cond.wait(m_mutex.get())){
            m_mutex.unlock();
            return false;
        }
    }

    m_front = (m_front + 1) % m_max_size;
    item = m_array[m_front];
    m_size--;
    m_mutex.unlock();
    return true;
}

