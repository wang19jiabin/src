#pragma once
#include <memory>

class Connection {
public:
  static std::shared_ptr<Connection> create(int, int);
  virtual ~Connection() = default;
  virtual bool read() = 0;
  virtual bool write() = 0;
  virtual void reply(std::unique_ptr<struct Task>) = 0;
};
