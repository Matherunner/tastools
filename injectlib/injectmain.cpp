#include <cmath>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <cstdarg>
#ifdef OPPOSINGFORCE
#include <dlfcn.h>
#endif
#include "symutils.hpp"
#include "common.hpp"
#include "movement.hpp"
#include "customhud.hpp"

#ifdef OPPOSINGFORCE
#define HLSO_NAME "opfor.so"
#else
#define HLSO_NAME "hl.so"
#endif

struct edict_s;
struct entity_state_s;
struct KeyValueData_s;
struct entvars_s;

class CWorld
{
    void KeyValue(KeyValueData_s *keydat);
};

class CBasePlayer
{
    int TakeDamage(entvars_s *, entvars_s *, float, int);
};

class CGauss
{
    void StartFire();
};

#ifdef OPPOSINGFORCE
typedef void *(*dlsym_func_t)(void *, const char *);
#endif
typedef void (*GameDLLInit_func_t)();
typedef int (*AddToFullPack_func_t)(entity_state_s *, int, edict_s *, edict_s *, int, int, unsigned char *);
typedef void (*PM_Move_func_t)(uintptr_t, int);
typedef int (*PM_FlyMove_func_t)();
typedef void (*PM_WalkMove_func_t)();
typedef void (*SCR_UpdateScreen_func_t)();
typedef void (*InitInput_func_t)();
typedef void (*SV_SendClientMessages_func_t)();
typedef uintptr_t (*SZ_GetSpace_func_t)(uintptr_t, int);
typedef void (*PlayerPreThink_func_t)(edict_s *);
typedef void (*CWorld_KeyValue_func_t)(CWorld *, KeyValueData_s *);
typedef int (*CBasePlayer_TakeDamage_func_t)(CBasePlayer *, entvars_s *, entvars_s *, float, int);
typedef void (*Cmd_AddGameCommand_func_t)(const char *, void (*)());
typedef void (*CGauss_StartFire_func_t)(CGauss *);

static uintptr_t hwso_addr = 0;
static uintptr_t hlso_addr = 0;
static uintptr_t clso_addr = 0;
static symtbl_t hwso_st;
static symtbl_t hlso_st;
static symtbl_t clso_st;
static cvar_t sv_show_triggers;
static cvar_t sv_sim_qg;
static int in_walkmove = 0;
static int flymove_numtouches[2];
static float flymove_vel1[3];
static float flymove_pos1[3];

bool mvmt_clipped = false;
cvar_t sv_taslog;
bool tas_hook_initialized = false;

Cvar_RegisterVariable_func_t orig_Cvar_RegisterVariable = nullptr;
Cvar_SetValue_func_t orig_Cvar_SetValue = nullptr;
Con_Printf_func_t orig_Con_Printf = nullptr;

double *p_host_frametime = nullptr;
uintptr_t *pp_sv_player = nullptr;
const char *gamedir = nullptr;
unsigned int *p_g_ulFrameCount = nullptr;
uintptr_t *pp_gpGlobals = nullptr;

#ifdef OPPOSINGFORCE
static dlsym_func_t orig_dlsym = nullptr;
#endif
static GameDLLInit_func_t orig_GameDLLInit = nullptr;
static AddToFullPack_func_t orig_AddToFullPack = nullptr;
static InitInput_func_t orig_InitInput = nullptr;
static SCR_UpdateScreen_func_t orig_SCR_UpdateScreen = nullptr;
static PM_Move_func_t orig_hl_PM_Move = nullptr;
static PM_Move_func_t orig_cl_PM_Move = nullptr;
static PM_FlyMove_func_t orig_hl_PM_FlyMove = nullptr;
static PM_FlyMove_func_t orig_cl_PM_FlyMove = nullptr;
static PM_WalkMove_func_t orig_hl_PM_WalkMove = nullptr;
static PM_WalkMove_func_t orig_cl_PM_WalkMove = nullptr;
static SV_SendClientMessages_func_t orig_SV_SendClientMessages = nullptr;
static SZ_GetSpace_func_t orig_SZ_GetSpace = nullptr;
static PlayerPreThink_func_t orig_PlayerPreThink = nullptr;
static CWorld_KeyValue_func_t orig_CWorld_KeyValue = nullptr;
static CBasePlayer_TakeDamage_func_t orig_CBasePlayer_TakeDamage = nullptr;
static CGauss_StartFire_func_t orig_hl_CGauss_StartFire = nullptr;
static CGauss_StartFire_func_t orig_cl_CGauss_StartFire = nullptr;
static Cmd_AddGameCommand_func_t orig_Cmd_AddGameCommand = nullptr;
static Cmd_Argv_func_t orig_Cmd_Argv = nullptr;

