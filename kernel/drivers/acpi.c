#include <dos.h>
struct ACPI_RSDP* RSDP;
struct ACPI_RSDT* RSDT;
struct ACPI_FADT* FADT;

char checksum(unsigned char* addr, unsigned int length) {
  int i = 0;
  unsigned char sum = 0;

  for (i = 0; i < length; i++) {
    sum += ((unsigned char*)addr)[i];
  }

  return sum == 0;
}

unsigned int* acpi_find_rsdp(void) {
  unsigned int* addr;

  for (addr = (unsigned int*)0x000e0000; addr < (unsigned int*)0x00100000;
       addr++) {
    if (memcmp(addr, "RSD PTR ", 8) == 0) {
      if (checksum((unsigned char*)addr, ((struct ACPI_RSDP*)addr)->Length)) {
        return addr;
      }
    }
  }
  return 0;
}

unsigned int acpi_find_table(char* Signature) {
  uint8_t *ptr, *ptr2;
  uint32_t len;
  uint8_t* rsdt = (uint8_t*)RSDT;
  // iterate on ACPI table pointers
  for (len = *((uint32_t*)(rsdt + 4)), ptr2 = rsdt + 36; ptr2 < rsdt + len;
       ptr2 += rsdt[0] == 'X' ? 8 : 4) {
    ptr = (uint8_t*)(uintptr_t)(rsdt[0] == 'X' ? *((uint64_t*)ptr2)
                                               : *((uint32_t*)ptr2));
    if (!memcmp(ptr, Signature, 4)) {
      return (unsigned)ptr;
    }
  }
  // printk("not found.\n");
  return 0;
}

void init_acpi(void) {
  RSDP = (struct ACPI_RSDP*)acpi_find_rsdp();
  if (RSDP == 0)
    return;
  RSDT = (struct ACPI_RSDT*)RSDP->RsdtAddress;
  // checksum(RSDT, RSDT->header.Length);
  if (!checksum((unsigned char*)RSDT, RSDT->header.Length))
    return;

  FADT = (struct ACPI_FADT*)acpi_find_table("FACP");
  if (!checksum((unsigned char*)FADT, FADT->h.Length))
    return;

  if (!(io_in16(FADT->PM1aControlBlock) & 1)) {
    if (FADT->SMI_CommandPort && FADT->AcpiEnable) {
      io_out8(FADT->SMI_CommandPort, FADT->AcpiEnable);
      int i, j;
      for (i = 0; i < 300; i++) {
        if (io_in16(FADT->PM1aControlBlock) & 1)
          break;
        for (j = 0; j < 1000000; j++)
          ;
      }
      if (FADT->PM1bControlBlock) {
        for (; i < 300; i++) {
          if (io_in16(FADT->PM1bControlBlock) & 1)
            break;
          for (j = 0; j < 1000000; j++)
            ;
        }
      }
    }
  }
  lapic_find();
  hpet_initialize();
  return;
}

/*
 * \_S5 Object
 * -----------------------------------
 * NameOP | \(可选) | _  | S  | 5  | _
 * 08     | 5A     | 5F | 53 | 35 | 5F
 * -----------------------------------
 * PackageOP | PkgLength | NumElements | prefix Num | prefix Num | prefix Num |
 * prefix Num 12        | 0A        | 04          | 0A     05  | 0A     05  | 0A
 * 05  | 0A     05
 * -----------------------------------
 * PkgLength: bit6~7为长度的字节数-1;bit4~5保留;bit0~3为长度的低4位
 * prefix:	0A Byte
 * 			0B Word
 * 			0C DWord
 * 			0D String
 * 			0E Qword
 */

