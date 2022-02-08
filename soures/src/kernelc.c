#include "system.h"
struct FIFO8 keyfifo,mousefifo;
struct MOUSE_DEC mdec;
struct timer Timer;
int task_num = 4;
int caps_lock = 0,shift = 0;
int memsize;
int mx=39,my=12;
char *line;
static char keytable[0x54] = {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', 0,
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 10, 0, 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 0, '|', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 97, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
    '2', '3', '0', '.'
};
static char keytable1[0x54] = {
	0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', 0,
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 10, 0, 'a', 's',
	'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
	'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 70, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
	'2', '3', '0', '.'
};
static char mouse_cur_graphic[16][16] = {
	"*...............",
	"**..............",
	"*O*.............",
	"*OO*............",
	"*OOO*...........",
	"*OOOO*..........",
	"*OOOOO*.........",
	"*OOOOOO*........",
	"*OOOOOOO*.......",
	"*OOOO*****......",
	"*OO*O*..........",
	"*O*.*O*.........",
	"**..*O*.........",
	"*....*O*........",
	".....*O*........",
	"......*........."
};
char pro_memman[1024]={0};	// 1bit = 16bytes
void print_32bits_ascil(unsigned int n)
{
    //ï¿½ï¿½Ò»ï¿½ï¿½32Î»ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½4ï¿½ï¿½ï¿½Ö½ï¿½ï¿½ï¿½ï¿?
    char str[32];
    int i = 0;
    for (i = 0; i < 4; i++)
    {
        str[i] = n & 0xff;
        n >>= 8;
    }
    //ï¿½ï¿½×ªï¿½Ö·ï¿½ï¿½ï¿½

    for (i = 0; i < 4; i++)
    {
        printchar(str[i]);
    }
}
void getCPUBrand(char *cBrand)
{
    print_32bits_ascil(get_cpu4(0x80000002));
    print_32bits_ascil(get_cpu5(0x80000002));
    print_32bits_ascil(get_cpu6(0x80000002));
    print_32bits_ascil(get_cpu7(0x80000002));
    print_32bits_ascil(get_cpu4(0x80000003));
    print_32bits_ascil(get_cpu5(0x80000003));
    print_32bits_ascil(get_cpu6(0x80000003));
    print_32bits_ascil(get_cpu7(0x80000003));
    print_32bits_ascil(get_cpu4(0x80000004));
    print_32bits_ascil(get_cpu5(0x80000004));
    print_32bits_ascil(get_cpu6(0x80000004));
    print_32bits_ascil(get_cpu7(0x80000004));
    printf("\n");
}
void shell(void)
{
    struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	// ï¿½ï¿½ï¿½ï¿½ï¿½Ö¿ï¿½
	SwitchTo320X200X256();
	SwitchToText8025();
    int i;
    int cpuid[3] = {get_cpu1(), get_cpu3(), get_cpu2()};
	
    clear();
	char *autoexec=fopen("autoexec.bat");
	if (autoexec == 0) {
		printf("Boot Error:No AUTOEXEC.BAT in Drive ");
		printchar(*(char *)(0x7dfd));
		print("\n");
	} else {
		run_bat("autoexec.bat");
	}
	print("AUTOEXEC.BAT has done!\n");
	print("KeyBoard Ready!\n");
    print("VGA Ready!\n");
	print("Mouse Ready!\n");
    printf("Ram Size:%dMB\n", memsize/(1024*1024));
    char cpu[100] = {0};
    //È¡ï¿½ï¿½ a b c ï¿½Ðµï¿½ï¿½Ä¸ï¿½ï¿½Ö½ï¿½ ï¿½ï¿½ï¿½ï¿½cpu
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
	print("\n");
	line = (char *)memman_alloc(memman,1024);
    while (1)
    {
        char drive[2] = {*(char *)(0x7dfd), 0};
        printf("%s:\\>", drive);
		int i;
		clean(line,1024);
		input(line,1024);
        command_run(line);
    }
}
void KernelMain(void)
{
    char keybuf[32];
	char mousebuf[128];
    char fifobuf[128];
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;
    clear();
    init_gdtidt();	// ÖØÐÂÉèÖÃGDT/IDT
    init_pic();
    io_sti(); 	// IDT/PICÉèÖÃÍê³É£¬·Å¿ªÍâ²¿ÖÐ¶Ï 
    io_out8(PIC0_IMR, 0xf8); // ·Å¿ª¼üÅÌ¡¢PIC1¡¢¼ÆÊ±Æ÷µÄPIC(11111000)
	io_out8(PIC1_IMR, 0xef); // ·Å¿ªÊó±ê(11101111)

    fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	
	init_acpi();	// ³õÊ¼»¯ACPI
// VMware²»Ö§³ÖACPI ËùÒÔÇë²»ÒªÓÃVMware
	
	init_keyboard();	// ï¿½ï¿½Ê¼ï¿½ï¿½ï¿½ï¿½ï¿½Ìµï¿½Â·
	enable_mouse(&mdec);	// ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Â·
	mouse_sleep(&mdec);	// ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ß£ï¿½ï¿½ï¿½ï¿½ï¿½fifoï¿½ï´«ï¿½ï¿½ï¿½ï¿½ï¿½Ý£ï¿½
	
	// ï¿½ï¿½Ê¼ï¿½ï¿½memman
    struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	memsize=memtest(0x00400000,0xbfffffff);
    memman_init(memman);
	memman_free(memman,0x00001000,0x00006c00); // ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Òªï¿½ï¿½ï¿½ï¿½
	memman_free(memman,0x00007e00,0x00097200);
	memman_free(memman,0x00400000,memsize-0x00400000);
	
	shell();	// ï¿½ï¿½ï¿½ï¿½shell
}
int get_memsize()
{
	return memsize;
}
void sleep(int s)
{
    init_pit();
    while (timerctl.count != s)
    {
        putchar(0);
    }
}
char *itoa(int n)
{
    char *res;
    sprintf(res, "%d", n);
    return res;
}
char ascii2num(char c)
{
	if (c > 'A'-1 && c < 'F'+1) {
		c=c-0x37;
	} else if (c > 'a'-1 && c < 'f'+1) {
		c=c-0x57;
	} else if (c > '0'-1 && c < '9'+1) {
		c=c-0x30;
	}
	return c;
}
char num2ascii(char c)
{
	if (c > 0x9 && c < 0x10) {
		c=c+0x37;
	} else if (c < 0x0a){
		c=c+0x30;
	}
	return c;
}
void input(char *ptr,int len)
{
	int i;
	for (i = 0; i != len; i++) {
		char in = getch();
		if (in == '\n') {
			ptr[i] = 0;
			print("\n");
			break;
		} else if (in == '\b') {
			if (i == 0) {
				i--;
				continue;
			}
			i--;
			ptr[i]=0;
			i--;
			printchar('\b');
			continue;
		}
		printchar(in);
		ptr[i]=in;
	}
}
char getch()
{
	char ch;
	ch = input_char_inSM();
	if (shift == 0 && caps_lock == 0) {	// shiftï¿½ï¿½caps_lock×´Ì¬Îª0
		return keytable1[ch];	// ï¿½ï¿½ï¿½ï¿½Ð¡Ð´ï¿½Ö·ï¿½
	} else if (shift == 1 || caps_lock == 1) {	// shiftï¿½ï¿½caps_lock×´Ì¬Îª1
		return keytable[ch];	// ï¿½ï¿½ï¿½Ø´ï¿½Ð´ï¿½Ö·ï¿½
	} else if (shift == 1 && caps_lock == 1) { // shiftï¿½ï¿½caps_lock×´Ì¬Îª1
		return keytable1[ch];	// ï¿½ï¿½ï¿½ï¿½Ð¡Ð´ï¿½Ö·ï¿½
	}
}
int input_char_inSM()
{
    int i;
    while (1) {
        if (fifo8_status(&keyfifo) == 0) {  //ï¿½ï¿½ï¿½ï¿½Ã»ï¿½Ð±ï¿½ï¿½ï¿½ï¿½Â»ï¿½ï¿½ï¿½
            io_stihlt(); //Ó²ï¿½ï¿½ÏµÍ³ï¿½ï¿½ï¿½ï¿½Ö±ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ä³ï¿½ï¿½ï¿½Ð¶ï¿½
        } else {
            i = fifo8_get(&keyfifo); //ï¿½ï¿½È¡ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
            io_sti();
            if (i < 0x80) {
                break;
            }
        }
    }
    return i;
}
void mouseinput()
{
	char mousecopy[80];
	mx=39;
	my=12;
	int i,mx1=mx,my1=my,bufx=39*8,bufy=12*16;
	mouse_ready(&mdec);
	while(1) {
		*(char *)(0xb8000+my1*160+mx1*2+1)=0x07;	// ï¿½ï¿½Ô­ï¿½ï¿½Î»ï¿½ÃµÄ¹ï¿½ê»?ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½×ªï¿½ï¿½É«ï¿½ï¿½
		*(char *)(0xb8000+my*160+mx*2+1)=0x70;	// ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Î»ï¿½ÃµÄ¹ï¿½ê£?ï¿½ï¿½×ªï¿½ï¿½É«ï¿½ï¿½
		if (fifo8_status(&keyfifo) != 0) {
			char c = fifo8_get(&keyfifo);
			if (c == 0x01) {
				break;
			}
		}
		if (fifo8_status(&mousefifo) == 0) {
			io_stihlt();
		} else {
			i = fifo8_get(&mousefifo);
			io_sti();
			if (mouse_decode(&mdec,i) != 0) {
				mx1 = mx;	// ï¿½ï¿½ï¿½ï¿½Ô­ï¿½ï¿½Î»ï¿½ï¿½
				my1 = my;
				bufx += mdec.x;
				bufy += mdec.y;
				mx = bufx / 8;	// ï¿½ï¿½ï¿½ï¿½Ä£Ê½Ò»ï¿½ï¿½ï¿½ï¿½ï¿½Ö³ï¿½8ï¿½ï¿½ï¿½ï¿½
				my = bufy / 16;	// ï¿½ï¿½ï¿½ï¿½Ä£Ê½Ò»ï¿½ï¿½ï¿½ï¿½ï¿½Ö¿ï¿½16ï¿½ï¿½ï¿½ï¿½
				if (bufx > 79*8) {
					bufx=79*8;
				} else if (bufx < 0) {
					bufx=0;
				}
				if (bufy > 24*16) {
					bufy=24*16;
				} else if (bufy < 0) {
					bufy=0;
				}
			}
		}
	}
	mouse_sleep(&mdec);
	return;
}
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat)
{
	if (mdec->phase == 1) {
		if (dat == 0xfa) {	// ACK
			return 0;
		}
		mdec->buf[0] = dat;
		mdec->phase = 2;
		return 0;
	} else if (mdec->phase == 2) {
		mdec->buf[1] = dat;
		mdec->phase = 3;
		return 0;
	} else if (mdec->phase == 3) {
		mdec->buf[2] = dat;
		mdec->phase = 1;
		mdec->btn = mdec->buf[0] & 0x07;	// ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½È¡ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Î»ï¿½ï¿½
		mdec->x = mdec->buf[1];	// x
		mdec->y = mdec->buf[2];	// y
		if ((mdec->buf[0] & 0x10) != 0) {
			mdec->x |= 0xffffff00;
		}
		if ((mdec->buf[0] & 0x20) != 0) {
			mdec->y |= 0xffffff00;
		}
		mdec->y = -mdec->y;	// yÈ¡ï¿½ï¿½
		return 1;
	}
	return -1;
}
void inthandler36(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)
{
	// 36ºÅÏµÍ³µ÷ÓÃÖÐ¶Ï
	// eax=0x01 »ñÈ¡ÏµÍ³°æ±¾ºÅ£¨ASCIIÂë£©
	// eax=0x02 ´òÓ¡Ò»¸ö×Ö·û
	// eax=0x03 VGAÏÔÊ¾·½Ãæ
		// ebx=0x01 ÇÐ»»µ½80X25ÎÄ±¾Ä£Ê½
		// ebx=0x02 ÇÐ»»µ½320X200X256Í¼ÐÎÄ£Ê½
		// ebx=0x03 Í¼ÐÎÄ£Ê½ÏÂ´òÓ¡×Ö·û
		// ebx=0x04 Í¼ÐÎÄ£Ê½ÏÂ´òÓ¡ºº×Ö
		// ebx=0x05 Í¼ÐÎÄ£Ê½ÏÂ»­·½¿é
		// ebx=0x06 Í¼ÐÎÄ£Ê½ÏÂ»­µã
		// ebx=0x07 Í¼ÐÎÄ£Ê½ÏÂ´òÓ¡×Ö·û´®
		// ebx=0x08 Í¼ÐÎÄ£Ê½ÏÂ´òÓ¡ºº×Ö´®
	// eax=0x04 ÉèÖÃ¹â±ê×ø±ê
	// eax=0x05 ´òÓ¡×Ö·û´®
	// eax=0x06 µÈ´ý
	// eax=0x08 Õ¼ÓÃÒ»¿éÄÚ´æ
	// eax=0x09 ÊÍ·ÅÒ»¿éÄÚ´æ
	// eax=0x0c ÎÄ×ÖÄ£Ê½ÏÂ»­·½¿é
	// eax=0x0d ·äÃùÆ÷·¢Éù
	// eax=0x0e µÃµ½¹â±ê×ø±ê
	// eax=0x0f ÆôÓÃÊó±ê²¢»ñÈ¡Êó±êµã»÷Ê±µÄ×ø±êºÍ°´¼ü£¨ÎÄ×ÖÄ£Ê½£©
	// eax=0x10 ÆôÓÃÊó±ê²¢»ñÈ¡Êó±êµã»÷Ê±µÄ×ø±êºÍ°´¼ü£¨Í¼ÐÎÄ£Ê½£©
	// eax=0x16 ¼üÅÌÊäÈë·½Ãæ
		// ebx=0x01 µÃµ½¼üÅÌÊäÈë×Ö·ûµÄASCIIÂë
		// ebx=0x02 µÃµ½¼üÅÌÊäÈë×Ö·ûµÄÉ¨ÃèÂë
		// ebx=0x03 µÃµ½¼üÅÌÊäÈëµÄ×Ö·û´®
	// eax=0x17 ´ÅÅÌ¶ÁÐ´·½Ãæ
		// ebx=0x01 ¶ÁÓ²ÅÌ
		// ebx=0x02 Ð´Ó²ÅÌ
	// eax=0x18 ÏÔÊ¾BMPÍ¼Æ¬
	// eax=0x19 ÔËÐÐÏµÍ³ÃüÁî
	// eax=0x1a ÎÄ¼þ¹ÜÀí·½Ãæ
		// ebx=0x01 ²éÑ¯ÎÄ¼þ´óÐ¡
		// ebx=0x02 ¼ÓÔØÎÄ¼þÈëÄÚ´æ
	// eax=0x1b µÃµ½ÊäÈëµÄÃüÁîÐÐ
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
    int cs_base = *((int *)(0xfe8));
	int ds_base = *((int *)(0xfec));
	int alloc_addr = *((int *)(0xff0));	// ï¿½ï¿½ï¿½ï¿½allocï¿½ï¿½ï¿½ï¿½Ê¼ï¿½ï¿½Ö·
	if (eax == 0x01) {
		intreturn(eax,ebx,ecx,0x302e3561,esi,edi,ebp);	// 0.5a
	} else if (eax == 0x02) {
        printchar((edx&0x000000ff));
    } else if (eax == 0x03) {
        if (ebx == 0x01) {
            SwitchToText8025();
        } else if (ebx == 0x02) {
            SwitchTo320X200X256();
        } else if (ebx == 0x03) {
            Draw_Char(ecx, edx, esi, edi);
        } else if (ebx == 0x04) {
            PrintChineseChar(ecx, edx, edi, esi);
		} else if (ebx == 0x05) {
			Draw_Box(ecx,edx,esi,edi,ebp);
		} else if (ebx == 0x06) {
			Draw_Px(ecx,edx,esi);
		} else if (ebx == 0x07) {
			Draw_Str(ecx,edx,esi+ds_base,edi);
		} else if (ebx == 0x08) {
			PrintChineseStr(ecx,edx,edi,esi+ds_base);
		}
    } else if (eax == 0x04) {
        gotoxy(edx, ecx);
    } else if (eax == 0x05) {
        print(edx + ds_base);
    } else if (eax == 0x06) {
		sleep(edx);
	} else if (eax == 0x08) {
		int i,len;
		ecx = (ecx+128)/128;
		if (ecx > 1024) {
			intreturn(eax,ebx,ecx,0,esi,edi,ebp);
		}
		for (len=ecx,i = 0; i < 1024; i++) {
			if (len == 0) {	// ï¿½Òµï¿½ï¿½ï¿½ï¿½Ê´ï¿½Ð¡ï¿½Ò¿ï¿½ï¿½Ðµï¿½ï¿½Ú´ï¿½
				for (len = 0;len < ecx;len++) {
					pro_memman[i-1-len] = 0xff;	// Õ¼ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ú´ï¿½
				}
				intreturn(eax,ebx,ecx,(alloc_addr-ds_base)+(i-ecx)*128,esi,edi,ebp);
				// ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Æ¬ï¿½Ú´ï¿½ï¿½ï¿½ï¿½Ê¼ï¿½ï¿½Ö?
				i = 0;
				break;
			}
			if (pro_memman[i] == 0x00) {	// ï¿½ï¿½ï¿½ï¿½
				len--;
			} else if (pro_memman[i] == 0xff && len != 0) {	// Õ¼ï¿½ï¿½ï¿½ï¿½lenï¿½Ñ¾ï¿½ï¿½ä»¯
				len=ecx;	// lenï¿½ï¿½Ô­
			}
		}
		if (i == 1023) {	// Ã»ï¿½Òµï¿½ï¿½ï¿½ï¿½Êµï¿½ï¿½Ú´ï¿½
			intreturn(eax,ebx,ecx,0,esi,edi,ebp);
			// ï¿½ï¿½ï¿½ï¿½0
		}
	} else if (eax == 0x09) {
		int i;
		ecx = (ecx+128)/128;
		edx = (edx+ds_base-alloc_addr)/128;
		for (i = edx; i < ecx+edx; i++) {
			pro_memman[i] = 0x00;	// ï¿½Í·ï¿½edx~(ecx+edx)ï¿½ï¿½ï¿½Ú´ï¿½
		}
	} else if (eax == 0x0c) {
		int i,j;
		for (i = ebx; i < edx; i++) {
			for (j = ecx; j < esi; j++) {
				*(char *)(0xb8000+i*160+j*2+1)=(char)(edi&0x000000ff);
			}
		}
	} else if (eax == 0x0e) {
		intreturn(eax,ebx,get_y(),get_x(),esi,edi,ebp);
	} else if (eax == 0x0d) {
		beep(ebx,ecx,edx);
	} else if (eax == 0x0f) {
		mx=39;
		my=12;
		int i,mx1=mx,my1=my,bufx=39*8,bufy=12*16;
		mouse_ready(&mdec);
		for (;;) {
			*(char *)(0xb8000+my1*160+mx1*2+1)=0x07;	// ï¿½ï¿½Ô­ï¿½ï¿½Î»ï¿½ÃµÄ¹ï¿½ê»?ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½×ªï¿½ï¿½É«ï¿½ï¿½
			*(char *)(0xb8000+my*160+mx*2+1)=0x70;	// ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Î»ï¿½ÃµÄ¹ï¿½ê£?ï¿½ï¿½×ªï¿½ï¿½É«ï¿½ï¿½
			if (fifo8_status(&mousefifo) == 0) {
				io_stihlt();
			} else {
				i = fifo8_get(&mousefifo);
				io_sti();
				if (mouse_decode(&mdec,i) != 0) {
					if ((mdec.btn & 0x01) != 0) {	// ï¿½ï¿½ï¿?
						intreturn(eax,ebx,mx,my,1,edi,ebp);
						break;
					} else if ((mdec.btn & 0x02) != 0) {	// ï¿½Ò¼ï¿½
						intreturn(eax,ebx,mx,my,2,edi,ebp);
						break;
					} else if ((mdec.btn & 0x04) != 0) {	// ï¿½ï¿½ï¿½ï¿½
						intreturn(eax,ebx,mx,my,3,edi,ebp);
						break;
					}
					mx1 = mx;	// ï¿½ï¿½ï¿½ï¿½Ô­ï¿½ï¿½Î»ï¿½ï¿½
					my1 = my;
					bufx += mdec.x;
					bufy += mdec.y;
					mx = bufx / 8;	// ï¿½ï¿½ï¿½ï¿½Ä£Ê½Ò»ï¿½ï¿½ï¿½ï¿½ï¿½Ö³ï¿½8ï¿½ï¿½ï¿½ï¿½
					my = bufy / 16;	// ï¿½ï¿½ï¿½ï¿½Ä£Ê½Ò»ï¿½ï¿½ï¿½ï¿½ï¿½Ö¿ï¿½16ï¿½ï¿½ï¿½ï¿½
					if (bufx > 79*8) {
						bufx=79*8;
					} else if (bufx < 0) {
						bufx=0;
					}
					if (bufy > 24*16) {
						bufy=24*16;
					} else if (bufy < 0) {
						bufy=0;
					}
				}
			}
		}
		mouse_sleep(&mdec);
	} else if (eax == 0x10) {
		int	gmx=152,gmy=92;
		int i;
		int x,y;
		mouse_ready(&mdec);
		for (;;) {
			for (y = 0; y < 16; y++) {	// »­ÏÖÔÚÊó±ê
				for (x = 0; x < 16; x++) {
					if (mouse_cur_graphic[y][x] == '*') {
						*(char *)(0xa0000 + (gmy+y)*320 + (gmx+x)) = 0;
					} else if (mouse_cur_graphic[y][x] == 'O') {
						*(char *)(0xa0000 + (gmy+y)*320 + (gmx+x)) = 7;
					}
				}
			}
			if (fifo8_status(&mousefifo) == 0) {
				io_stihlt();
			} else {
				i = fifo8_get(&mousefifo);
				io_sti();
				if (mouse_decode(&mdec,i) != 0) {
					for (y = 0; y < 16; y++) {	// »­µôÔ­À´µÄ
						for (x = 0; x < 16; x++) {
							*(char *)(0xa0000 + (gmy+y)*320 + (gmx+x)) = 14;
						}
					}
					if ((mdec.btn & 0x01) != 0) {	// ï¿½ï¿½ï¿?
						intreturn(eax,ebx,gmx,gmy,1,edi,ebp);
						break;
					} else if ((mdec.btn & 0x02) != 0) {	// ï¿½Ò¼ï¿½
						intreturn(eax,ebx,gmx,gmy,2,edi,ebp);
						break;
					} else if ((mdec.btn & 0x04) != 0) {	// ï¿½ï¿½ï¿½ï¿½
						intreturn(eax,ebx,gmx,gmy,3,edi,ebp);
						break;
					}
					gmx += mdec.x;
					gmy += mdec.y;
					if (gmx > 310) {
						gmx=310;
					} else if (gmx < 0) {
						gmx=0;
					}
					if (gmy > 184) {
						gmy=184;
					} else if (gmy < 0) {
						gmy=0;
					}
				}
			}
		}
		mouse_sleep(&mdec);
	} else if (eax == 0x16) {
		if (ebx == 0x01) {
			intreturn(eax, ebx, ecx, getch(), esi, edi, ebp);
		} else if (ebx == 0x02) {
			intreturn(eax, ebx, ecx, input_char_inSM(), esi, edi, ebp);
		} else if (ebx == 0x03) {
			input((char *)(edx+ds_base),ecx);
		}
	} else if (eax == 0x17) {
		if (ebx == 0x01) {
			drivers_idehdd_read(ecx,edx,esi+ds_base);
		} else if (ebx == 0x02) {
			drivers_idehdd_write(ecx,edx,esi+ds_base);
		}
	} else if (eax == 0x18) {
		bmpview(edx + ds_base);
	} else if (eax == 0x19) {
		command_run(edx + ds_base);
	} else if (eax == 0x1a) {
		if (ebx == 0x01) {
			struct FILEINFO *finfo;
			finfo = file_search((char *)(ds_base+edx),(struct FILEINFO *)(ADR_DISKIMG + 0x002600),224);
			intreturn(eax,ebx,ecx,finfo->size,esi,edi,ebp);
		} else if (ebx == 0x02) {
			struct FILEINFO *finfo;
			finfo = file_search((char *)(ds_base+edx),(struct FILEINFO *)(ADR_DISKIMG + 0x002600),224);
			char *p=fopen((char *)(ds_base+edx));
			char *q=ds_base+esi;
			int i;
			if (p != 0) {
				for (i = 0; i != finfo->size; i++) {
					q[i] = p[i];
				}
				intreturn(1,ebx,ecx,edx,esi,edi,ebp);
			} else {
				intreturn(0,ebx,ecx,edx,esi,edi,ebp);
			}
		}
	} else if (eax == 0x1b) {
		int i;
		char *bes = (char *)(edx + ds_base);
		for (i = 0; i < strlen(line); i++) {
			bes[i] = line[i];
		}
		bes[i] = 0;
	}
	return;
}
char bcd2hex(char bcd)
{
	char i;
	if (bcd > 0x10 || bcd == 0x10) {
		i = (bcd/0x10)*6;
		i = i+bcd-(bcd/0x10)*0x10;
		return i;
	} else {
		return bcd;
	}
}
char hex2bcd(char hex)
{
	char i;
	if (hex > 0x0a || hex == 0x0a) {
		i = (hex/0x0a)*0x10;
		i = i+hex-(hex/0x0a)*0x0a;
		return i;
	} else {
		return hex;
	}
}
void init_gdtidt(void)
{
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;
    struct GATE_DESCRIPTOR *idt = (struct GATE_DESCRIPTOR *)ADR_IDT;
    int i;

    /* GDTï¿½ï¿½Ê¼ï¿½ï¿½ */
    for (i = 0; i <= LIMIT_GDT / 8; i++)
    {
        set_segmdesc(gdt + i, 0, 0, 0);
    }
    set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, AR_DATA32_RW);
    set_segmdesc(gdt + 2, LIMIT_BOTPAK, ADR_BOTPAK, AR_CODE32_ER);
    load_gdtr(LIMIT_GDT, ADR_GDT);

    /* IDTï¿½ï¿½Ê¼ï¿½ï¿½ */
    for (i = 0; i <= LIMIT_IDT / 8; i++)
    {
        set_gatedesc(idt + i, 0, 0, 0);
    }
    load_idtr(LIMIT_IDT, ADR_IDT);

    /* IDTï¿½ï¿½ï¿½ï¿½*/
	set_gatedesc(idt + 0x00, (int)asm_error0, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x01, (int)asm_error1, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x03, (int)asm_error3, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x04, (int)asm_error4, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x05, (int)asm_error5, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x06, (int)asm_error6, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x07, (int)asm_error7, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x08, (int)asm_error8, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x09, (int)asm_error9, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x0a, (int)asm_error10, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x0b, (int)asm_error11, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x0c, (int)asm_error12, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x0d, (int)asm_error13, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x0e, (int)asm_error14, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x10, (int)asm_error16, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x11, (int)asm_error17, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x12, (int)asm_error18, 2 * 8, AR_INTGATE32);
    set_gatedesc(idt + 0x20, (int)asm_inthandler20, 2 * 8, AR_INTGATE32);
    set_gatedesc(idt + 0x21, (int)asm_inthandler21, 2 * 8, AR_INTGATE32);
    set_gatedesc(idt + 0x36, (int)asm_inthandler36, 2 * 8, AR_INTGATE32);
    set_gatedesc(idt + 0x2c, (int)asm_inthandler2c, 2 * 8, AR_INTGATE32);

    return;
}
void inthandler21(int *esp)
{
    unsigned char data, s[4];
    /* Í¨ÖªPIC IRQ-01 ï¿½Ñ¾ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿? ï¿½ï¿½ï¿½ò²»´ï¿½ï¿½ï¿½ï¿½ï¿½Ò»ï¿½ï¿½ï¿½Ð¶ï¿½ */
    io_out8(PIC0_OCW2, 0x61);
    data = io_in8(PORT_KEYDAT);
	if (data == 0x2a || data == 0x36) {	// ï¿½ï¿½ï¿½ï¿½shift
		shift=1;
		return;
	}
	if (data == 0x3a) {	// ï¿½ï¿½ï¿½ï¿½Caps Lock
		caps_lock = caps_lock ^ 1;
		return;
	}
	if (data == 0xaa || data == 0xb6) {	// ï¿½É¿ï¿½shift
		shift=0;
		return;
	}
    fifo8_put(&keyfifo, data);

    return;
}
void inthandler2c(int *esp)
{
	unsigned char data;
	io_out8(PIC1_OCW2,0x64);
	io_out8(PIC0_OCW2,0x62);
	data = io_in8(PORT_KEYDAT);
	if (mdec.sleep == 0) {	// ï¿½ï¿½ï¿½Ã»ï¿½ï¿½ï¿½ï¿½ï¿½ï¿?
		fifo8_put(&mousefifo,data);
	}
	return;
}
int get_caps_lock()
{
	return caps_lock;
}
int get_shift()
{
	return shift;
}
char *get_point_of_keytable()
{
	return keytable;
}
char *get_point_of_keytable1()
{
	return keytable1;
}
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar)
{
    if (limit > 0xfffff)
    {
        ar |= 0x8000; /* G_bit = 1 */
        limit /= 0x1000;
    }
    sd->limit_low = limit & 0xffff;
    sd->base_low = base & 0xffff;
    sd->base_mid = (base >> 16) & 0xff;
    sd->access_right = ar & 0xff;
    sd->limit_high = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
    sd->base_high = (base >> 24) & 0xff;
    return;
}