static cvar_t *p_r_norefresh = nullptr;
static uintptr_t p_pmove = 0;
static int *p_g_onladder = nullptr;
static uintptr_t p_g_Gauss = 0;

static const int EF_NODRAW = 128;
static const int kRenderTransColor = 1;
static const int kRenderFxPulseFastWide = 4;

void abort_with_err(const char *errstr, ...)
{
    va_list args;
    va_start(args, errstr);
    std::fprintf(stderr, "TAS ERROR: ");
    std::vfprintf(stderr, errstr, args);
    va_end(args);
    std::abort();
}

static inline const char *hlname_to_string(unsigned int name)
{
    return (const char *)(*(uintptr_t *)(*pp_gpGlobals + 0x98) + name);
}

static void load_cl_symbols()
{
    std::string clso_fullpath;
    get_loaded_lib_info("client.so", clso_addr, clso_fullpath);
    if (!clso_addr)
        abort_with_err("Failed to get the base address of client.so.");
    clso_st = get_symbols(clso_fullpath.c_str());

    orig_cl_PM_Move = (PM_Move_func_t)(clso_addr + clso_st["PM_Move"]);
    orig_cl_PM_FlyMove = (PM_FlyMove_func_t)(clso_addr + clso_st["PM_FlyMove"]);
    orig_cl_PM_WalkMove = (PM_WalkMove_func_t)(clso_addr + clso_st["PM_WalkMove"]);
    orig_InitInput = (InitInput_func_t)(clso_addr + clso_st["_Z9InitInputv"]);
    orig_cl_CGauss_StartFire = (CGauss_StartFire_func_t)(clso_addr + clso_st["_ZN6CGauss9StartFireEv"]);
    p_g_Gauss = clso_addr + clso_st["g_Gauss"];
}

static void load_hl_symbols()
{
    std::string hlso_fullpath;
    get_loaded_lib_info(HLSO_NAME, hlso_addr, hlso_fullpath);
    if (!hlso_addr)
        abort_with_err("Failed to get the base address of %s.", HLSO_NAME);
    hlso_st = get_symbols(hlso_fullpath.c_str());

    orig_hl_PM_Move = (PM_Move_func_t)(hlso_addr + hlso_st["PM_Move"]);
    orig_hl_PM_FlyMove = (PM_FlyMove_func_t)(hlso_addr + hlso_st["PM_FlyMove"]);
    orig_hl_PM_WalkMove = (PM_WalkMove_func_t)(hlso_addr + hlso_st["PM_WalkMove"]);
    orig_GameDLLInit = (GameDLLInit_func_t)(hlso_addr + hlso_st["_Z11GameDLLInitv"]);
    orig_AddToFullPack = (AddToFullPack_func_t)(hlso_addr + hlso_st["_Z13AddToFullPackP14entity_state_siP7edict_sS2_iiPh"]);
    orig_PlayerPreThink = (PlayerPreThink_func_t)(hlso_addr + hlso_st["_Z14PlayerPreThinkP7edict_s"]);
    orig_CWorld_KeyValue = (CWorld_KeyValue_func_t)(hlso_addr + hlso_st["_ZN6CWorld8KeyValueEP14KeyValueData_s"]);
    orig_CBasePlayer_TakeDamage = (CBasePlayer_TakeDamage_func_t)(hlso_addr + hlso_st["_ZN11CBasePlayer10TakeDamageEP9entvars_sS1_fi"]);
    orig_hl_CGauss_StartFire = (CGauss_StartFire_func_t)(hlso_addr + hlso_st["_ZN6CGauss9StartFireEv"]);

    pp_gpGlobals = (uintptr_t *)(hlso_addr + hlso_st["gpGlobals"]);
    p_g_ulFrameCount = (unsigned int *)(hlso_addr + hlso_st["g_ulFrameCount"]);
    p_g_onladder = (int *)(hlso_addr + hlso_st["g_onladder"]);
}

