#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "pm_defs.h"

DECLARE_MESSAGE(m_PlrInfo, Velocity)
DECLARE_MESSAGE(m_PlrInfo, EntHealth)
DECLARE_MESSAGE(m_PlrInfo, PlaneNZ)
DECLARE_MESSAGE(m_PlrInfo, DispVec)
DECLARE_MESSAGE(m_PlrInfo, EntClassN)
DECLARE_MESSAGE(m_PlrInfo, Selfgauss)

extern float g_ColorYellow[3];
static int line_height;
extern playermove_t *pmove;
extern StrafeType strafetype;

inline void reset_color()
{
	gEngfuncs.pfnDrawSetTextColor(g_ColorYellow[0], g_ColorYellow[1], g_ColorYellow[2]);
}

int CHudPlrInfo::Init()
{
	m_iFlags |= HUD_ACTIVE;
	gHUD.AddHudElem(this);

	int tmpwidth;
	GetConsoleStringSize("M", &tmpwidth, &line_height);
	memset(m_velocity, 0, sizeof(m_velocity));
	memset(m_dispVec, 0, sizeof(m_dispVec));
	m_entHealth = 0;
	m_planeNZ = 0;
	m_entClassname = "N/A";
	m_sgaussDist = -1;

	HOOK_MESSAGE(Velocity);
	HOOK_MESSAGE(EntHealth);
	HOOK_MESSAGE(PlaneNZ);
	HOOK_MESSAGE(DispVec);
	HOOK_MESSAGE(EntClassN);
	HOOK_MESSAGE(Selfgauss);

	CVAR_CREATE("hud_blocked", "1", 0);

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

int CHudPlrInfo::MsgFunc_EntClassN(const char *name, int size, void *buf)
{
	BEGIN_READ(buf, size);
	m_entClassname = READ_STRING();
	return 1;
}

int CHudPlrInfo::MsgFunc_Selfgauss(const char *name, int size, void *buf)
{
	BEGIN_READ(buf, size);
	m_sgaussDist = READ_FLOAT();
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
	float viewangles[3];
	gEngfuncs.GetViewAngles(viewangles);
	if (viewangles[0] > 180)
		viewangles[0] -= 360;
	if (viewangles[1] > 180)
		viewangles[1] -= 360;
	snprintf(numstr, sizeof(numstr), "Y: %.8g", viewangles[1]);
	DrawConsoleString(10, line_height * 7, numstr);
	snprintf(numstr, sizeof(numstr), "P: %.8g", viewangles[0]);
	DrawConsoleString(10, line_height * 8, numstr);
}

void CHudPlrInfo::DrawEntClassname()
{
	char numstr[30];
	int width, tmph;
	GetConsoleStringSize("CN: ", &width, &tmph);
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

void CHudPlrInfo::DrawStrafetype()
{
	char dispstr[7], c;
	switch (strafetype) {
	case Rightstrafe:
		c = 'R';
		break;
	case Leftstrafe:
		c = 'L';
		break;
	case Linestrafe:
		c = 'F';
		break;
	case Backpedal:
		c = 'B';
		break;
	default:
		c = '-';
	}
	snprintf(dispstr, sizeof(dispstr), "ST: %c", c);
	if (c != '-')
		gEngfuncs.pfnDrawSetTextColor(0, 1, 0);
	DrawConsoleString(10, line_height * 12, dispstr);
	if (c != '-')
		reset_color();
}

void CHudPlrInfo::DrawSelfgauss()
{
	char numstr[30];
	if (m_sgaussDist != -1)
	{
		snprintf(numstr, sizeof(numstr), "SG: %.8g", m_sgaussDist);
		gEngfuncs.pfnDrawSetTextColor(0, 1, 0);
	}
	else
	{
		strcpy(numstr, "SG: -");
	}
	DrawConsoleString(10, line_height * 13, numstr);
	if (m_sgaussDist != -1)
		reset_color();
}

void CHudPlrInfo::DrawBlocked(float flTime)
{
	static float startTime = 0;

	if (!pmove || !CVAR_GET_FLOAT("hud_blocked"))
		return;

	if (pmove->numtouch)
		startTime = flTime;

	float tdiff = flTime - startTime;
	const float DURATION = 0.1;
	if (tdiff > DURATION || tdiff < 0)
		return;

	const int BOX_LEN = 120;
	int x = (ScreenWidth - BOX_LEN) / 2;
	int y = (ScreenHeight - BOX_LEN) / 2;
	FillRGBA(x, y, BOX_LEN, BOX_LEN, 255, 255, 0, 255 * (1 - tdiff / DURATION));
}

int CHudPlrInfo::Draw(float flTime)
{
	reset_color();

	DrawBlocked(flTime);
	DrawVelocity();
	DrawEntHealth();
	DrawPlaneZA();
	DrawDistances();
	DrawViewangles();
	DrawEntClassname();
	DrawMyCrosshair();
	DrawDucked();
	DrawStrafetype();
	DrawSelfgauss();

	return 1;
}
