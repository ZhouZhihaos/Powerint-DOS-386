// 有些注释在转编码时乱了 将就看看吧
#include "system.h"
void command_run(char *cmdline)
{
	int i,addr;
	char c;
	char *p;
    if (strcmp("dir", cmdline) == 0) {
        cmd_dir();
		return;
    } else if (strncmp("type ", cmdline, 5) == 0) {
        type_deal(cmdline);
		return;
    } else if (strcmp("cls", cmdline) == 0) {
        clear();
		return;
    } else if (strcmp("ver", cmdline) == 0) {
		printf("Powerint DOS 386 Version %s\n",VERSION);
		print("Copyright (C) 2021-2022 zhouzhihao & min0911\n");
		print("THANKS TOOLS BY HARIBOTE OS\n\n");
		return;
	} else if (strcmp("time",cmdline) == 0) {
		char *time="The current time is:00:00:00";
		io_out8(0x70,0);	// 缁?
		c = io_in8(0x71);
		time[27]=(c&0x0f)+0x30;
		time[26]=(c>>4)+0x30;
		io_out8(0x70,2);	// 閸?
		c = io_in8(0x71);
		time[24]=(c&0x0f)+0x30;
		time[23]=(c>>4)+0x30;
		io_out8(0x70,4);	// 閺?
		c = io_in8(0x71);
		time[21]=(c&0x0f)+0x30;
		time[20]=(c>>4)+0x30;
		print(time);
		print("\n\n");
		return;
	} else if (strcmp("date",cmdline) == 0) {
		char *date="The current date is:2000\\00\\00,";
		io_out8(0x70,9);	// 楠?
		c = io_in8(0x71);
		date[23]=(c&0x0f)+0x30;
		date[22]=(c>>4)+0x30;
		io_out8(0x70,8);	// 閺?
		c = io_in8(0x71);
		date[26]=(c&0x0f)+0x30;
		date[25]=(c>>4)+0x30;
		io_out8(0x70,7);	// 閺?
		c = io_in8(0x71);
		date[29]=(c&0x0f)+0x30;
		date[28]=(c>>4)+0x30;
		print(date);
		io_out8(0x70,6);	// 閺勭喐婀?
		c = io_in8(0x71);
		if (c == 1)
			print("Sunday");
		if (c == 2)
			print("Monday");
		if (c == 3)
			print("Tuesday");
		if (c == 4)
			print("Wednesday");
		if (c == 5)
			print("Thursday");
		if (c == 6)
			print("Friday");
		if (c == 7)
			print("Saturday");
		print("\n\n");
		return;
    } else if (strncmp("echo ", cmdline,5) == 0) {
        print(cmdline+5);
		print("\n");
		return;
    } else if (strncmp("poke ", cmdline,5) == 0) {
		addr=ascii2num(cmdline[5])*0x10000000+ascii2num(cmdline[6])*0x1000000;
		addr=addr+ascii2num(cmdline[7])*0x100000+ascii2num(cmdline[8])*0x10000;
		addr=addr+ascii2num(cmdline[9])*0x1000+ascii2num(cmdline[10])*0x100;
		addr=addr+ascii2num(cmdline[11])*0x10+ascii2num(cmdline[12]);
		p=addr;
		c=ascii2num(cmdline[14])*0x10+ascii2num(cmdline[15]);
		p[0]=c;
		print("\n");
		return;
    } else if (strncmp("visit ", cmdline,6) == 0) {
		addr=ascii2num(cmdline[6])*0x10000000+ascii2num(cmdline[7])*0x1000000;
		addr=addr+ascii2num(cmdline[8])*0x100000+ascii2num(cmdline[9])*0x10000;
		addr=addr+ascii2num(cmdline[10])*0x1000+ascii2num(cmdline[11])*0x100;
		addr=addr+ascii2num(cmdline[12])*0x10+ascii2num(cmdline[13]);
		p=addr;
		c=p[0];
		printchar(num2ascii(c>>4));
		printchar(num2ascii(c&0x0f));
		print("\n");
		return;
	} else if (strcmp("pcinfo", cmdline) == 0) {
		pcinfo();
	} else if (strcmp("mem", cmdline) == 0) {
		mem();
	} else if (strncmp("bmpview ", cmdline,8) == 0) {
		bmpview(cmdline+8);
		char c;
		for (;;) {
			c = input_char_inSM();
			if (c == 0x01) {	// ESC閹稿绗呴惍?
				SwitchToText8025();
				break;
			}
		}
		return;
	} else if (strncmp("chview ",cmdline,7) == 0) {
		chview(cmdline+7);
	} else if (strncmp("beep ",cmdline,5) == 0) {
		int point,notes,dup;
		point=ascii2num(*(char*)(cmdline+5));
		notes=ascii2num(*(char*)(cmdline+7));
		dup=ascii2num(*(char*)(cmdline+9));
		beep(point,notes,dup);
	} else if (strcmp("mouse",cmdline) == 0) {
		mouseinput();
	} else if (strcmp("reboot",cmdline) == 0) {
		io_out8(0xcf9,0x0e);	// 写入0x02，实现软件重启
								// 写入0x04，实现硬件重启
								// 写入0x0e，实现软硬件重启
	} else if (strcmp("halt",cmdline) == 0) {
		acpi_shutdown();
	} else if (strncmp("pak ",cmdline,4) == 0) {
		int i;
		for (i = 4; cmdline[i] != ' '; i++);
		pak(cmdline+4,cmdline+i+1);
	} else if (strncmp("unpak ",cmdline,6) == 0) {
		unpak(cmdline+6);
	} else if (strcmp("casm",cmdline) == 0) {
		casm_shell();
	// 閸欘亣鍏橀崷銊р�栭惄妯诲⒔鐞涘瞼娈戦崨鎴掓姢
//	} else if (*(char *)(0x7dfd)=='C') {	// C閻╂ê鎯庨崝?
	} else if (strncmp("mkfile ",cmdline,7) == 0) {
		mkfile(cmdline+7);
		return;
	} else if (strncmp("del ",cmdline,4) == 0){
		del(cmdline);
		return;
	} else if (strncmp("edit ",cmdline,5) == 0) {
		edit(cmdline);
		return;
	// 閸欘亣鍏橀崷銊ㄨ拫閻╂ɑ澧界悰宀�娈戦崨鎴掓姢
	} else if (*(char *)(0x7dfd)=='A') {	// A閻╂ê鎯庨崝?
		if (strcmp("setup", cmdline) == 0) {
			setup();
			return;
		} else if (cmd_app(cmdline) == 0) {
			if (run_bat(cmdline) == 0) {
				print("Bad Command!\n\n");
				return;
			}
		}
	} else {
		if (cmd_app(cmdline) == 0) {
			if (run_bat(cmdline) == 0) {
				print("Bad Command!\n\n");
				return;
			}
		}
	}
}
void print_date(unsigned short _date,unsigned short _time)
{
    // 提取出年
    unsigned short year = _date & 65024;
    year = year >> 9;
    // 提取月
    unsigned short month = _date &480;
    month = month>> 5;
    // 提取日
    unsigned short day = _date & 31;
	
	unsigned short hour = _time & 63488;
    hour = hour >> 11;
    //提取分钟
    unsigned short minute = _time & 2016;
    minute = minute >> 5;
    printf("%02u-%02u-%02u %02u:%02u", (year + 1980),month, day,hour,minute);
}
void cmd_dir()
{
    struct FILEINFO *finfo = (struct FILEINFO *)(ADR_DISKIMG + 0x002600);
    int i, j, k;
    char s[30];
	for (i = 0; i != 30;i++) {
		s[i] = 0;
	}
    print("FILENAME   EXT    LENGTH       DATE\n");
    for (i = 0; i < 224; i++)
    {
        if (finfo[i].name[0] == 0x00)
        {
            break;
        }
        if (finfo[i].name[0] != 0xe5)
        {
            if ((finfo[i].type & 0x18) == 0)
            {

                for (j = 0; j < 8; j++)
                {
                    s[j] = finfo[i].name[j];
                }
                s[9] = finfo[i].ext[0];
                s[10] = finfo[i].ext[1];
                s[11] = finfo[i].ext[2];

                if (s[0] != '+')
                {
                    for (k = 0; k < 12; ++k)
                    {
                        if (k == 9)
                        {
                            print("   ");
                        }
                        if (s[k] == '\n')
                        {
                            print("   ");
                        }
                        else
                        {

                            putchar(s[k]);
                        }
                    }
                    print("    ");
                    print(itoa(finfo[i].size));
					gotoxy(31,get_y());
					print_date(finfo[i].date,finfo[i].time);
                    print("\n");
                }
            }
        }
    }
    print("\n");
    //&s = 0;
    return;
}
void type_deal(char *cmdline)
{
    char *name;
    int i;
    struct FILEINFO *finfo;
    for (i = 0; i < strlen(cmdline); i++)
    {
        name[i] = cmdline[i + 5];
    }
    finfo = file_search(name, (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
    if (finfo == 0)
    {
        print(name);
        print(" not found!\n\n");
    }
    else
    {
        char *p = fopen(name);
		for (i = 0;i != finfo->size; i++) {
			printchar(p[i]);
		}
        print("\n");
    }
	return;
}
void mkfile(char *name)
{
	char s[12];
	int i,j;
	struct FILEINFO *finfo = (struct FILEINFO *)(ADR_DISKIMG + 0x002600);
	// 閺傚洣娆㈤崥宥堟祮閸?
	for (j = 0; j != 12; j++) {
		s[j] = ' ';
	}
   	j = 0;
	for (i = 0; name[i] != 0; i++)
	{
		if (j >= 11)
		{
			return;
		}
		if (name[i] == '.' && j <= 8)
		{
			j = 8;
		}
		else
		{
			s[j] = name[i];
			if ('a' <= s[j] && s[j] <= 'z')
			{
				s[j] -= 0x20;
			}
			j++;
		}
	}
	// 閺屻儲澹樼粚杞扮秴
	for (i = 0;;i++) {
		if (finfo[i].name[0] == 0x00) {
			finfo = finfo+i;
			break;
		} 
	}
	// 閺傚洣娆㈡穱鈩冧紖閸愭瑥鍙?
	for (i = 0;i != 8;i++) { finfo->name[i]=s[i]; }
	for (i = 8;i != 11;i++) { finfo->ext[i-8] = s[i]; }
	finfo->type = 0x20;
	// 閺堫剚鏋冩禒鍓佺殻=娑撳﹣閲滈弬鍥︽缁?娑撳﹣閲滈弬鍥︽婢堆冪毈/512+1
	finfo->clustno = finfo[-1].clustno+(finfo[-1].size/512)+1;
	for (i = 0; i != 10;i++) { finfo->reserve[i]=0; }
	// time閸滃畳ate妞ょ懓鍘涙稉宥囶吀
	finfo->time = 0;
	finfo->date = 0;
	// 閺傚洣娆㈡径褍鐨弳鍌涙娑?
	finfo->size = 0;
//	drivers_idehdd_write(19,1,(ADR_DISKIMG+0x2600));	// 閸愭瑧娲拌ぐ鏇炲隘
	sleep(1);
	return;
}
void del(char *cmdline)
{
    char *name;
    int i;
    struct FILEINFO *finfo;
    for (i = 0; i < strlen(cmdline); i++)
    {
        name[i] = cmdline[i + 4];
    }
	finfo=file_search(name, (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
	if (finfo == 0) {
		print(name);
		print(" not found!\n\n");
		return;
	}
	finfo->name[0] = 0xe5;
//	drivers_idehdd_write(19,1,(ADR_DISKIMG+0x2600));	// 閸愭瑧娲拌ぐ鏇炲隘
	sleep(1);
	return;
}
void edit(char *cmdline)
{
    char *name;
    int i;
    struct FILEINFO *finfo;
    for (i = 0; i < strlen(cmdline); i++)
    {
        name[i] = cmdline[i + 5];
    }
    finfo = file_search(name, (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
    if (finfo == 0)
    {
        print(name);
        print(" not found!\n\n");
        return;
    }
    finfo->size = 0;
    char *p = fopen(name);
    for (;;finfo->size++) {
		p[finfo->size] = getch();
		printchar(p[finfo->size]);
		if (p[finfo->size] == 0x0a) {	// 閹稿绨￠崶鐐舵簠閿?
			break;
		}
	}
    print("Waiting...");
//	drivers_idehdd_write(19,1,(ADR_DISKIMG+0x2600)); // 閸愭瑧娲拌ぐ鏇炲隘
	sleep(1);
//	drivers_idehdd_write((31+finfo->clustno),1,p);	// 閸愭瑦鏆熼幑顔煎隘
	sleep(1);
    print("Done!\n\n");
    return;
}
void bmpview(char *filename) {
	int i,j;
	char *p = fopen(filename);
	unsigned short pxsize,pysize,start;
	unsigned int length;
	if (p == 0) {
		print("Can't find file ");
		print(filename);
		print("\n");
		return;
	}
	if (p[0] != 'B' || p[1] != 'M') {
		print("Isn't BMP photo,format error.\n");
		return;
	}
	pxsize=*(unsigned short*)(p+0x12);	// 閸ュ墽澧栭梹?
	pysize=*(unsigned short*)(p+0x16);	// 閸ュ墽澧栫�?
	length=*(unsigned int*)(p+2);	// 閸ュ墽澧栨径褍鐨?
	start=*(unsigned short*)(p+0xa);	// 閸ュ墽澧栭惃鍕佸?
	// step1:鏉╂稑鍙嗛崶鎯ц埌濡�崇础
	SwitchTo320X200X256();	// 閸掑洦宕查崚鏉挎禈瑜般垺膩瀵?
	// step2:鐠嬪啳澹?
	io_out8(VGA_DAC_WRITE_INDEX,0);
	for (i = 0; i != 256; i++) {
		io_out8(VGA_DAC_DATA,p[0x36+i*4+2]/4);
		io_out8(VGA_DAC_DATA,p[0x36+i*4+1]/4);
		io_out8(VGA_DAC_DATA,p[0x36+i*4]/4);
	}
	// step3:閺勫墽銇?
	for (i = 0; i < pysize; i++) {
		for (j = 0; j < pxsize; j++) {
			Draw_Px(j,i,p[length-(i*pxsize+pxsize-j)]);
		}
	}
	return;
}
void chview(char *filename) {
	struct FILEINFO *finfo=file_search(filename,(struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
	char *p=fopen(filename);
	int i,x=0,y=0;
	if (p == 0) {
		printf("Can't find file %s!",&filename);
	}
	SwitchTo320X200X256();	// 切换到图形模式
	for (i = 0; i != finfo->size; i++) {
		if (y == 11) {
			break;
		}
		if (x == 40) {
			y++;
			x=0;
		}
		unsigned char str[3];
		unsigned char c;
		str[0]=p[i];
		str[1]=p[i+1];
		str[2]=0;
		if (str[0] == 0x0d && str[1] == 0x0a) {
			y++;
			x=0;
			i++;
			continue;
		}
		if (GetCHorEN(str) == 1) {	// 是中文？
			if (x == 39) {
				y++;
				x=0;
			}
			if (x%2 != 0) {	// x可以整除2吗？
				PrintChineseChar(x/2+1,y,0x07,*(short*)str);
			} else {
				PrintChineseChar(x/2,y,0x07,*(short*)str);
			}
			i++;
			x+=2;
			continue;
		}
		c = p[i];
		Draw_Char(x,y,c,0x07);
		x++;
	}
	if (y < 11) {
		goto nomore;
	}
	goto start2; 
again2:
	sleep(150);
	memcpy(0xa0000,0xa1400,0xe600);
	int j;
	for (j = 0; j != 0x1400; j++) {
		*(char *)(0xae600+j)=0;
	}
start2:
	x=0;
	y=11;
	for (;i != finfo->size;i++) {
		unsigned char str[3];
		unsigned char c;
		str[0]=p[i];
		str[1]=p[i+1];
		str[2]=0;
		if (x == 40) {
			goto again2;
		}
		if (str[0] == 0x0d && str[1] == 0x0a) {
			i+=2;
			goto again2;
		}
		if (GetCHorEN(str) == 1) {	// 是中文？
			if (x > 38) {
				goto again2;
			}
			if (x%2 != 0) {	// x可以整除2吗？
				PrintChineseChar(x/2+1,y,0x07,*(short*)str);
			} else {
				PrintChineseChar(x/2,y,0x07,*(short*)str);
			}
			i++;
			x+=2;
			continue;
		}
		c = p[i];
		Draw_Char(x,y,c,0x07);
		x++;
	}
nomore:
	for (;;) {
		char c;
		c = input_char_inSM();
		if (c == 0x01) {
			SwitchToText8025();
			break;
		}
	}
	return;
}
void casm_shell() {
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;
	int i,j,point,point2=0,listflag=0;
	unsigned char c;
	char casmline[64]={0};
	unsigned char outcode[1024]={0};	// 娑撳瓨妞傜�涙ɑ鏂佺紓鏍槯閸氬簼鍞惍浣告勾閸?
	unsigned char temp[30];
	char *runapp, *alloc;
	char *keytable=get_point_of_keytable();
	char *keytable1=get_point_of_keytable1();
	print("Welcome to CASM version 0.1!\n");
	print("Copyright (C) ASM by min0911 2022\n");
	print("SHELL by zhouzhihao 2022\n");
	while(1) {
jmpout:
		print("\nCASM>>>");
		clean(casmline,64);
		input(casmline,64);
command:
		if (strcmp("code",casmline)==0) {
			point2=0;	// point2閿涘牏鈻兼惔蹇涙毐鎼达讣绱?
			listflag=0;
			clean(casmline,64);
			for (i = 0;;) {
				printchar(0x08);
again:
				printf("\n%x ",i);
				for (j = 0;j != 64;j++) {
					c = input_char_inSM();
					if (c == 0x01) {	// ESC閹稿绗呴惍?
						print("\n");
						goto jmpout;
					} else if (keytable1[c] == 0x0a) {
						goto nextline;
					} else if (keytable1[c] == '\b' && i > 0) {
						j--;
						casmline[j]=0;
						j--;
						printchar('\b');
						continue;
					}
					if (get_shift() == 0 && get_caps_lock() == 0) {	// shift閸滃畱aps_lock閻樿埖鈧椒璐?
						printchar(keytable1[c]);
						casmline[j]=keytable1[c];
					} else if (get_shift() == 1 || get_caps_lock() == 1) {	// shift閹存溈aps_lock閻樿埖鈧椒璐?
						printchar(keytable[c]);
						casmline[j]=keytable[c];
					} else if (get_shift() == 1 && get_caps_lock() == 1) { // shift閸滃畱aps_lock閻樿埖鈧椒璐?
						printchar(keytable1[c]);
						casmline[j]=keytable1[c];
					}
				}
nextline:
				sprintf(&temp,"%s",_Asm(casmline,i));
				// 高亮
				gotoxy(2,get_y());
				// 指令高亮黄(bg)红(font)色
				if (i < 0x10) {
					j = 2;
				} else if (i < 0x100){
					j = 3;
				} else {
					j = 4;
				}
				for (; *(char *)(0xb8000+160*get_y()+j*2) != 0x20; j++) {
					*(char *)(0xb8000+160*get_y()+j*2+1)=0x01;
				}
				j++;
				// 寄存器和数据高亮黄(bg)蓝(font)色
				for (; *(char *)(0xb8000+160*get_y()+j*2) != 0x20; j++) {
					if (*(char *)(0xb8000+160*get_y()+j*2) == ',') {
						continue;
					}
					*(char *)(0xb8000+160*get_y()+j*2+1)=0x04;
				}
				int k,kt;
				for (k=0,kt=0;k < strlen(temp); k++) {
					if (temp[k] != ' ') {
						kt++;
					}
				}
				i += kt/2;
				for (point = 0; point != (strlen(temp)+1)/3; point++) {
					outcode[point2]=(ascii2num(temp[point*3])<<4)+ascii2num(temp[point*3+1]);
					point2++;
				}
				clean(casmline,64);
			}
		} else if (strcmp("list",casmline)==0) {
			for (i = 0;i < point2;i++) {
				c = outcode[i]>>4;
				printchar(num2ascii(c));
				c = outcode[i]&0x0f;
				printchar(num2ascii(c));
				printchar(0x20);
			}
			listflag = 1;
		} else if (strcmp("run",casmline)==0) {
			if (listflag == 0) {	// 韫囧懘銆忛崗鍫熷⒔鐞涘ist閸︺劍澧界悰瀹簎n
				print("Please 'list' first!");
			} else if (listflag == 1) {
				runapp = (char *)memman_alloc_4k(memman, point2+128*1024);	// 閻㈠疇顕瑀unapp閸愬懎鐡?
				*((int *)(0xfe8))=runapp;	// cs_base
				*((int *)(0xfec))=runapp;	// ds_base
				runapp[0] = 'A';	// 表示是汇编编写
				memcpy(runapp+1,outcode,point2);
				set_segmdesc(gdt + 3, point2+1, (int)runapp, AR_CODE32_ER);
				set_segmdesc(gdt + 4, point2+1 + 128*1024, (int)runapp, AR_DATA32_RW);
				start_app(1, 3 * 8, 4 * 8);	// 鐠哄疇娴嗛崚鏉款槻閸掕泛鍩宺unapp閻ㄥ嫮鈻兼惔?
				memman_free_4k(memman,(int)runapp,point2+128*1024);
			}
		} else if (strcmp("exit",casmline)==0) {
			print("\n");
			return;
		} else if (strncmp("cas ",casmline,4)==0) {
			int addr=0;
			char *p;
			struct FILEINFO *finfo;
			point2 = 0;
			p = fopen(casmline+4);
			finfo = file_search(casmline+4,(struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
			if (finfo == 0) {
				print("File not find!\n");
				return;
			}
			for (i = 0; i != finfo->size; i++) {
				printchar(p[i]);
			}
			print("\nThis file,right(Y/N)?");
			c = getch();
			if (c == 'Y' || c == 'y') {
				printchar(c);
				print("\n");
				clean(casmline,64);
				i = 0;
				for (j = 0;i+j != finfo->size+1;) {
					for (i = 0;p[i+j] != 0x0d && p[i+j+1] != 0x0a; i++) {
						if (i+j == finfo->size+1) {
							goto outside;
						}
						casmline[i] = p[i+j];
					}
					j=j+i+2;
outside:
					sprintf(&temp,"%s",_Asm(casmline,addr));
					int k,kt;
					for (k=0,kt=0;k < strlen(temp); k++) {
						if (temp[k] != ' ') {
							kt++;
						}
					}
					addr += kt/2;
					for (point = 0; point != (strlen(temp)+1)/3; point++) {
						outcode[point2]=(ascii2num(temp[point*3])<<4)+ascii2num(temp[point*3+1]);
						point2++;
					}
					clean(casmline,64);
				}
				printf("The program out size is:%d",point2);
			} else {
				printchar(c);
				return;
			}
		} else if (strncmp("out ",casmline,4)==0) {
			if (listflag == 0) {
				print("Please 'list' first!");
			} else if (listflag == 1){
				struct FILEINFO *finfo;
				char *p;
				mkfile(casmline+4);
				p = fopen(casmline+4);
				finfo=file_search(casmline+4,(struct FILEINFO *)(ADR_DISKIMG + 0x002600),224);
				finfo->size=point2+1;
				p[0] = 'A';	// 表示是汇编编写
				memcpy(p+1,outcode,point2);
			}
		} else {
			command_run(casmline);
			printchar(0x08);
		}
	}
}
void clean(char *s,int len)
{
	int i;
	for (i = 0;i != len;i++) {
		s[i] = 0;
	}
	return;
}
void pcinfo()
{
	char cpu[100] = {0};
	int cpuid[3] = {get_cpu1(), get_cpu3(), get_cpu2()};
	//閸欐牕鍤?a b c 娑擃厾娈戦崶娑楅嚋鐎涙濡?鐎涙ê鍙哻pu
	cpu[0] = cpuid[0] & 0xff;
	cpu[1] = (cpuid[0] >> 8) & 0xff;
	cpu[2] = (cpuid[0] >> 16) & 0xff;
	cpu[3] = (cpuid[0] >> 24) & 0xff;
	cpu[4] = cpuid[1] & 0xff;
	cpu[5] = (cpuid[1] >> 8) & 0xff;
	cpu[6] = (cpuid[1] >> 16) & 0xff;
	cpu[7] = (cpuid[1] >> 24) & 0xff;
	cpu[8] = cpuid[2] & 0xff;
	cpu[9] = (cpuid[2] >> 8) & 0xff;
	cpu[10] = (cpuid[2] >> 16) & 0xff;
	cpu[11] = (cpuid[2] >> 24) & 0xff;
	cpu[12] = 0;
	printf("CPU:%s ", cpu);
	char cpu1[100] = {0};
	getCPUBrand(cpu1);
	printf("Ram Size:%dMB\n", get_memsize()/(1024*1024));
	return;
}
void mem()
{
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	printf("All:%dMB  Free:%dKB\n",get_memsize()/(1024*1024),memman_total(memman)/1024);
	return;
}
void pak(char *pakname,char *filenames) {
	char filename[16]={0};
	int i;
	for (i = 0; pakname[i] != ' '; i++) {
		filename[i] = pakname[i];
	}
	mkfile(filename);
	struct FILEINFO *pakfinfo = file_search(filename,(struct FILEINFO *)(ADR_DISKIMG + 0x002600),224);
	struct FILEINFO *finfo;
	char *pak=fopen(filename);
	char *file;
	int filenum;
	int len=0;
	int point;
	for (i = 0,filenum = 0; i < strlen(filenames); filenum++) {
		for (; filenames[i] != ' '; i++) {
			if (i > strlen(filenames)) {
				break;
			}
		}
		i++;	// 这个i++是为了避开' '
	}
	pak[0] = 'P';
	pak[1] = 'A';
	pak[2] = 'K';
	*(short *)&pak[3] =(short)filenum;
	pakfinfo->size+=filenum*16+16;	// 初始pak文件大小=文件信息大小+文件头大小
	for (i = 0; i != filenum; i++) {
		clean(filename,16);
		for (point = 0; filenames[point+len] != ' '; point++) {
			if (filenames[point+len] == 0)
				goto out;
			filename[point] = filenames[point+len];
		}
out:
		len += point+1;
		filename[point] = 0;
		finfo = file_search(filename,(struct FILEINFO *)(ADR_DISKIMG + 0x002600),224);
		file = fopen(filename);
		if (finfo == 0) {
			print("File not find!\n\n");
			break;
		}
		int j;
		// 首先把文件信息写入
		// 文件名
		for (j = 0; j != 12; j++) {
			pak[i*16+j+16] = finfo->name[j];
		}
		// 文件属性
		pak[i*16+12+16] = 0x20;
		// 文件起始地址
		*(short *)&pak[i*16+12+16] = (short)pakfinfo->size;
		// 文件长度
		*(short *)&pak[i*16+14+16] = (short)finfo->size;
		// 文件内容
		for (j = 0; j != finfo->size; j++) {
			pak[j+pakfinfo->size] = file[j];
		}
		pakfinfo->size+=finfo->size; // 将这个文件大小加上
	}
	return;
}
void unpak(char *pakname) {
	struct FILEINFO *finfo;
	char *file;
	char *pak = fopen(pakname);
	if (pak == 0) {
		print("File not find!\n\n");
		return;
	}
	if (pak[0] != 'P' || pak[1] != 'A' || pak[2] != 'K') {
		print("Is't Powerint DOS 386 PAK file!\n\n");
		return;
	}
	int i;
	char filename[16];
	short filenum = pak[3];
	for (i = 0; i != filenum; i++) {	// 文件名转换
		clean(filename,16);
		int j,flag=0,point=0;
		for (j = 0; j != 12; j++) {
			if (pak[16+i*16+j] == ' ') {
				if (flag == 0) {
					filename[point] = '.';
					point++;
					flag=1;
				}
				continue;
			}
			filename[point] = pak[16+i*16+j];
			point++;
		}
		// 新建文件
		filename[0] = 'U';	// 防止重名
		mkfile(filename);
		finfo = file_search(filename,(struct FILEINFO *)(ADR_DISKIMG + 0x002600),224);
		file = fopen(filename);
		unsigned short addr = *(short *)&pak[16+i*16+12];
		unsigned short length = *(short *)&pak[16+i*16+14];
		finfo->size = length;
		// 文件内容拷贝
		for (j = 0; j != length; j++) {
			file[j] = pak[addr+j];
		}
	}
	return;
}
void PUTCHINESE(int x, int y, char color, unsigned short CH)
{
	int i, j, k, offset;
	int flag;
	unsigned char buffer[32];
	unsigned char word[2] = {CH&0xff,(CH&0xff00)>>8}; // 閺�瑙勫灇娴ｇ姷娈戞潪顒傜垳閸氬海娈戝Ч澶婄摟缂傛牜鐖?
	unsigned char key[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
	unsigned char *p = fopen("HZK16");
	offset = (94 * (unsigned int)(word[0] - 0xa0 - 1) + (word[1] - 0xa0 - 1)) * 32;
	p = p + offset;
	//娴犲氦绻栨稉顏勬勾閸р偓 鐠?2娑擃亜鐡ч懞?
	for (i = 0; i < 32; i++)
	{
		buffer[i] = p[i];
	}
	for (k = 0; k < 16; k++)
	{
		for (j = 0; j < 2; j++)
		{
			for (i = 0; i < 8; i++)
			{
				flag = buffer[k * 2 + j] & key[i];
				if (flag)
				{
					Draw_Px(x + i + j * 8, y + k, color);
				}
			}
		}
	}
}
void PrintChineseChar(int x, int y, char color, unsigned short Cchar)
{
	PUTCHINESE(x * 16, y * 16, color, Cchar);
}
void PrintChineseStr(int x, int y, char color, unsigned char *str)
{
	int i;
	for (i = 0; i < strlen(str); i += 2)
	{
		unsigned char cstr[3] = {str[i], str[i + 1]};
		PrintChineseChar(x + i / 2, y, color, *(short*)(cstr));
	}
}
int GetCHorEN(unsigned char *str)
{
	if (str[0] > 0x80 && str[1] > 0x80) {
		return 1;
	} else if (str[0] > 0x80 && str[1] < 0x80) {
		return 0;
	} else {
		return 0;
	}
}
void beep(int point,int notes,int dup) {
	short low[7] = {138, 147, 165, 175, 196, 220, 247};
	short high[7] = {262, 294, 330, 349, 392, 440, 494};
	short middle[7] = {524, 587, 659, 698, 784, 880, 988};
	if (point == 1) {
		gensound(low[notes-1],dup);
	} else if (point == 2) {
		gensound(middle[notes-1],dup);
	} else if (point == 3) {
		gensound(high[notes-1],dup);
	}
	return;
}
