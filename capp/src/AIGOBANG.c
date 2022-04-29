#include <syscall.h>
#include <rand.h>
#define HEIGHT 19
#define WIDTH 19
int map[HEIGHT][WIDTH];
int Is_Player = 1;
static int x, y;
int Win()
{

    int i,j;
    for(i=0;i<19;i++)
    {
        for(j=0;j<19;j++)
        {
            int w_count=0,b_count=0;
            int k,l;
            for(k=0;k<5;k++)
            {
                if(map[i][j]==1)
                {
                    if(map[i+k][j]==1)
                        b_count++;
                    else
                        break;
                }
                else if(map[i][j]==2)
                {
                    if(map[i+k][j]==2)
                        w_count++;
                    else
                        break;
                }
            }
            if(b_count==5)
                return 1;
            else if(w_count==5)
                return 2;
            else
            {
                b_count=0;
                w_count=0;
            }
            for(l=0;l<5;l++)
            {
                if(map[i][j]==1)
                {
                    if(map[i][j+l]==1)
                        b_count++;
                    else
                        break;
                }
                else if(map[i][j]==2)
                {
                    if(map[i][j+l]==2)
                        w_count++;
                    else
                        break;
                }
            }
            if(b_count==5)
                return 1;
            else if(w_count==5)
                return 2;
            else
            {
                b_count=0;
                w_count=0;
            }
            for(k=0;k<5;k++)
            {
                if(map[i][j]==1)
                {
                    if(map[i+k][j+k]==1)
                        b_count++;
                    else
                        break;
                }
                else if(map[i][j]==2)
                {
                    if(map[i+k][j+k]==2)
                        w_count++;
                    else
                        break;
                }
            }
            if(b_count==5)
                return 1;
            else if(w_count==5)
                return 2;
            else
            {
                b_count=0;
                w_count=0;
            }
            for(k=0;k<5;k++)
            {
                if(map[i][j]==1)
                {
                    if(map[i-k][j+k]==1)
                        b_count++;
                    else
                        break;
                }
                else if(map[i][j]==2)
                {
                    if(map[i-k][j+k]==2)
                        w_count++;
                    else
                        break;
                }
            }
            if(b_count==5)
                return 1;
            else if(w_count==5)
                return 2;
            else
            {
                b_count=0;
                w_count=0;
            }

        }
    }
}
void ViewMap()
{
	goto_xy(0, 0);
	for (int i = 0; i < HEIGHT; i++)
	{
		for (int j = 0; j < WIDTH; j++)
		{
			if (map[i][j] == 0)
			{
				if (i != x || j != y)
				{
					putch(' ');
				}
				else
				{
					putch('>');
				}
			}
			else if (map[i][j] == 1)
			{
				print("O");
			}
			else if (map[i][j] == 2)
			{
				print("@");
			}
		}
		print("\n");
	}
}
int main(int argc,char **argv)
{
	//  print("This program is a bad\n");
	//  return;
	system("cls");
	while (1)
	{
		ViewMap();
		char ch = getch();
		if (ch == 'w')
		{
			if (x > 0)
			{
				x--;
			}
		}
		if (ch == 's')
		{
			if (x < HEIGHT - 1)
			{
				x++;
			}
		}
		if (ch == 'a')
		{
			if (y > 0)
			{
				y--;
			}
		}
		if (ch == 'd')
		{
			if (y < WIDTH - 1)
			{
				y++;
			}
		}
		if (ch == 10)
		{
			if (map[x][y] == 0)
			{
				map[x][y] = 1;

				gobang_ai();
			}
		}
		if (Win() == 1)
		{
			system("cls");
			print("Black Win!\n");
			break;
		}
		else if (Win() == 2)
		{
			system("cls");
			print("White Win!\n");
			break;
		}
	}
	return 0;
}
void random_x_y(int *x, int *y)
{ /*随机落子*/
	int a = 0;
	int b = 0;

	mysrand(RAND());
	while (1)
	{

		a = myrand() % 18 + 0;
		b = myrand() % 18 + 0;
		if (map[a][b] == 0)
		{
			break;
		}
	}
	*x = a;
	*y = b;
}

/*
 * 函数功能：判断是否有威胁（对ai）或者进攻[doge] 并且返回出对应的x y值
 * 函数原型：void who_is_win_for_ai_get_weixie(int *war,int *x,int *y)
 *
 *


*/

