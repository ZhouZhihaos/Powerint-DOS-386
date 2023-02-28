#include <drivers.h>

struct ACPI_RSDP *RSDP;
struct ACPI_RSDT *RSDT;
struct ACPI_FADT *FADT;

char checksum(unsigned char *addr, unsigned int length) {
  int i = 0;
  unsigned char sum = 0;

  for (i = 0; i < length; i++) {
    sum += ((unsigned char *)addr)[i];
  }

  return sum == 0;
}

unsigned int *acpi_find_rsdp(void) {
  unsigned int *addr;

  for (addr = (unsigned int *)0x000e0000; addr < (unsigned int *)0x00100000;
       addr++) {
    if (memcmp(addr, "RSD PTR ", 8) == 0) {
      if (checksum((unsigned char *)addr, ((struct ACPI_RSDP *)addr)->Length)) {
        return addr;
      }
    }
  }
  return 0;
}

unsigned int acpi_find_table(char *Signature) {
  int i, length = (RSDT->header.Length - sizeof(RSDT->header)) / 4;
  struct ACPISDTHeader *header;
  for (i = 0; i < length; i++) {
    header = (struct ACPISDTHeader *)((unsigned int)RSDT->Entry + i * 4),
    sizeof(struct ACPISDTHeader);
    if (memcmp(header->Signature, Signature, 4) == 0) {
      return (unsigned int)header;
    }
  }
  return 0;
}

void init_acpi(void) {
  RSDP = (struct ACPI_RSDP *)acpi_find_rsdp();
  if (RSDP == 0)
    return;
  RSDT = (struct ACPI_RSDT *)RSDP->RsdtAddress;
  // checksum(RSDT, RSDT->header.Length);
  if (!checksum((unsigned char *)RSDT, RSDT->header.Length))
    return;

  FADT = (struct ACPI_FADT *)acpi_find_table("FACP");
  if (!checksum((unsigned char *)FADT, FADT->h.Length))
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
  struct ACPISDTHeader *header =
      (struct ACPISDTHeader *)acpi_find_table("DSDT");
  char *S5Addr = (char *)header;
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
