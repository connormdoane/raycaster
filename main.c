#include "pixul.h"
#include "map.h"

#include <math.h>
#include <dirent.h>
#include <string.h>

#define MAX_MAPS 32
#define MAP_DIR "maps"

#define SCREEN_W 500
#define SCREEN_H 200
#define GAME_W 250
#define MINIMAP_X 260
#define MINIMAP_Y 10

typedef struct Position_t {
  double x;
  double y;
  double dirx;
  double diry;
  double camx;
  double camy;
} Position;

Position p = {2.5, 2.5, 1.0, 0.0, 0.0, 0.66};

void reset_player() {
  p.x = 2.5;
  p.y = 2.5;
  p.dirx = 1.0;
  p.diry = 0.0;
  p.camx = 0.0;
  p.camy = 0.66;
}

double lookspeed = 0.03;
double movespeed = 0.03;

void render() {
  for (int i = 0; i < GAME_W; i++) {
    double camx = 2 * i / (double) GAME_W - 1;
    double rayx = p.dirx + p.camx * camx;
    double rayy = p.diry + p.camy * camx;

    int mapx = (int)p.x;
    int mapy = (int)p.y;

    double sidedistx;
    double sidedisty;

    double deltax = fabs(rayx) < 1e-6 ? 1e30 : fabs(1 / rayx);
    double deltay = fabs(rayy) < 1e-6 ? 1e30 : fabs(1 / rayy);

    double walldistance;

    int stepx;
    int stepy;

    if (rayx < 0) {
      stepx = -1;
      sidedistx = (p.x - mapx) * deltax;
    } else {
      stepx = 1;
      sidedistx = (mapx + 1.0 - p.x) * deltax;
    }

    if (rayy < 0) {
      stepy = -1;
      sidedisty = (p.y - mapy) * deltay;
    } else {
      stepy = 1;
      sidedisty = (mapy + 1.0 - p.y) * deltay;
    }

    int hit = 0;
    int side = 0;

    // DDA
    while (!hit) {
      // Jump to the next side
      if (sidedistx < sidedisty) {
        sidedistx += deltax;
        mapx += stepx;
        side = 0;
      } else {
        sidedisty += deltay;
        mapy += stepy;
        side = 1;
      }
      int hit_tile = get_tile(mapx, mapy);
      if (hit_tile && hit_tile != -1) hit = hit_tile;
    }

    walldistance = side ? sidedisty - deltay : sidedistx - deltax;
    if (walldistance < 0.0001) walldistance = 0.0001;

    int lineheight = (int)(SCREEN_H / walldistance);

    int linebottom = -lineheight / 2 + SCREEN_H / 2;
    if (linebottom < 0) linebottom = 0;
    int linetop = lineheight / 2 + SCREEN_H / 2;
    if (linetop >= SCREEN_H) linetop = SCREEN_H - 1;

    P_Color c;
    switch(hit) {
    case 1: c = (P_Color){255, 0, 0, 255};break;
    case 2: c = (P_Color){0, 255, 0, 255};break;
    case 3: c = (P_Color){0, 0, 255, 255};break;
    case 4: c = (P_Color){0, 255, 255, 255};break;
    case 5: c = (P_Color){255, 255, 0, 255};break;
    default: c = (P_Color){255, 255, 255, 255};break;
    }

    if (side) {
      c.r = c.r/2;
      c.g = c.g/2;
      c.b = c.b/2;
    }

    P_SetVerLine(i, linebottom, linetop, c);
  }
}

void keyboard(int* look, int* movex, int* movey) {
  if (P_KeyDown(SDL_SCANCODE_J)) {
    *look = -1;
  } else if (P_KeyDown(SDL_SCANCODE_L)) {
    *look = 1;
  } else {
    *look = 0;
  }

  if (P_KeyDown(SDL_SCANCODE_W)) {
    *movex = 1;
  } else if (P_KeyDown(SDL_SCANCODE_S)) {
    *movex = -1;
  } else {
    *movex = 0;
  }

  if (P_KeyDown(SDL_SCANCODE_D)) {
    *movey = -1;
  } else if (P_KeyDown(SDL_SCANCODE_A)) {
    *movey = 1;
  } else {
    *movey = 0;
  }
}

