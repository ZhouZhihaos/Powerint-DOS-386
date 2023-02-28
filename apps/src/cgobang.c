#include <syscall.h>
int map[19][19]; // 0: empty, 1: black, 2: white
int turn = 1; // 1: black, 2: white
int x,y; //player's move
void ViewMap()
{
    int i,j;
    goto_xy(0,0);
    for(i=0;i<19;i++)
    {
        for(j=0;j<19;j++)
        {
            if(map[i][j]==0)
                putch(' ');
            else if(map[i][j]==1)
                putch('X');
            else if(map[i][j]==2)
                putch('O');
        }
        putch('\n');
    }
}
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
int main(int argc,char **argv)
{
    for(int i=0;i<19;i++)
    {
        for(int j=0;j<19;j++)
        {
            map[i][j]=0;
        }
    }
    x = 0;
    y = 0;
    for(int i = 0;i<25;i++)
    {
        for(int j = 0;j<80;j++)
        {
            print("=");
        }
    }
    while (1)
    {
        ViewMap();
        goto_xy(x,y);
        if(turn == 1)
            print("X");
        else
            print("O");
        char ch = getch();
        if(ch=='w')
        {
            if(y>0)
                y--;
        }
        else if(ch=='s')
        {
            if(y<18)
                y++;
        }
        else if(ch=='a')
        {
            if(x>0)
                x--;
        }
        else if(ch=='d')
        {
            if(x<18)
                x++;
        }
        else if(ch==' ')
        {
            if(map[y][x]==0)
            {
                if(turn==1)
                {
                    map[y][x]=1;
                    turn=2;
                }
                else if(turn==2)
                {
                    map[y][x]=2;
                    turn=1;
                }
            }
        }
        if(Win()==1)
        {
            system("cls");
            goto_xy(0,0);
            print("Black Win!");
            break;
        }
        else if(Win()==2)
        {
            system("cls");
            goto_xy(0,0);
            print("White Win!");
            break;
        }
    }
    return 0;
}