static void load_hw_symbols()
{
    std::string hwso_fullpath;
    get_loaded_lib_info("hw.so", hwso_addr, hwso_fullpath);
    if (!hwso_addr)
        abort_with_err("Failed to get the base address of hw.so.");
    hwso_st = get_symbols(hwso_fullpath.c_str());

    orig_Cvar_RegisterVariable = (Cvar_RegisterVariable_func_t)(hwso_addr + hwso_st["Cvar_RegisterVariable"]);
    orig_Cvar_SetValue = (Cvar_SetValue_func_t)(hwso_addr + hwso_st["Cvar_SetValue"]);
    orig_Cmd_AddGameCommand = (Cmd_AddGameCommand_func_t)(hwso_addr + hwso_st["Cmd_AddGameCommand"]);
    orig_Cmd_Argv = (Cmd_Argv_func_t)(hwso_addr + hwso_st["Cmd_Argv"]);
    orig_SCR_UpdateScreen = (SCR_UpdateScreen_func_t)(hwso_addr + hwso_st["SCR_UpdateScreen"]);
    orig_SV_SendClientMessages = (SV_SendClientMessages_func_t)(hwso_addr + hwso_st["SV_SendClientMessages"]);
    orig_SZ_GetSpace = (SZ_GetSpace_func_t)(hwso_addr + hwso_st["SZ_GetSpace"]);
    orig_Con_Printf = (Con_Printf_func_t)(hwso_addr + hwso_st["Con_Printf"]);

    gamedir = (const char *)(hwso_addr + hwso_st["com_gamedir"]);
    p_host_frametime = (double *)(hwso_addr + hwso_st["host_frametime"]);
    pp_sv_player = (uintptr_t *)(hwso_addr + hwso_st["sv_player"]);
    p_r_norefresh = (cvar_t *)(hwso_addr + hwso_st["r_norefresh"]);
}

// Note that this function is called before GameDLLInit.
void InitInput()
{
    if (!tas_hook_initialized) {
        load_cl_symbols();
        initialize_movement(clso_addr, clso_st, hwso_addr, hwso_st);
    }
    orig_InitInput();
}

static void change_plr_hp()
{
    if (!pp_sv_player || !*pp_sv_player)
        return;
    *(float *)(*pp_sv_player + 0x80 + 0x160) = std::atof(orig_Cmd_Argv(1));
}

static void change_plr_ap()
{
    if (!pp_sv_player || !*pp_sv_player)
        return;
    *(float *)(*pp_sv_player + 0x80 + 0x1bc) = std::atof(orig_Cmd_Argv(1));
}

