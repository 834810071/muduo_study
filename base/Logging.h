//
// Created by jxq on 19-7-4.
//

#ifndef MYMUDUO_LOGGING_H
#define MYMUDUO_LOGGING_H


#include "LogStream.h"
#include "Timestamp.h"

namespace muduo
{
    class TimeZone;

    class Logger
    {
    public:
        // TRACE，细粒度最高的日志信息，打印的最详细
        // DEBUG，细粒度级别上对调试有帮助的日志信息
        // INFO，粗粒度级别上强调程序的运行信息
        // WARN，程序能正常运行，但存在潜在风险的信息
        // ERROR，执行出错，但不影响程序继续执行的错误信息
        // FATAL，将导致程序退出的严重信息
        enum LogLevel
        {
            TRACE,
            DEBUG,
            INFO,
            WARN,
            ERROR,
            FATAL,
            NUM_LOG_LEVELS,
        };

        // SourceFile保存着调用LOG_*语句所在的源文件名和行号
        // compile time calculation of basename of source file
        class SourceFile
        {
        public:
            template<int N>
            SourceFile(const char (&arr)[N])
                : data_(arr),
                  size_(N-1)
            {
                // 查找一个字符c在另一个字符串str中末次出现的位置（也就是从str的右侧开始查找字符c首次出现的位置），
                // 并返回这个位置的地址。如果未能找到指定字符，那么函数将返回NULL。
                const char* slash = strrchr(data_, '/');    // slash : 斜线
                if (slash)
                {
                    data_ = slash + 1;
                    size_ -= static_cast<int>(data_ - arr);
                }
            }

            explicit SourceFile(const char* filename)
                : data_(filename)
            {
                const char* slash = strrchr(filename, '/');
                if (slash)
                {
                    data_ = slash + 1;
                }
                size_ = static_cast<int>(strlen(data_));
            }

            const char* data_;  // 文件名
            int size_;          // 文件名称长度
        };

        // 构造Impl对象
        Logger(SourceFile file, int line);
        Logger(SourceFile file, int line, LogLevel level);
        Logger(SourceFile file, int line, LogLevel level, const char* func);
        Logger(SourceFile file, int line, bool toAbort);
        ~Logger();

        // 返回Impl的LogStream对象
        LogStream& stream()
        {
            return impl_.stream_;
        }

        static LogLevel logLevel();
        static void setLogLevel(LogLevel level);

        // 输出函数，将日志信息输出
        typedef void (*OutputFunc)(const char* msg, int len);
        // 刷新缓冲区
        typedef void (*FlushFunc)();
        static void setOutput(OutputFunc);
        static void setFlush(FlushFunc);
        static void setTimeZone(const TimeZone& tz);

    private:

        // Impl保存着所有Logger需要的数据 将对象与数据分开
        class Impl  // 实际上上logger类内部的一个嵌套类，封装了Logger的缓冲区stream_
        {
        public:
            typedef Logger::LogLevel LogLevel;
            Impl(LogLevel level, int old_errno, const SourceFile& file, int line);  // 级别错误文件行
            void formatTime();
            void finish();

            Timestamp time_;        // 当前时间
            LogStream stream_;      // 构造日志缓冲区，该缓冲区重载了各种<<，都是将数据格式到LogStream的内部成员缓冲区buffer里
            LogLevel level_;        // 级别
            int line_;              // 行
            SourceFile basename_;   // 基本名称
        };

        Impl impl_; // Logger构造这个对象
    };

    extern Logger::LogLevel g_logLevel;

    inline Logger::LogLevel Logger::logLevel()
    {
        return g_logLevel;
    }

    //
    // CAUTION: do not write:
    //
    // if (good)
    //   LOG_INFO << "Good news";
    // else
    //   LOG_WARN << "Bad news";
    //
    // this expends to
    //
    // if (good)
    //   if (logging_INFO)
    //     logInfoStream << "Good news";
    //   else
    //     logWarnStream << "Bad news";
    //

    // 使用if条件判断，如果当前级别大于TRACE，就相当于没有下面一行代码，不会编译，下同。
    // 当客端调用这些宏时，相当于构造了一个无名临时Logger对象并且调用了它的stream()方法，然后该对象析构，
    // 我们可以去看看Logger的析构函数做了哪些事情，见上面
    /*
     * __FILE__:返回所在文件名
     * __LINE__:返回所在行数
     * __func__:返回所在函数名
     *
     * 这些都是无名对象，当使用LOG_* << "***"时，
     * 1.构造Logger类型的临时对象，返回LogStream类型变量
     * 2.调用LogStream重载的operator<<操作符，将数据写入到LogStream的Buffer中
     * 3.当前语句结束，Logger临时对象析构，调用Logger析构函数，将LogStream中的数据输出
     */
    #define LOG_TRACE if (muduo::Logger::logLevel() <= muduo::Logger::TRACE) \
      muduo::Logger(__FILE__, __LINE__, muduo::Logger::TRACE, __func__).stream()
    #define LOG_DEBUG if (muduo::Logger::logLevel() <= muduo::Logger::DEBUG) \
      muduo::Logger(__FILE__, __LINE__, muduo::Logger::DEBUG, __func__).stream()
    #define LOG_INFO if (muduo::Logger::logLevel() <= muduo::Logger::INFO) \
      muduo::Logger(__FILE__, __LINE__).stream()
    #define LOG_WARN muduo::Logger(__FILE__, __LINE__, muduo::Logger::WARN).stream()
    #define LOG_ERROR muduo::Logger(__FILE__, __LINE__, muduo::Logger::ERROR).stream()
    #define LOG_FATAL muduo::Logger(__FILE__, __LINE__, muduo::Logger::FATAL).stream()
    #define LOG_SYSERR muduo::Logger(__FILE__, __LINE__, false).stream()
    #define LOG_SYSFATAL muduo::Logger(__FILE__, __LINE__, true).stream()
    #define LOG muduo::Logger(__FILE__, __LINE__).stream()

    const char* strerror_tl(int savedErrno);

    // Taken from glog/logging.h
    //
    // Check that the input is non NULL.  This very useful in constructor
    // initializer lists.

    #define CHECK_NOTNULL(val) \
  ::muduo::CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

    // A small helper for CHECK_NOTNULL().
    template <typename T>
    T* CheckNotNull(Logger::SourceFile file, int line, const char *names, T* ptr)
    {
        if (ptr == NULL)
        {
            Logger(file, line, Logger::FATAL).stream() << names;
        }
        return ptr;
    }
}


#endif //MYMUDUO_LOGGING_H


























