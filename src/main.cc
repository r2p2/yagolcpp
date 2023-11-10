#include <cstring>
#include <random>
#include <raylib.h>

#include <memory>
#include <random>
#include <span>
#include <stdint.h>
#include <string>
#include <utility>

constexpr bool is_alive(uint8_t cell) { return cell & 0x80; }

constexpr uint8_t neighbors(uint8_t cell) { return cell & 0x7f; }

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
  GOL(int64_t width, int64_t height) : _rng_gen(std::random_device{}()) {
    resize(width, height);
  }

  int64_t width() const { return _width; }
  int64_t height() const { return _height; }

  std::span<uint8_t> const array() const { return _array_curr; }

  void set(int64_t x, int64_t y) { set(to_index(x, y, width())); }

  void set(int64_t i) { _set(i, _array_curr); }

  void clear(int64_t x, int64_t y) { clear(to_index(x, y, width())); }

  void clear(int64_t i) { _array_curr[i] &= 0x7f; }

  void fill(int64_t p) {
    std::uniform_int_distribution<> distrib(0, 100);
    for (int64_t i = 0; i < _array_curr.size(); ++i) {
      if (distrib(_rng_gen) <= p) {
        set(i);
      } else {
        clear(i);
      }
    }
  }

  void resize(int64_t width, int64_t height) {
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
    if (array[i] & 0x80)
      return;

    auto const &[x, y] = from_index(i, width());

    array[to_index(wrap(0, width(), x - 1), wrap(0, height(), y - 1),
                   width())] += 1;
    array[to_index(wrap(0, width(), x), wrap(0, height(), y - 1), width())] +=
        1;
    array[to_index(wrap(0, width(), x + 1), wrap(0, height(), y - 1),
                   width())] += 1;

    array[to_index(wrap(0, width(), x - 1), wrap(0, height(), y), width())] +=
        1;
    // array[i] += 1;
    array[i] |= 0x80;
    array[to_index(wrap(0, width(), x + 1), wrap(0, height(), y), width())] +=
        1;

    array[to_index(wrap(0, width(), x - 1), wrap(0, height(), y + 1),
                   width())] += 1;
    array[to_index(wrap(0, width(), x), wrap(0, height(), y + 1), width())] +=
        1;
    array[to_index(wrap(0, width(), x + 1), wrap(0, height(), y + 1),
                   width())] += 1;
  }

  void _clear(int64_t i, std::span<uint8_t> &array) {
    if (!(array[i] & 0x80))
      return;

    auto const &[x, y] = from_index(i, width());

    array[to_index(wrap(0, width(), x - 1), wrap(0, height(), y - 1),
                   width())] -= 1;
    array[to_index(wrap(0, width(), x), wrap(0, height(), y - 1), width())] -=
        1;
    array[to_index(wrap(0, width(), x + 1), wrap(0, height(), y - 1),
                   width())] -= 1;

    array[to_index(wrap(0, width(), x - 1), wrap(0, height(), y), width())] -=
        1;
    // array[i] -= 1;
    array[i] &= 0x7f;
    array[to_index(wrap(0, width(), x + 1), wrap(0, height(), y), width())] -=
        1;

    array[to_index(wrap(0, width(), x - 1), wrap(0, height(), y + 1),
                   width())] -= 1;
    array[to_index(wrap(0, width(), x), wrap(0, height(), y + 1), width())] -=
        1;
    array[to_index(wrap(0, width(), x + 1), wrap(0, height(), y + 1),
                   width())] -= 1;
  }

private:
  int64_t _width;
  int64_t _height;

  uint8_t *_array;
  std::span<uint8_t> _array_curr;
  std::span<uint8_t> _array_next;

  std::mt19937 _rng_gen;
};

int main() {
  static_assert(is_alive(0x82), "should be alive");
  static_assert(!is_alive(0x02), "should not be alive");
  static_assert(neighbors(0x82) == 2, "should be 2");
  static_assert(neighbors(0x03) == 3, "should be 3");
  static_assert(wrap(0, 10, 5) == 5, "should be 5");
  static_assert(wrap(0, 10, 0) == 0, "should be 0");
  static_assert(wrap(0, 10, 10) == 0, "should be 0");
  static_assert(wrap(0, 10, 11) == 1, "should be 0");
  static_assert(wrap(0, 10, -1) == 9, "should be 9");
  static_assert(wrap(0, 10, -2) == 8, "should be 9");

  int64_t tgt_window_width = 800;
  int64_t tgt_window_height = 600;
  int64_t scalar = 1;

  bool paused = false;
  GOL gol(tgt_window_width / scalar, tgt_window_height / scalar);

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(tgt_window_width, tgt_window_height, "yagolcpp");
  // SetTargetFPS(60);

  while (!WindowShouldClose()) {
    if (IsWindowResized()) {
      tgt_window_width = GetRenderWidth();
      tgt_window_height = GetRenderHeight();
      gol.resize(tgt_window_width / scalar, tgt_window_height / scalar);
    }
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      auto const mouse_pos = GetMousePosition();
      gol.set(mouse_pos.x / scalar, mouse_pos.y / scalar);
    }
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
      auto const mouse_pos = GetMousePosition();
      gol.clear(mouse_pos.x / scalar, mouse_pos.y / scalar);
    }
    if (IsKeyPressed(KEY_ONE)) {
      gol.fill(10);
    }
    if (IsKeyPressed(KEY_TWO)) {
      gol.fill(20);
    }
    if (IsKeyPressed(KEY_THREE)) {
      gol.fill(30);
    }
    if (IsKeyPressed(KEY_SPACE)) {
      paused = !paused;
    }
    if (IsKeyPressed(KEY_RIGHT)) {
      if (paused) {
        gol.iterate();
      }
    }

    auto const timedelta = GetFrameTime() * 100;
    auto const actual_fps = 1.0 / GetFrameTime();

    auto const &array = gol.array();
    auto const width = gol.width();
    auto const height = gol.width();

    BeginDrawing();
    {
      ClearBackground(RAYWHITE);
      for (int64_t i = 0; i < array.size(); ++i) {
        auto const x = i % width;
        auto const y = i / width;
        if (array[i] & 0x80)
          DrawRectangle(x * scalar, y * scalar, scalar, scalar, GRAY);
      }

      DrawText(std::to_string(timedelta).c_str(), 10, 10, 20, BLACK);
      DrawText(std::to_string(actual_fps).c_str(), 10, 25, 20, BLACK);

      if (paused) {
        DrawText("PAUSED", tgt_window_width / 2,
                 tgt_window_height / 2 + (std::sin(GetTime()) * 100) - 50, 30,
                 BLACK);
      }
    }
    EndDrawing();

    if (!paused) {
      gol.iterate();
    }
  }

  CloseWindow();

  return 0;
}
