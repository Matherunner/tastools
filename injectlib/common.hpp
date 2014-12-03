#ifndef TASCOMMON_H
#define TASCOMMON_H

#include <cstdint>
#include "customhud.hpp"

struct edict_s;
struct entity_state_s;
typedef struct cvar_s
{
    const char *name;
    const char *string;
    int flags;
    float value;
    struct cvar_s *next;
} cvar_t;

const int FL_DUCKING = 1 << 14;

typedef void (*Cvar_RegisterVariable_func_t)(cvar_t *);
typedef void (*Cvar_SetValue_func_t)(const char *, float);

extern bool tas_hook_initialized;

extern Cvar_SetValue_func_t orig_Cvar_SetValue;
extern Cvar_RegisterVariable_func_t orig_Cvar_RegisterVariable;

extern double *p_host_frametime;
extern uintptr_t *pp_sv_player;
extern const char *game_dir;
extern unsigned int *p_g_ulFrameCount;

void abort_with_err(const char *errstr);

#endif
