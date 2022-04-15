#include <gtest/gtest.h>

std::map<char, std::function<void(char)>> callbacks;

void recvMsgs(std::function<bool()> until) {
  while (!until()) {
    char msg;
    auto n = read(STDIN_FILENO, &msg, sizeof(msg));
    if (n > 0 && msg != '\n')
      callbacks.at(msg)(msg);
  }
}

class Test : public testing::Test {
protected:
  static void SetUpTestCase() { std::cout << __func__ << std::endl; }

  void SetUp() override {
    std::cout << __func__ << std::endl;
    for (char msg = 'a'; msg < 'z'; ++msg) {
      callbacks[msg] = [this](auto msg) {
        std::cout << msg << std::endl;
        ++_msgs;
      };
    }
  }

protected:
  size_t _msgs = 0;
};

TEST_F(Test, test) {
  recvMsgs([this] { return _msgs > 3; });
  EXPECT_EQ(_msgs, 4);
}
