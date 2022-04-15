#include <gtest/gtest.h>

using Count = std::pair<std::atomic_size_t, std::atomic_size_t>;

template <typename> class Weak;

template <typename T = int> class Shared final {
  friend class Weak<T>;

public:
  explicit Shared(T *ptr = nullptr)
      : _ptr(ptr), _cnt(ptr ? new Count(1, 1) : nullptr) {}

  explicit Shared(const Weak<T> &);

  Shared(const Shared &s) {
    if (s._cnt)
      ++s._cnt->first;
    else
      assert(!s._ptr);

    _ptr = s._ptr;
    _cnt = s._cnt;
  }

  Shared &operator=(Shared s) {
    swap(s);
    return *this;
  }

  ~Shared() { reset(); }

  void reset() {
    if (_cnt && --_cnt->first == 0) {
      delete _ptr;
      if (--_cnt->second == 0)
        delete _cnt;
    }

    _ptr = nullptr;
    _cnt = nullptr;
  }

  const Count *count() const { return _cnt; }

private:
  void swap(Shared &s) {
    std::swap(_ptr, s._ptr);
    std::swap(_cnt, s._cnt);
  }

  T *_ptr;
  Count *_cnt;
};

template <typename T = int> class Weak final {
  friend class Shared<T>;

public:
  Weak() = default;

  Weak(const Shared<T> &s) {
    if (s._cnt)
      ++s._cnt->second;
    else
      assert(!s._ptr);

    _ptr = s._ptr;
    _cnt = s._cnt;
  }

  Weak(const Weak &w) {
    if (w._cnt)
      ++w._cnt->second;
    else
      assert(!w._ptr);

    _ptr = w._ptr;
    _cnt = w._cnt;
  }

  Weak &operator=(Weak w) {
    swap(w);
    return *this;
  }

  ~Weak() { reset(); }

  void reset() {
    if (_cnt && --_cnt->second == 0)
      delete _cnt;

    _ptr = nullptr;
    _cnt = nullptr;
  }

  Shared<T> lock() const { return Shared<T>(*this); }

  const Count *count() const { return _cnt; }

private:
  void swap(Weak &w) {
    std::swap(_ptr, w._ptr);
    std::swap(_cnt, w._cnt);
  }

  T *_ptr = nullptr;
  Count *_cnt = nullptr;
};

template <typename T> Shared<T>::Shared(const Weak<T> &w) {
  if (w._cnt) {
    size_t cnt = w._cnt->first;
    do {
      if (cnt == 0) {
        _ptr = nullptr;
        _cnt = nullptr;
        return;
      }
    } while (w._cnt->first.compare_exchange_weak(cnt, cnt + 1));
  } else {
    assert(!w._ptr);
  }

  _ptr = w._ptr;
  _cnt = w._cnt;
}

TEST(s, weak) {
  Weak<> w;
  ASSERT_EQ(w.count(), nullptr);

  Weak<> w0 = w;
  ASSERT_EQ(w0.count(), nullptr);

  w = w0;
  ASSERT_EQ(w.count(), nullptr);

  Shared<> s1(new int);
  ASSERT_EQ(s1.count()->first, 1);
  ASSERT_EQ(s1.count()->second, 1);

  Weak<> w1 = s1;
  ASSERT_EQ(w1.count()->first, 1);
  ASSERT_EQ(w1.count()->second, 2);

  w = s1;
  ASSERT_EQ(w.count()->first, 1);
  ASSERT_EQ(w.count()->second, 3);

  w0 = w;
  ASSERT_EQ(w0.count()->first, 1);
  ASSERT_EQ(w0.count()->second, 4);

  Shared<> s2 = w1.lock();
  ASSERT_EQ(s2.count()->first, 2);
  ASSERT_EQ(s2.count()->second, 4);

  s1.reset();
  ASSERT_EQ(w.count()->first, 1);
  ASSERT_EQ(w.count()->second, 4);

  s2.reset();
  ASSERT_EQ(w.count()->first, 0);
  ASSERT_EQ(w.count()->second, 3);

  Shared<> s = w.lock();
  ASSERT_EQ(s.count(), nullptr);

  w.reset();
  w0.reset();
  w1.reset();
  ASSERT_EQ(w.count(), nullptr);
}
