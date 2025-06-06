#include <cinttypes>

#include "ytlib/misc/misc_macro.h"

#include "raylib.h"

int32_t main(int32_t argc, char** argv) {
  DBG_PRINT("-------------------start game-------------------");

  const int screenWidth = 800;
  const int screenHeight = 450;

  InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    BeginDrawing();

    ClearBackground(RAYWHITE);

    DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);

    EndDrawing();
  }

  CloseWindow();

  DBG_PRINT("********************end game*******************");
  return 0;
}
