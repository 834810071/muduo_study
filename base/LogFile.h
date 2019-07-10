//
// Created by jxq on 19-7-4.
//

#ifndef MYMUDUO_LOGFILE_H
#define MYMUDUO_LOGFILE_H

#include "Mutex.h"
#include "Types.h"

#include <memory>

namespace muduo
{
    namespace FileUtil
    {
        class AppendFile;
    }

    class LogFile : noncopyable
    {
    public:
        LogFile(const string& basename,
                off_t rollSize,
                bool threadSafe = true,
                int flushInterval = 3,
                int checkEveryN = 1024);    // 默认分割行数1024
        ~LogFile();

        void append(const char* logline, int len);  // 将一行长度为len添加到日志文件中
        void flush();   // 刷新
        bool rollFile();

    private:
        // 不加锁的append方式
        void append_unlock(const char* logline, int len);

        // 获取日志文件的名称
        static string getLogFileName(const string& basename, time_t *now);

        const string basename_;     // 日志文件basename
        const off_t rollSize_;      // off_t 文件操作指针移动; 日志文件达到rolsize生成一个新文件
        const int flushInterval_;   // 日志写入间隔时间
        const int checkEveryN_;

        int count_;                 // 计数器，检测是否需要换新文件

        std::unique_ptr<MutexLock> mutex_;  // 加锁
        // 开始记录日志时间
        // （调整至零点时间，如12.04:11.24和 11.12.04:12.50，调整零点都是12.04:00.00，是同一天，只用来比较同一天，和日志名无关
        time_t startOfPeriod_;  // time_t 这种类型就是用来存储从1970年到现在经过了多少秒
        time_t lastRoll_;   // 上一次滚动日志文件时间
        time_t lastFlush_;  // 上一次日志写入文件时间
        std::unique_ptr<FileUtil::AppendFile> file_;    // 文件智能指针

        const static int kRollPerSeconds_ = 60 * 60 * 24;
    };
}   // namespace muduo


#endif //MYMUDUO_LOGFILE_H
