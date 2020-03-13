#include <iostream>
#include "../../../EventLoop.h"

void print(muduo::EventLoop* loop, int* count)
{
  if (*count < 5)
  {
    std::cout << *count << "\n";
    ++(*count);

    loop->runAfter(1, std::bind(print, loop, count));
  }
  else
  {
    loop->quit();
  }
}

int main()
{
  muduo::EventLoop loop;
  int count = 0;
  // Note: loop.runEvery() is better for this use case.
  loop.runAfter(1, std::bind(print, &loop, &count));
  loop.loop();
  std::cout << "Final count is " << count << "\n";
}