P_Color get_tile_color(int tile) {
  switch(tile) {
    case 1: return (P_Color){255, 0, 0, 255};
    case 2: return (P_Color){0, 255, 0, 255};
    case 3: return (P_Color){0, 0, 255, 255};
    case 4: return (P_Color){0, 255, 255, 255};
    case 5: return (P_Color){255, 255, 0, 255};
    default: return (P_Color){100, 100, 100, 255};
  }
}

double cast_ray(double rayx, double rayy) {
  int mapx = (int)p.x;
  int mapy = (int)p.y;

  double deltax = fabs(rayx) < 1e-6 ? 1e30 : fabs(1 / rayx);
  double deltay = fabs(rayy) < 1e-6 ? 1e30 : fabs(1 / rayy);

  double sidedistx, sidedisty;
  int stepx, stepy, side = 0;

  if (rayx < 0) {
    stepx = -1;
    sidedistx = (p.x - mapx) * deltax;
  } else {
    stepx = 1;
    sidedistx = (mapx + 1.0 - p.x) * deltax;
  }

  if (rayy < 0) {
    stepy = -1;
    sidedisty = (p.y - mapy) * deltay;
  } else {
    stepy = 1;
    sidedisty = (mapy + 1.0 - p.y) * deltay;
  }

  int hit = 0;
  while (!hit) {
    if (sidedistx < sidedisty) {
      sidedistx += deltax;
      mapx += stepx;
      side = 0;
    } else {
      sidedisty += deltay;
      mapy += stepy;
      side = 1;
    }
    int hit_tile = get_tile(mapx, mapy);
    if (hit_tile && hit_tile != -1) hit = hit_tile;
  }

  double dist = side ? sidedisty - deltay : sidedistx - deltax;
  if (dist < 0.0001) dist = 0.0001;
  return dist;
}

void draw_line(int x0, int y0, int x1, int y1, P_Color color) {
  int dx = abs(x1 - x0);
  int dy = abs(y1 - y0);
  int sx = x0 < x1 ? 1 : -1;
  int sy = y0 < y1 ? 1 : -1;
  int err = dx - dy;

  while (1) {
    P_Set(x0, y0, color);
    if (x0 == x1 && y0 == y1) break;
    int e2 = 2 * err;
    if (e2 > -dy) { err -= dy; x0 += sx; }
    if (e2 < dx) { err += dx; y0 += sy; }
  }
}

void render_minimap() {
  int minimap_size = 180;
  int scale = minimap_size / (m.w > m.h ? m.w : m.h);
  if (scale < 1) scale = 1;

  P_Color empty_color = {40, 40, 40, 255};
  P_Color player_color = {255, 255, 0, 255};
  P_Color fov_color = {255, 200, 0, 180};

  for (int x = 0; x < m.w; x++) {
    for (int y = 0; y < m.h; y++) {
      int tile = get_tile(x, y);
      P_Color c = tile ? get_tile_color(tile) : empty_color;
      for (int px = 0; px < scale; px++) {
        for (int py = 0; py < scale; py++) {
          P_Set(MINIMAP_X + x * scale + px, MINIMAP_Y + y * scale + py, c);
        }
      }
    }
  }

  int player_px = MINIMAP_X + (int)(p.x * scale);
  int player_py = MINIMAP_Y + (int)(p.y * scale);

  int num_rays = 50;
  int prev_end_x = -1, prev_end_y = -1;

  for (int i = 0; i <= num_rays; i++) {
    double t = (double)i / num_rays;
    double rayx = p.dirx + p.camx * (2 * t - 1);
    double rayy = p.diry + p.camy * (2 * t - 1);

    double dist = cast_ray(rayx, rayy);

    int end_x = player_px + (int)(rayx * dist * scale);
    int end_y = player_py + (int)(rayy * dist * scale);

    draw_line(player_px, player_py, end_x, end_y, fov_color);

    if (prev_end_x >= 0) {
      draw_line(prev_end_x, prev_end_y, end_x, end_y, fov_color);
    }
    prev_end_x = end_x;
    prev_end_y = end_y;
  }

  for (int dx = -2; dx <= 2; dx++) {
    for (int dy = -2; dy <= 2; dy++) {
      P_Set(player_px + dx, player_py + dy, player_color);
    }
  }
}

