

#include <stdio.h>
#include "../../../EventLoop.h"
#include "../../../EventLoopThread.h"

//
// Minimize locking
//

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
    // cout is not thread safe
    //std::cout << "Final count is " << count_ << "\n";
    printf("Final count is %d\n", count_);
  }

  void print1()
  {
    bool shouldQuit = false;
    int count = 0;

    {
      muduo::MutexLockGuard lock(mutex_);
      if (count_ < 10)
      {
        count = count_;
        ++count_;
      }
      else
      {
        shouldQuit = true;
      }
    }

    // out of lock
    if (shouldQuit)
    {
      // printf("loop1_->quit()\n");
      loop1_->quit();
    }
    else
    {
      // cout is not thread safe
      //std::cout << "Timer 1: " << count << "\n";
      printf("Timer 1: %d\n", count);
      loop1_->runAfter(1, std::bind(&Printer::print1, this));
    }
  }

  void print2()
  {
    bool shouldQuit = false;
    int count = 0;

    {
      muduo::MutexLockGuard lock(mutex_);
      if (count_ < 10)
      {
        count = count_;
        ++count_;
      }
      else
      {
        shouldQuit = true;
      }
    }

    // out of lock
    if (shouldQuit)
    {
      // printf("loop2_->quit()\n");
      loop2_->quit();
    }
    else
    {
      // cout is not thread safe
      //std::cout << "Timer 2: " << count << "\n";
      printf("Timer 2: %d\n", count);
      loop2_->runAfter(1, std::bind(&Printer::print2, this));
    }
  }

private:

  muduo::MutexLock mutex_;
  muduo::EventLoop* loop1_;
  muduo::EventLoop* loop2_;
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

