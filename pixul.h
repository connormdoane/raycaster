#ifndef PIXUL_H_
#define PIXUL_H_

#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct P_Color_t {
  int r;
  int g;
  int b;
  int a;
} P_Color;

typedef P_Color** P_Image;

typedef struct P_Window_t {
  int w; // width
  int h; // height
  int pw; // pixel width
  int ph; // pixel height
  int ps; // pixel size
  SDL_Window* window;
  SDL_Renderer* renderer;
  P_Image image;
} P_Window;

P_Window* window;

const Uint8* keys;
bool* pressed;

void P_Create(char* title, int width, int height, int pixel_width, int pixel_height) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stderr, "[SDL_Init Error] %s\n", SDL_GetError());
    return;
  }
  window = malloc(sizeof(P_Window));
  window->w  = width;
  window->h  = height;
  window->pw = pixel_width;
  window->ph = pixel_height;

  window->ps = width / pixel_width;
  if (height / pixel_height < window->ps) window->ps = height / pixel_height;

  window->window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
  if (window->window == NULL) {
    fprintf(stderr, "[SDL_CreateWindow Error] %s\n", SDL_GetError());
    SDL_Quit();
    return;
  }

  window->renderer = SDL_CreateRenderer(window->window, -1, SDL_RENDERER_SOFTWARE);
  if (window->renderer == NULL) {
    fprintf(stderr, "[SDL_CreateRenderer Error] %s\n", SDL_GetError());
    SDL_DestroyWindow(window->window);
    SDL_Quit();
    return;
  }

  window->image = calloc(pixel_width, sizeof(P_Color*));
  for (int i = 0; i < pixel_width; i++) {
    window->image[i] = calloc(pixel_height, sizeof(P_Color));
  }

  pressed = calloc(512, sizeof(bool));
}

void P_DestroyImage(P_Image image) {
  for (int i = 0; i < window->pw; i++) {
    free(image[i]);
  }
  free(window->image);
}

void P_Destroy() {
  P_DestroyImage(window->image);

  free(pressed);

  SDL_DestroyRenderer(window->renderer);
  SDL_DestroyWindow(window->window);
  SDL_Quit();
}

void P_Update() {
  SDL_SetRenderDrawColor(window->renderer, 255, 255, 255, 255);
  SDL_RenderClear(window->renderer);

  for (int i = 0; i < window->pw; i++) {
    for (int j = 0; j < window->ph; j++) {
      SDL_Rect rect = {i * window->ps, j * window->ps, window->ps, window->ps};
      SDL_SetRenderDrawColor(window->renderer, window->image[i][j].r, window->image[i][j].g, window->image[i][j].b, window->image[i][j].a);
      SDL_RenderFillRect(window->renderer, &rect);
    }
  }

  SDL_RenderPresent(window->renderer);
}

void P_Clear(P_Color color) {
  for (int i = 0; i < window->pw; i++) {
    for (int j = 0; j < window->ph; j++) {
      window->image[i][j].r = color.r;
      window->image[i][j].g = color.g;
      window->image[i][j].b = color.b;
      window->image[i][j].a = color.a;
    }
  }
}

int P_KeyDown(SDL_Scancode key) {
  return keys[key] != 0;
}

int P_KeyPress(SDL_Scancode key) {
  if (keys[key]) {
    if (!pressed[key]) {
      pressed[key] = true;
      return 1;
    }
  }
  else pressed[key] = false;
  return 0;
}

int P_Done() {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT) {
      return 1;
    }
  }
  SDL_PumpEvents();
  keys = SDL_GetKeyboardState(NULL);
  return 0;
}

void P_Set(int x, int y, P_Color color) {
  if (x < 0 || y < 0 || x >= window->pw || y >= window->ph) return;
  window->image[x][y].r = color.r;
  window->image[x][y].g = color.g;
  window->image[x][y].b = color.b;
  window->image[x][y].a = color.a;
}

void P_SetImage(P_Image image) {
  P_DestroyImage(window->image);
  window->image = image;
}

#endif 
