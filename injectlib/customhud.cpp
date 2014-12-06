#include <cstring>
#include <cmath>
#include "customhud.hpp"
#include "common.hpp"

struct TraceResult
{
    int fAllSolid;
    int fStartSolid;
    int fInOpen;
    int fInWater;
    float flFraction;
    float vecEndPos[3];
    float flPlaneDist;
    float vecPlaneNormal[3];
    uintptr_t pHit;
    int iHitgroup;
};

typedef struct {
    int x, y;
} POSITION;

class CHudBase
{
public:
    POSITION m_pos;
    int m_type;
    int m_iFlags;
    virtual ~CHudBase() {}
    virtual int Init(void) { return 0; }
    virtual int VidInit(void) { return 0; }
    virtual int Draw(float) { return 0; }
    virtual void Think(void) { return; }
    virtual void Reset(void) { return; }
    virtual void InitHUDData(void) {}
};

class CHudPlrInfo : CHudBase
{
public:
    int Init();
    int Draw(float flTime);
};

typedef void (*PF_makevectors_I_func_t)(const float *);
typedef void (*PF_traceline_DLL_func_t)(const float *, const float *, int, uintptr_t, void *);
typedef int (*DrawConsoleString_func_t)(int, int, const char *);
typedef void (*DrawSetTextColor_func_t)(float, float, float);
typedef void (*AddHudElem_func_t)(uintptr_t, void *);

static uintptr_t p_gHUD = 0;
static uintptr_t p_gEngfuncs = 0;
static CHudPlrInfo hudPlrInfo;
static float default_color[3] = {1.0, 0.7, 0.0};

static PF_makevectors_I_func_t orig_PF_makevectors_I = nullptr;
static PF_traceline_DLL_func_t orig_PF_traceline_DLL = nullptr;
static AddHudElem_func_t orig_AddHudElem = nullptr;
static DrawConsoleString_func_t orig_DrawConsoleString = nullptr;
static DrawSetTextColor_func_t orig_DrawSetTextColor = nullptr;

static float get_entity_health()
{
    float *plrorigin = (float *)(*pp_sv_player + 0x80 + 0x8);
    float *viewofs = (float *)(*pp_sv_player + 0x80 + 0x174);
    float start[3] = {plrorigin[0] + viewofs[0], plrorigin[1] + viewofs[1],
                      plrorigin[2] + viewofs[2]};
    orig_PF_makevectors_I((float *)(*pp_sv_player + 0x80 + 0x74));
    float *g_forward = (float *)(*pp_gpGlobals + 0x28);
    float end[3];
    for (int i = 0; i < 3; i++)
        end[i] = plrorigin[i] + 8192 * g_forward[i];

    TraceResult trace;
    orig_PF_traceline_DLL(start, end, 0, *pp_sv_player, &trace);
    return *(float *)(trace.pHit + 0x80 + 0x160);
}

int CHudPlrInfo::Init()
{
    m_iFlags |= 1;
    orig_AddHudElem(p_gHUD, this);
    return 1;
}

int CHudPlrInfo::Draw(float)
{
    orig_DrawSetTextColor(default_color[0], default_color[1],
                          default_color[2]);

    char dispstr[30];

    float *vel = (float *)(*pp_sv_player + 0x80 + 0x20);
    snprintf(dispstr, sizeof(dispstr), "H: %.8g\n",
             std::hypot(vel[0], vel[1]));
    orig_DrawConsoleString(10, 20, dispstr);
    snprintf(dispstr, sizeof(dispstr), "V: %.8g\n", vel[2]);
    orig_DrawConsoleString(10, 30, dispstr);

    float *viewangles = (float *)(*pp_sv_player + 0x80 + 0x74);
    snprintf(dispstr, sizeof(dispstr), "Y: %.8g\n", viewangles[1]);
    orig_DrawConsoleString(10, 40, dispstr);
    snprintf(dispstr, sizeof(dispstr), "P: %.8g\n", viewangles[0]);
    orig_DrawConsoleString(10, 50, dispstr);

    float health = *(float *)(*pp_sv_player + 0x80 + 0x160);
    snprintf(dispstr, sizeof(dispstr), "HP: %.8g\n", health);
    orig_DrawConsoleString(10, 60, dispstr);

    float ent_hp = get_entity_health();
    snprintf(dispstr, sizeof(dispstr), "EHP: %.8g\n", ent_hp);
    orig_DrawConsoleString(10, 70, dispstr);

    int ducked = *(int *)(*pp_sv_player + 0x80 + 0x1a4) & FL_DUCKING;
    if (ducked)
        orig_DrawSetTextColor(1, 0, 1);
    orig_DrawConsoleString(10, 80, ducked ? "ducked" : "standing");
    if (ducked)
        orig_DrawSetTextColor(default_color[0], default_color[1],
                              default_color[2]);

    return 1;
}

void initialize_customhud(uintptr_t clso_addr, const symtbl_t &clso_st,
                          uintptr_t hwso_addr, const symtbl_t &hwso_st)
{
    orig_PF_makevectors_I = (PF_makevectors_I_func_t)(hwso_addr + hwso_st.at("PF_makevectors_I"));
    orig_PF_traceline_DLL = (PF_traceline_DLL_func_t)(hwso_addr + hwso_st.at("PF_traceline_DLL"));
    p_gHUD = (uintptr_t)(clso_addr + clso_st.at("gHUD"));
    p_gEngfuncs = (uintptr_t)(clso_addr + clso_st.at("gEngfuncs"));
    orig_AddHudElem = (AddHudElem_func_t)(clso_addr + clso_st.at("_ZN4CHud10AddHudElemEP8CHudBase"));
    orig_DrawConsoleString = *(DrawConsoleString_func_t *)(p_gEngfuncs + 0x6c);
    orig_DrawSetTextColor = *(DrawSetTextColor_func_t *)(p_gEngfuncs + 0x70);
    hudPlrInfo.Init();
}
