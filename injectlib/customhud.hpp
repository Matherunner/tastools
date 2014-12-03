#ifndef CUSTOMHUD_H
#define CUSTOMHUD_H

#include "symutils.hpp"

void initialize_customhud(uintptr_t clso_addr, const symtbl_t &clso_st,
                          uintptr_t hwso_addr, const symtbl_t &hwso_st);

#endif
