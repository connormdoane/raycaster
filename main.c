#include "pixul.h"

#define w 500
#define h 400

void render() {

}

int main() {
  P_Create("Raycaster", 500, 400, w, h);

  int** board = init_board();
  setup();

  while (!P_Done()) {
    P_Clear((P_Color){0,0,0,255});

    render();

    // MOVE
    if (P_KeyPress(SDL_SCANCODE_W)) {
      if (direction != 1) direction = 0;
    }
    if (P_KeyPress(SDL_SCANCODE_S)) {
      if (direction != 0) direction = 1;
    }
    if (P_KeyPress(SDL_SCANCODE_A)) {
      if (direction != 3) direction = 2;
    }
    if (P_KeyPress(SDL_SCANCODE_D)) {
      if (direction != 2) direction = 3;
    }

    // QUIT
    if (P_KeyPress(SDL_SCANCODE_Q)) {
      break;
    }

    P_Update();
    usleep(10000);
  }

  destroy_board(board);
  P_Destroy();

  return 0;
}