void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar)
{
    gd->offset_low = offset & 0xffff;
    gd->selector = selector;
    gd->dw_count = (ar >> 8) & 0xff;
    gd->access_right = ar & 0xff;
    gd->offset_high = (offset >> 16) & 0xffff;
    return;
}

void init_pic(void)
{
    io_out8(PIC0_IMR, 0xff); /* ï¿½ï¿½Ö¹ï¿½ï¿½ï¿½ï¿½ï¿½Ð¶ï¿½ */
    io_out8(PIC1_IMR, 0xff); /* ï¿½ï¿½Ö¹ï¿½ï¿½ï¿½ï¿½ï¿½Ð¶ï¿½ */

    io_out8(PIC0_ICW1, 0x11);   /* ï¿½ï¿½Ôµï¿½ï¿½ï¿½ï¿½Ä£Ê½ï¿½ï¿½edge trigger modeï¿½ï¿½ */
    io_out8(PIC0_ICW2, 0x20);   /* IRQ0-7ï¿½ï¿½INT20-27ï¿½ï¿½ï¿½ï¿½ */
    io_out8(PIC0_ICW3, 1 << 2); /* PIC1ï¿½ï¿½IRQ2ï¿½ï¿½ï¿½ï¿½ */
    io_out8(PIC0_ICW4, 0x01);   /* ï¿½Þ»ï¿½ï¿½ï¿½ï¿½ï¿½Ä£Ê½ */

    io_out8(PIC1_ICW1, 0x11); /* ï¿½ï¿½Ôµï¿½ï¿½ï¿½ï¿½Ä£Ê½ï¿½ï¿½edge trigger modeï¿½ï¿½ */
    io_out8(PIC1_ICW2, 0x28); /* IRQ8-15ï¿½ï¿½INT28-2fï¿½ï¿½ï¿½ï¿½ */
    io_out8(PIC1_ICW3, 2);    /* PIC1ï¿½ï¿½IRQ2ï¿½ï¿½ï¿½ï¿½ */
    io_out8(PIC1_ICW4, 0x01); /* ï¿½Þ»ï¿½ï¿½ï¿½ï¿½ï¿½Ä£Ê½ */

    io_out8(PIC0_IMR, 0xfb); /* 11111011 PIC1ï¿½ï¿½ï¿½ï¿½È«ï¿½ï¿½ï¿½ï¿½Ö¹ */
    io_out8(PIC1_IMR, 0xff); /* 11111111 ï¿½ï¿½Ö¹ï¿½ï¿½ï¿½ï¿½ï¿½Ð¶ï¿½ */
    return;
}

