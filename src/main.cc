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

void random_fill(GOL &gol) {
  std::mt19937 rng_gen(std::random_device{}());
  std::uniform_int_distribution<> distrib(0, 100);
  for (std::size_t i = 0; i < gol.array().size(); ++i) {
    if (distrib(rng_gen) <= 50 /* percent */) {
      gol.set(i);
    }
  }
}

int main() {
  static_assert(is_alive(0x03), "should be alive");
  static_assert(!is_alive(0x02), "should not be alive");
  static_assert(neighbors(0x04) == 2, "should be 2");
  static_assert(neighbors(0x06) == 3, "should be 3");

  int64_t tgt_window_width = 800;  // superfluous -> use screen.width
  int64_t tgt_window_height = 600; // superfluous -> use screen.height
  int64_t scalar = 1;

  bool paused = false;
  GOL gol(tgt_window_width / scalar, tgt_window_height / scalar);

  { // initial randomization
    random_fill(gol);
  }

  SetTraceLogLevel(LOG_ERROR);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(tgt_window_width, tgt_window_height, "yagolcpp");

  Rectangle screen{.x = 0,
                   .y = 0,
                   .width = (float)tgt_window_width,
                   .height = (float)tgt_window_height};

  Rectangle camera{.x = 0,
                   .y = 0,
                   .width = (float)gol.width(),
                   .height = (float)gol.height()};

  Rectangle camera_target{.x = 0,
                          .y = 0,
                          .width = (float)gol.width(),
                          .height = (float)gol.height()};

  Image screen_image = image_new(gol.width(), gol.height());
  Texture2D screen_texture = LoadTextureFromImage(screen_image);
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    auto const frame_time = GetFrameTime();
    if (IsWindowResized()) {
      tgt_window_width = GetRenderWidth();
      tgt_window_height = GetRenderHeight();
      screen.width = tgt_window_width;
      screen.height = tgt_window_height;
      camera_target.width = screen.width;
      camera_target.height = screen.height;
      camera.width = screen.width;
      camera.height = screen.height;
      gol.resize(tgt_window_width / scalar, tgt_window_height / scalar);
      random_fill(gol);

      image_resize(screen_image, gol.width(), gol.height());
      texture_reload(screen_texture, screen_image);
    }
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      Vector2 mouse_delta = GetMouseDelta();
      camera_target.x -= camera.width * mouse_delta.x / screen.width;
      camera_target.y -= camera.height * mouse_delta.y / screen.height;
    }
    if (IsMouseButtonUp(MOUSE_BUTTON_LEFT)) {
      camera_target.x = camera_target.x;
      camera_target.y = camera_target.y;
    }
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
      auto const mouse_pos = GetMousePosition();
      gol.clear(mouse_pos.x / scalar, mouse_pos.y / scalar);
    }
    if (IsKeyPressed(KEY_SPACE)) {
      auto const mouse_pos = GetMousePosition();
      auto const x = camera.x + camera.width * mouse_pos.x / screen.width;
      auto const y = camera.y + camera.height * mouse_pos.y / screen.height;

      if (gol.is_set(x, y)) {
        gol.clear(x, y);
      } else {
        gol.set(x, y);
      }
    }
    if (IsKeyPressed(KEY_C)) {
      gol.clear();
    }
    if (IsKeyPressed(KEY_R)) {
      random_fill(gol);
    }
    if (IsKeyPressed(KEY_P)) {
      paused = !paused;
    }
    if (IsKeyPressed(KEY_RIGHT)) {
      if (paused) {
        gol.iterate();
      }
    }

    { // tug camera along
      auto const mouse_wheel_move = GetMouseWheelMove();
      if (mouse_wheel_move) {
        auto const camera_delta_width =
            camera.width * 0.10 * mouse_wheel_move; // mouse_wheel_move;
        auto const camera_delta_height =
            camera.height * 0.10 * mouse_wheel_move; // mouse_wheel_move;

        Vector2 mouse_pos = GetMousePosition();
        camera_target.x += (mouse_pos.x / screen.width) * camera_delta_width;
        camera_target.y += (mouse_pos.y / screen.height) * camera_delta_height;

        camera_target.width -= camera_delta_width;
        camera_target.height -= camera_delta_height;
      }
      camera.x += (camera_target.x - camera.x) * 5 * frame_time;
      camera.y += (camera_target.y - camera.y) * 5 * frame_time;
      camera.width += (camera_target.width - camera.width) * 5 * frame_time;
      camera.height += (camera_target.height - camera.height) * 5 * frame_time;

      // wrap around
      if (camera_target.x == camera.y && camera_target.y == camera.y) {
        camera.x = (int)camera.x % (int)screen.width;
        camera.y = (int)camera.y % (int)screen.height;
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
