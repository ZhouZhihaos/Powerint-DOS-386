#include "system.h"
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;
void PCI_CONFIGADDRESS(unsigned int Bus, unsigned int f, unsigned int equipment, unsigned int adder)
{
    unsigned int cmd = 0;
    cmd = 0x80000000 + (unsigned int)adder + ((unsigned int)f << 8) + ((unsigned int)equipment << 11) + ((unsigned int)Bus << 16);
    // cmd = cmd | 0x01;
    io_out32(0xcf8, cmd);
}
void init_PCI(unsigned int adder_Base)
{
    unsigned int i, BUS, Equipment, F, ADDER, *i1;
    unsigned char *PCI_DATA = adder_Base, *PCI_DATA1;
    for (BUS = 0; BUS < 256; BUS++)
    { //查询总线
        for (Equipment = 0; Equipment < 32; Equipment++)
        { //查询设备
            for (F = 0; F < 8; F++)
            { //查询功能
                PCI_CONFIGADDRESS(BUS, F, Equipment, 0);
                if (io_in32(0xcfc) != 0xFFFFFFFF)
                {
                    //当前插槽有设备
                    //把当前设备信息映射到PCI数据区
					int key = 1;
                    while (key)
                    {
                        // if (*PCI_DATA == 0x00) {
                        //此配置表为空
                        // printk("PCI_DATA:%x\n", PCI_DATA);
                        //getch();
                        PCI_DATA1 = PCI_DATA;
                        *PCI_DATA1 = 0xFF; //表占用标志
                        // printk("PCI:%x\n", &PCI_DATA[0]);
                        PCI_DATA1++;
                        *PCI_DATA1 = BUS; //总线号
                        PCI_DATA1++;
                        *PCI_DATA1 = Equipment; //设备号
                        PCI_DATA1++;
                        *PCI_DATA1 = F; //功能号
                        PCI_DATA1++;
                        PCI_DATA1 = PCI_DATA1 + 8;
                        // printk("BUS:%x\n", BUS);
                        // printk("Equipment:%x\n", Equipment);
                        // printk("F:%x\n", F);
                        //写入寄存器配置
                        for (ADDER = 0; ADDER < 256; ADDER = ADDER + 4)
                        {
                            PCI_CONFIGADDRESS(BUS, F, Equipment, ADDER);
                            i = io_in32(0xcfc);
                            i1 = i;
                            //*i1 = PCI_DATA1;
                            memcpy(PCI_DATA1, &i, 4);
                            PCI_DATA1 = PCI_DATA1 + 4;
                        }
                        //}
                        PCI_DATA+=12;
                        struct PCI_CONFIG_SPACE_PUCLIC *PCI_CONFIG_SPACE = (struct PCI_CONFIG_SPACE_PUCLIC *)PCI_DATA;
                        PCI_DATA-=12;
                        /*printk("PCI_CONFIG_SPACE:%08x\n", PCI_CONFIG_SPACE);
                        printk("PCI_CONFIG_SPACE->VendorID:%08x\n", PCI_CONFIG_SPACE->VendorID);
                        printk("PCI_CONFIG_SPACE->DeviceID:%08x\n", PCI_CONFIG_SPACE->DeviceID);
                        printk("PCI_CONFIG_SPACE->Command:%08x\n", PCI_CONFIG_SPACE->Command);
                        printk("PCI_CONFIG_SPACE->Status:%08x\n", PCI_CONFIG_SPACE->Status);
                        printk("PCI_CONFIG_SPACE->RevisionID:%08x\n", PCI_CONFIG_SPACE->RevisionID);
                        printk("PCI_CONFIG_SPACE->ProgIF:%08x\n", PCI_CONFIG_SPACE->ProgIF);
                        printk("PCI_CONFIG_SPACE->SubClass:%08x\n", PCI_CONFIG_SPACE->SubClass);
                        printk("PCI_CONFIG_SPACE->BaseCode:%08x\n", PCI_CONFIG_SPACE->BaseClass);
                        printk("PCI_CONFIG_SPACE->CacheLineSize:%08x\n", PCI_CONFIG_SPACE->CacheLineSize);
                        printk("PCI_CONFIG_SPACE->LatencyTimer:%08x\n", PCI_CONFIG_SPACE->LatencyTimer);
                        printk("PCI_CONFIG_SPACE->HeaderType:%08x\n", PCI_CONFIG_SPACE->HeaderType);
                        printk("PCI_CONFIG_SPACE->BIST:%08x\n", PCI_CONFIG_SPACE->BIST);
                        printk("PCI_CONFIG_SPACE->BaseAddr0:%08x\n", PCI_CONFIG_SPACE->BaseAddr[0]);
                        printk("PCI_CONFIG_SPACE->BaseAddr1:%08x\n", PCI_CONFIG_SPACE->BaseAddr[1]);
                        printk("PCI_CONFIG_SPACE->BaseAddr2:%08x\n", PCI_CONFIG_SPACE->BaseAddr[2]);
                        printk("PCI_CONFIG_SPACE->BaseAddr3:%08x\n", PCI_CONFIG_SPACE->BaseAddr[3]);
                        printk("PCI_CONFIG_SPACE->BaseAddr4:%08x\n", PCI_CONFIG_SPACE->BaseAddr[4]);
                        printk("PCI_CONFIG_SPACE->BaseAddr5:%08x\n", PCI_CONFIG_SPACE->BaseAddr[5]);
                        printk("PCI_CONFIG_SPACE->CardbusCISPtr:%08x\n", PCI_CONFIG_SPACE->CardbusCIS);
                        printk("PCI_CONFIG_SPACE->SubsystemVendorID:%08x\n", PCI_CONFIG_SPACE->SubVendorID);
                        printk("PCI_CONFIG_SPACE->SubsystemID:%08x\n", PCI_CONFIG_SPACE->SubSystemID);
                        printk("PCI_CONFIG_SPACE->ExpansionROMBaseAddr:%08x\n", PCI_CONFIG_SPACE->ROMBaseAddr);
                        printk("PCI_CONFIG_SPACE->CapabilitiesPtr:%08x\n", PCI_CONFIG_SPACE->CapabilitiesPtr);
                        printk("PCI_CONFIG_SPACE->Reserved1:%08x\n", PCI_CONFIG_SPACE->Reserved[0]);
                        printk("PCI_CONFIG_SPACE->Reserved2:%08x\n", PCI_CONFIG_SPACE->Reserved[1]);
                        printk("PCI_CONFIG_SPACE->InterruptLine:%08x\n", PCI_CONFIG_SPACE->InterruptLine);
                        printk("PCI_CONFIG_SPACE->InterruptPin:%08x\n", PCI_CONFIG_SPACE->InterruptPin);
                        printk("PCI_CONFIG_SPACE->MinGrant:%08x\n", PCI_CONFIG_SPACE->MinGrant);
                        printk("PCI_CONFIG_SPACE->MaxLatency:%08x\n", PCI_CONFIG_SPACE->MaxLatency);*/
                        PCI_DATA = PCI_DATA + 0x110;
                        key = 0;
                    }
                }
            }
        }
    }
    //函数执行完PCI_DATA就是PCI设备表的结束地址
    
}
//outl
static inline void outl(unsigned short port, unsigned int value)
{
    asm volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}
