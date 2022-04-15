#include "queue.h"
#include <condition_variable>
#include <mutex>
#include <queue>

class QueueImpl : public Queue {
public:
  void push(std::unique_ptr<Task>) override;
  std::unique_ptr<Task> wait() override;
  std::unique_ptr<Task> pop() override;

private:
  std::mutex _m;
  std::condition_variable _cv;
  std::queue<std::unique_ptr<Task>> _q;
};

std::unique_ptr<Queue> Queue::create() { return std::make_unique<QueueImpl>(); }

void QueueImpl::push(std::unique_ptr<Task> t) {
  std::lock_guard<std::mutex> lg(_m);
  _q.push(std::move(t));
  _cv.notify_one();
}

std::unique_ptr<Task> QueueImpl::wait() {
  std::unique_lock<std::mutex> ul(_m);
  _cv.wait(ul, [this] { return !_q.empty(); });
  auto t = std::move(_q.front());
  _q.pop();
  return t;
}

std::unique_ptr<Task> QueueImpl::pop() {
  std::unique_ptr<Task> t;
  std::lock_guard<std::mutex> lg(_m);
  if (!_q.empty()) {
    t = std::move(_q.front());
    _q.pop();
  }

  return t;
}
