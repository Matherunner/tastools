#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

DECLARE_MESSAGE(m_PlrInfo, Velocity)
DECLARE_MESSAGE(m_PlrInfo, EntHealth)

extern float g_ColorYellow[3];
static int line_height;

int CHudPlrInfo::Init()
{
	m_iFlags |= HUD_ACTIVE;
	gHUD.AddHudElem(this);

	int tmpwidth;
	GetConsoleStringSize("M", &tmpwidth, &line_height);
	memset(m_velocity, 0, sizeof(m_velocity));
	m_entHealth = 0;

	HOOK_MESSAGE(Velocity);
	HOOK_MESSAGE(EntHealth);

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

void CHudPlrInfo::DrawVelocity()
{
	char numstr[30];
	snprintf(numstr, 30, "H: %.8g", hypot(m_velocity[0], m_velocity[1]));
	DrawConsoleString(10, line_height, numstr);
	snprintf(numstr, 30, "V: %.8g", m_velocity[2]);
	DrawConsoleString(10, line_height * 2, numstr);
}

void CHudPlrInfo::DrawEntHealth()
{
	char numstr[30];
	snprintf(numstr, 30, "EH: %.8g", m_entHealth);
	DrawConsoleString(10, line_height * 3, numstr);
}

int CHudPlrInfo::Draw(float flTime)
{
	gEngfuncs.pfnDrawSetTextColor(g_ColorYellow[0], g_ColorYellow[1], g_ColorYellow[2]);

	DrawVelocity();
	DrawEntHealth();

	return 1;
}
