//
//  Thread.h
//

#ifndef Thread_h
#define Thread_h

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <utility>
#include <functional>
#include <thread>

using std::thread, std::vector, std::pair;

class Thread
{
public:
  Thread()
      : nth(nCpus()), threads(vector<thread>(nth)) {}
  Thread(int size)
      : nth(size < nCpus()
                ? size
                : nCpus()),
        size(size),
        threads(vector<thread>(nth)) { chunk_ranges(); }

  static int nCpus() { return int(thread::hardware_concurrency()); }

  void chunk_ranges() // Create the result and calculate the stride size and the remainder if any.
  {
    int stride = size / nth,
        first = 0,
        extra = size % nth;

    for (int i = 0; i < nth; i++)
    {
      int last = first + stride;
      if (extra > 0)
      {
        extra--;
        last++;
      }
      ranges.push_back(pair(first, last));
      first = last;
    }
  }

  int from(int t) { return ranges[t].first; }
  int to(int t) { return ranges[t].second; }

  void run(std::function<void(int, int, int)> const &lambda)
  { // t, from, to
    for (int t = 0; t < nth; t++)
    {
      threads[t] = thread([this, lambda, t]()
                          { lambda(t, from(t), to(t)); });
    }
    for (int t = 0; t < nth; t++)
      threads[t].join();
  }

  void run(std::function<void(int)> const &lambda)
  { // i
    for (int t = 0; t < nth; t++)
    {
      threads[t] = thread([this, lambda, t]()
                          {
                            for (int i = from(t); i < to(t); i++)
                              lambda(i);
                          });
    }
    for (int t = 0; t < nth; t++)
      threads[t].join();
  }

  void run(std::function<void(int, int)> const &lambda)
  { // t, i
    for (int t = 0; t < nth; t++)
    {
      threads[t] = thread([this, lambda, t]()
                          {
                            for (int i = from(t); i < to(t); i++)
                              lambda(t, i);
                          });
    }
    for (int t = 0; t < nth; t++)
      threads[t].join();
  }

  void run(std::function<void(void)> const &lambda)
  { // ()
    for (int t = 0; t < nth; t++)
    {
      threads[t] = thread([this, lambda, t]()
                          {
                            for (int i = from(t); i < to(t); i++)
                              lambda();
                          });
    }
    for (int t = 0; t < nth; t++)
      threads[t].join();
  }

  int nth = nCpus(), size = 0;
  vector<thread> threads;
  vector<pair<int, int>> ranges;
};

#endif /* Thread_h */
