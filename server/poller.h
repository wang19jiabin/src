#pragma once
#include <memory>

class Poller {
public:
  static std::unique_ptr<Poller> create(int);
  virtual ~Poller() = default;
  virtual void poll() = 0;
};
