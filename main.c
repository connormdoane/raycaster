#include "pixul.h"
#include "map.h"

#include <math.h>

#define w 250
#define h 200

typedef struct Position_t {
  double x;
  double y;
  double dirx;
  double diry;
  double camx;
  double camy;
} Position;

Position p = {2.5, 2.5, 1.0, 0.0, 0.0, 0.66};

double lookspeed = 0.03;
double movespeed = 0.03;

void render() {
  for (int i = 0; i < w; i++) {
    double camx = 2 * i / (double) w - 1;
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

    int lineheight = (int)(h / walldistance);

    int linebottom = -lineheight / 2 + h / 2;
    if (linebottom < 0) linebottom = 0;
    int linetop = lineheight / 2 + h / 2;
    if (linetop >= h) linetop = h - 1;

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

int main() {
  P_Create("Raycaster", 500, 400, w, h);

  init_map(0);
  load_map("maps/map1");

  int look = 0;
  int movex = 0;
  int movey = 0;

  while (!P_Done()) {
    P_Clear((P_Color){0,0,0,255});

    render();
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
