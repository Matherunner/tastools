#ifndef MOVEMENT_H
#define MOVEMENT_H

#include "symutils.hpp"

void initialize_movement(uintptr_t clso_addr, const symtbl_t &clso_st,
                         uintptr_t hwso_addr, const symtbl_t &hwso_st);

#endif
