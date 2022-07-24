#ifndef _CMD_H
#define _CMD_H
#include <define.h>
// command.c
void command_run(char *cmdline);
void print_date(unsigned short _date, unsigned short _time);
void pci_list();
void cmd_dir();
void type_deal(char *cmdline);
void edit(char *cmdline);
void chview(char *filename);
void pcinfo();
void mem();
void set_cons_color(char color);
char get_cons_color();
void mouseinput();
void cmd_tl();
void cmd_vbetest();
// execbatch.c
int run_bat(char *cmdline);
struct TASK *start_drv(char *cmdline);
int cmd_app(char *cmdline);
// CASM.c
char *_Asm(const char *asm_code1, int addr);
void compile_file(FILE *fp_asm, FILE *fp);
// setup.c
void lba2chs(int lba, int cyline, int header, int sector);
int chs2lba(int cyline, int header, int sector);
void setup(void);
// chat.c
void chat_cmd();
void chat_gui();
#endif