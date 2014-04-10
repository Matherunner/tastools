#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "pm_defs.h"

DECLARE_MESSAGE(m_PlrInfo, Velocity)
DECLARE_MESSAGE(m_PlrInfo, EntHealth)
DECLARE_MESSAGE(m_PlrInfo, PlaneNZ)
DECLARE_MESSAGE(m_PlrInfo, DispVec)
DECLARE_MESSAGE(m_PlrInfo, Viewangles)
DECLARE_MESSAGE(m_PlrInfo, EntClassN)

extern float g_ColorYellow[3];
static int line_height;
extern playermove_t *pmove;

int CHudPlrInfo::Init()
{
	m_iFlags |= HUD_ACTIVE;
	gHUD.AddHudElem(this);

	int tmpwidth;
	GetConsoleStringSize("M", &tmpwidth, &line_height);
	memset(m_velocity, 0, sizeof(m_velocity));
	memset(m_dispVec, 0, sizeof(m_dispVec));
	memset(m_viewangles, 0, sizeof(m_viewangles));
	m_entHealth = 0;
	m_planeNZ = 0;
	m_entClassname = "N/A";

	HOOK_MESSAGE(Velocity);
	HOOK_MESSAGE(EntHealth);
	HOOK_MESSAGE(PlaneNZ);
	HOOK_MESSAGE(DispVec);
	HOOK_MESSAGE(Viewangles);
	HOOK_MESSAGE(EntClassN);

	return 1;
}

int CHudPlrInfo::MsgFunc_Velocity(const char *name, int size, void *buf)
{
	BEGIN_READ(buf, size);
	m_velocity[0] = READ_FLOAT();
	m_velocity[1] = READ_FLOAT();
	m_velocity[2] = READ_FLOAT();
	return 1;
}

int CHudPlrInfo::MsgFunc_EntHealth(const char *name, int size, void *buf)
{
	BEGIN_READ(buf, size);
	m_entHealth = READ_FLOAT();
	return 1;
}

int CHudPlrInfo::MsgFunc_DispVec(const char *name, int size, void *buf)
{
	BEGIN_READ(buf, size);
	m_dispVec[0] = READ_FLOAT();
	m_dispVec[1] = READ_FLOAT();
	m_dispVec[2] = READ_FLOAT();
	return 1;
}

int CHudPlrInfo::MsgFunc_PlaneNZ(const char *name, int size, void *buf)
{
	BEGIN_READ(buf, size);
	m_planeNZ = READ_FLOAT();
	return 1;
}

int CHudPlrInfo::MsgFunc_Viewangles(const char *name, int size, void *buf)
{
	BEGIN_READ(buf, size);
	m_viewangles[0] = READ_FLOAT();
	m_viewangles[1] = READ_FLOAT();
	return 1;
}

int CHudPlrInfo::MsgFunc_EntClassN(const char *name, int size, void *buf)
{
	BEGIN_READ(buf, size);
	m_entClassname = READ_STRING();
	return 1;
}

void CHudPlrInfo::DrawVelocity()
{
	char numstr[30];
	snprintf(numstr, sizeof(numstr), "H: %.8g", hypot(m_velocity[0], m_velocity[1]));
	DrawConsoleString(10, line_height, numstr);
	snprintf(numstr, sizeof(numstr), "V: %.8g", m_velocity[2]);
	DrawConsoleString(10, line_height * 2, numstr);
	int r, g, b;
	UnpackRGB(r, g, b, RGB_YELLOWISH);
	FillRGBA(10, (int)(line_height * 3.5) - 1, 100, 1, r, g, b, 255);
}

void CHudPlrInfo::DrawEntHealth()
{
	char numstr[30];
	snprintf(numstr, sizeof(numstr), "EH: %.8g", m_entHealth);
	DrawConsoleString(10, line_height * 9, numstr);
}

void CHudPlrInfo::DrawPlaneZA()
{
	char numstr[30];
	snprintf(numstr, sizeof(numstr), "ZA: %.8g", acos(m_planeNZ) * 180 / M_PI);
	DrawConsoleString(10, line_height * 4, numstr);
}

void CHudPlrInfo::DrawDistances()
{
	char numstr[30];
	snprintf(numstr, sizeof(numstr), "HD: %.8g", hypot(m_dispVec[0], m_dispVec[1]));
	DrawConsoleString(10, line_height * 5, numstr);
	snprintf(numstr, sizeof(numstr), "VD: %.8g", m_dispVec[2]);
	DrawConsoleString(10, line_height * 6, numstr);
}

void CHudPlrInfo::DrawViewangles()
{
	char numstr[30];
	snprintf(numstr, sizeof(numstr), "Y: %.8g", m_viewangles[1]);
	DrawConsoleString(10, line_height * 7, numstr);
	snprintf(numstr, sizeof(numstr), "P: %.8g", m_viewangles[0]);
	DrawConsoleString(10, line_height * 8, numstr);
}

void CHudPlrInfo::DrawEntClassname()
{
	char numstr[30];
	int width, tmph;
	GetConsoleStringSize("EC: ", &width, &tmph);
	DrawConsoleString(10, line_height * 10, "CN: ");
	DrawConsoleString(10 + width, line_height * 10, m_entClassname);
}

void CHudPlrInfo::DrawMyCrosshair()
{
	const int chwidth = 4;
	FillRGBA((ScreenWidth - chwidth) / 2, (ScreenHeight - chwidth) / 2, chwidth, chwidth, 255, 0, 0, 255);
}

void CHudPlrInfo::DrawDucked()
{
	int w, h;
	GetConsoleStringSize("DS: ", &w, &h);
	DrawConsoleString(10, line_height * 11, "DS: ");
	if (pmove && (pmove->flags & FL_DUCKING))
		FillRGBA(10 + w + 1, line_height * 11 + 1, line_height - 1, line_height - 1, 255, 0, 255, 255);
}

int CHudPlrInfo::Draw(float flTime)
{
	gEngfuncs.pfnDrawSetTextColor(g_ColorYellow[0], g_ColorYellow[1], g_ColorYellow[2]);

	DrawVelocity();
	DrawEntHealth();
	DrawPlaneZA();
	DrawDistances();
	DrawViewangles();
	DrawEntClassname();
	DrawMyCrosshair();
	DrawDucked();

	return 1;
}
