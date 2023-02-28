#include <dosldr.h>
void Disk_Read(unsigned int lba, unsigned int number, void *buffer,
               char drive) {
  for (int i = 0; i != number; i++) {
    if (drive == 'A') {
      
      fdc_rw(lba + i, (unsigned char *)(buffer + i * 512), 1, 1);
    } else if (drive == 'C') {
      drivers_idehdd_read(lba + i, 1, (unsigned short *)(buffer + i * 512));
    }
  }
}
void Disk_Write(unsigned int lba, unsigned int number, void *buffer,
                char drive) {
  for (int i = 0; i != number; i++) {
    if (drive == 'A') {
      fdc_rw(lba + i, (unsigned char *)(buffer + i * 512), 0, 1);
    } else if (drive == 'C') {
      drivers_idehdd_write(lba + i, 1, (unsigned short *)(buffer + i * 512));
    }
  }
}