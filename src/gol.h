#pragma once

#include "types.h"

#include <cstdint>
#include <cstring>
#include <utility>

constexpr bool is_alive(uint8_t cell) { return cell & 0x01; }

constexpr uint8_t neighbors(uint8_t cell) { return cell >> 1; }

constexpr std::pair<int64_t, int64_t> from_index(int64_t i, int64_t width) {
  return std::make_pair(i % width, i / width);
}

constexpr int64_t to_index(int64_t x, int64_t y, int64_t width) {
  return y * width + x;
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

  Span<uint8_t> const array() const { return _array_curr; }

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
    _array_curr = Span<uint8_t>(_array, _width * _height);
    _array_next = Span<uint8_t>(_array + _width * _height, width * _height);
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
  void _set(int64_t i, Span<uint8_t> &array) {
    if (is_alive(array[i]))
      return;

    _change(i, array, +1);
  }

  void _clear(int64_t i, Span<uint8_t> &array) {
    if (!is_alive(array[i]))
      return;

    _change(i, array, -1);
  }

  void _change(int64_t i, Span<uint8_t>& array, int dir) {
    auto const &[x, y] = from_index(i, width());

    auto const xp = x - 1 >= 0 ? x - 1 : width() - 1;
    auto const xn = x + 1 < width() ? x + 1 : 0;
    auto const yp = y - 1 >= 0 ? y - 1 : height() - 1;
    auto const yn = y + 1 < height() ? y + 1 : 0;

    auto const nb_change = 2*dir;

    array[to_index(xp, yp, width())] += nb_change;
    array[to_index(x, yp, width())] += nb_change;
    array[to_index(xn, yp, width())] += nb_change;

    array[to_index(xp, y, width())] += nb_change;
    array[i] += dir;
    array[to_index(xn, y, width())] += nb_change;

    array[to_index(xp, yn, width())] += nb_change;
    array[to_index(x, yn, width())] += nb_change;
    array[to_index(xn, yn, width())] += nb_change;
  }

private:
  int64_t _width = 0;
  int64_t _height = 0;

  uint8_t *_array = 0;
  Span<uint8_t> _array_curr;
  Span<uint8_t> _array_next;
};
