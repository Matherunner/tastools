#ifndef SYMUTILS_H
#define SYMUTILS_H

#include <unordered_map>
#include <string>
#include <elf.h>

typedef std::unordered_map<std::string, Elf32_Addr> symtbl_t;
void get_loaded_lib_info(const char *libname, uintptr_t &addr,
                         std::string &fullpath);
symtbl_t get_symbols(const char *libpath);

#endif
