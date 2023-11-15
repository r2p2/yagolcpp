#pragma once

#include <cstdint>
#include <utility>
#include <span>
#include <cstring>

constexpr bool is_alive(uint8_t cell) { return cell & 0x01; }

constexpr uint8_t neighbors(uint8_t cell) { return cell >> 1; }

constexpr std::pair<int64_t, int64_t> from_index(int64_t i, int64_t width) {
  return std::make_pair(i % width, i / width);
}

constexpr int64_t to_index(int64_t x, int64_t y, int64_t width) {
  return y * width + x;
}

constexpr int64_t wrap(int64_t start, int64_t limit, int64_t n) {
  return ((n % (limit - start)) + (limit - start)) % (limit - start) + start;
}

class GOL {
public:
  GOL(int64_t width, int64_t height) { resize(width, height); }

  ~GOL() {
    if (_array)
      delete[] _array;
  }

  int64_t width() const { return _width; }
  int64_t height() const { return _height; }

  std::span<uint8_t> const array() const { return _array_curr; }

  void set(int64_t x, int64_t y) { set(to_index(x, y, width())); }

  void set(int64_t i) { _set(i, _array_curr); }

  void clear(int64_t x, int64_t y) { clear(to_index(x, y, width())); }

  void clear(int64_t i) { _clear(i, _array_curr); }

  void clear() { memset(_array, 0x00, width() * height()); }

  void resize(int64_t width, int64_t height) {
    if (_array)
      delete[] _array;

    _width = width;
    _height = height;
    _array = new uint8_t[_width * _height * 2]{};
    _array_curr = std::span<uint8_t>(_array, _width * _height);
    _array_next =
        std::span<uint8_t>(_array + _width * _height, width * _height);
  }

  void iterate() {
    memcpy(_array_next.data(), _array_curr.data(), _array_curr.size());
    for (int64_t i = 0; i < _array_curr.size(); ++i) {
      auto const &cell = _array_curr[i];
      auto const &nr_neighbors = neighbors(cell);
      if (is_alive(cell)) {
        if (nr_neighbors < 2 or nr_neighbors > 3) {
          _clear(i, _array_next);
        }
      } else if (nr_neighbors == 3) {
        _set(i, _array_next);
      }
    }
    std::swap(_array_curr, _array_next);
  }

private:
  void _set(int64_t i, std::span<uint8_t> &array) {
    if (is_alive(array[i]))
      return;

    auto const &[x, y] = from_index(i, width());

    array[to_index(wrap(0, width(), x - 1), wrap(0, height(), y - 1),
                   width())] += 2;
    array[to_index(wrap(0, width(), x), wrap(0, height(), y - 1), width())] +=
        2;
    array[to_index(wrap(0, width(), x + 1), wrap(0, height(), y - 1),
                   width())] += 2;

    array[to_index(wrap(0, width(), x - 1), wrap(0, height(), y), width())] +=
        2;
    array[i] += 1;
    array[to_index(wrap(0, width(), x + 1), wrap(0, height(), y), width())] +=
        2;

    array[to_index(wrap(0, width(), x - 1), wrap(0, height(), y + 1),
                   width())] += 2;
    array[to_index(wrap(0, width(), x), wrap(0, height(), y + 1), width())] +=
        2;
    array[to_index(wrap(0, width(), x + 1), wrap(0, height(), y + 1),
                   width())] += 2;
  }

  void _clear(int64_t i, std::span<uint8_t> &array) {
    if (!is_alive(array[i]))
      return;

    auto const &[x, y] = from_index(i, width());

    array[to_index(wrap(0, width(), x - 1), wrap(0, height(), y - 1),
                   width())] -= 2;
    array[to_index(wrap(0, width(), x), wrap(0, height(), y - 1), width())] -=
        2;
    array[to_index(wrap(0, width(), x + 1), wrap(0, height(), y - 1),
                   width())] -= 2;

    array[to_index(wrap(0, width(), x - 1), wrap(0, height(), y), width())] -=
        2;
    array[i] -= 1;
    array[to_index(wrap(0, width(), x + 1), wrap(0, height(), y), width())] -=
        2;

    array[to_index(wrap(0, width(), x - 1), wrap(0, height(), y + 1),
                   width())] -= 2;
    array[to_index(wrap(0, width(), x), wrap(0, height(), y + 1), width())] -=
        2;
    array[to_index(wrap(0, width(), x + 1), wrap(0, height(), y + 1),
                   width())] -= 2;
  }

private:
  int64_t _width = 0;
  int64_t _height = 0;

  uint8_t *_array = 0;
  std::span<uint8_t> _array_curr;
  std::span<uint8_t> _array_next;
};