int acpi_shutdown(void) {
  int i;
  unsigned short SLP_TYPa, SLP_TYPb;
  struct ACPISDTHeader* header = (struct ACPISDTHeader*)acpi_find_table("DSDT");
  char* S5Addr = (char*)header;
  int dsdtLength = (header->Length - sizeof(struct ACPISDTHeader)) / 4;

  for (i = 0; i < dsdtLength; i++) {
    if (memcmp(S5Addr, "_S5_", 4) == 0)
      break;
    S5Addr++;
  }
  if (i < dsdtLength) {
    if ((*(S5Addr - 1) == 0x08 ||
         (*(S5Addr - 2) == 0x08 && *(S5Addr - 1) == '\\')) &&
        *(S5Addr + 4) == 0x12) {
      S5Addr += 5;
      S5Addr += ((*S5Addr & 0xc0) >> 6) + 2;

      if (*S5Addr == 0x0a)
        S5Addr++;
      SLP_TYPa = *(S5Addr) << 10;
      S5Addr++;

      if (*S5Addr == 0x0a)
        S5Addr++;
      SLP_TYPb = *(S5Addr) << 10;
      S5Addr++;
    }
    // 关于PM1x_CNT_BLK的描述见 ACPI Specification Ver6.3 4.8.3.2.1
    io_out16(FADT->PM1aControlBlock, SLP_TYPa | 1 << 13);
    if (FADT->PM1bControlBlock != 0) {
      io_out16(FADT->PM1bControlBlock, SLP_TYPb | 1 << 13);
    }
  }
  return 1;
}
// copy from EXOS(https://gitee.com/yywd123/EXOS)
typedef struct {
  char sign[4];
  uint32_t len;
  char revision;
  char checksum;
  char oemid[6];
  uint64_t oem_table_id;
  uint32_t oem_revision;
  uint32_t creator_id;
  uint32_t creator_revision;
} __attribute__((packed)) MADT;
typedef struct {
  uint64_t configurationAndCapability;
  uint64_t comparatorValue;
  uint64_t fsbInterruptRoute;
  uint64_t unused;
} __attribute__((packed)) HpetTimer;

typedef struct {
  uint64_t generalCapabilities;
  uint64_t reserved0;
  uint64_t generalConfiguration;
  uint64_t reserved1;
  uint64_t generalIntrruptStatus;
  uint8_t reserved3[0xc8];
  uint64_t mainCounterValue;
  uint64_t reserved4;
  HpetTimer timers[0];
} __attribute__((packed)) HpetInfo;
typedef struct {
  uint8_t addressSpaceID;
  uint8_t registerBitWidth;
  uint8_t registerBitOffset;
  uint8_t accessWidth;  //  acpi 3.0
  uintptr_t address;
} __attribute__((packed)) AcpiAddress;
typedef struct {
  uint32_t signature;
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  uint8_t oem[6];
  uint8_t oemTableID[8];
  uint32_t oemVersion;
  uint32_t creatorID;
  uint32_t creatorVersion;
  uint8_t hardwareRevision;
  uint8_t comparatorCount : 5;
  uint8_t counterSize : 1;
  uint8_t reserved : 1;
  uint8_t legacyReplacement : 1;
  uint16_t pciVendorId;
  AcpiAddress hpetAddress;
  uint8_t hpetNumber;
  uint16_t minimumTick;
  uint8_t pageProtection;
} __attribute__((packed)) HPET;
static HpetInfo* hpetInfo = NULL;
static uint64_t hpetPeriod = 0;

void hpet_initialize() {
  HPET* hpet = acpi_find_table("HPET");
  if (!hpet) {
    printk("can not found acpi hpet table");
  }
  hpetInfo = (HpetInfo*)hpet->hpetAddress.address;
  printk("hpetInfo %016x\n", hpetInfo);

  uint32_t counterClockPeriod = hpetInfo->generalCapabilities >> 32;
  hpetPeriod = counterClockPeriod / 1000000;
  printk("hpet period: 0x%016x\n", hpetPeriod);

  hpetInfo->generalConfiguration |= 1;  //  启用hpet

  // Logger::log(Logger::INFO, "hpet successfully enabled");
}
uint64_t nanoTime() {
  return hpetInfo->mainCounterValue * hpetPeriod;
}
void usleep(uint64_t nano) {
  uint64_t targetTime = nanoTime();
  uint64_t after = 0;
  while (1) {
    uint64_t n = nanoTime();
    if (n < targetTime) {
      after += 0xffffffff - targetTime + n;
      targetTime = n;
    } else {
      after += n - targetTime;
      targetTime = n;
    }
    if (after >= nano) {
      return;
    }
  }
}
void lapic_find() {
  MADT* p = acpi_find_table("APIC");
  printk("%08x\n", p);
  printk("MADT : len = %08x\n", p->len);
  printk("oemid : ");
  for (int i = 0; i < 6; i++) {
    printk("%c", p->oemid[i]);
  }
  
}