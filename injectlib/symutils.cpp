#include <cstdio>
#include <cstring>
#include <link.h>
#include "symutils.hpp"

struct libbasearg_t
{
    const char *libname;
    uintptr_t baseaddr;
    std::string fullpath;
};

static int phdr_callback(dl_phdr_info *info, size_t, void *data)
{
    libbasearg_t *arg = (libbasearg_t *)data;
    if (std::strcmp(basename(info->dlpi_name), arg->libname) == 0) {
        arg->baseaddr = info->dlpi_addr;
        arg->fullpath = info->dlpi_name;
    }
    return 0;
}

void get_loaded_lib_info(const char *libname, uintptr_t &addr,
                         std::string &fullpath)
{
    libbasearg_t arg = {libname, 0, ""};
    dl_iterate_phdr(phdr_callback, &arg);
    addr = arg.baseaddr;
    fullpath = arg.fullpath;
}

static long get_file_size(std::FILE *file)
{
    long orig_pos = std::ftell(file);
    std::fseek(file, 0, SEEK_END);
    long filesize = std::ftell(file);
    std::fseek(file, orig_pos, SEEK_SET);
    return filesize;
}

symtbl_t get_symbols(const char *libpath)
{
    symtbl_t sym_straddr_tbl;
    std::FILE *libfile = std::fopen(libpath, "r");
    if (!libfile)
        return sym_straddr_tbl;
    long filesize = get_file_size(libfile);
    char *filedat = new char[filesize];
    std::fread(filedat, 1, filesize, libfile);
    std::fclose(libfile);

    Elf32_Ehdr *elf_hdr = (Elf32_Ehdr *)filedat;
    Elf32_Shdr *sh_hdr = (Elf32_Shdr *)(filedat + elf_hdr->e_shoff);
    Elf32_Shdr *sh_shstrhdr = sh_hdr + elf_hdr->e_shstrndx;
    char *sh_shstrtab = filedat + sh_shstrhdr->sh_offset;

    int i;
    for (i = 0; sh_hdr[i].sh_type != SHT_DYNSYM; i++);
    Elf32_Sym *symtab = (Elf32_Sym *)(filedat + sh_hdr[i].sh_offset);
    uint64_t st_num_entries = sh_hdr[i].sh_size / sizeof(Elf32_Sym);

    for (i = 0; sh_hdr[i].sh_type != SHT_STRTAB ||
             strcmp(sh_shstrtab + sh_hdr[i].sh_name, ".dynstr") != 0; i++);
    char *sh_strtab = filedat + sh_hdr[i].sh_offset;

    for (uint64_t i = 0; i < st_num_entries; i++)
        sym_straddr_tbl[std::string(sh_strtab + symtab[i].st_name)] =
            symtab[i].st_value;

    delete[] filedat;
    return sym_straddr_tbl;
}
