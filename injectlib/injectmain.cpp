#include <cmath>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include "symutils.hpp"
#include "common.hpp"
#include "movement.hpp"
#include "customhud.hpp"

struct edict_s;
struct entity_state_s;

typedef void (*ServerActivate_func_t)(edict_s *, int, int);
typedef int (*AddToFullPack_func_t)(entity_state_s *, int, edict_s *, edict_s *, int, int, unsigned char *);
typedef void (*PM_Move_func_t)(void *, int);
typedef void (*SCR_UpdateScreen_func_t)();
typedef void (*InitInput_func_t)();
typedef void (*SV_SendClientMessages_func_t)();
typedef uintptr_t (*SZ_GetSpace_func_t)(uintptr_t, int);

static uintptr_t hwso_addr = 0;
static uintptr_t hlso_addr = 0;
static uintptr_t clso_addr = 0;
static symtbl_t hwso_st;
static symtbl_t hlso_st;
static symtbl_t clso_st;
static cvar_t sv_show_triggers;

bool tas_hook_initialized = false;

Cvar_RegisterVariable_func_t orig_Cvar_RegisterVariable = nullptr;
Cvar_SetValue_func_t orig_Cvar_SetValue = nullptr;

double *p_host_frametime = nullptr;
uintptr_t *pp_sv_player = nullptr;
const char *gamedir = nullptr;
unsigned int *p_g_ulFrameCount = nullptr;
uintptr_t *pp_gpGlobals = nullptr;

static ServerActivate_func_t orig_ServerActivate = nullptr;
static AddToFullPack_func_t orig_AddToFullPack = nullptr;
static InitInput_func_t orig_InitInput = nullptr;
static SCR_UpdateScreen_func_t orig_SCR_UpdateScreen = nullptr;
static PM_Move_func_t orig_hl_PM_Move = nullptr;
static PM_Move_func_t orig_cl_PM_Move = nullptr;
static SV_SendClientMessages_func_t orig_SV_SendClientMessages = nullptr;
static SZ_GetSpace_func_t orig_SZ_GetSpace = nullptr;

static cvar_t *p_r_norefresh = nullptr;

static const int EF_NODRAW = 128;
static const int kRenderTransColor = 1;
static const int kRenderFxPulseFastWide = 4;

void abort_with_err(const char *errstr)
{
    std::fprintf(stderr, "TAS ERROR: %s\n", errstr);
    std::abort();
}

static inline const char *hlname_to_string(unsigned int name)
{
    return (const char *)(*(uintptr_t *)(*pp_gpGlobals + 0x98) + name);
}

static void load_cl_symbols()
{
    clso_addr = get_loaded_lib_addr("client.so");
    if (!clso_addr)
        abort_with_err("Failed to get the base address of client.so.");
    chdir(gamedir);
    clso_st = get_symbols("cl_dlls/client.so");
    chdir("..");

    orig_cl_PM_Move = (PM_Move_func_t)(clso_addr + clso_st["PM_Move"]);
    orig_InitInput = (InitInput_func_t)(clso_addr + clso_st["_Z9InitInputv"]);
}

static void load_hl_symbols()
{
    hlso_addr = get_loaded_lib_addr("hl.so");
    if (!hlso_addr)
        abort_with_err("Failed to get the base address of hl.so.");
    chdir(gamedir);
    hlso_st = get_symbols("dlls/hl.so");
    chdir("..");

    orig_hl_PM_Move = (PM_Move_func_t)(hlso_addr + hlso_st["PM_Move"]);
    orig_ServerActivate = (ServerActivate_func_t)(hlso_addr + hlso_st["_Z14ServerActivateP7edict_sii"]);
    orig_AddToFullPack = (AddToFullPack_func_t)(hlso_addr + hlso_st["_Z13AddToFullPackP14entity_state_siP7edict_sS2_iiPh"]);

    pp_gpGlobals = (uintptr_t *)(hlso_addr + hlso_st["gpGlobals"]);
    p_g_ulFrameCount = (unsigned int *)(hlso_addr + hlso_st["g_ulFrameCount"]);
}

static void load_hw_symbols()
{
    hwso_addr = get_loaded_lib_addr("hw.so");
    if (!hwso_addr)
        abort_with_err("Failed to get the base address of hw.so.");
    hwso_st = get_symbols("hw.so");

    orig_Cvar_RegisterVariable = (Cvar_RegisterVariable_func_t)(hwso_addr + hwso_st["Cvar_RegisterVariable"]);
    orig_Cvar_SetValue = (Cvar_SetValue_func_t)(hwso_addr + hwso_st["Cvar_SetValue"]);
    orig_SCR_UpdateScreen = (SCR_UpdateScreen_func_t)(hwso_addr + hwso_st["SCR_UpdateScreen"]);
    orig_SV_SendClientMessages = (SV_SendClientMessages_func_t)(hwso_addr + hwso_st["SV_SendClientMessages"]);
    orig_SZ_GetSpace = (SZ_GetSpace_func_t)(hwso_addr + hwso_st["SZ_GetSpace"]);

    gamedir = (const char *)(hwso_addr + hwso_st["com_gamedir"]);
    p_host_frametime = (double *)(hwso_addr + hwso_st["host_frametime"]);
    pp_sv_player = (uintptr_t *)(hwso_addr + hwso_st["sv_player"]);
    p_r_norefresh = (cvar_t *)(hwso_addr + hwso_st["r_norefresh"]);
}

