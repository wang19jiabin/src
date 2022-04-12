#include "queue.h"
#include <condition_variable>
#include <mutex>
#include <queue>

class QueueImpl : public Queue {
public:
  void push(std::shared_ptr<Task>) override;
  std::shared_ptr<Task> wait() override;
  std::shared_ptr<Task> pop() override;

private:
  std::mutex _m;
  std::condition_variable _cv;
  std::queue<std::shared_ptr<Task>> _q;
};

std::unique_ptr<Queue> Queue::create() { return std::make_unique<QueueImpl>(); }

void QueueImpl::push(std::shared_ptr<Task> t) {
  std::lock_guard<std::mutex> lg(_m);
  _q.push(t);
  _cv.notify_one();
}

std::shared_ptr<Task> QueueImpl::wait() {
  std::unique_lock<std::mutex> ul(_m);
  _cv.wait(ul, [this] { return !_q.empty(); });
  auto t = _q.front();
  _q.pop();
  return t;
}

std::shared_ptr<Task> QueueImpl::pop() {
  std::shared_ptr<Task> t;
  std::lock_guard<std::mutex> lg(_m);
  if (!_q.empty()) {
    t = _q.front();
    _q.pop();
  }

  return t;
}
