//
// Created by jxq on 19-7-2.
//

#ifndef MYMUDUO_THREADLOCALSINGLETON_H
#define MYMUDUO_THREADLOCALSINGLETON_H

#include "noncopyable.h"

#include <assert.h>
#include <pthread.h>

namespace muduo
{
    template<typename T>
    class ThreadLocalSingleton : noncopyable
    {
    public:
        ThreadLocalSingleton() = delete;

        ~ThreadLocalSingleton() = delete;

        // 保证只有一次
        static T& instance() {
            if (!t_value_) {
                t_value_ = new T();
                deleter_.set(t_value_);
            }
            return *t_value_;
        }

        static T *pointer()
        {
            return t_value_;
        }

    private:
        static void destructor(void *obj) {
            assert(obj == t_value_);
            // 对于只声明，未进行具体定义类型，例如A，sizeof(A)结果为0
            // 利用此特性，若T是未定义类型，则T_must_be_complete_type在编译期间会报错
            typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];  // 数组
            // 消除unused-local-typedefs编译错误提示
            T_must_be_complete_type dummy;
            (void) dummy;
            delete t_value_;
            t_value_ = 0;
        }

        class Deleter
        {
        public:
            Deleter()
            {
                pthread_key_create(&pkey_, &ThreadLocalSingleton::destructor);
            }

            ~Deleter()
            {
                pthread_key_delete(pkey_);
            }

            void set(T* newObj)
            {
                assert(pthread_getspecific(pkey_) == NULL);
                pthread_setspecific(pkey_, newObj);
            }

            pthread_key_t pkey_;
        };

        // __thread修饰，指针是POD类型数据
        // Plain old data structure 基础类型
        // 对于POD类型的线程本地存储，可以用__thread关键字

        // t_value_ 虽然也是静态数据成员，但加了__thread 修饰符，故每一个线程都会有一份
        static __thread T* t_value_;
        static Deleter deleter_;    // 静态数据成员，为所有线程所共享
    };

    template<typename T>
    __thread T* ThreadLocalSingleton<T>::t_value_ = 0;

    template<typename T>
    typename ThreadLocalSingleton<T>::Deleter ThreadLocalSingleton<T>::deleter_;
}

#endif //MYMUDUO_THREADLOCALSINGLETON_H
