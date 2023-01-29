#ifndef _DRIVERS_H
#define _DRIVERS_H
#include <define.h>
// acpi.c
char checksum(unsigned char* addr, unsigned int length);
unsigned int* acpi_find_rsdp(void);
unsigned int acpi_find_table(char* Signature);
void init_acpi(void);
int acpi_shutdown(void);
// beep.c
void beep(int point, int notes, int dup);
// cmos.c
unsigned char read_cmos(unsigned char p);
void write_cmos(unsigned char p, unsigned char data);
unsigned int get_hour_hex();
unsigned int get_min_hex();
unsigned int get_sec_hex();
unsigned int get_day_of_month();
unsigned int get_day_of_week();
unsigned int get_mon_hex();
unsigned int get_year();
void write_cmos_time(unsigned int year,
                     unsigned char mon,
                     unsigned char day,
                     unsigned char hour,
                     unsigned char min);
// com.c
int init_serial(void);
int serial_received();
char read_serial();
int is_transmit_empty();
void write_serial(char a);
// dma.c
void dma_xfer(unsigned char channel,
              unsigned long address,
              unsigned int length,
              unsigned char read);
// floppy.c
void init_floppy();
void flint(int* esp);
int fdc_rw(int block,
           unsigned char* blockbuff,
           int read,
           unsigned long nosectors);
int fdc_rw_ths(int track,
               int head,
               int sector,
               unsigned char* blockbuff,
               int read,
               unsigned long nosectors);
int read_block(int block, unsigned char* blockbuff, unsigned long nosectors);
int write_block(int block, unsigned char* blockbuff, unsigned long nosectors);
int write_floppy_for_ths(int track,
                         int head,
                         int sec,
                         unsigned char* blockbuff,
                         unsigned long nosec);
void block2hts(int block, int* track, int* head, int* sector);
void hts2block(int track, int head, int sector, int* block);
// harddisk.c
void drivers_idehdd_read(unsigned int LBA,
                         unsigned int number,
                         unsigned short* buffer);
void drivers_idehdd_write(unsigned int LBA,
                          unsigned int number,
                          unsigned short* buffer);
struct IDEHardDiskInfomationBlock* drivers_idehdd_info();
// keyboard.c
void wait_KBC_sendready(void);
void init_keyboard(void);
int getch();
int input_char_inSM();
void inthandler21(int* esp);
// mouse.c
void enable_mouse(struct MOUSE_DEC* mdec);
void mouse_sleep(struct MOUSE_DEC* mdec);
void mouse_ready(struct MOUSE_DEC* mdec);
int mouse_decode(struct MOUSE_DEC* mdec, unsigned char dat);
void mouseinput();
void inthandler2c(int* esp);
// pci.c
uint32_t read_pci(uint8_t bus,
                  uint8_t device,
                  uint8_t function,
                  uint8_t registeroffset);
void write_pci(uint8_t bus,
               uint8_t device,
               uint8_t function,
               uint8_t registeroffset,
               uint32_t value);
uint32_t PCI_READ_COMMAND_STATUS(uint8_t bus, uint8_t slot, uint8_t func);
void PCI_WRITE_COMMAND_STATUS(uint8_t bus,
                              uint8_t slot,
                              uint8_t func,
                              uint32_t value);
uint8_t PCI_Get_Drive_IRQ_LINE(uint8_t bus, uint8_t slot, uint8_t func);
uint32_t PCI_Get_PORT_Base(uint8_t bus, uint8_t slot, uint8_t func);
void PCI_GET_DEVICE(uint16_t vendor_id,
                    uint16_t device_id,
                    uint8_t* bus,
                    uint8_t* slot,
                    uint8_t* func);
void PCI_CONFIGADDRESS(unsigned int Bus,
                       unsigned int f,
                       unsigned int equipment,
                       unsigned int adder);
void init_PCI(unsigned int adder_Base);
void PCI_ClassCode_Print(
    struct PCI_CONFIG_SPACE_PUCLIC* pci_config_space_puclic);
// pic.c
void init_pic(void);
void send_eoi(int irq);
void ClearMaskIrq(unsigned char irq);
void Maskirq(unsigned char irq);
// pcnet.c
static void set_handler(int IRQ, int addr);
void into_32bitsRW();
void into_16bitsRW();
void reset_card();
void Activate();
void init_pcnet_card();
void PcnetSend(uint8_t* buffer, int size);
void PCNET_IRQ(int* esp);
// vbe.c
int SwitchVBEMode(int mode);
int check_vbe_mode(int mode, struct VBEINFO* vinfo);
void SwitchToText8025_BIOS();
void SwitchTo320X200X256_BIOS();
void* GetSVGACardMemAddress();
char* GetSVGACharOEMString();
VESAModeInfo* GetVESAModeInfo(int mode);
void get_all_mode();
int set_mode(int width, int height, int bpp);
// vga.c
void write_regs(unsigned char* regs);
void SwitchTo320X200X256();
void SwitchToText8025();
void Set_Font(char* file);
static void set_plane(unsigned p);
void pokew(int setmentaddr, int offset, short value);
void pokeb(int setmentaddr, int offset, char value);
void set_palette(int start, int end, unsigned char* rgb);
static void vmemwr(unsigned dst_off, unsigned char* src, unsigned count);
void init_palette(void);
unsigned get_fb_seg(void);
// rtc.c
void rtc_handler();
void init_rtc();
// driver.c
void init_driver();
drv_t driver_malloc(char* drv_file, drv_type_t drv_type);
void driver_free(drv_t driver);
void driver_call(drv_t driver, int func, void* arg);
void driver_set_handler(drv_t driver, int func_addr, int handler_num);
drv_t driver_find(drv_type_t type);
// vdisk.c
int init_vdisk();
int register_vdisk(vdisk vd);
int logout_vdisk(char drive);
int rw_vdisk(char drive,
             unsigned int lba,
             unsigned char* buffer,
             unsigned int number,
             int read);
bool have_vdisk(char drive);
//rtl8139.c
bool rtl8139_find_card();
void init_rtl8139_card();
#endif