void draw_text(int x, int y, const char* text, P_Color color) {
  int cx = x;
  for (int i = 0; text[i]; i++) {
    char c = text[i];
    if (c >= 'a' && c <= 'z') c -= 32;

    unsigned char patterns[38][5] = {
      {0x7C,0xA2,0x92,0x8A,0x7C}, // 0
      {0x00,0x84,0xFE,0x80,0x00}, // 1
      {0xC4,0xA2,0x92,0x92,0x8C}, // 2
      {0x44,0x82,0x92,0x92,0x6C}, // 3
      {0x30,0x28,0x24,0xFE,0x20}, // 4
      {0x4E,0x8A,0x8A,0x8A,0x72}, // 5
      {0x78,0x94,0x92,0x92,0x60}, // 6
      {0x02,0xE2,0x12,0x0A,0x06}, // 7
      {0x6C,0x92,0x92,0x92,0x6C}, // 8
      {0x0C,0x92,0x92,0x52,0x3C}, // 9
      {0xFC,0x12,0x12,0x12,0xFC}, // A
      {0xFE,0x92,0x92,0x92,0x6C}, // B
      {0x7C,0x82,0x82,0x82,0x44}, // C
      {0xFE,0x82,0x82,0x82,0x7C}, // D
      {0xFE,0x92,0x92,0x92,0x82}, // E
      {0xFE,0x12,0x12,0x12,0x02}, // F
      {0x7C,0x82,0x92,0x92,0x74}, // G
      {0xFE,0x10,0x10,0x10,0xFE}, // H
      {0x00,0x82,0xFE,0x82,0x00}, // I
      {0x40,0x80,0x80,0x80,0x7E}, // J
      {0xFE,0x10,0x28,0x44,0x82}, // K
      {0xFE,0x80,0x80,0x80,0x80}, // L
      {0xFE,0x04,0x08,0x04,0xFE}, // M
      {0xFE,0x04,0x08,0x10,0xFE}, // N
      {0x7C,0x82,0x82,0x82,0x7C}, // O
      {0xFE,0x12,0x12,0x12,0x0C}, // P
      {0x7C,0x82,0xA2,0x42,0xBC}, // Q
      {0xFE,0x12,0x32,0x52,0x8C}, // R
      {0x4C,0x92,0x92,0x92,0x64}, // S
      {0x02,0x02,0xFE,0x02,0x02}, // T
      {0x7E,0x80,0x80,0x80,0x7E}, // U
      {0x3E,0x40,0x80,0x40,0x3E}, // V
      {0x7E,0x80,0x70,0x80,0x7E}, // W
      {0xC6,0x28,0x10,0x28,0xC6}, // X
      {0x06,0x08,0xF0,0x08,0x06}, // Y
      {0xC2,0xA2,0x92,0x8A,0x86}, // Z
      {0x00,0x00,0x00,0x00,0x00}, // space
      {0x00,0x60,0x60,0x00,0x00}, // .
    };

    int idx = 36;
    if (c >= '0' && c <= '9') idx = c - '0';
    else if (c >= 'A' && c <= 'Z') idx = c - 'A' + 10;
    else if (c == '.') idx = 37;

    for (int col = 0; col < 5; col++) {
      for (int row = 0; row < 8; row++) {
        if (patterns[idx][col] & (1 << row)) {
          P_Set(cx + col, y + row, color);
        }
      }
    }
    cx += 6;
  }
}

