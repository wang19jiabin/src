#include <gtest/gtest.h>
#include <list>
#include <queue>

using Task = std::function<void(const std::function<void()> &)>;

std::queue<std::function<void()>> events;
std::map<size_t, std::shared_ptr<class Group>> manager;

class Handler {
public:
  Handler(size_t id, const Task &task) : _id(id), _task(task) {
    std::cout << __func__ << ":" << _id << std::endl;
  }
  ~Handler() { std::cout << __func__ << ":" << _id << std::endl; }
  size_t id() const { return _id; }
  void execute(const std::function<void()> &done) {
    std::cout << __func__ << ":" << _id << std::endl;
    _task(done);
  }

private:
  size_t _id;
  Task _task;
};

class Group : public std::enable_shared_from_this<Group> {
public:
  Group(size_t id) : _id(id) {
    std::cout << __func__ << ":" << _id << std::endl;
  }
  ~Group() { std::cout << __func__ << ":" << _id << std::endl; }
  void schedule(std::unique_ptr<Handler> &&handler) {
    _pending.push_back(std::move(handler));
    if (_running) {
      std::ostringstream oss;
      for (auto &handler : _pending)
        oss << handler->id() << " ";

      std::cout << "pending " << _pending.size() << ":" << oss.str()
                << std::endl;
    } else {
      start();
    }
  }

private:
  void start() {
    _running = std::move(_pending.front());
    _pending.pop_front();
    auto done = [wp = weak_from_this()] {
      auto sp = wp.lock();
      assert(sp);
      sp->finish();
    };
    _running->execute(done);
  }

  void finish() {
    std::cout << __func__ << ":" << _running->id() << std::endl;
    auto event = [wp = weak_from_this()] {
      auto sp = wp.lock();
      assert(sp);
      if (sp->_pending.empty())
        manager.erase(sp->_id);
      else
        sp->start();
    };
    events.push(event);
  }

private:
  size_t _id;
  std::unique_ptr<Handler> _running;
  std::list<std::unique_ptr<Handler>> _pending;
};

void loop() {
  while (!events.empty()) {
    std::cout << "<<< event" << std::endl;
    events.front()();
    std::cout << ">>> event" << std::endl;
    events.pop();
  }
}

class Test : public testing::Test {
protected:
  void SetUp() override {
    assert(events.empty());
    assert(manager.empty());
  }

  void TearDown() override {
    while (!_steps.empty()) {
      std::cout << "<<< step" << std::endl;
      _steps.front()();
      std::cout << ">>> step" << std::endl;
      _steps.pop();
      loop();
    }

    assert(events.empty());
    EXPECT_TRUE(manager.empty());
    manager.clear();
  }

protected:
  std::queue<std::function<void()>> _steps;
};

void schedule(size_t id, const Task &task) {
  auto gid = id / 10;
  auto it = manager.find(gid);
  if (it == manager.end()) {
    auto group = std::make_shared<Group>(gid);
    auto p = manager.emplace(gid, group);
    assert(p.second);
    it = p.first;
  }
  auto handler = std::make_unique<Handler>(id, task);
  it->second->schedule(std::move(handler));
}

TEST_F(Test, 0) {
  auto task = [](auto done) { done(); };
  _steps.push(bind(schedule, 11, task));
  _steps.push(bind(schedule, 12, task));
  _steps.push(bind(schedule, 13, task));
}

TEST_F(Test, 1) {
  Task task = [](auto done) {};
  _steps.push(bind(schedule, 11, task));

  task = [](auto done) { done(); };
  _steps.push(bind(schedule, 12, task));
  _steps.push(bind(schedule, 13, task));
}

TEST_F(Test, 2) {
  Task task = [this](auto done) {
    auto task = [call = done](auto done) {
      call(); // 11 done
      done(); // 21 done
    };
    _steps.push(bind(schedule, 21, task));
  };
  _steps.push(bind(schedule, 11, task));

  task = [](auto done) { done(); };
  _steps.push(bind(schedule, 12, task));
  _steps.push(bind(schedule, 13, task));
}
