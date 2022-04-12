#pragma once

#include <memory>

struct Task {
  std::string string;
  std::weak_ptr<class Connection> connection;
  size_t sent = 0;
};

class Queue {
public:
  static std::unique_ptr<Queue> create();
  virtual ~Queue() = default;
  virtual void push(std::shared_ptr<Task>) = 0;
  virtual std::shared_ptr<Task> wait() = 0;
  virtual std::shared_ptr<Task> pop() = 0;
};
