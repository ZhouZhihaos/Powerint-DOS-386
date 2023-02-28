#include <syscall.h>
#include <gui.h>
#include <stdio.h>
#include <rand.h>
int x = 16 * 9, y = 0;
int ox = 16 * 9, oy = 0;
char map[41][20] = {0};
char keyflag[4] = {0};
int PowerMain(int argc, char **argv)
{
  window_t win = create_window(0, 0, 335, 672, "Tetris");
  SDraw_Box(win, 6, 27, 326, 667, 0, 335);
  int time = 1000;
  timer_alloc();
  timer_settime(time);
  mysrand(RAND());
  color_t color = myrand();
  while(1) {
	if (_kbhit()) {
	  char c = getch();
	  if (c == -3) {	// left
	    keyflag[0] = 1;
	  } else if (c == -4) {	// right
	    keyflag[1] = 1;
	  } else if (c == -1) {	// up
	    keyflag[2] = 1;
      } else if (c == -2) { // down
	    keyflag[3] = 1;
	  }
	}
	if (timer_out()) {
      time = 1000;
	  if (keyflag[0] == 1 && x != 0) {	// left
	    x -= 16;
	  }
	  if (keyflag[1] == 1 && x != 304) { 	// right
	    x += 16;
	  }
      if (keyflag[3] == 1) {	// down
	    time = 200;
	  }
	  keyflag[0] = 0;
	  keyflag[1] = 0;
	  keyflag[2] = 0;
	  keyflag[3] = 0;
	  if (y == 640 || map[(y+16)/16][x/16] == ' ') {	// 到最底层了 // 底下有方块
	    map[y/16][x/16] = ' ';
		for (int i = 0; i != 41; i++) {
	      if (strcmp(map[i], "                    ") == 0) {
		    SDraw_Box(win, 6, (i-1)*16+27, 326, (i-1)*16+43, 0, 335);
			for (int j = 0; i != 20; j++) {
			  map[i][j] = 0;
			}
	      }
		}
	    y = 0;
		oy = y;
		x = 16 * 9;
		ox = x;
		mysrand(RAND());
	    color = myrand();
		timer_settime(time);
	  } else {	// 一般情况
	    SDraw_Box(win, ox+6, oy+27, ox+22, oy+43, 0, 335);
	    SDraw_Box(win, x+6, y+27, x+22, y+43, 0x00ffffff, 335);
	    SDraw_Box(win, x+7, y+28, x+21, y+42, color, 335);
	    ox = x;
	    oy = y;
	    y += 16;
	    timer_settime(time);
	  }
	}
    window_t id;
    if(MessageLength(WINDOW_CLOSE_MSG_ID) == 4) {
      GetMessage(&id, WINDOW_CLOSE_MSG_ID);
      if(id == win) {
        close_window(win);
        return 0;
      }
    }
  }
}
