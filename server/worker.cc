#include "worker.h"
#include "connection.h"
#include "queue.h"

extern std::unique_ptr<Queue> queue;

class WorkerImpl : public Worker {
public:
  void work() override;
};

std::unique_ptr<Worker> Worker::create() {
  return std::make_unique<WorkerImpl>();
}

void WorkerImpl::work() {
  while (true) {
    auto task = queue->wait();
    task->data += "\n";
    auto conn = task->connection.lock();
    if (conn)
      conn->reply(std::move(task));
  }
}
