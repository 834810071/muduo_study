#include "../../../EventLoop.h"
#include "../../../../base/noncopyable.h"
#include "../../../EventLoopThread.h"

#include <iostream>

class Printer : muduo::noncopyable
{
 public:
  Printer(muduo::EventLoop* loop1, muduo::EventLoop* loop2)
    : loop1_(loop1),
      loop2_(loop2),
      count_(0)
  {
    loop1_->runAfter(1, std::bind(&Printer::print1, this));
    loop2_->runAfter(1, std::bind(&Printer::print2, this));
  }

  ~Printer()
  {
    std::cout << "Final count is " << count_ << "\n";
  }

  void print1()
  {
    muduo::MutexLockGuard lock(mutex_);
    if (count_ < 10)
    {
      std::cout << "Timer 1: " << count_ << "\n";
      ++count_;

      loop1_->runAfter(1, std::bind(&Printer::print1, this));
    }
    else
    {
      loop1_->quit();
    }
  }

  void print2()
  {
    muduo::MutexLockGuard lock(mutex_);
    if (count_ < 10)
    {
      std::cout << "Timer 2: " << count_ << "\n";
      ++count_;

      loop2_->runAfter(1, std::bind(&Printer::print2, this));
    }
    else
    {
      loop2_->quit();
    }
  }

private:

  muduo::MutexLock mutex_;
  muduo::EventLoop* loop1_ PT_GUARDED_BY(mutex_);
  muduo::EventLoop* loop2_ PT_GUARDED_BY(mutex_);
  int count_ GUARDED_BY(mutex_);
};

int main()
{
  std::unique_ptr<Printer> printer;  // make sure printer lives longer than loops, to avoid
                                     // race condition of calling print2() on destructed object.
  muduo::EventLoop loop;
  muduo::EventLoopThread loopThread;
  muduo::EventLoop* loopInAnotherThread = loopThread.startLoop();
  printer.reset(new Printer(&loop, loopInAnotherThread));
  loop.loop();
}

