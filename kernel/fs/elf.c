#include <ELF.h>
#include <dos.h>
#define MAX(a, b) a > b ? a : b
bool elf32Validate(Elf32_Ehdr* hdr) {
  return hdr->e_ident[EI_MAG0] == ELFMAG0 && hdr->e_ident[EI_MAG1] == ELFMAG1 &&
         hdr->e_ident[EI_MAG2] == ELFMAG2 && hdr->e_ident[EI_MAG3] == ELFMAG3;
}
uint32_t elf32_get_max_vaddr(Elf32_Ehdr* hdr) {
  Elf32_Phdr* phdr = (Elf32_Phdr*)((uint32_t)hdr + hdr->e_phoff);
  uint32_t max = 0;
  for (int i = 0; i < hdr->e_phnum; i++) {
    uint32_t size = MAX(
        phdr->p_filesz,
        phdr->p_memsz);  // 如果memsz大于filesz 说明这是bss段，我们以最大的为准
    max = MAX(max, phdr->p_vaddr + size);
    phdr++;
  }
  return max;
}
void load_segment(Elf32_Phdr* phdr, void* p, void* elf) {
  //printf("%08x %08x %d\n",phdr->p_vaddr,phdr->p_offset,phdr->p_filesz);
  memcpy(p + phdr->p_vaddr, elf + phdr->p_offset, phdr->p_filesz);
  if (phdr->p_memsz > phdr->p_filesz) {  // 这个是bss段
    memset(p + phdr->p_vaddr + phdr->p_filesz, 0,
           phdr->p_memsz - phdr->p_filesz);
  }
}
uint32_t load_elf(uint8_t* p, Elf32_Ehdr* hdr) {
  Elf32_Phdr* phdr = (Elf32_Phdr*)((uint32_t)hdr + hdr->e_phoff);
  for (int i = 0; i < hdr->e_phnum; i++) {
    load_segment(phdr,p,(void *)hdr);
    phdr++;
  }
  return hdr->e_entry;
}
void elf32LoadData(Elf32_Ehdr* elfhdr, uint8_t* ptr) {
  uint8_t* p = (uint8_t*)elfhdr;
  for (int i = 0; i < elfhdr->e_shnum; i++) {
    Elf32_Shdr* shdr =
        (Elf32_Shdr*)(p + elfhdr->e_shoff + sizeof(Elf32_Shdr) * i);

    if (shdr->sh_type != SHT_PROGBITS || !(shdr->sh_flags & SHF_ALLOC)) {
      continue;
    }

    for (int i = 0; i < shdr->sh_size; i++) {
      ptr[shdr->sh_addr + i] = p[shdr->sh_offset + i];
    }
  }
}
