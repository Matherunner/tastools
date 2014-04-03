#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

int CHudPlrInfo::Init()
{
	m_iFlags |= HUD_ACTIVE;
	gHUD.AddHudElem(this);

	return 1;
}

int CHudPlrInfo::Draw(float flTime)
{


	return 1;
}
