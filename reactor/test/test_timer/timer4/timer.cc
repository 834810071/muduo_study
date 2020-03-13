

#include <iostream>
#include "../../../../base/noncopyable.h"
#include "../../../EventLoop.h"

class Printer : muduo::noncopyable
{
 public:
  Printer(muduo::EventLoop* loop)
    : loop_(loop),
      count_(0)
  {
    // Note: loop.runEvery() is better for this use case.
    loop_->runAfter(1, std::bind(&Printer::print, this));
  }

  ~Printer()
  {
    std::cout << "Final count is " << count_ << "\n";
  }

  void print()
  {
    if (count_ < 5)
    {
      std::cout << count_ << "\n";
      ++count_;

      loop_->runAfter(1, std::bind(&Printer::print, this));
    }
    else
    {
      loop_->quit();
    }
  }

private:
  muduo::EventLoop* loop_;
  int count_;
};

int main()
{
  muduo::EventLoop loop;
  Printer printer(&loop);
  loop.loop();
}

