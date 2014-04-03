#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

DECLARE_MESSAGE(m_PlrInfo, Velocity)
DECLARE_MESSAGE(m_PlrInfo, EntHealth)
DECLARE_MESSAGE(m_PlrInfo, PlaneNZ)
DECLARE_MESSAGE(m_PlrInfo, DispVec)
DECLARE_MESSAGE(m_PlrInfo, Viewangles)

extern float g_ColorYellow[3];
static int line_height;

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

	HOOK_MESSAGE(Velocity);
	HOOK_MESSAGE(EntHealth);
	HOOK_MESSAGE(PlaneNZ);
	HOOK_MESSAGE(DispVec);
	HOOK_MESSAGE(Viewangles);

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

void CHudPlrInfo::DrawVelocity()
{
	char numstr[30];
	snprintf(numstr, sizeof(numstr), "H: %.8g", hypot(m_velocity[0], m_velocity[1]));
	DrawConsoleString(10, line_height, numstr);
	snprintf(numstr, sizeof(numstr), "V: %.8g", m_velocity[2]);
	DrawConsoleString(10, line_height * 2, numstr);
}

void CHudPlrInfo::DrawEntHealth()
{
	char numstr[30];
	snprintf(numstr, sizeof(numstr), "EH: %.8g", m_entHealth);
	DrawConsoleString(10, line_height * 3, numstr);
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

int CHudPlrInfo::Draw(float flTime)
{
	gEngfuncs.pfnDrawSetTextColor(g_ColorYellow[0], g_ColorYellow[1], g_ColorYellow[2]);

	DrawVelocity();
	DrawEntHealth();
	DrawPlaneZA();
	DrawDistances();
	DrawViewangles();

	return 1;
}