void GameDLLInit()
{
    if (!tas_hook_initialized) {
        // We only initialise the custom HUD here because it will fail to
        // initialise if we do it in InitInput instead.
        initialize_customhud(clso_addr, clso_st, hwso_addr, hwso_st);
        load_hl_symbols();
        orig_Cmd_AddGameCommand("ch_health", change_plr_hp);
        orig_Cmd_AddGameCommand("ch_armor", change_plr_ap);
        tas_hook_initialized = true; // finally, everything is initialised
    }
    orig_GameDLLInit();
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

void PlayerPreThink(edict_s *ent)
{
    if (sv_taslog.value) {
        orig_Con_Printf("prethink %u %.8g\n", *p_g_ulFrameCount,
                        *(float *)(*pp_gpGlobals + 0x4));
        orig_Con_Printf("health %.8g %.8g\n",
                        *(float *)((uintptr_t)ent + 0x80 + 0x160),
                        *(float *)((uintptr_t)ent + 0x80 + 0x1bc));
    }
    orig_PlayerPreThink(ent);
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

    sv_taslog.name = "sv_taslog";
    sv_taslog.string = "0";
    orig_Cvar_RegisterVariable(&sv_taslog);

    sv_sim_qg.name = "sv_sim_qg";
    sv_sim_qg.string = "0";
    orig_Cvar_RegisterVariable(&sv_sim_qg);
}

static void print_tasinfo(uintptr_t pmove, int server, int num)
{
    if (!server || !sv_taslog.value)
        return;

    if (num == 1) {
        uintptr_t cmd = pmove + 0x45458;
        orig_Con_Printf("usercmd %d %u %.8g %.8g\n",
                        *(char *)(cmd + 0x2), *(unsigned short *)(cmd + 0x1e),
                        *(float *)(cmd + 0x4), *(float *)(cmd + 0x8));
        orig_Con_Printf("fsu %.8g %.8g %.8g\n",
                        *(float *)(cmd + 0x10), *(float *)(cmd + 0x14),
                        *(float *)(cmd + 0x18));
        orig_Con_Printf("fg %.8g %.8g\n", *(float *)(pmove + 0xc4),
                        *(float *)(pmove + 0xc0));
        orig_Con_Printf("pa %.8g %.8g\n", *(float *)(pmove + 0xa0),
                        *(float *)(pmove + 0xa4));
    } else if (num == 2)
        orig_Con_Printf("ntl %d %d\n", mvmt_clipped, *p_g_onladder);

    float *pos = (float *)(pmove + 0x38);
    orig_Con_Printf("pos %d %.8g %.8g %.8g\n", num, pos[0], pos[1], pos[2]);

    float *vel = (float *)(pmove + 0x5c);
    float *basevel = (float *)(pmove + 0x74);
    orig_Con_Printf("pmove %d %.8g %.8g %.8g %.8g %.8g %.8g %d %u %d %d\n",
                    num, vel[0], vel[1], vel[2],
                    basevel[0], basevel[1], basevel[2],
                    *(int *)(pmove + 0x90), *(unsigned int *)(pmove + 0xb8),
                    *(int *)(pmove + 0xe0), *(int *)(pmove + 0xe4));
}

extern "C" void PM_Move(uintptr_t ppmove, int server)
{
    p_pmove = ppmove;
    mvmt_clipped = false;
    print_tasinfo(ppmove, server, 1);
    (server ? orig_hl_PM_Move : orig_cl_PM_Move)(ppmove, server);
    print_tasinfo(ppmove, server, 2);
}

extern "C" int PM_FlyMove()
{
    if (!*(int *)(p_pmove + 0x4))
        return orig_cl_PM_FlyMove();

    const int *p_numtouch = (const int *)(p_pmove + 0x4548c);
    int old_numtouch = *p_numtouch;
    int ret = orig_hl_PM_FlyMove();
    if (!in_walkmove) {
        mvmt_clipped = *p_numtouch - old_numtouch;
        return ret;
    }

    flymove_numtouches[in_walkmove - 1] = *p_numtouch - old_numtouch;
    if (in_walkmove == 1) {
        for (int i = 0; i < 3; i++) {
            flymove_vel1[i] = ((float *)(p_pmove + 0x5c))[i];
            flymove_pos1[i] =((float *)(p_pmove + 0x38))[i];
        }
    }

    in_walkmove++;
    return ret;
}

extern "C" void PM_WalkMove()
{
    if (!*(int *)(p_pmove + 0x4)) {
        orig_cl_PM_WalkMove();
        return;
    }

    in_walkmove = 1;
    orig_hl_PM_WalkMove();
    if (in_walkmove == 1) {     // if PM_FlyMove wasn't called
        mvmt_clipped = 0;
        in_walkmove = 0;
        return;
    }
    in_walkmove = 0;

    float *vel = (float *)(p_pmove + 0x5c);
    float *origin = (float *)(p_pmove + 0x38);

    if (vel[0] == flymove_vel1[0] && vel[1] == flymove_vel1[1] &&
        vel[2] == flymove_vel1[2] && origin[0] == flymove_pos1[0] &&
        origin[1] == flymove_pos1[1] && origin[2] == flymove_pos1[2])
        mvmt_clipped = flymove_numtouches[0];
    else
        mvmt_clipped = flymove_numtouches[1];
}

void CWorld::KeyValue(KeyValueData_s *keydat)
{
    char *keystr = *(char **)((uintptr_t)keydat + 4);
    if (strcmp(keystr, "startdark") == 0)
        return;
    orig_CWorld_KeyValue(this, keydat);
}

int CBasePlayer::TakeDamage(entvars_s *pevInflictor, entvars_s *pevAttacker,
                            float flDamage, int bitsDamageType)
{
    if (sv_taslog.value)
        orig_Con_Printf("dmg %.8g %d\n", flDamage, bitsDamageType);
    return orig_CBasePlayer_TakeDamage(this, pevInflictor, pevAttacker,
                                       flDamage, bitsDamageType);
}

static inline void call_orig_startfire(CGauss *cgauss)
{
    if ((uintptr_t)cgauss == p_g_Gauss)
        orig_cl_CGauss_StartFire(cgauss);
    else
        orig_hl_CGauss_StartFire(cgauss);
}

void CGauss::StartFire()
{
    if (!sv_sim_qg.value) {
        call_orig_startfire(this);
        return;
    }

    uintptr_t p_player = *(uintptr_t *)((uintptr_t)this + 0x80);
    float *p_start_charge = (float *)(p_player + 0x640);
    float old_start_charge = *p_start_charge;
    *p_start_charge = 0;
    call_orig_startfire(this);
    *p_start_charge = old_start_charge;
}

#ifdef OPPOSINGFORCE
extern "C" void CL_CreateMove(float, void *, int);

extern "C" void *dlsym(void *handle, const char *symbol)
{
    if (symbol && strcmp(symbol, "CL_CreateMove") == 0)
        return (void *)CL_CreateMove;
    return orig_dlsym(handle, symbol);
}

static __attribute__((constructor)) void Constructor()
{
    std::string libdl_fullpath;
    uintptr_t libdl_addr;
    symtbl_t libdl_symbols;
    get_loaded_lib_info("libdl.so.2", libdl_addr, libdl_fullpath);
    libdl_symbols = get_symbols(libdl_fullpath.c_str());
    for (auto it = libdl_symbols.begin(); it != libdl_symbols.end(); ++it) {
        if (std::strncmp(it->first.c_str(), "dlsym", 5) == 0) {
            orig_dlsym = (dlsym_func_t)(libdl_addr + it->second);
            break;
        }
    }
}
#endif
