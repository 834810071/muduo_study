//
// Created by jxq on 19-7-2.
//

#ifndef MYMUDUO_THREADLOCAL_H
#define MYMUDUO_THREADLOCAL_H

#include "Mutex.h"
#include "noncopyable.h"

#include <pthread.h>

namespace muduo
{
    template<typename T>
    class ThreadLocal : noncopyable
    {
    public:
        ThreadLocal()
        {
            MCHECK(pthread_key_create(&pkey_, &ThreadLocal::destructor));
        }

        ~ThreadLocal()
        {
            MCHECK(pthread_key_delete(pkey_));
        }

        T& value()
        {
            T* perThreadValue = static_cast<T*>(pthread_getspecific(pkey_));
            if (!perThreadValue)
            {
                T* newObj = new T();
                MCHECK(pthread_setspecific(pkey_, newObj));
                perThreadValue = newObj;
            }
            return *perThreadValue;
        }

    private:
        static void destructor(void* x)
        {
            T* obj = static_cast<T*>(x);
            typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
            T_must_be_complete_type  dummy;
            (void) dummy;
            delete obj;
        }

        pthread_key_t pkey_;    // 线程特定数据
    };
}   // namespace muduo

#endif //MYMUDUO_THREADLOCAL_H
