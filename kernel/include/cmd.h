#ifndef _CMD_H
#define _CMD_H
#include <define.h>
// command.c
void command_run(char *cmdline);
void pci_list();
void cmd_dir(char **args);
void tree(struct FAT_FILEINFO *directory);
void type_deal(char *cmdline);
void pcinfo();
void mem();
void cmd_tl();
void cmd_vbetest();
int compress_one_file(char* infilename, char* outfilename);
int decompress_one_file(char* infilename, char* outfilename);
void longName28dot3(char *result, char *src_in);
// execbatch.c
int run_bat(char *cmdline);
struct TASK *start_drv(char *cmdline);
int cmd_app(char *cmdline);
// CASM.c
char *_Asm(const char *asm_code1, int addr);
void compile_file(FILE *fp_asm, FILE *fp);
// chat.c
void chat_cmd();
void chat_gui();
// netgobang.c
void netgobang();
#endif