#ifndef TASCOMMON_H
#define TASCOMMON_H

#include <cstdint>

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
typedef void (*GetSetViewAngles_func_t)(float *);
typedef void (*Con_Printf_func_t)(const char *, ...);

extern Cvar_SetValue_func_t orig_Cvar_SetValue;
extern Cvar_RegisterVariable_func_t orig_Cvar_RegisterVariable;
extern GetSetViewAngles_func_t orig_GetViewAngles;
extern Con_Printf_func_t orig_Con_Printf;

extern double *p_host_frametime;
extern uintptr_t *pp_sv_player;
extern const char *game_dir;
extern unsigned int *p_g_ulFrameCount;
extern uintptr_t *pp_gpGlobals;

void abort_with_err(const char *errstr, ...);

#endif
