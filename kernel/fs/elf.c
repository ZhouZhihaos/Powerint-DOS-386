#include <ELF.h>
#include <dos.h>
bool elf32Validate(Elf32_Ehdr* hdr) {
  return hdr->e_ident[EI_MAG0] == ELFMAG0 && hdr->e_ident[EI_MAG1] == ELFMAG1 &&
         hdr->e_ident[EI_MAG2] == ELFMAG2 && hdr->e_ident[EI_MAG3] == ELFMAG3;
}
void elf32LoadData(Elf32_Ehdr* elfhdr, uint8_t* ptr) {
  uint8_t *p = (uint8_t*)elfhdr;
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
