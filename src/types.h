#pragma once

#include <cstddef>
#include <stdexcept>

template <class T> class Span {
public:
  Span() = default;
  Span(T *ptr, std::size_t size) : _ptr(ptr), _size(size) {}

  T const &operator[](std::size_t i) const {
#ifdef SLOW
    if (i >= _size)
      throw std::out_of_range("i >= size");
#endif
    return _ptr[i];
  }

  T &operator[](std::size_t i) {
#ifdef SLOW
    if (i >= _size)
      throw std::out_of_range("i >= size");
#endif
    return _ptr[i];
  }

  constexpr T *data() const { return _ptr; }
  constexpr std::size_t size() const { return _size; }

private:
  T *_ptr = nullptr;
  std::size_t _size = 0;
};
