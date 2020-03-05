//
// Created by jxq on 19-7-4.
//

#include "Logging.h"
#include "CurrentThread.h"
#include "Timestamp.h"
#include "TimeZone.h"

#include <error.h>
#include <stdio.h>
#include <string.h>

#include <sstream>

namespace muduo
{
    __thread char t_errnobuf[512];
    __thread char t_time[64];
    __thread time_t t_lastSecond;

    const char* strerror_tl(int savedErrno)
    {
        // 对于函数strerror_r，
        // 第一个参数errnum是错误代码，
        // 第二个参数buf是用户提供的存储错误描述的缓存，
        // 第三个参数n是缓存的大小。
        return strerror_r(savedErrno, t_errnobuf, sizeof t_errnobuf);
    }

    Logger::LogLevel initLogLevel() {   // 初始化日志级别
        // getenv()用来取得参数name环境变量的内容
        if (::getenv("MUDUO_LOG_TRACE")) {  // 获取TRACE环境变量，如果有，返回它
            return Logger::TRACE;
        }
        else if (::getenv("MUDUO_LOG_DEBUG"))   // 获取DEBUG环境变量，如果有，返回它
        {
            return Logger::DEBUG;
        }
        else
        {
            return Logger::INFO;    // 如果它们都没有，就使用INFO级别
        }
    }

    Logger::LogLevel g_logLevel = initLogLevel();   // 初始化日志级别

    const char* LogLevelName[Logger::NUM_LOG_LEVELS] =
    {
        "TRACE ",
        "DEBUG ",
        "INFO  ",
        "WARN  ",
        "ERROR ",
        "FATAL ",
    };

    class T // 编译时获取字符串长度的类
    {
    public:
        T(const char* str, unsigned len)
            : str_(str),
              len_(len)
        {

        }

        const char* str_;
        const unsigned len_;
    };

    // LogStream的重载，输出
    inline LogStream& operator <<(LogStream& s, T v)
    {
        s.append(v.str_, v.len_);
        return s;
    }

    inline LogStream& operator <<(LogStream& s, const Logger::SourceFile& v)
    {
        s.append(v.data_, v.size_);
        return s;
    }

    void defaultOutput(const char* msg, int len)
    {
        size_t n = fwrite(msg, 1, len, stdout); // 默认输出内容到stdout
        // FIXME check n
        (void) n;
    }

    void defaultFlush()
    {
        fflush(stdout); // 默认刷新stdout
    }

    Logger::OutputFunc g_output = defaultOutput;    // 默认输出方法
    Logger::FlushFunc g_flush = defaultFlush;   // 默认刷新方法
    TimeZone g_logTimeZone;
}   // namespace muduo

using namespace muduo;

                                    // 错误码 没有传0
Logger::Impl::Impl(LogLevel level, int savedErrno, const muduo::Logger::SourceFile &file, int line)
    : time_(Timestamp::now()),  // 当前时间
      stream_(),                // 初始化logger[Impl]的四个成员
      level_(level),            // 日志级别
      line_(line),              // 调用LOG_*<<所在行，由__LINE__获取
      basename_(file)           // 调用LOG_*<<所在文件名，由__FILE__获取
{
    formatTime();   // 格式化时间 写入LogStream中
    CurrentThread::tid();   // 缓存当前线程id
    stream_ << " ";
    stream_ << T(CurrentThread::tidString(), CurrentThread::tidStringLength()); // 格式化线程tid字符串
    stream_ << " ";
    stream_ << T(LogLevelName[level], 6);   // 格式化级别，对应成字符串，先输出到缓冲区
    // 如果有错误，写入错误信息
    if (savedErrno != 0)
    {
        stream_ << strerror_tl(savedErrno) << " (errno = " << savedErrno << ")";    // 如果错误码不为0，还要输出相对应信息
    }
}

void Logger::Impl::formatTime()
{
    int64_t microSecondsSinceEpoch = time_.microSecondsSinceEpoch();
    time_t seconds = static_cast<time_t>(microSecondsSinceEpoch / Timestamp::kMicroSecondsPerSecond);
    int microseconds = static_cast<int>(microSecondsSinceEpoch % Timestamp::kMicroSecondsPerSecond);
    if (seconds != t_lastSecond)
    {
        t_lastSecond = seconds;
        struct tm tm_time;
        if (g_logTimeZone.valid())
        {
            tm_time = g_logTimeZone.toLocalTime(seconds);
        }
        else
        {
            ::gmtime_r(&seconds, &tm_time); // FIXME TimeZone::fromUtcTime
        }

        int len = snprintf(t_time, sizeof t_time, "%4d%02d%02d %02d:%02d:%02d",
                           tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                           tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
        assert(len == 17); (void)len;
    }

    stream_ << T(t_time, 17);
//    if (g_logTimeZone.valid())
//    {
//        Fmt us(".%06d", microseconds);
//        //printf("%s\n", us.data());
//        //assert(us.length() == 8);
//        assert(us.length() == 7);
//       // printf("%d\n", us.length());
//        stream_ << T(t_time, 17) << T(us.data(), 7);
//    }
//    else
//    {
//        Fmt us(".%06dZ ", microseconds);
//        assert(us.length() == 9);
//        stream_ << T(t_time, 17) << T(us.data(), 9);
//    }
}

// 将名字行输进缓冲区
void Logger::Impl::finish()
{
    stream_ << " - " << basename_ << ':' << line_ << '\n';
}

Logger::Logger(SourceFile file, int line)
        : impl_(INFO, 0, file, line)
{
}

Logger::Logger(SourceFile file, int line, LogLevel level, const char* func)
        : impl_(level, 0, file, line)
{
    // 格式化函数名称，上面的构造函数没有函数名称，不同的构造函数
    impl_.stream_ << func << ' ';
}

Logger::Logger(SourceFile file, int line, LogLevel level)
        : impl_(level, 0, file, line)
{
}

Logger::Logger(SourceFile file, int line, bool toAbort)
        : impl_(toAbort ? FATAL : ERROR, errno, file, line)
{
}

// 由于是临时对象，所以析构函数接管了所有日志输出的任务
//
// 1. 为日志信息添加后缀，通常是源文件名和所在行号
// 2. 从LogStream的缓冲区中回去所有日志信息，包括前缀和后缀
// 3. 调用输出函数，默认将日志信息打印到屏幕上
// 4. 如果日志级别是FATAL，那么会立即刷新缓冲区，同时发送abort终止程序运行

// 析构函数中会调用impl_的finish方法
Logger::~Logger()
{
    impl_.finish(); // 将名字行数输入缓冲区

    const LogStream::Buffer& buf(stream().buffer());    // 将缓冲区以引用方式获得

    // 调用全部输出方法，输出缓冲区内容，默认是输出到stdout
    g_output(buf.data(), buf.length());

    // 如果当前日志级别是FATAL，表示是个终止程序的严重错误，将输出缓冲区的信息刷新到屏幕上，结束程序
    if (impl_.level_ == FATAL)
    {
        g_flush();
        abort();
    }
}

// 设置日志级别
void Logger::setLogLevel(Logger::LogLevel level)
{
    g_logLevel = level;
}

// 设置输出函数，代替默认
void Logger::setOutput(OutputFunc out)
{
    g_output = out;
}

// 设置刷新函数，代替默认
void Logger::setFlush(FlushFunc flush)
{
    g_flush = flush;
}

void Logger::setTimeZone(const TimeZone& tz)
{
    g_logTimeZone = tz;
}
























