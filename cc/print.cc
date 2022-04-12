#include <iostream>

template <typename T = const char *>
void print(std::ostream &s, const T &t = "") {
  s << t << std::endl;
}

template <typename T, typename... V>
void print(std::ostream &s, const T &t, const V &... v) {
  s << t;
  print(s, v...);
}

#define PRINT(...)                                                             \
  do {                                                                         \
    std::cout << __LINE__ << ": ";                                             \
    print(std::cout, ##__VA_ARGS__);                                           \
  } while (0)

int main() {
  PRINT();
  PRINT("int=", 0, " string=", std::string("string"));
}