int cmd_app(char *cmdline)
{
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
    struct FILEINFO *finfo;
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;
    char name[18], *p, *q, *alloc;
    int i, segsiz, datsiz, dathrb, esp;
    for (i = 0; i < 13; i++)
    {
        if (cmdline[i] <= ' ')
        {
            break;
        }
        name[i] = cmdline[i];
    }
    name[i] = 0;
    finfo = file_search(name, (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
    if (finfo == 0 && name[i - 1] != '.') {
        name[i] = '.';
        name[i + 1] = 'B';
        name[i + 2] = 'I';
        name[i + 3] = 'N';
        name[i + 4] = 0;
        finfo = file_search(name, (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
    }
    if (finfo != 0)
    {
		for (i = 0; i != 1024; i++) {
			pro_memman[i] = 0x00;	// ÊÍ·Å³ÌÐòÓÃÀ´allocµÄÄÚ´æ
		}
		if ((fopen(name))[0] == 'A') {	// »ã±à
			p = (char *)memman_alloc_4k(memman, finfo->size+128*1024);
			// ¶à¼Ó128KBÓÃÀ´¸ø³ÌÐòalloc
			*((int *)(0xfe8)) = p;	// cs_base
			*((int *)(0xfec)) = p;	// ds_base
			*((int *)(0xff0)) = p+finfo->size;
			memcpy(p, fopen(name), finfo->size);
			set_segmdesc(gdt + 3, finfo->size - 1, (int)p, AR_CODE32_ER);
			set_segmdesc(gdt + 4, finfo->size - 1 + 128*1024, (int)p, AR_DATA32_RW);
			start_app(1, 3 * 8, 4 * 8);
			memman_free_4k(memman, (int)p, finfo->size+128*1024);
			print("\n");
		} else if ((fopen(name))[0] == 'C'){	// C
			p = (char *)memman_alloc_4k(memman, finfo->size);
			*((int *)(0xfe8)) = p;	// cs_base
			memcpy(p, fopen(name), finfo->size);
			segsiz = *((int *)(p + 0x0000 + 6 + 1));
			esp = *((int *)(p + 0x000c + 6 + 1));
			datsiz = *((int *)(p + 0x0010 + 6 + 1));
			dathrb = *((int *)(p + 0x0014 + 6 + 1));
			q = (char *) memman_alloc_4k(memman, segsiz+128*1024);
			// ¶àÕ¼ÓÃ128KBÓÃÀ´¸ø³ÌÐò½øÐÐÄÚ´æ¹ÜÀí£¨½ô¸úÔÚÊý¾ÝÇøºó£©
			*((int *)(0xfec)) = q;	// ds_base
			*((int *)(0xff0)) = q+segsiz;	// ½«³ÌÐòallocµÄÆðÊ¼µØÖ··ÅÔÚ0xff0
			set_segmdesc(gdt + 3, finfo->size - 1, (int)p, AR_CODE32_ER);
			set_segmdesc(gdt + 4, segsiz - 1 + 128*1024, (int)q, AR_DATA32_RW + 0x60);
			for (i = 0; i < datsiz; i++)
			{
				q[esp + i] = p[dathrb + i + 6 + 1];
			}
			start_app(1, 3 * 8, 4 * 8);
			memman_free_4k(memman, (int) p, finfo->size);
			memman_free_4k(memman, (int) q, segsiz+128*1024);
			print("\n");
		} else {
			print("Isn't Powerint DOS 386 Execute File.\n\n");
		}
        return 1;
    }
    return 0;
}

int run_bat(char *cmdline)
{
	struct FILEINFO *finfo;
	char *file;
	int i,j=0;
	char name[18]={0};
	char file1[128]={0};
    for (i = 0; i < 13; i++)
    {
        if (cmdline[i] <= ' ')
        {
            break;
        }
        name[i] = cmdline[i];
    }
    name[i] = 0;
	finfo = file_search(name, (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
	file = fopen(name);
	if(finfo == 0)
	{
		if (finfo == 0 && name[i - 1] != '.')
		{
			/*ï¿½ï¿½ï¿½ï¿½ï¿½Ò²ï¿½ï¿½ï¿½ï¿½Ä¼ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ä¼ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ï¡ï¿?.batï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ñ°ï¿½ï¿½*/
			name[i] = '.';
			name[i + 1] = 'B';
			name[i + 2] = 'A';
			name[i + 3] = 'T';
			name[i + 4] = 0;
			finfo = file_search(name, (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
			file = fopen(name);
			if (finfo == 0) {
				return 0;
			}
			goto ok;
		}
		return 0;
	}
ok:
	for (i = 0; i != finfo->size; i++) {
		if (file[i] == 0x0d && file[i+1] == 0x0a) {
			command_run(file1);
			i++;
			j=0;
			int p;
			for (p = 0; p != 128; p++) {
				file1[p] = 0;
			}
			continue;
		}
		file1[j] = file[i];
		j++;
	}
	int p;
	command_run(file1);
	for (p = 0; p != 128; p++) {
		file1[p] = 0;
	}
	return 1;
}
void ERROR0()
{
	saveregisters();	// ï¿½ï¿½Ó¦loadregisters();
	ERROR(0,"#DE");
}
void ERROR1()
{
	saveregisters();
	ERROR(1,"#DB");
}
void ERROR3()
{
	saveregisters();
	ERROR(3,"#BP");
}
void ERROR4()
{
	saveregisters();
	ERROR(4,"#OF");
}
void ERROR5()
{
	saveregisters();
	ERROR(5,"#BR");
}
void ERROR6()
{
	saveregisters();
	ERROR(6,"#UD");
}
void ERROR7()
{
	saveregisters();
	ERROR(7,"#NM");
}
void ERROR8()
{
	saveregisters();
	ERROR(8,"#DF");
}
void ERROR9()
{
	saveregisters();
	ERROR(9,"#MF");
}
void ERROR10()
{
	saveregisters();
	ERROR(10,"#TS");
}
void ERROR11()
{
	saveregisters();
	ERROR(11,"#NP");
}
void ERROR12()
{
	saveregisters();
	ERROR(12,"#SS");
}
void ERROR13()
{
	saveregisters();
	ERROR(13,"#GP");
}
void ERROR14()
{
	saveregisters();
	ERROR(14,"#PF");
}
void ERROR16()
{
	saveregisters();
	ERROR(16,"#MF");
}
void ERROR17()
{
	saveregisters();
	ERROR(17,"#AC");
}
void ERROR18()
{
	saveregisters();
	ERROR(18,"#MC");
}
void ERROR19()
{
	saveregisters();
	ERROR(19,"#XF");
}
void ERROR(int CODE, char *TIPS)
{
    SwitchToText8025();
    clear();
    int i, j;
    for (i = 0; i < 160; i++)
    {
        for (j = 0; j < 25; j++)
        {
            //ï¿½ï¿½ï¿½×°ï¿½ï¿½ÖµÄ´ï¿½ï¿½ï¿½ï¿½ï¿½Ê¾
            if (i % 2 == 1)
            {
                *(char *)(0xb8000 + j * 160 + i) = 0x47;
            }
        }
    }
    Move_Cursor(0, 0);
	//ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
	beep(2,7,8);
    //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ï¢ï¿½ï¿½ï¿½ï¿½Ê¾
    printf("Sorry!Your computer has some problem!\n");
    printf("Please check your computer and try again.Or Restart your computer.\n");
    printf("Now You just look at the Error Message.\n");
    printf("You can rebuild PowerintDOS 386 too.\n");
    printf("The System Version:%s\n", VERSION);
    printf("Error Code:%08x\n", CODE);
    printf("Error Message:%s\n", TIPS);

	loadregisters();	// ï¿½ï¿½Ô­ï¿½Ä´ï¿½ï¿½ï¿½
    //ï¿½ï¿½ï¿½ï¿½Ä´ï¿½ï¿½ï¿½ï¿½ï¿½Ö?
	int eax;
    asm("movl %%eax,%0" : "=g"(eax));
    printf("eax=%08x\n", eax);
    int ebx;
    asm("movl %%ebx,%0" : "=g"(ebx));
    printf("ebx=%08x\n", ebx);
    int ecx;
    asm("movl %%ecx,%0" : "=g"(ecx));
    printf("ecx=%08x\n", ecx);
    int edx;
    asm("movl %%edx,%0" : "=g"(edx));
    printf("edx=%08x\n", edx);
    int esi;
    asm("movl %%esi,%0" : "=g"(esi));
    printf("esi=%08x\n", esi);
    int edi;
    asm("movl %%edi,%0" : "=g"(edi));
    printf("edi=%08x\n", edi);
    int ebp;
    asm("movl %%ebp,%0" : "=g"(ebp));
    printf("ebp=%08x\n", ebp);
    int esp;
    asm("movl %%esp,%0" : "=g"(esp));
    printf("esp=%08x\n", esp);
	printf("eip=%08x\n", get_eip());
    //ï¿½Î¼Ä´ï¿½ï¿½ï¿½
    int cs;
    asm("movl %%cs,%0" : "=g"(cs));
    printf("cs=%04x\n", cs);
    int ds;
    asm("movl %%ds,%0" : "=g"(ds));
    printf("ds=%04x\n", ds);
    int es;
    asm("movl %%es,%0" : "=g"(es));
    printf("es=%04x\n", es);
    int fs;
    asm("movl %%fs,%0" : "=g"(fs));
    printf("fs=%04x\n", fs);
    int gs;
    asm("movl %%gs,%0" : "=g"(gs));
    printf("gs=%04x\n", gs);
    int ss;
    asm("movl %%ss,%0" : "=g"(ss));
    printf("ss=%04x\n", ss);
    for (;;){}
}
