cmake_minimum_required(VERSION 3.13)
project(mymuduo)

set(CMAKE_CXX_STANDARD 14)

set(base_src
        Timestamp.cpp
        Timestamp.h
        copyable.h
        Types.h
        noncopyable.h
        Atomic.h
        Exception.cpp
        Exception.h
        CurrentThread.cpp
        CurrentThread.h
        Thread.h
        Thread.cpp
        Mutex.h
        Condition.cpp
        Condition.h
        CountDownLatch.h
        CountDownLatch.cpp
        BlockingQueue.h
        BoundedBlockingQueue.h
        ThreadPool.cpp
        ThreadPool.h
        Singleton.h
        ThreadLocal.h
        ThreadLocalSingleton_test.h
        Logging.h
        Logging.cpp
        LogStream.cpp
        LogStream.h
        LogFile.cpp
        LogFile.h
        StringPiece.h
        TimeZone.cpp
        TimeZone.h
        Date.cpp
        Date.h
        FileUtil.cc
        FileUtil.h
        ProcessInfo.cc
        ProcessInfo.h
        AsyncLogging.cpp
        AsyncLogging.h)

add_library(base ${base_src})
target_link_libraries(base pthread)