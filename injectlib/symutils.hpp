#ifndef SYMUTILS_H
#define SYMUTILS_H

#include <unordered_map>
#include <string>
#include <elf.h>

typedef std::unordered_map<std::string, Elf32_Addr> symtbl_t;
uintptr_t get_loaded_lib_addr(const char *libname);
symtbl_t get_symbols(const char *libpath);

#endif
