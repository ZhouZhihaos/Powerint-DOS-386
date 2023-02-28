#include <drivers.h>
#define PCI_COMMAND_PORT 0xCF8
#define PCI_DATA_PORT 0xCFC
#define MemoryMapping 0
#define InputOutput 1
typedef struct BaseAddressRegister {
  int prefetchable;
  uint8_t *address;
  uint32_t size;
  int type;
} BaseAddressRegister;
uint32_t read_pci(uint8_t bus, uint8_t device, uint8_t function,
              uint8_t registeroffset) {
  uint32_t id = 1 << 31 | ((bus & 0xff) << 16) | ((device & 0x1f) << 11) |
                ((function & 0x07) << 8) | (registeroffset & 0xfc);
  io_out32(PCI_COMMAND_PORT, id);
  uint32_t result = io_in32(PCI_DATA_PORT);
  return result >> (8 * (registeroffset % 4));
}
void write_pci(uint8_t bus, uint8_t device, uint8_t function,
           uint8_t registeroffset, uint32_t value) {
  uint32_t id = 1 << 31 | ((bus & 0xff) << 16) | ((device & 0x1f) << 11) |
                ((function & 0x07) << 8) | (registeroffset & 0xfc);
  io_out32(PCI_COMMAND_PORT, id);
  io_out32(PCI_DATA_PORT, value);
}
uint32_t PCI_READ_COMMAND_STATUS(uint8_t bus, uint8_t slot, uint8_t func) {
  return read_pci(bus, slot, func, 0x04);
}
// write command status register
void PCI_WRITE_COMMAND_STATUS(uint8_t bus, uint8_t slot, uint8_t func,
                              uint32_t value) {
  write_pci(bus, slot, func, 0x04, value);
}
BaseAddressRegister GetBaseAddressRegister(uint8_t bus, uint8_t device,
                                           uint8_t function, uint8_t bar) {
  BaseAddressRegister result;

  uint32_t headertype = read_pci(bus, device, function, 0x0e) & 0x7e;
  int maxBARs = 6 - 4 * headertype;
  if (bar >= maxBARs)
    return result;

  uint32_t bar_value = read_pci(bus, device, function, 0x10 + 4 * bar);
  result.type = (bar_value & 1) ? InputOutput : MemoryMapping;

  if (result.type == MemoryMapping) {
    switch ((bar_value >> 1) & 0x3) {
    case 0: // 32
    case 1: // 20
    case 2: // 64
      break;
    }
  } else {
    result.address = (uint8_t *)(bar_value & ~0x3);
    result.prefetchable = 0;
  }
  return result;
}
uint8_t PCI_Get_Drive_IRQ_LINE(uint8_t bus, uint8_t slot, uint8_t func) {
  return (uint8_t)read_pci(bus, slot, func, 0x3c);
}
uint32_t PCI_Get_PORT_Base(uint8_t bus, uint8_t slot, uint8_t func) {
  extern int PCI_ADDR_BASE;
  unsigned char *pci_drive = PCI_ADDR_BASE;
  for (;; pci_drive += 0x110 + 4) {
    if (pci_drive[0] == 0xff) {
      if (pci_drive[1] == bus && pci_drive[2] == slot && pci_drive[3] == func) {
        uint32_t *PORT_POINT = pci_drive + 272;
        return *PORT_POINT;
      }
    } else {
      break;
    }
  }
  return 0;
}
void PCI_GET_DEVICE(uint16_t vendor_id, uint16_t device_id, uint8_t *bus,
                    uint8_t *slot, uint8_t *func) {
  extern int PCI_ADDR_BASE;
  unsigned char *pci_drive = PCI_ADDR_BASE;
  for (;; pci_drive += 0x110 + 4) {
    if (pci_drive[0] == 0xff) {
      struct PCI_CONFIG_SPACE_PUCLIC *pci_config_space_puclic;
      pci_config_space_puclic =
          (struct PCI_CONFIG_SPACE_PUCLIC *)(pci_drive + 0x0c);
      if (pci_config_space_puclic->VendorID == vendor_id &&
          pci_config_space_puclic->DeviceID == device_id) {
        *bus = pci_drive[1];
        *slot = pci_drive[2];
        *func = pci_drive[3];
        return;
      }
    } else {
      break;
    }
  }
}
void PCI_CONFIGADDRESS(unsigned int Bus, unsigned int f, unsigned int equipment,
                       unsigned int adder) {
  unsigned int cmd = 0;
  cmd = 0x80000000 + (unsigned int)adder + ((unsigned int)f << 8) +
        ((unsigned int)equipment << 11) + ((unsigned int)Bus << 16);
  // cmd = cmd | 0x01;
  io_out32(PCI_COMMAND_PORT, cmd);
}
void init_PCI(unsigned int adder_Base) {
  unsigned int i, BUS, Equipment, F, ADDER, *i1;
  unsigned char *PCI_DATA = adder_Base, *PCI_DATA1;
  for (BUS = 0; BUS < 256; BUS++) {                    //查询总线
    for (Equipment = 0; Equipment < 32; Equipment++) { //查询设备
      for (F = 0; F < 8; F++) {                        //查询功能
        PCI_CONFIGADDRESS(BUS, F, Equipment, 0);
        if (io_in32(PCI_DATA_PORT) != 0xFFFFFFFF) {
          //当前插槽有设备
          //把当前设备信息映射到PCI数据区
          int key = 1;
          while (key) {
            //此配置表为空
            // printk("PCI_DATA:%x\n", PCI_DATA);
            // getch();
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
            for (ADDER = 0; ADDER < 256; ADDER = ADDER + 4) {
              PCI_CONFIGADDRESS(BUS, F, Equipment, ADDER);
              i = io_in32(PCI_DATA_PORT);
              i1 = i;
              //*i1 = PCI_DATA1;
              memcpy(PCI_DATA1, &i, 4);
              PCI_DATA1 = PCI_DATA1 + 4;
            }
            for (uint8_t barNum = 0; barNum < 6; barNum++) {
              BaseAddressRegister bar =
                  GetBaseAddressRegister(BUS, Equipment, F, barNum);
              if (bar.address && (bar.type == InputOutput)) {
                PCI_DATA1 += 4;
                int i = ((uint32_t)(bar.address));
                memcpy(PCI_DATA1, &i, 4);
              }
            }
            /*PCI_DATA += 12;
            struct PCI_CONFIG_SPACE_PUCLIC *PCI_CONFIG_SPACE = (struct
            PCI_CONFIG_SPACE_PUCLIC *)PCI_DATA; PCI_DATA -= 12;
            printk("PCI_CONFIG_SPACE:%08x\n", PCI_CONFIG_SPACE);
            printk("PCI_CONFIG_SPACE->VendorID:%08x\n",
            PCI_CONFIG_SPACE->VendorID);
            printk("PCI_CONFIG_SPACE->DeviceID:%08x\n",
            PCI_CONFIG_SPACE->DeviceID);
            printk("PCI_CONFIG_SPACE->Command:%08x\n",
            PCI_CONFIG_SPACE->Command);
            printk("PCI_CONFIG_SPACE->Status:%08x\n", PCI_CONFIG_SPACE->Status);
            printk("PCI_CONFIG_SPACE->RevisionID:%08x\n",
            PCI_CONFIG_SPACE->RevisionID);
            printk("PCI_CONFIG_SPACE->ProgIF:%08x\n", PCI_CONFIG_SPACE->ProgIF);
            printk("PCI_CONFIG_SPACE->SubClass:%08x\n",
            PCI_CONFIG_SPACE->SubClass);
            printk("PCI_CONFIG_SPACE->BaseCode:%08x\n",
            PCI_CONFIG_SPACE->BaseClass);
            printk("PCI_CONFIG_SPACE->CacheLineSize:%08x\n",
            PCI_CONFIG_SPACE->CacheLineSize);
            printk("PCI_CONFIG_SPACE->LatencyTimer:%08x\n",
            PCI_CONFIG_SPACE->LatencyTimer);
            printk("PCI_CONFIG_SPACE->HeaderType:%08x\n",
            PCI_CONFIG_SPACE->HeaderType);
            printk("PCI_CONFIG_SPACE->BIST:%08x\n", PCI_CONFIG_SPACE->BIST);
            printk("PCI_CONFIG_SPACE->BaseAddr0:%08x\n",
            PCI_CONFIG_SPACE->BaseAddr[0]);
            printk("PCI_CONFIG_SPACE->BaseAddr1:%08x\n",
            PCI_CONFIG_SPACE->BaseAddr[1]);
            printk("PCI_CONFIG_SPACE->BaseAddr2:%08x\n",
            PCI_CONFIG_SPACE->BaseAddr[2]);
            printk("PCI_CONFIG_SPACE->BaseAddr3:%08x\n",
            PCI_CONFIG_SPACE->BaseAddr[3]);
            printk("PCI_CONFIG_SPACE->BaseAddr4:%08x\n",
            PCI_CONFIG_SPACE->BaseAddr[4]);
            printk("PCI_CONFIG_SPACE->BaseAddr5:%08x\n",
            PCI_CONFIG_SPACE->BaseAddr[5]);
            printk("PCI_CONFIG_SPACE->CardbusCISPtr:%08x\n",
            PCI_CONFIG_SPACE->CardbusCIS);
            printk("PCI_CONFIG_SPACE->SubsystemVendorID:%08x\n",
            PCI_CONFIG_SPACE->SubVendorID);
            printk("PCI_CONFIG_SPACE->SubsystemID:%08x\n",
            PCI_CONFIG_SPACE->SubSystemID);
            printk("PCI_CONFIG_SPACE->ExpansionROMBaseAddr:%08x\n",
            PCI_CONFIG_SPACE->ROMBaseAddr);
            printk("PCI_CONFIG_SPACE->CapabilitiesPtr:%08x\n",
            PCI_CONFIG_SPACE->CapabilitiesPtr);
            printk("PCI_CONFIG_SPACE->Reserved1:%08x\n",
            PCI_CONFIG_SPACE->Reserved[0]);
            printk("PCI_CONFIG_SPACE->Reserved2:%08x\n",
            PCI_CONFIG_SPACE->Reserved[1]);
            printk("PCI_CONFIG_SPACE->InterruptLine:%08x\n",
            PCI_CONFIG_SPACE->InterruptLine);
            printk("PCI_CONFIG_SPACE->InterruptPin:%08x\n",
            PCI_CONFIG_SPACE->InterruptPin);
            printk("PCI_CONFIG_SPACE->MinGrant:%08x\n",
            PCI_CONFIG_SPACE->MinGrant);
            printk("PCI_CONFIG_SPACE->MaxLatency:%08x\n",
            PCI_CONFIG_SPACE->MaxLatency); for (int i = 0; i < 272+4; i++)
            {
                printk("%02x ", PCI_DATA[i]);
            }
            printk("\n");*/
            PCI_DATA = PCI_DATA + 0x110 + 4;
            key = 0;
          }
        }
      }
    }
  }
  //函数执行完PCI_DATA就是PCI设备表的结束地址
}
void PCI_ClassCode_Print(
    struct PCI_CONFIG_SPACE_PUCLIC *pci_config_space_puclic) {
  unsigned char *pci_drive = (unsigned char *)pci_config_space_puclic - 12;
  printf("BUS:%02x ", pci_drive[1]);
  printf("EQU:%02x ", pci_drive[2]);
  printf("F:%02x ", pci_drive[3]);
  printf("IO Port:%08x ",
         PCI_Get_PORT_Base(pci_drive[1], pci_drive[2], pci_drive[3]));
  printf("IRQ Line:%02x ",
         PCI_Get_Drive_IRQ_LINE(pci_drive[1], pci_drive[2], pci_drive[3]));
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
    else if (pci_config_space_puclic->SubClass == 0x4 ||
             pci_config_space_puclic->SubClass == 0x9)
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