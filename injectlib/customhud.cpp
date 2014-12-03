#include <cstring>
#include <cmath>
#include "customhud.hpp"
#include "common.hpp"

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

typedef int (*DrawConsoleString_func_t)(int, int, const char *);
typedef void (*DrawSetTextColor_func_t)(float, float, float);
typedef void (*AddHudElem_func_t)(uintptr_t, void *);

static uintptr_t p_gHUD = 0;
static uintptr_t p_gEngfuncs = 0;
static CHudPlrInfo hudPlrInfo;
static float default_color[3] = {1.0, 0.7, 0.0};

static AddHudElem_func_t orig_AddHudElem = nullptr;
static DrawConsoleString_func_t orig_DrawConsoleString = nullptr;
static DrawSetTextColor_func_t orig_DrawSetTextColor = nullptr;

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

    float health = *(float *)(*pp_sv_player + 0x80 + 0x160);
    snprintf(dispstr, sizeof(dispstr), "HP: %.8g\n", health);
    orig_DrawConsoleString(10, 40, dispstr);

    int ducked = *(int *)(*pp_sv_player + 0x80 + 0x1a4) & FL_DUCKING;
    if (ducked)
        orig_DrawSetTextColor(1, 0, 1);
    orig_DrawConsoleString(10, 50, ducked ? "ducked" : "standing");
    if (ducked)
        orig_DrawSetTextColor(default_color[0], default_color[1],
                              default_color[2]);

    return 1;
}

void initialize_customhud(uintptr_t clso_addr, const symtbl_t &clso_st)
{
    p_gHUD = (uintptr_t)(clso_addr + clso_st.at("gHUD"));
    p_gEngfuncs = (uintptr_t)(clso_addr + clso_st.at("gEngfuncs"));
    orig_AddHudElem = (AddHudElem_func_t)(clso_addr + clso_st.at("_ZN4CHud10AddHudElemEP8CHudBase"));
    orig_DrawConsoleString = *(DrawConsoleString_func_t *)(p_gEngfuncs + 0x6c);
    orig_DrawSetTextColor = *(DrawSetTextColor_func_t *)(p_gEngfuncs + 0x70);
    hudPlrInfo.Init();
}
