#pragma once
#include <memory>

struct Task {
  Task(std::string &&s, std::weak_ptr<class Connection> c)
      : data(std::move(s)), connection(c) {}
  size_t sent = 0;
  std::string data;
  std::weak_ptr<class Connection> connection;
};

class Queue {
public:
  static std::unique_ptr<Queue> create();
  virtual ~Queue() = default;
  virtual void push(std::unique_ptr<Task>) = 0;
  virtual std::unique_ptr<Task> wait() = 0;
  virtual std::unique_ptr<Task> pop() = 0;
};
