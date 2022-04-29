#include <syscall.h>
#include <rand.h>
#define gotoxy goto_xy

#define High 17 //游戏画面尺寸
#define Width 50

//全局变量
int cells[High][Width]; //所有位置细胞 生是1，死是0
int temp = 0;			//判断是否加速
char input = 0;

void startup() //数据的初始化
{
	int i, j;
	mysrand(RAND());
	for (i = 0; i < High; i++) //随机初始化
	{
		for (j = 0; j < Width; j++)
		{
			cells[i][j] = myrand() % 2;
		}
	}
}

void show() //显示画面
{
	gotoxy(0, 0); //将光标移动到原点位置，以下重画清屏
	int i, j;
	for (i = 0; i <= High; i++)
	{
		for (j = 0; j <= Width; j++)
		{
			if (cells[i][j] == 1)
				print("*"); //输出活的细胞
			else
				print(" ");
		}
		print("\n");
	}
	print("===================================================\n");
	print("                    MENU                          +\n");
	print("1. Press W accelerate                             +\n");
	print("2. Press the s key to slow down                   +\n");
	print("3. Press the space to return to the initial speed +\n");
	print("4. Press R to reset                               +\n");
	print("===================================================\n");
	if (temp == 0)
		sleep(6);
	if (temp == 1)
		sleep(3);
	if (temp == 2)
		sleep(12);
}

void updateWithoutInput() //与用户输入无关的更新
{
	int NewCells[High][Width]; //下一帧的细胞情况
	int NeibourNumber;		   //统计邻居的个数
	int i, j;
	for (i = 1; i <= High - 1; i++)
	{
		for (j = 1; j <= Width - 1; j++)
		{
			NeibourNumber = cells[i - 1][j - 1] + cells[i - 1][j] + cells[i - 1][j + 1] + cells[i][j - 1] +
							cells[i][j + 1] + cells[i + 1][j - 1] + cells[i + 1][j] + cells[i + 1][j + 1];
			if (NeibourNumber == 3)
				NewCells[i][j] = 1;
			else if (NeibourNumber == 2)
				NewCells[i][j] = cells[i][j];
			else
				NewCells[i][j] = 0;
		}
	}
	for (i = 1; i <= High - 1; i++)
	{
		for (j = 1; j <= Width - 1; j++)
			cells[i][j] = NewCells[i][j];
	}
}

void updateWithInput() //与用户输入有关的更新
{
	if (_kbhit())
	{
		input = getch();
		if (input == 'w' || input == 'W')
			temp = 1;
		if (input == 's' || input == 'S')
			temp = 2;
		if (input == ' ')
			temp = 0;
		if (input == 'r' || input == 'R')
			startup(); //数据的初始化
	}
}

int main(int argc,char **argv)
{
	startup(); //数据的初始化
	while (1)
	{
		show();				  //显示画面
		updateWithoutInput(); //与用户输入无关的更新
		updateWithInput();	  //与用户输入有关的更新
	}
	return 0;
}
