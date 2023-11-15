#include <cstring>
#include <memory>
#include <random>
#include <raylib.h>
#include <span>
#include <stdint.h>
#include <string>
#include <utility>

// #define DEBUG

#ifdef DEBUG
#include <iostream>
#endif

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

Image image_new(int width, int height) {
  return Image{.data = malloc(width * height * 4),
               .width = width,
               .height = height,
               .mipmaps = 1,
               .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
}
void image_resize(Image &image, int width, int height) {
  delete[] (char *)image.data;
  image.data = new char[width * height * 4];
  image.width = width;
  image.height = height;
}
void texture_reload(Texture &texture, Image &image) {
  UnloadTexture(texture);
  texture = LoadTextureFromImage(image);
}

int main() {
  static_assert(is_alive(0x03), "should be alive");
  static_assert(!is_alive(0x02), "should not be alive");
  static_assert(neighbors(0x04) == 2, "should be 2");
  static_assert(neighbors(0x06) == 3, "should be 3");
  static_assert(wrap(0, 10, 5) == 5, "should be 5");
  static_assert(wrap(0, 10, 0) == 0, "should be 0");
  static_assert(wrap(0, 10, 10) == 0, "should be 0");
  static_assert(wrap(0, 10, 11) == 1, "should be 0");
  static_assert(wrap(0, 10, -1) == 9, "should be 9");
  static_assert(wrap(0, 10, -2) == 8, "should be 9");

  int64_t tgt_window_width = 800;
  int64_t tgt_window_height = 600;
  int64_t scalar = 2;

  bool paused = false;
  GOL gol(tgt_window_width / scalar, tgt_window_height / scalar);

  { // initial randomization
    std::mt19937 rng_gen(std::random_device{}());
    std::uniform_int_distribution<> distrib(0, 100);
    for (std::size_t i = 0; i < gol.array().size(); ++i) {
      if (distrib(rng_gen) <= 50 /* percent */) {
        gol.set(i);
      }
    }
  }

  SetTraceLogLevel(LOG_ERROR);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(tgt_window_width, tgt_window_height, "yagolcpp");

  Image screen_image = image_new(gol.width(), gol.height());
  Texture2D screen_texture = LoadTextureFromImage(screen_image);
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    if (IsWindowResized()) {
      tgt_window_width = GetRenderWidth();
      tgt_window_height = GetRenderHeight();
      gol.resize(tgt_window_width / scalar, tgt_window_height / scalar);
      image_resize(screen_image, gol.width(), gol.height());
      texture_reload(screen_texture, screen_image);
    }
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      auto const mouse_pos = GetMousePosition();
      gol.set(mouse_pos.x / scalar, mouse_pos.y / scalar);
    }
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
      auto const mouse_pos = GetMousePosition();
      gol.clear(mouse_pos.x / scalar, mouse_pos.y / scalar);
    }
    if (IsKeyPressed(KEY_C)) {
      gol.clear();
    }
    if (IsKeyPressed(KEY_SPACE)) {
      paused = !paused;
    }
    if (IsKeyPressed(KEY_RIGHT)) {
      if (paused) {
        gol.iterate();
      }
    }

#ifdef DEBUG
    auto const ts_start_image_update = GetTime();
#endif
    auto const &array = gol.array();
    auto const array_size = array.size();
    for (std::size_t i = 0; i < array_size; ++i) {
      reinterpret_cast<uint32_t *>(screen_image.data)[i] =
          !is_alive(array[i]) * 0x00FFFFFF + 0xFF000000;
    }
#ifdef DEBUG
    auto const ts_end_image_update = GetTime();
#endif

#ifdef DEBUG
    auto const ts_start_update_texture = GetTime();
#endif
    Color *const pixels = LoadImageColors(screen_image);
    UpdateTexture(screen_texture, pixels);
    UnloadImageColors(pixels);
#ifdef DEBUG
    auto const ts_end_update_texture = GetTime();
#endif

#ifdef DEBUG
    auto const ts_start_drawing = GetTime();
#endif
    BeginDrawing();
    {
      Rectangle camera{.x = 0,
                       .y = 0,
                       .width = (float)gol.width(),
                       .height = (float)gol.height()};
      Rectangle screen{.x = 0,
                       .y = 0,
                       .width = (float)tgt_window_width,
                       .height = (float)tgt_window_height};
      DrawTexturePro(screen_texture, camera, screen, {0, 0}, 0.0f, WHITE);

      DrawText(std::to_string(GetFrameTime() * 100).c_str(), 10, 10, 20, BLACK);
      DrawText(std::to_string(1.0 / GetFrameTime()).c_str(), 10, 25, 20, BLACK);

      if (paused) {
        int text_width = MeasureText("PAUSED", 30);
        DrawText("PAUSED", tgt_window_width / 2 - text_width / 2,
                 tgt_window_height / 2.0f + (std::sin(GetTime()) * 100) - 50,
                 30, BLACK);
      }
    }
    EndDrawing();
#ifdef DEBUG
    auto const ts_end_drawing = GetTime();
#endif

#ifdef DEBUG
    auto const ts_start_iterate = GetTime();
#endif
    if (!paused) {
      gol.iterate();
    }
#ifdef DEBUG
    auto const ts_end_iterate = GetTime();
#endif

#ifdef DEBUG
    std::cout << " image: "
              << (ts_end_image_update - ts_start_image_update) * 100
              << " ms texture: "
              << (ts_end_update_texture - ts_start_update_texture) * 100
              << " ms draw: " << (ts_end_drawing - ts_start_drawing) * 100
              << " ms iterate: " << (ts_end_iterate - ts_start_iterate) * 100
              << " ms" << std::endl;
#endif
  }

  UnloadImage(screen_image);
  CloseWindow();

  return 0;
}
