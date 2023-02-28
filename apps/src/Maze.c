#include <math.h>
#include <stdio.h>
#include <syscall.h>

#define getchar getch
#define putchar putch

void setcursor(int x, int y) {
  goto_xy(x, y);
}

typedef struct {
  int x;
  int y;
} coord;

coord player;
coord spawn = {1, 1};
int speed = 1;

typedef struct {
  char show;
  bool passable;
  void (*onstep)(void);
} Block;

void nothing_func(void) {}

Block DIRT = {'~', true, nothing_func};
Block WALL = {'#', false, nothing_func};

void congraduate_func(void) {
  // MessageBox(NULL, "Congratulate!You've passed this section!",
  // "Congratulate!",
  //            MB_OK);
}

Block END = {'@', true, congraduate_func};

void trap_func(void) {
  player = spawn;
}

Block TRAP = {'^', true, trap_func};

void speedup_func(void) {
  speed = 2;
}

void speeddown_func(void) {
  speed = 1;
}

Block SPEEDUP = {'>', true, speedup_func};
Block SPEEDDOWN = {'<', true, speeddown_func};

extern Block EXPLORE;
void explored_func(void) {
  EXPLORE.show = 'D';
  // MessageBox(NULL,
  //            "Wow, you've reach the explore grid!Take the screenshot to the "
  //            "developer!",
  //            "Explore", MB_OK);
  system("cls");
  printf(
      "Wow, you've reach the explore grid!Take the screenshot to the "
      "developer!\n");
}

Block EXPLORE = {'G', true, explored_func};

#define MAP_WIDTH (24)
#define MAP_HEIGHT (24)
Block* map[MAP_WIDTH][MAP_HEIGHT];

bool coord_equal(coord a, coord b) {
  return a.x == b.x && a.y == b.y;
}

void render(void) {
  setcursor(0, 0);
  for (int y = MAP_HEIGHT - 1; y >= 0; y--) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      if (coord_equal(player, (coord){x, y})) {
        putchar('X');
      } else {
        if (map[x][y] != NULL) {
          putchar(map[x][y]->show);
        } else {
          putchar(' ');
        }
      }
    }
    putchar('\n');
  }
}

bool IsKeyPressed(int nVirtKey) {
  if (_kbhit()) {
    unsigned int c = getch();
    if (c & 95 == nVirtKey) {
      return true;
    }
  }
  return false;
}

#define TICK_LEN (20)
#define INPUT_FR (60)
#define INPUT_CNT (INPUT_FR / TICK_LEN)

bool player_passable(coord blk) {
  Block* block = map[blk.x][blk.y];
  return block->passable;
}

void process_input(void) {
  static int counter = INPUT_CNT;

  if (counter < INPUT_CNT) {
    counter++;
    return;
  }

  for (int i = 0; i < speed; i++) {
    coord new_coord = player;
    if (_kbhit()) {
      unsigned int c = getch() & 95;
      if (c == 'W') {
        new_coord.y++;
      }
      if (c == 'A') {
        new_coord.x--;
      }
      if (c == 'S') {
        new_coord.y--;
      }
      if (c == 'D') {
        new_coord.x++;
      }
    }
    if (!coord_equal(new_coord, player) && player_passable(new_coord)) {
      player = new_coord;
      Block* stepon = map[player.x][player.y];
      stepon->onstep();
    }
  }

  counter = 1;
}

void init_map(void) {
  const char* map_str[MAP_HEIGHT] = {
      "########################", "#~~~~~~~~^~~~~~~~~~~~~G#",
      "#~~^^^^^~~^~~~~~~^~~~~~#", "##~~^~~~^~~^~~~~~^~~~~~#",
      "#~~~~~^~~^~~^~~~~~^~~~~#", "#~^^^^#^~~^~~^~~~~~^~~~#",
      "#^~~~~~~^~~^~~^~~~~^~~~#", "^~~^~^#~~~^~^~^~~~~^~~~#",
      "^~^~~~#^^#~^~~^~~~~~~~~#", "^~~~^~^~~~^~~^~~~^~~~~~#",
      "^~#~~~^~^~~~^~~~^~~^~~~#", "^~^#~^#~~^^^~^~~~~~~~^~#",
      "^~~^~~#^~^~~~~~~~~^~~~^#", "^^~#^~^~~^#~~~~^~~~~~~~#",
      "^~~~~~^~^~~<#^~~#~~~~^~#", "^~^~#~#~~^^~~~^~~~~^^~~#",
      "^~~~~~^^~^~~~^~~~^^~^~~#", "###^#~#~~^~^~~~^~~~~~^~#",
      "#~~~~~^~^^~~^~~~^~^~~~~#", "#^#~#^#>~~~~#~#~~~~~~~~#",
      "#~~~~~#~~#~~^~~~~~~~^~~#", "#^#^#~#^^^^#^^^^^^^^^^^#",
      "#~~~~~^~~~~~~~~~~~~~~~@#", "########################"};

  for (int y = MAP_HEIGHT - 1; y >= 0; y--) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      if (map_str[y][x] == '#') {
        map[x][MAP_HEIGHT - y - 1] = &WALL;
      } else if (map_str[y][x] == '^') {
        map[x][MAP_HEIGHT - y - 1] = &TRAP;
      } else if (map_str[y][x] == '~') {
        map[x][MAP_HEIGHT - y - 1] = &DIRT;
      } else if (map_str[y][x] == '@') {
        map[x][MAP_HEIGHT - y - 1] = &END;
      } else if (map_str[y][x] == '>') {
        map[x][MAP_HEIGHT - y - 1] = &SPEEDUP;
      } else if (map_str[y][x] == '<') {
        map[x][MAP_HEIGHT - y - 1] = &SPEEDDOWN;
      } else if (map_str[y][x] == 'G') {
        map[x][MAP_HEIGHT - y - 1] = &EXPLORE;
      } else {
        map[x][MAP_HEIGHT - y - 1] = NULL;
      }
    }
  }
}

int main(int argc, const char** argv) {
  printf("Loading...\n");

  system("cls");

  player = spawn;

  init_map();

  while (true) {
    timer_alloc();
    timer_settime(TICK_LEN);

    render();
    process_input();

    while (!timer_out())
      ;
    timer_free();
  }
  return 0;
}
