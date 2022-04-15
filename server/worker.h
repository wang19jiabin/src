#pragma once
#include <memory>

class Worker {
public:
  static std::unique_ptr<Worker> create();
  virtual ~Worker() = default;
  virtual void work() = 0;
};