//inl
static inline int inl(unsigned short port)
{
    unsigned int ret;
    asm volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint16_t tmp = 0;
 
    // Create configuration address as per Figure 1
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
 
    // Write out the address
    outl(0xCF8, address);
    // Read in the data
    // (offset & 2) * 8) = 0 will choose the first word of the 32-bit register
    tmp = (uint16_t)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
    return tmp;
}
uint16_t pciCheckVendor(uint8_t bus, uint8_t slot) {
    uint16_t vendor, device;
    /* Try and read the first configuration register. Since there are no
     * vendors that == 0xFFFF, it must be a non-existent device. */
    if ((vendor = pciConfigReadWord(bus, slot, 0, 0)) != 0xFFFF) {
       device = pciConfigReadWord(bus, slot, 0, 2);
       //. . .
    } return (vendor);
}
void PCI_ClassCode_Print(struct PCI_CONFIG_SPACE_PUCLIC *pci_config_space_puclic) {
	unsigned char *pci_drive = (unsigned char *)pci_config_space_puclic-12;
	printf("BUS:%02x ", pci_drive[1]);
	printf("EQU:%02x ", pci_drive[2]);
	printf("F:%02x ", pci_drive[3]);
	if (pci_config_space_puclic->BaseClass == 0x0) {
		printf("Nodefined ");
		if (pci_config_space_puclic->SubClass == 0x0)
			printf("Non-VGA-Compatible Unclassified Device\n");
		else if (pci_config_space_puclic->SubClass == 0x1)
			printf("VGA-Compatible Unclassified Device\n");
	} else if (pci_config_space_puclic->BaseClass == 0x1) {
		printf("Mass Storage Controller ");
		if (pci_config_space_puclic->SubClass == 0x0)
			printf("SCSI Bus Controller\n");
		else if (pci_config_space_puclic->SubClass == 0x1)
			printf("IDE Controller\n");
		else if (pci_config_space_puclic->SubClass == 0x2)
			printf("Floppy Disk Controller\n");
		else if (pci_config_space_puclic->SubClass == 0x3)
			printf("IPI Bus Controller\n");
		else if (pci_config_space_puclic->SubClass == 0x4)
			printf("RAID Controller\n");
		else if (pci_config_space_puclic->SubClass == 0x5)
			printf("ATA Controller\n");
		else if (pci_config_space_puclic->SubClass == 0x6)
			printf("Serial ATA Controller\n");
		else if (pci_config_space_puclic->SubClass == 0x7)
			printf("Serial Attached SCSI Controller\n");
		else if (pci_config_space_puclic->SubClass == 0x8)
			printf("Non-Volatile Memory Controller\n");
		else
			printf("\n");
	} else if (pci_config_space_puclic->BaseClass == 0x2) {
		printf("Network Controller ");
		if (pci_config_space_puclic->SubClass == 0x0)
			printf("Ethernet Controller\n");
		else if (pci_config_space_puclic->SubClass == 0x1)
			printf("Token Ring Controller\n");
		else if (pci_config_space_puclic->SubClass == 0x2)
			printf("FDDI Controller\n");
		else if (pci_config_space_puclic->SubClass == 0x3)
			printf("ATM Controller\n");
		else if (pci_config_space_puclic->SubClass == 0x4)
			printf("ISDN Controller\n");
		else if (pci_config_space_puclic->SubClass == 0x5)
			printf("WorldFip Controller\n");
		else if (pci_config_space_puclic->SubClass == 0x6)
			printf("PICMG 2.14 Multi Computing Controller\n");
		else if (pci_config_space_puclic->SubClass == 0x7)
			printf("Infiniband Controller\n");
		else if (pci_config_space_puclic->SubClass == 0x8)
			printf("Fabric Controller\n");
		else
			printf("\n");
	} else if (pci_config_space_puclic->BaseClass == 0x3) {
		printf("Display Controller ");
		if (pci_config_space_puclic->SubClass == 0x0)
			printf("VGA Compatible Controller\n");
		else if (pci_config_space_puclic->SubClass == 0x1)
			printf("XGA Controller\n");
		else if (pci_config_space_puclic->SubClass == 0x2)
			printf("3D Controller (Not VGA-Compatible)\n");
		else
			printf("\n");
	} else if (pci_config_space_puclic->BaseClass == 0x4) {
		printf("Multimedia Controller ");
		printf("\n");
	} else if (pci_config_space_puclic->BaseClass == 0x5) {
		printf("Memory Controller ");
		printf("\n");
	} else if (pci_config_space_puclic->BaseClass == 0x6) {
		printf("Bridge ");
		if (pci_config_space_puclic->SubClass == 0x0)
			printf("Host Bridge\n");
		else if (pci_config_space_puclic->SubClass == 0x1)
			printf("ISA Bridge\n");
		else if (pci_config_space_puclic->SubClass == 0x2)
			printf("EISA Bridge\n");
		else if (pci_config_space_puclic->SubClass == 0x3)
			printf("MCA Bridge\n");
		else if (pci_config_space_puclic->SubClass == 0x4 || pci_config_space_puclic->SubClass == 0x9)
			printf("PCI-to-PCI Bridge\n");
		else if (pci_config_space_puclic->SubClass == 0x5)
			printf("PCMCIA Bridge\n");
		else if (pci_config_space_puclic->SubClass == 0x6)
			printf("NuBus Bridge\n");
		else if (pci_config_space_puclic->SubClass == 0x7)
			printf("CardBus Bridge\n");
		else if (pci_config_space_puclic->SubClass == 0x8)
			printf("RACEway Bridge\n");
		else if (pci_config_space_puclic->SubClass == 0xA)
			printf("InfiniBand-to-PCI Host Bridge\n");
		else
			printf("\n");
	} else if (pci_config_space_puclic->BaseClass == 0x7) {
		printf("Simple Communication Controller ");
		printf("\n");
	} else if (pci_config_space_puclic->BaseClass == 0x8) {
		printf("Base System Peripheral ");
		printf("\n");
	} else if (pci_config_space_puclic->BaseClass == 0x9) {
		printf("Input Device Controller ");
		printf("\n");
	} else if (pci_config_space_puclic->BaseClass == 0xA) {
		printf("Docking Station ");
		printf("\n");
	} else if (pci_config_space_puclic->BaseClass == 0xB) {
		printf("Processor ");
		printf("\n");
	} else {
		printf("Unknow\n");
	}
}