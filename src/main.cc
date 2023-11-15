#include "gol.h"

#include <memory>
#include <random>
#include <raylib.h>
#include <string>

// #define DEBUG

#ifdef DEBUG
#include <iostream>
#endif

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
    UpdateTexture(screen_texture, screen_image.data);
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