void who_is_win_for_ai_get_weixie(int *war, int *x, int *y)
{

	/**
	 * 可造成威胁的有     4子与3子
	 * 我们要将这个函数魔改
	 * 功能如下：
	 * 1.判断是否有活三 并发出警告 ----funish
	 * 2.判断是否有活二 并发出警告 ----finish
	 * 3.判断是否有活一 并发出警告 ----finish
	 * 4.判断是否有活四 并发出警告 ----finish
	 * 5.判断是否有眠三 并发出警告 ----finish
	 * 6.判断是否有眠四 并发出警告 ----finish
	 * 7.判断是否有眠二 并发出警告 ----finish
	 * 8.判断是否有眠一 并发出警告 ----finish
	 *
	 *
	 * 顺序：    (4)>(6)>(1)>(5)>(2)>(7)>(3)>(8)
	 * 参数为0 === 判断是否有威胁{
	 * 如果 有威胁 返回 1
	 * 没有则返回 0
	 *
	 * }
	 * 参数为1 === 获取有威胁地方的x值
	 * 参数为2 === 获取有威胁地方的y值
	 *
	 *	附加：进攻的我还没加进去[doge]
	附：进攻部分的变量名：player 实际指的是AI的连字子数量

	*/

	/****************检测AI是否可以直接获胜（属于进攻部分）*********************/
	//判断是否有活四
	//正着斜
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			if (map[i - 1][j - 1] != 1 && map[i + 4][j + 4] != 1 && map[i - 1][j - 1] != 2 && map[i + 4][j + 4] != 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				for (int k = 0; k < 4; k++)
				{
					if (map[i + k][j + k] == 2)
					{ //判断是否连子
						player++;
					}
				}
			}
			if (player == 4)
			{
				*war = 1; //代表有威胁
				*y = j + 4;
				*x = i + 4;
				return;
			}
		}
	}
	//反着斜
	/**
	 * 向上：(x-n)(y+n)
	 * 向下：(x+n)(y-n)
	 *
	 *
	 */
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			if (map[i - 1][j + 1] != 1 && map[i + 4][j - 4] != 1 && map[i - 1][j + 1] != 2 && map[i + 4][j - 4] != 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				for (int k = 0; k < 4; k++)
				{
					if (map[i + k][j - k] == 2)
					{
						player++;
					}
				}
			}
			if (player == 4)
			{
				*war = 1; //代表有威胁
				*y = j - 4;
				*x = i + 4;
				return;
			}
		}
	}
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			if (map[i][j - 1] != 1 && map[i][j + 4] != 1 && map[i][j - 1] != 2 && map[i][j + 4] != 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				for (int k = 0; k < 4; k++)
				{
					if (map[i][j + k] == 2)
					{
						player++;
					}
				}
			}
			if (player == 4)
			{
				*war = 1; //代表有威胁
				*y = j + 4;
				*x = i;
				return;
			}
		}
	}
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			if (map[i - 1][j] != 2 && map[i + 4][j] != 1 && map[i - 1][j] != 2 && map[i + 4][j] != 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				for (int k = 0; k < 4; k++)
				{
					if (map[i + k][j] == 2)
					{
						player++;
					}
				}
			}
			if (player == 4)
			{
				*war = 1; //代表有威胁
				*y = j;
				*x = i + 4;
				return;
			}
		}
	}
	//判断是否有眠四并发出警告
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			int mode = 0;
			if (map[i - 1][j - 1] != 2 && map[i - 1][j - 1] != 1 && map[i + 4][j + 4] == 1 && map[i + 4][j + 4] == 1)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				mode = 1;
				for (int k = 0; k < 4; k++)
				{
					if (map[i + k][j + k] == 2)
					{
						player++;
					}
				}
			}
			else if (map[i + 4][j + 4] != 2 && map[i + 4][j + 4] != 1 && map[i - 1][j - 1] == 1 && map[i - 1][j - 1] == 1)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				// cout<<"OK!"<<endl;
				mode = 2;
				for (int k = 3; k >= 0; k--)
				{
					if (map[i + k][j + k] == 2)
					{
						// cout<<"OKK"<<endl;
						player++;
					}
				}
			}
			if (player == 4)
			{
				if (mode == 1)
				{
					*war = 1; //代表有威胁
					*y = j - 1;
					*x = i - 1;
				}
				else if (mode == 2)
				{
					*war = 1; //代表有威胁
					*y = j + 4;
					*x = i + 4;
				}
				return;
			}
		}
	}
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			int mode = 0;
			if (map[i - 1][j + 1] != 2 && map[i - 1][j + 1] != 1 && map[i + 4][j - 4] == 1 && map[i + 4][j - 4] == 1)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				mode = 1;
				for (int k = 0; k < 4; k++)
				{
					if (map[i + k][j - k] == 2)
					{
						player++;
					}
				}
			}
			else if (map[i + 4][j - 4] != 2 && map[i + 4][j - 4] != 1 && map[i - 1][j + 1] == 1 && map[i - 1][j + 1] == 1)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				// cout<<"OK!"<<endl;
				mode = 2;
				for (int k = 3; k >= 0; k--)
				{
					if (map[i + k][j - k] == 2)
					{
						// cout<<"OKK"<<endl;
						player++;
					}
				}
			}
			if (player == 4)
			{
				if (mode == 1)
				{
					*war = 1; //代表有威胁
					*y = j + 1;
					*x = i - 1;
				}
				else if (mode == 2)
				{
					*war = 1; //代表有威胁
					*y = j - 4;
					*x = i + 4;
				}
				return;
			}
		}
	}
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			int mode = 0;
			if (map[i][j - 1] != 2 && map[i][j - 1] != 1 && map[i][j + 4] == 1 && map[i][j + 4] == 1)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				mode = 1;
				for (int k = 0; k < 4; k++)
				{
					if (map[i][j + k] == 2)
					{
						player++;
					}
				}
			}
			else if (map[i][j + 4] != 2 && map[i][j + 4] != 1 && map[i][j - 1] == 1 && map[i][j - 1] == 1)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				// cout<<"OK!"<<endl;
				mode = 2;
				for (int k = 3; k >= 0; k--)
				{
					if (map[i][j + k] == 2)
					{
						// cout<<"OKK"<<endl;
						player++;
					}
				}
			}
			if (player == 4)
			{
				if (mode == 1)
				{
					*war = 1; //代表有威胁
					*y = j - 1;
					*x = i;
				}
				else if (mode == 2)
				{
					*war = 1; //代表有威胁
					*y = j + 4;
					*x = i;
				}
				return;
			}
		}
	}
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			int mode = 0;
			if (map[i - 1][j] != 2 && map[i - 1][j] != 1 && map[i + 4][j] == 1 && map[i + 4][j] == 1)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				mode = 1;
				for (int k = 0; k < 4; k++)
				{
					if (map[i + k][j] == 2)
					{
						player++;
					}
				}
			}
			else if (map[i + 4][j] != 2 && map[i + 4][j] != 1 && map[i - 1][j] == 1 && map[i - 1][j] == 1)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				// cout<<"OK!"<<endl;
				mode = 2;
				for (int k = 3; k >= 0; k--)
				{
					if (map[i + k][j] == 2)
					{
						// cout<<"OKK"<<endl;
						player++;
					}
				}
			}
			if (player == 4)
			{
				if (mode == 1)
				{
					*war = 1; //代表有威胁
					*y = j;
					*x = i - 1;
				}
				else if (mode == 2)
				{
					*war = 1; //代表有威胁
					*y = j;
					*x = i + 4;
				}
				return;
			}
		}
	}
	//如果程序进行到了这里 说明AI不能直接取胜
	/****进攻之前先保命***/
	//判断是否有活四
	//正着斜
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			if (map[i - 1][j - 1] != 2 && map[i + 4][j + 4] != 2 && map[i - 1][j - 1] != 1 && map[i + 4][j + 4] != 1)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				for (int k = 0; k < 4; k++)
				{
					if (map[i + k][j + k] == 1)
					{
						player++;
					}
				}
			}
			if (player == 4)
			{
				*war = 1; //代表有威胁
				*y = j + 4;
				*x = i + 4;
				return;
			}
		}
	}
	//反着斜
	/**
	 * 向上：(x-n)(y+n)
	 * 向下：(x+n)(y-n)
	 *
	 *
	 */
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			if (map[i - 1][j + 1] != 2 && map[i + 4][j - 4] != 2 && map[i - 1][j + 1] != 1 && map[i + 4][j - 4] != 1)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				for (int k = 0; k < 4; k++)
				{
					if (map[i + k][j - k] == 1)
					{
						player++;
					}
				}
			}
			if (player == 4)
			{
				*war = 1; //代表有威胁
				*y = j - 4;
				*x = i + 4;
				return;
			}
		}
	}
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			if (map[i][j - 1] != 2 && map[i][j + 4] != 2 && map[i][j - 1] != 1 && map[i][j + 4] != 1)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				for (int k = 0; k < 4; k++)
				{
					if (map[i][j + k] == 1)
					{
						player++;
					}
				}
			}
			if (player == 4)
			{
				*war = 1; //代表有威胁
				*y = j + 4;
				*x = i;
				return;
			}
		}
	}
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			if (map[i - 1][j] != 2 && map[i + 4][j] != 2 && map[i - 1][j] != 1 && map[i + 4][j] != 1)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				for (int k = 0; k < 4; k++)
				{
					if (map[i + k][j] == 1)
					{
						player++;
					}
				}
			}
			if (player == 4)
			{
				*war = 1; //代表有威胁
				*y = j;
				*x = i + 4;
				return;
			}
		}
	}
	//判断是否有眠四并发出警告
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			int mode = 0;
			if (map[i - 1][j - 1] != 2 && map[i - 1][j - 1] != 1 && map[i + 4][j + 4] == 2 && map[i + 4][j + 4] == 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				mode = 1;
				for (int k = 0; k < 4; k++)
				{
					if (map[i + k][j + k] == 1)
					{
						player++;
					}
				}
			}
			else if (map[i + 4][j + 4] != 2 && map[i + 4][j + 4] != 1 && map[i - 1][j - 1] == 2 && map[i - 1][j - 1] == 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				// cout<<"OK!"<<endl;
				mode = 2;
				for (int k = 3; k >= 0; k--)
				{
					if (map[i + k][j + k] == 1)
					{
						// cout<<"OKK"<<endl;
						player++;
					}
				}
			}
			if (player == 4)
			{
				if (mode == 1)
				{
					*war = 1; //代表有威胁
					*y = j - 1;
					*x = i - 1;
				}
				else if (mode == 2)
				{
					*war = 1; //代表有威胁
					*y = j + 4;
					*x = i + 4;
				}
				return;
			}
		}
	}
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			int mode = 0;
			if (map[i - 1][j + 1] != 2 && map[i - 1][j + 1] != 1 && map[i + 4][j - 4] == 2 && map[i + 4][j - 4] == 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				mode = 1;
				for (int k = 0; k < 4; k++)
				{
					if (map[i + k][j - k] == 1)
					{
						player++;
					}
				}
			}
			else if (map[i + 4][j - 4] != 2 && map[i + 4][j - 4] != 1 && map[i - 1][j + 1] == 2 && map[i - 1][j + 1] == 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				// cout<<"OK!"<<endl;
				mode = 2;
				for (int k = 3; k >= 0; k--)
				{
					if (map[i + k][j - k] == 1)
					{
						// cout<<"OKK"<<endl;
						player++;
					}
				}
			}
			if (player == 4)
			{
				if (mode == 1)
				{
					*war = 1; //代表有威胁
					*y = j + 1;
					*x = i - 1;
				}
				else if (mode == 2)
				{
					*war = 1; //代表有威胁
					*y = j - 4;
					*x = i + 4;
				}
				return;
			}
		}
	}
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			int mode = 0;
			if (map[i][j - 1] != 2 && map[i][j - 1] != 1 && map[i][j + 4] == 2 && map[i][j + 4] == 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				mode = 1;
				for (int k = 0; k < 4; k++)
				{
					if (map[i][j + k] == 1)
					{
						player++;
					}
				}
			}
			else if (map[i][j + 4] != 2 && map[i][j + 4] != 1 && map[i][j - 1] == 2 && map[i][j - 1] == 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				// cout<<"OK!"<<endl;
				mode = 2;
				for (int k = 3; k >= 0; k--)
				{
					if (map[i][j + k] == 1)
					{
						// cout<<"OKK"<<endl;
						player++;
					}
				}
			}
			if (player == 4)
			{
				if (mode == 1)
				{
					*war = 1; //代表有威胁
					*y = j - 1;
					*x = i;
				}
				else if (mode == 2)
				{
					*war = 1; //代表有威胁
					*y = j + 4;
					*x = i;
				}
				return;
			}
		}
	}
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			int mode = 0;
			if (map[i - 1][j] != 2 && map[i - 1][j] != 1 && map[i + 4][j] == 2 && map[i + 4][j] == 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				mode = 1;
				for (int k = 0; k < 4; k++)
				{
					if (map[i + k][j] == 1)
					{
						player++;
					}
				}
			}
			else if (map[i + 4][j] != 2 && map[i + 4][j] != 1 && map[i - 1][j] == 2 && map[i - 1][j] == 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				// cout<<"OK!"<<endl;
				mode = 2;
				for (int k = 3; k >= 0; k--)
				{
					if (map[i + k][j] == 1)
					{
						// cout<<"OKK"<<endl;
						player++;
					}
				}
			}
			if (player == 4)
			{
				if (mode == 1)
				{
					*war = 1; //代表有威胁
					*y = j;
					*x = i - 1;
				}
				else if (mode == 2)
				{
					*war = 1; //代表有威胁
					*y = j;
					*x = i + 4;
				}
				return;
			}
		}
	}
	//判断是否是活三
	//正着斜
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			if (map[i - 1][j - 1] != 2 && map[i + 3][j + 3] != 2 && map[i - 1][j - 1] != 1 && map[i + 3][j + 3] != 1)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				for (int k = 0; k < 3; k++)
				{
					if (map[i + k][j + k] == 1)
					{
						player++;
					}
				}
			}
			if (player == 3)
			{
				*war = 1; //代表有威胁
				*y = j + 3;
				*x = i + 3;
				return;
			}
		}
	}
	//反着斜
	/**
	 * 向上：(x-n)(y+n)
	 * 向下：(x+n)(y-n)
	 *
	 *
	 */
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			if (map[i - 1][j + 1] != 2 && map[i + 3][j - 3] != 2 && map[i - 1][j + 1] != 1 && map[i + 3][j - 3] != 1)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				for (int k = 0; k < 3; k++)
				{
					if (map[i + k][j - k] == 1)
					{
						player++;
					}
				}
			}
			if (player == 3)
			{
				*war = 1; //代表有威胁
				*y = j - 3;
				*x = i + 3;
				return;
			}
		}
	}
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			if (map[i][j - 1] != 2 && map[i][j + 3] != 2 && map[i][j - 1] != 1 && map[i][j + 3] != 1)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				for (int k = 0; k < 3; k++)
				{
					if (map[i][j + k] == 1)
					{
						player++;
					}
				}
			}
			if (player == 3)
			{
				*war = 1; //代表有威胁
				*y = j + 3;
				*x = i;
				return;
			}
		}
	}
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			if (map[i - 1][j] != 2 && map[i + 3][j] != 2 && map[i - 1][j] != 1 && map[i + 3][j] != 1)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				for (int k = 0; k < 3; k++)
				{
					if (map[i + k][j] == 1)
					{
						player++;
					}
				}
			}
			if (player == 3)
			{
				*war = 1; //代表有威胁
				*y = j;
				*x = i + 3;
				return;
			}
		}
	}

	/****没有对自己致命的威胁 所以开始进攻 （属于进攻部分）注：这里的威胁是UP cv之后懒得删了
	其实这个 war就是个标识符 判断这个函数有无结果传出去

	****/
	//判断是否是活三
	//正着斜
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			if (map[i - 1][j - 1] != 1 && map[i + 3][j + 3] != 1 && map[i - 1][j - 1] != 2 && map[i + 3][j + 3] != 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				for (int k = 0; k < 3; k++)
				{
					if (map[i + k][j + k] == 2)
					{
						player++;
					}
				}
			}
			if (player == 3)
			{
				*war = 1; //代表有威胁
				*y = j + 3;
				*x = i + 3;
				return;
			}
		}
	}
	//反着斜
	/**
	 * 向上：(x-n)(y+n)
	 * 向下：(x+n)(y-n)
	 *
	 *
	 */
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			if (map[i - 1][j + 1] != 1 && map[i + 3][j - 3] != 1 && map[i - 1][j + 1] != 2 && map[i + 3][j - 3] != 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				for (int k = 0; k < 3; k++)
				{
					if (map[i + k][j - k] == 2)
					{
						player++;
					}
				}
			}
			if (player == 3)
			{
				*war = 1; //代表有威胁
				*y = j - 3;
				*x = i + 3;
				return;
			}
		}
	}
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			if (map[i][j - 1] != 1 && map[i][j + 3] != 1 && map[i][j - 1] != 2 && map[i][j + 3] != 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				for (int k = 0; k < 3; k++)
				{
					if (map[i][j + k] == 2)
					{
						player++;
					}
				}
			}
			if (player == 3)
			{
				*war = 1; //代表有威胁
				*y = j + 3;
				*x = i;
				return;
			}
		}
	}
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			if (map[i - 1][j] != 1 && map[i + 3][j] != 1 && map[i - 1][j] != 2 && map[i + 3][j] != 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				for (int k = 0; k < 3; k++)
				{
					if (map[i + k][j] == 2)
					{
						player++;
					}
				}
			}
			if (player == 3)
			{
				*war = 1; //代表有威胁
				*y = j;
				*x = i + 3;
				return;
			}
		}
	}
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			int mode = 0;
			if (map[i - 1][j - 1] != 2 && map[i - 1][j - 1] != 1 && map[i + 3][j + 3] == 1 && map[i + 3][j + 3] == 1)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				mode = 1;
				for (int k = 0; k < 3; k++)
				{
					if (map[i + k][j + k] == 1)
					{
						player++;
					}
				}
			}
			else if (map[i + 3][j + 3] != 2 && map[i + 3][j + 3] != 1 && map[i - 1][j - 1] == 1 && map[i - 1][j - 1] == 1)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				// cout<<"OK!"<<endl;
				mode = 2;
				for (int k = 2; k >= 0; k--)
				{
					if (map[i + k][j + k] == 1)
					{
						// cout<<"OKK"<<endl;
						player++;
					}
				}
			}
			if (player == 3)
			{
				if (mode == 1)
				{
					*war = 1; //代表有威胁
					*y = j - 1;
					*x = i - 1;
				}
				else if (mode == 2)
				{
					*war = 1; //代表有威胁
					*y = j + 3;
					*x = i + 3;
				}
				return;
			}
		}
	}
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			int mode = 0;
			if (map[i - 1][j + 1] != 2 && map[i - 1][j + 1] != 1 && map[i + 3][j - 3] == 1 && map[i + 3][j - 3] == 1)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				mode = 1;
				for (int k = 0; k < 3; k++)
				{
					if (map[i + k][j - k] == 1)
					{
						player++;
					}
				}
			}
			else if (map[i + 3][j - 3] != 2 && map[i + 3][j - 3] != 1 && map[i - 1][j + 1] == 1 && map[i - 1][j + 1] == 1)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				// cout<<"OK!"<<endl;
				mode = 2;
				for (int k = 2; k >= 0; k--)
				{
					if (map[i + k][j - k] == 1)
					{
						// cout<<"OKK"<<endl;
						player++;
					}
				}
			}
			if (player == 3)
			{
				if (mode == 1)
				{
					*war = 1; //代表有威胁
					*y = j + 1;
					*x = i - 1;
				}
				else if (mode == 2)
				{
					*war = 1; //代表有威胁
					*y = j - 3;
					*x = i + 3;
				}
				return;
			}
		}
	}
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			int mode = 0;
			if (map[i][j - 1] != 2 && map[i][j - 1] != 1 && map[i][j + 3] == 1 && map[i][j + 3] == 1)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				mode = 1;
				for (int k = 0; k < 3; k++)
				{
					if (map[i][j + k] == 1)
					{
						player++;
					}
				}
			}
			else if (map[i][j + 3] != 2 && map[i][j + 3] != 1 && map[i][j - 1] == 1 && map[i][j - 1] == 1)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				// cout<<"OK!"<<endl;
				mode = 2;
				for (int k = 2; k >= 0; k--)
				{
					if (map[i][j + k] == 1)
					{
						// cout<<"OKK"<<endl;
						player++;
					}
				}
			}
			if (player == 3)
			{
				if (mode == 1)
				{
					*war = 1; //代表有威胁
					*y = j - 1;
					*x = i;
				}
				else if (mode == 2)
				{
					*war = 1; //代表有威胁
					*y = j + 3;
					*x = i;
				}
				return;
			}
		}
	}
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			int mode = 0;
			if (map[i - 1][j] != 2 && map[i - 1][j] != 1 && map[i + 3][j] == 1 && map[i + 3][j] == 1)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				mode = 1;
				for (int k = 0; k < 3; k++)
				{
					if (map[i + k][j] == 1)
					{
						player++;
					}
				}
			}
			else if (map[i + 3][j] != 2 && map[i + 3][j] != 1 && map[i - 1][j] == 1 && map[i - 1][j] == 1)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				// cout<<"OK!"<<endl;
				mode = 2;
				for (int k = 2; k >= 0; k--)
				{
					if (map[i + k][j] == 1)
					{
						// cout<<"OKK"<<endl;
						player++;
					}
				}
			}
			if (player == 3)
			{
				if (mode == 1)
				{
					*war = 1; //代表有威胁
					*y = j;
					*x = i - 1;
				}
				else if (mode == 2)
				{
					*war = 1; //代表有威胁
					*y = j;
					*x = i + 3;
				}
				return;
			}
		}
	}

	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			if (map[i - 1][j - 1] != 1 && map[i + 2][j + 2] != 1 && map[i - 1][j - 1] != 2 && map[i + 2][j + 2] != 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				for (int k = 0; k < 2; k++)
				{
					if (map[i + k][j + k] == 2)
					{
						player++;
					}
				}
			}
			if (player == 2)
			{
				*war = 1; //代表有威胁
				*y = j + 2;
				*x = i + 2;
				return;
			}
		}
	}
	//反着斜
	/**
	 * 向上：(x-n)(y+n)
	 * 向下：(x+n)(y-n)
	 *
	 *
	 */
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			if (map[i - 1][j + 1] != 1 && map[i + 2][j - 2] != 1 && map[i - 1][j + 1] != 2 && map[i + 2][j - 2] != 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				for (int k = 0; k < 2; k++)
				{
					if (map[i + k][j - k] == 2)
					{
						player++;
					}
				}
			}
			if (player == 2)
			{
				*war = 1; //代表有威胁
				*y = j - 2;
				*x = i + 2;
				return;
			}
		}
	}
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			if (map[i][j - 1] != 1 && map[i][j + 2] != 1 && map[i][j - 1] != 2 && map[i][j + 2] != 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				for (int k = 0; k < 2; k++)
				{
					if (map[i][j + k] == 2)
					{
						player++;
					}
				}
			}
			if (player == 2)
			{
				*war = 1; //代表有威胁
				*y = j + 2;
				*x = i;
				return;
			}
		}
	}
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			if (map[i - 1][j] != 1 && map[i + 2][j] != 1 && map[i - 1][j] != 2 && map[i + 2][j] != 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				for (int k = 0; k < 2; k++)
				{
					if (map[i + k][j] == 2)
					{
						player++;
					}
				}
			}
			if (player == 2)
			{
				*war = 1; //代表有威胁
				*y = j;
				*x = i + 2;
				return;
			}
		}
	}

	/****无法进攻 开始判断其他对自己有威胁 但没那么大的***/

	//判断是否有眠三
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			int mode = 0;
			if (map[i - 1][j - 1] != 2 && map[i - 1][j - 1] != 1 && map[i + 3][j + 3] == 2 && map[i + 3][j + 3] == 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				mode = 1;
				for (int k = 0; k < 3; k++)
				{
					if (map[i + k][j + k] == 1)
					{
						player++;
					}
				}
			}
			else if (map[i + 3][j + 3] != 2 && map[i + 3][j + 3] != 1 && map[i - 1][j - 1] == 2 && map[i - 1][j - 1] == 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				// cout<<"OK!"<<endl;
				mode = 2;
				for (int k = 2; k >= 0; k--)
				{
					if (map[i + k][j + k] == 1)
					{
						// cout<<"OKK"<<endl;
						player++;
					}
				}
			}
			if (player == 3)
			{
				if (mode == 1)
				{
					*war = 1; //代表有威胁
					*y = j - 1;
					*x = i - 1;
				}
				else if (mode == 2)
				{
					*war = 1; //代表有威胁
					*y = j + 3;
					*x = i + 3;
				}
				return;
			}
		}
	}
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			int mode = 0;
			if (map[i - 1][j + 1] != 2 && map[i - 1][j + 1] != 1 && map[i + 3][j - 3] == 2 && map[i + 3][j - 3] == 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				mode = 1;
				for (int k = 0; k < 3; k++)
				{
					if (map[i + k][j - k] == 1)
					{
						player++;
					}
				}
			}
			else if (map[i + 3][j - 3] != 2 && map[i + 3][j - 3] != 1 && map[i - 1][j + 1] == 2 && map[i - 1][j + 1] == 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				// cout<<"OK!"<<endl;
				mode = 2;
				for (int k = 2; k >= 0; k--)
				{
					if (map[i + k][j - k] == 1)
					{
						// cout<<"OKK"<<endl;
						player++;
					}
				}
			}
			if (player == 3)
			{
				if (mode == 1)
				{
					*war = 1; //代表有威胁
					*y = j + 1;
					*x = i - 1;
				}
				else if (mode == 2)
				{
					*war = 1; //代表有威胁
					*y = j - 3;
					*x = i + 3;
				}
				return;
			}
		}
	}
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			int mode = 0;
			if (map[i][j - 1] != 2 && map[i][j - 1] != 1 && map[i][j + 3] == 2 && map[i][j + 3] == 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				mode = 1;
				for (int k = 0; k < 3; k++)
				{
					if (map[i][j + k] == 1)
					{
						player++;
					}
				}
			}
			else if (map[i][j + 3] != 2 && map[i][j + 3] != 1 && map[i][j - 1] == 2 && map[i][j - 1] == 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				// cout<<"OK!"<<endl;
				mode = 2;
				for (int k = 2; k >= 0; k--)
				{
					if (map[i][j + k] == 1)
					{
						// cout<<"OKK"<<endl;
						player++;
					}
				}
			}
			if (player == 3)
			{
				if (mode == 1)
				{
					*war = 1; //代表有威胁
					*y = j - 1;
					*x = i;
				}
				else if (mode == 2)
				{
					*war = 1; //代表有威胁
					*y = j + 3;
					*x = i;
				}
				return;
			}
		}
	}
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			int mode = 0;
			if (map[i - 1][j] != 2 && map[i - 1][j] != 1 && map[i + 3][j] == 2 && map[i + 3][j] == 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				mode = 1;
				for (int k = 0; k < 3; k++)
				{
					if (map[i + k][j] == 1)
					{
						player++;
					}
				}
			}
			else if (map[i + 3][j] != 2 && map[i + 3][j] != 1 && map[i - 1][j] == 2 && map[i - 1][j] == 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				// cout<<"OK!"<<endl;
				mode = 2;
				for (int k = 2; k >= 0; k--)
				{
					if (map[i + k][j] == 1)
					{
						// cout<<"OKK"<<endl;
						player++;
					}
				}
			}
			if (player == 3)
			{
				if (mode == 1)
				{
					*war = 1; //代表有威胁
					*y = j;
					*x = i - 1;
				}
				else if (mode == 2)
				{
					*war = 1; //代表有威胁
					*y = j;
					*x = i + 3;
				}
				return;
			}
		}
	}

	//判断是否有活二
	//正着斜
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			if (map[i - 1][j - 1] != 2 && map[i + 2][j + 2] != 2 && map[i - 1][j - 1] != 1 && map[i + 2][j + 2] != 1)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				for (int k = 0; k < 2; k++)
				{
					if (map[i + k][j + k] == 1)
					{
						player++;
					}
				}
			}
			if (player == 2)
			{
				*war = 1; //代表有威胁
				*y = j + 2;
				*x = i + 2;
				return;
			}
		}
	}
	//反着斜
	/**
	 * 向上：(x-n)(y+n)
	 * 向下：(x+n)(y-n)
	 *
	 *
	 */
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			if (map[i - 1][j + 1] != 2 && map[i + 2][j - 2] != 2 && map[i - 1][j + 1] != 1 && map[i + 2][j - 2] != 1)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				for (int k = 0; k < 2; k++)
				{
					if (map[i + k][j - k] == 1)
					{
						player++;
					}
				}
			}
			if (player == 2)
			{
				*war = 1; //代表有威胁
				*y = j - 2;
				*x = i + 2;
				return;
			}
		}
	}
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			if (map[i][j - 1] != 2 && map[i][j + 2] != 2 && map[i][j - 1] != 1 && map[i][j + 2] != 1)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				for (int k = 0; k < 2; k++)
				{
					if (map[i][j + k] == 1)
					{
						player++;
					}
				}
			}
			if (player == 2)
			{
				*war = 1; //代表有威胁
				*y = j + 2;
				*x = i;
				return;
			}
		}
	}
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			if (map[i - 1][j] != 2 && map[i + 2][j] != 2 && map[i - 1][j] != 1 && map[i + 2][j] != 1)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				for (int k = 0; k < 2; k++)
				{
					if (map[i + k][j] == 1)
					{
						player++;
					}
				}
			}
			if (player == 2)
			{
				*war = 1; //代表有威胁
				*y = j;
				*x = i + 2;
				return;
			}
		}
	}

	//判断是否有眠二
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			int mode = 0;
			if (map[i - 1][j - 1] != 2 && map[i - 1][j - 1] != 1 && map[i + 2][j + 2] == 2 && map[i + 2][j + 2] == 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				mode = 1;
				for (int k = 0; k < 2; k++)
				{
					if (map[i + k][j + k] == 1)
					{
						player++;
					}
				}
			}
			else if (map[i + 2][j + 2] != 2 && map[i + 2][j + 2] != 1 && map[i - 1][j - 1] == 2 && map[i - 1][j - 1] == 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				// cout<<"OK!"<<endl;
				mode = 2;
				for (int k = 1; k >= 0; k--)
				{
					if (map[i + k][j + k] == 1)
					{
						// cout<<"OKK"<<endl;
						player++;
					}
				}
			}
			if (player == 2)
			{
				if (mode == 1)
				{
					*war = 1; //代表有威胁
					*y = j - 1;
					*x = i - 1;
				}
				else if (mode == 2)
				{
					*war = 1; //代表有威胁
					*y = j + 2;
					*x = i + 2;
				}
				return;
			}
		}
	}
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			int mode = 0;
			if (map[i - 1][j + 1] != 2 && map[i - 1][j + 1] != 1 && map[i + 2][j - 2] == 2 && map[i + 2][j - 2] == 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				mode = 1;
				for (int k = 0; k < 2; k++)
				{
					if (map[i + k][j - k] == 1)
					{
						player++;
					}
				}
			}
			else if (map[i + 2][j - 2] != 2 && map[i + 2][j - 2] != 1 && map[i - 1][j + 1] == 2 && map[i - 1][j + 1] == 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				// cout<<"OK!"<<endl;
				mode = 2;
				for (int k = 1; k >= 0; k--)
				{
					if (map[i + k][j - k] == 1)
					{
						// cout<<"OKK"<<endl;
						player++;
					}
				}
			}
			if (player == 2)
			{
				if (mode == 1)
				{
					*war = 1; //代表有威胁
					*y = j + 1;
					*x = i - 1;
				}
				else if (mode == 2)
				{
					*war = 1; //代表有威胁
					*y = j - 2;
					*x = i + 2;
				}
				return;
			}
		}
	}
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			int mode = 0;
			if (map[i][j - 1] != 2 && map[i][j - 1] != 1 && map[i][j + 2] == 2 && map[i][j + 2] == 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				mode = 1;
				for (int k = 0; k < 2; k++)
				{
					if (map[i][j + k] == 1)
					{
						player++;
					}
				}
			}
			else if (map[i][j + 2] != 2 && map[i][j + 2] != 1 && map[i][j - 1] == 2 && map[i][j - 1] == 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				// cout<<"OK!"<<endl;
				mode = 2;
				for (int k = 1; k >= 0; k--)
				{
					if (map[i][j + k] == 1)
					{
						// cout<<"OKK"<<endl;
						player++;
					}
				}
			}
			if (player == 2)
			{
				if (mode == 1)
				{
					*war = 1; //代表有威胁
					*y = j - 1;
					*x = i;
				}
				else if (mode == 2)
				{
					*war = 1; //代表有威胁
					*y = j + 2;
					*x = i;
				}
				return;
			}
		}
	}
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			int mode = 0;
			if (map[i - 1][j] != 2 && map[i - 1][j] != 1 && map[i + 2][j] == 2 && map[i + 2][j] == 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				mode = 1;
				for (int k = 0; k < 2; k++)
				{
					if (map[i + k][j] == 1)
					{
						player++;
					}
				}
			}
			else if (map[i + 2][j] != 2 && map[i + 2][j] != 1 && map[i - 1][j] == 2 && map[i - 1][j] == 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				// cout<<"OK!"<<endl;
				mode = 2;
				for (int k = 1; k >= 0; k--)
				{
					if (map[i + k][j] == 1)
					{
						// cout<<"OKK"<<endl;
						player++;
					}
				}
			}
			if (player == 2)
			{
				if (mode == 1)
				{
					*war = 1; //代表有威胁
					*y = j;
					*x = i - 1;
				}
				else if (mode == 2)
				{
					*war = 1; //代表有威胁
					*y = j;
					*x = i + 2;
				}
				return;
			}
		}
	}
	//判断是否有活一
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			if (map[i - 1][j - 1] != 2 && map[i + 1][j + 1] != 2 && map[i - 1][j - 1] != 1 && map[i + 1][j + 1] != 1)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				for (int k = 0; k < 1; k++)
				{
					if (map[i + k][j + k] == 1)
					{
						player++;
					}
				}
			}
			if (player == 1)
			{
				*war = 1; //代表有威胁
				*y = j + 1;
				*x = i + 1;
				return;
			}
		}
	}
	//反着斜
	/**
	 * 向上：(x-n)(y+n)
	 * 向下：(x+n)(y-n)
	 *
	 *
	 */
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			if (map[i - 1][j + 1] != 2 && map[i + 1][j - 1] != 2 && map[i - 1][j + 1] != 1 && map[i + 1][j - 1] != 1)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				for (int k = 0; k < 1; k++)
				{
					if (map[i + k][j - k] == 1)
					{
						player++;
					}
				}
			}
			if (player == 1)
			{
				*war = 1; //代表有威胁
				*y = j - 1;
				*x = i + 1;
				return;
			}
		}
	}
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			if (map[i][j - 1] != 2 && map[i][j + 1] != 2 && map[i][j - 1] != 1 && map[i][j + 1] != 1)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				for (int k = 0; k < 1; k++)
				{
					if (map[i][j + k] == 1)
					{
						player++;
					}
				}
			}
			if (player == 1)
			{
				*war = 1; //代表有威胁
				*y = j + 1;
				*x = i;
				return;
			}
		}
	}
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			if (map[i - 1][j] != 1 && map[i + 1][j] != 2 && map[i - 1][j] != 1 && map[i + 1][j] != 1)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				for (int k = 0; k < 1; k++)
				{
					if (map[i + k][j] == 1)
					{
						player++;
					}
				}
			}
			if (player == 1)
			{
				*war = 1; //代表有威胁
				*y = j;
				*x = i + 1;
				return;
			}
		}
	}
	//判断是否有眠一
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			int mode = 0;
			if (map[i - 1][j - 1] != 2 && map[i - 1][j - 1] != 1 && map[i + 1][j + 1] == 2 && map[i + 1][j + 1] == 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				mode = 1;
				for (int k = 0; k < 1; k++)
				{
					if (map[i + k][j + k] == 1)
					{
						player++;
					}
				}
			}
			else if (map[i + 1][j + 1] != 2 && map[i + 1][j + 1] != 1 && map[i - 1][j - 1] == 2 && map[i - 1][j - 1] == 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				// cout<<"OK!"<<endl;
				mode = 2;
				for (int k = 0; k >= 0; k--)
				{
					if (map[i + k][j + k] == 1)
					{
						// cout<<"OKK"<<endl;
						player++;
					}
				}
			}
			if (player == 1)
			{
				if (mode == 1)
				{
					*war = 1; //代表有威胁
					*y = j - 1;
					*x = i - 1;
				}
				else if (mode == 2)
				{
					*war = 1; //代表有威胁
					*y = j + 1;
					*x = i + 1;
				}
				return;
			}
		}
	}
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			int mode = 0;
			if (map[i - 1][j + 1] != 2 && map[i - 1][j + 1] != 1 && map[i + 1][j - 1] == 2 && map[i + 1][j - 1] == 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				mode = 1;
				for (int k = 0; k < 1; k++)
				{
					if (map[i + k][j - k] == 1)
					{
						player++;
					}
				}
			}
			else if (map[i + 1][j - 1] != 2 && map[i + 1][j - 1] != 1 && map[i - 1][j + 1] == 2 && map[i - 1][j + 1] == 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				// cout<<"OK!"<<endl;
				mode = 2;
				for (int k = 0; k >= 0; k--)
				{
					if (map[i + k][j - k] == 1)
					{
						// cout<<"OKK"<<endl;
						player++;
					}
				}
			}
			if (player == 1)
			{
				if (mode == 1)
				{
					*war = 1; //代表有威胁
					*y = j + 1;
					*x = i - 1;
				}
				else if (mode == 2)
				{
					*war = 1; //代表有威胁
					*y = j - 2;
					*x = i + 1;
				}
				return;
			}
		}
	}
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			int mode = 0;
			if (map[i][j - 1] != 2 && map[i][j - 1] != 1 && map[i][j + 1] == 2 && map[i][j + 1] == 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				mode = 1;
				for (int k = 0; k < 1; k++)
				{
					if (map[i][j + k] == 1)
					{
						player++;
					}
				}
			}
			else if (map[i][j + 1] != 2 && map[i][j + 1] != 1 && map[i][j - 1] == 2 && map[i][j - 1] == 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				// cout<<"OK!"<<endl;
				mode = 2;
				for (int k = 0; k >= 0; k--)
				{
					if (map[i][j + k] == 1)
					{
						// cout<<"OKK"<<endl;
						player++;
					}
				}
			}
			if (player == 1)
			{
				if (mode == 1)
				{
					*war = 1; //代表有威胁
					*y = j - 1;
					*x = i;
				}
				else if (mode == 2)
				{
					*war = 1; //代表有威胁
					*y = j + 1;
					*x = i;
				}
				return;
			}
		}
	}
	for (int i = 0; i < 19; ++i)
	{
		for (int j = 0; j < 19; ++j)
		{
			int player = 0;
			int mode = 0;
			if (map[i - 1][j] != 2 && map[i - 1][j] != 1 && map[i + 1][j] == 2 && map[i + 1][j] == 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				mode = 1;
				for (int k = 0; k < 1; k++)
				{
					if (map[i + k][j] == 1)
					{
						player++;
					}
				}
			}
			else if (map[i + 1][j] != 2 && map[i + 1][j] != 1 && map[i - 1][j] == 2 && map[i - 1][j] == 2)
			{ //我日你无头仙人 这么长 我好不容易写一次代码 你却让我输的 这么彻底！！！焯！！！！！！！！！
				// cout<<"OK!"<<endl;
				mode = 2;
				for (int k = 0; k >= 0; k--)
				{
					if (map[i + k][j] == 1)
					{
						// cout<<"OKK"<<endl;
						player++;
					}
				}
			}
			if (player == 1)
			{
				if (mode == 1)
				{
					*war = 1; //代表有威胁
					*y = j;
					*x = i - 1;
				}
				else if (mode == 2)
				{
					*war = 1; //代表有威胁
					*y = j;
					*x = i + 1;
				}
				return;
			}
		}
	}
	*war = 0; //说明没有威胁
	*x = 0;
	*y = 0;
	return;
}
void gobang_ai()
{
	/***/
	/*

	 *                 五子棋ai v 0.1  by min0911_
	@
	@
	@@@@@ date 2021    @@@@水平：菜鸟
	 */

	static int war = 0;
	static int x = 0;
	static int y = 0;
	who_is_win_for_ai_get_weixie(&war, &x, &y);
	if (war == 1)
	{
		map[x][y] = 2;
	}
	else
	{ /******应对无子可下的情况******/
		static int x1 = 0, x2 = 0;
		random_x_y(&x1, &x2);
		map[x1][x2] = 2;
	}
}