// Note that this function is called before ServerActivate.
void InitInput()
{
    if (!tas_hook_initialized) {
        load_cl_symbols();
        initialize_movement(clso_addr, clso_st, hwso_addr, hwso_st);
    }
    orig_InitInput();
}

void ServerActivate(edict_s *pEdictList, int edictCount, int clientMax)
{
    if (!tas_hook_initialized) {
        // We only initialise the custom HUD here because it will fail to
        // initialise if we do it in InitInput instead.
        initialize_customhud(clso_addr, clso_st, hwso_addr, hwso_st);
        load_hl_symbols();
        tas_hook_initialized = true; // finally, everything is initialised
    }
    orig_ServerActivate(pEdictList, edictCount, clientMax);
}

static void get_trigger_amt_colors(const char *type, int *amt, char colors[3])
{
    if (strcmp(type, "once") == 0) {
        *amt = 120;
        colors[0] = 0;
        colors[1] = 255;
        colors[2] = 255;
    } else if (strcmp(type, "multiple") == 0) {
        *amt = 120;
        colors[0] = 0;
        colors[1] = 0;
        colors[2] = 255;
    } else if (strcmp(type, "changelevel") == 0) {
        *amt = 180;
        colors[0] = 255;
        colors[1] = 0;
        colors[2] = 255;
    } else if (strcmp(type, "hurt") == 0) {
        *amt = 140;
        colors[0] = 255;
        colors[1] = 0;
        colors[2] = 0;
    } else if (strcmp(type, "push") == 0) {
        *amt = 120;
        colors[0] = 255;
        colors[1] = 255;
        colors[2] = 0;
    } else if (strcmp(type, "teleport") == 0) {
        *amt = 150;
        colors[0] = 0;
        colors[1] = 255;
        colors[2] = 0;
    } else {
        *amt = 100;
        colors[0] = 255;
        colors[1] = 255;
        colors[2] = 255;
    }
}

int AddToFullPack(entity_state_s *state, int e, edict_s *ent, edict_s *host,
                  int hostflags, int player, unsigned char *pSet)
{
    uintptr_t entvarsaddr = (uintptr_t)ent + 0x80;
    const char *classname = hlname_to_string(*(unsigned int *)entvarsaddr);

    if (std::strncmp(classname, "trigger_", 8) != 0 || !sv_show_triggers.value)
        return orig_AddToFullPack(state, e, ent, host, hostflags, player, pSet);

    int *p_effects = (int *)(entvarsaddr + 0x118);
    int old_effects = *p_effects;
    *p_effects &= ~EF_NODRAW;   // Trick orig_AddToFullPack
    int ret = orig_AddToFullPack(state, e, ent, host, hostflags, player, pSet);
    *p_effects = old_effects;

    if (ret) {
        uintptr_t stateaddr = (uintptr_t)state;
        *(int *)(stateaddr + 0x3c) &= ~EF_NODRAW;
        *(int *)(stateaddr + 0x48) = kRenderTransColor;
        *(int *)(stateaddr + 0x54) = kRenderFxPulseFastWide;
        get_trigger_amt_colors(classname + 8, (int *)(stateaddr + 0x4c),
                               (char *)(stateaddr + 0x50));
    }

    return ret;
}

extern "C" void SV_SendClientMessages()
{
    if (p_r_norefresh->value <= 2)
        orig_SV_SendClientMessages();
}

extern "C" uintptr_t SZ_GetSpace(uintptr_t buf, int len)
{
    if (p_r_norefresh->value > 2 &&
        *(int *)(buf + 0x10) + len > *(int *)(buf + 0xc)) {
        *(int *)(buf + 0x10) = 0;
    }
    return orig_SZ_GetSpace(buf, len);
}

extern "C" void SCR_UpdateScreen()
{
    if (p_r_norefresh->value <= 1)
        orig_SCR_UpdateScreen();
}

extern "C" void Cvar_Init()
{
    // We don't load symbols from hl.so here because the game has not loaded
    // hl.so into memory.
    if (!tas_hook_initialized)
        load_hw_symbols();

    sv_show_triggers.name = "sv_show_triggers";
    sv_show_triggers.string = "0";
    orig_Cvar_RegisterVariable(&sv_show_triggers);
}

extern "C" void PM_Move(void *ppmove, int server)
{
    (server ? orig_hl_PM_Move : orig_cl_PM_Move)(ppmove, server);
    float *orig_vel = (float *)(*pp_sv_player + 0x80 + 0x20);
    if (server)
        fprintf(stderr, "%u %.8g %.8g\n", *p_g_ulFrameCount, *p_host_frametime, std::hypot(orig_vel[0], orig_vel[1]));
}
