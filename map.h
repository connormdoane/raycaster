#ifndef MAP_H_
#define MAP_H_

#include <stdio.h>

typedef struct Map_t {
  int w;
  int h;
  Uint8** grid;
} Map;

Map m;

void init_map() {
  m.w = 7;
  m.h = 7;
  m.grid = malloc(m.w * sizeof(Uint8*));
  for (int i = 0; i < m.w; i++) {
    m.grid[i] = malloc(m.h * sizeof(Uint8));
    for (int j = 0; j < m.h; j++) {
      m.grid[i][j] = 0;
      if (i == 0 || j == 0 || i == m.w-1 || j == m.h-1) m.grid[i][j] = 1;
    }
  }
  m.grid[3][3] = 2;
}

void destroy_map() {
  for (int i = 0; i < m.w; i++) {
    free(m.grid[i]);
  }
  free(m.grid);
}

void load_map(char* filename) {
  destroy_map();
  m.w = 0;
  m.h = 0;

  char ch;
  FILE *fptr = fopen(filename, "r");

  while ((ch = fgetc(fptr)) != '\n') {
    m.w *= 10;
    m.w += ch - '0';
  }
  while ((ch = fgetc(fptr)) != '\n') {
    m.h *= 10;
    m.h += ch - '0';
  }

  m.grid = malloc(m.w * sizeof(Uint8*));
  for (int i = 0; i < m.w; i++) {
    m.grid[i] = malloc(m.h * sizeof(Uint8));
  }

  int cr = 0;
  int cc = 0;
  while ((ch = fgetc(fptr)) != EOF) {
    if (ch != '\n') {
      m.grid[cr][cc] = ch - '0';
      cr++;
    } else {
      cr = 0;
      cc++;
    }
  }

  fclose(fptr);
}

Uint8 get_tile(int x, int y) {
  if (x < 0 || y < 0 || x >= m.w || y >= m.h) return -1;
  return m.grid[x][y];
}

#endif
