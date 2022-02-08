#include <syscall.h>
void Main(void)
{
	SwitchTo320X200X256();
	PrintChineseStr(1,6,1,"祝大家新年快乐，万事如意，年年有余！");
	sleep(300);
	// 第一个节目：多彩<福>秀
	Draw_Box(0,0,319,199,0);
	Draw_Box(100,50,200,150,1);
	PrintChineseChar(9,6,7,*(unsigned short *)("福"));
	sleep(100);
	Draw_Box(120,70,220,170,2);
	PrintChineseChar(10,7,7,*(unsigned short *)("福"));
	sleep(100);
	Draw_Box(80,30,180,130,3);
	PrintChineseChar(8,5,7,*(unsigned short *)("福"));
	sleep(100);
	Draw_Box(120,30,220,130,4);
	PrintChineseChar(10,5,7,*(unsigned short *)("福"));
	sleep(100);
	Draw_Box(80,70,180,170,5);
	PrintChineseChar(8,7,7,*(unsigned short *)("福"));
	sleep(100);
	Draw_Box(0,0,100,100,6);
	PrintChineseChar(3,3,7,*(unsigned short *)("福"));
	sleep(100);
	Draw_Box(220,100,319,199,8);
	PrintChineseChar(17,9,7,*(unsigned short *)("福"));
	sleep(100);
	Draw_Box(220,0,319,100,9);
	PrintChineseChar(17,3,7,*(unsigned short *)("福"));
	sleep(100);
	Draw_Box(0,100,100,199,10);
	PrintChineseChar(3,9,7,*(unsigned short *)("福"));
	sleep(200);
	// 第二个节目：红屏循环输出“拜年啦”
	Draw_Box(0,0,319,199,1);
	int x,y;
	for (x = 0,y = 0;;x += 3) {
		if (x == 18) {
			PrintChineseStr(x,y,7,"拜年");
			if (y == 11) {
				break;
			}
			y++;
			x=0;
		}
		PrintChineseStr(x,y,7,"拜年啦");
		sleep(10);
	}
	sleep(200);
	// 第三个节目：游来游去的<福>
	Draw_Box(0,0,319,199,0);
	for (x = 0, y = 0;; x++) {
		if (x+100 == 319) {
			if (y+100 == 200) {
				break;
			}
			y+=100;
			x=0;
		}
		Draw_Box(x-100,y,x,y+100,0);
		Draw_Box(x,y,x+100,y+100,1);
		PrintChineseChar((x+50)/16,(y+50)/16,7,*(unsigned short *)("福"));
		sleep(1);
	}
	sleep(200);
	// 第四个节目：突发恶疾
	int i;
	for (i = 0; i != 50; i++) {
		mysrand(RAND());
		x=(myrand() % 219);
		y=(myrand() % 99);
		Draw_Box(x,y,x+100,y+100,1);
		PrintChineseChar((x+50)/16,(y+50)/16,7,*(unsigned short *)("福"));
		sleep(10);
	}
	sleep(200);
	beep(2,7,8);	// 图形模式结束曲
	SwitchToText8025();
	system("cls");
	int j;
	for (j = 0; j != 25; j++) {
		for (i = 0; i != 4; i++) {
			print("HappySpringFestival");
			sleep(1);
		}
	}
	int color;
	for (i = 0; i != 160*25; i+=2) {
		mysrand(RAND());
		color=(myrand() % 0xff);
		*(char *)(0xb8000 + i + 1) = color;
		sleep(1);
	}
	sleep(200);
	beep(2,7,8);	// 文字模式结束曲
	system("cls");
	print("By zhouzhihao 2022.C LANGUAGE!!!\n");
	print("HAPPY SPRING FESTIVAL EVEYONE!!!\n");
	return;
}