int select_map(char* selected_path) {
  char map_names[MAX_MAPS][64];
  int map_count = 0;

  DIR* dir = opendir(MAP_DIR);
  if (!dir) return 0;

  struct dirent* entry;
  while ((entry = readdir(dir)) && map_count < MAX_MAPS) {
    if (entry->d_name[0] == '.') continue;
    if (strstr(entry->d_name, "~")) continue;
    strncpy(map_names[map_count], entry->d_name, 63);
    map_names[map_count][63] = '\0';
    map_count++;
  }
  closedir(dir);

  if (map_count == 0) return 0;

  for (int i = 0; i < map_count - 1; i++) {
    for (int j = i + 1; j < map_count; j++) {
      if (strcmp(map_names[i], map_names[j]) > 0) {
        char temp[64];
        strcpy(temp, map_names[i]);
        strcpy(map_names[i], map_names[j]);
        strcpy(map_names[j], temp);
      }
    }
  }

  int selected = 0;
  P_Color white = {255, 255, 255, 255};
  P_Color yellow = {255, 255, 0, 255};
  P_Color gray = {100, 100, 100, 255};

  while (!P_Done()) {
    P_Clear((P_Color){0, 0, 0, 255});

    draw_text(SCREEN_W / 2 - 60, 20, "SELECT MAP", white);
    draw_text(SCREEN_W / 2 - 90, 35, "W.S TO MOVE  ENTER TO SELECT", gray);

    int start_y = 60;
    for (int i = 0; i < map_count; i++) {
      P_Color c = (i == selected) ? yellow : white;
      int y = start_y + i * 15;
      if (i == selected) {
        draw_text(SCREEN_W / 2 - 70, y, ">", yellow);
      }
      draw_text(SCREEN_W / 2 - 55, y, map_names[i], c);
    }

    if (P_KeyPress(SDL_SCANCODE_W) || P_KeyPress(SDL_SCANCODE_UP)) {
      selected = (selected - 1 + map_count) % map_count;
    }
    if (P_KeyPress(SDL_SCANCODE_S) || P_KeyPress(SDL_SCANCODE_DOWN)) {
      selected = (selected + 1) % map_count;
    }
    if (P_KeyPress(SDL_SCANCODE_RETURN) || P_KeyPress(SDL_SCANCODE_SPACE)) {
      snprintf(selected_path, 128, "%s/%s", MAP_DIR, map_names[selected]);
      return 1;
    }
    if (P_KeyPress(SDL_SCANCODE_Q) || P_KeyPress(SDL_SCANCODE_ESCAPE)) {
      return 0;
    }

    P_Update();
  }
  return 0;
}

int main(int argc, char* argv[]) {
  P_Create("Raycaster", 1000, 400, SCREEN_W, SCREEN_H);

  char map_path[128] = "";

  if (argc > 1) {
    strncpy(map_path, argv[1], 127);
    map_path[127] = '\0';
  } else {
    if (!select_map(map_path)) {
      P_Destroy();
      return 0;
    }
  }

  init_map();
  load_map(map_path);

  int look = 0;
  int movex = 0;
  int movey = 0;

  while (!P_Done()) {
    P_Clear((P_Color){0,0,0,255});

    render();
    render_minimap();
    keyboard(&look, &movex, &movey);

    if (look) {
      double olddirx = p.dirx;
      p.dirx = olddirx * cos(lookspeed*look) - p.diry * sin(lookspeed*look);
      p.diry = olddirx * sin(lookspeed*look) + p.diry * cos(lookspeed*look);
      double oldcamx = p.camx;
      p.camx = oldcamx * cos(lookspeed*look) - p.camy * sin(lookspeed*look);
      p.camy = oldcamx * sin(lookspeed*look) + p.camy * cos(lookspeed*look);
    }

    if (P_KeyDown(SDL_SCANCODE_LSHIFT) || P_KeyDown(SDL_SCANCODE_RSHIFT)) {
        movespeed = 0.06;
    } else {
        movespeed = 0.03;
    }

    if (movex) {
      double newx = p.x + p.dirx * movespeed * movex;
      double newy = p.y + p.diry * movespeed * movex;
      if (!get_tile((int)newx, (int)p.y)) p.x = newx;
      if (!get_tile((int)p.x, (int)newy)) p.y = newy;
    }
    if (movey) {
      double newx = p.x + p.diry * movespeed * movey;
      double newy = p.y + -p.dirx * movespeed * movey;
      if (!get_tile((int)newx, (int)p.y)) p.x = newx;
      if (!get_tile((int)p.x, (int)newy)) p.y = newy;
    }

    // CHANGE MAP
    if (P_KeyPress(SDL_SCANCODE_C)) {
      if (select_map(map_path)) {
        load_map(map_path);
        reset_player();
      }
    }

    // QUIT
    if (P_KeyPress(SDL_SCANCODE_Q)) {
      break;
    }

    P_Update();
    /* usleep(10000); */
  }

  destroy_map();
  P_Destroy();

  return 0;
}
