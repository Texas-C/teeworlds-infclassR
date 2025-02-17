#include "infected.h"

#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include <game/server/infclass/entities/infccharacter.h>
#include <game/server/infclass/infcplayer.h>
#include <game/server/teeinfo.h>

MACRO_ALLOC_POOL_ID_IMPL(CInfClassInfected, MAX_CLIENTS)

CInfClassInfected::CInfClassInfected(CInfClassPlayer *pPlayer)
	: CInfClassPlayerClass(pPlayer)
{
}

void CInfClassInfected::OnCharacterSpawned()
{
	CInfClassPlayerClass::OnCharacterSpawned();

	m_HealTick = 0;
}

void CInfClassInfected::GiveClassAttributes()
{
	CInfClassPlayerClass::GiveClassAttributes();

	m_pCharacter->GiveWeapon(WEAPON_HAMMER, -1);
	m_pCharacter->SetActiveWeapon(WEAPON_HAMMER);

	if (GameServer()->GetZombieCount() <= 1)
	{
		/* Lonely zombie */
		m_pCharacter->IncreaseArmor(10);
	}

	if(m_pCharacter->CanOpenPortals())
	{
		m_pCharacter->GiveWeapon(WEAPON_LASER, -1);
	}
}

void CInfClassInfected::SetupSkin(CTeeInfo *output)
{
	switch(PlayerClass())
	{
		case PLAYERCLASS_SMOKER:
			output->m_UseCustomColor = 1;
			output->SetSkinName("cammostripes");
			output->m_ColorBody = 3866368;
			output->m_ColorFeet = 65414;
			break;
		case PLAYERCLASS_BOOMER:
			output->SetSkinName("saddo");
			output->m_UseCustomColor = 1;
			output->m_ColorBody = 3866368;
			output->m_ColorFeet = 65414;
			break;
		case PLAYERCLASS_HUNTER:
			output->SetSkinName("warpaint");
			output->m_UseCustomColor = 1;
			output->m_ColorBody = 3866368;
			output->m_ColorFeet = 65414;
			break;
		case PLAYERCLASS_BAT:
			output->SetSkinName("limekitty");
			output->m_UseCustomColor = 1;
			output->m_ColorBody = 3866368;
			output->m_ColorFeet = 2866368;
			break;
		case PLAYERCLASS_GHOST:
			output->SetSkinName("twintri");
			output->m_UseCustomColor = 1;
			output->m_ColorBody = 3866368;
			output->m_ColorFeet = 65414;
			break;
		case PLAYERCLASS_SPIDER:
			output->SetSkinName("pinky");
			output->m_UseCustomColor = 1;
			output->m_ColorBody = 3866368;
			output->m_ColorFeet = 65414;
			break;
		case PLAYERCLASS_GHOUL:
			output->SetSkinName("cammo");
			output->m_UseCustomColor = 1;
			{
				int Hue = 58 * (1.0f - GetGhoulPercent());
				output->m_ColorBody = (Hue<<16) + (255<<8);
			}
			output->m_ColorFeet = 65414;
			break;
		case PLAYERCLASS_SLUG:
			output->SetSkinName("coala");
			output->m_UseCustomColor = 1;
			output->m_ColorBody = 3866368;
			output->m_ColorFeet = 65414;
			break;
		case PLAYERCLASS_VOODOO:
			output->SetSkinName("bluestripe");
			output->m_UseCustomColor = 1;
			if(m_pCharacter && !m_pCharacter->m_VoodooAboutToDie)
			{
				output->m_ColorBody = 3866368;
			}
			else
			{
				output->m_ColorBody = 6183936; // grey-green
			}
			output->m_ColorFeet = 65414;
			break;
		case PLAYERCLASS_UNDEAD:
			output->SetSkinName("redstripe");
			output->m_UseCustomColor = 1;
			output->m_ColorBody = 3014400;
			output->m_ColorFeet = 13168;
			break;
		case PLAYERCLASS_WITCH:
			output->SetSkinName("redbopp");
			output->m_UseCustomColor = 1;
			output->m_ColorBody = 16776744;
			output->m_ColorFeet = 13168;
			break;
		default:
			output->m_UseCustomColor = 0;
			output->SetSkinName("default");
	}
}

void CInfClassInfected::OnSlimeEffect(int Owner)
{
	if(PlayerClass() == PLAYERCLASS_SLUG)
		return;

	m_pCharacter->SetEmote(EMOTE_HAPPY, Server()->Tick());
	if(Server()->Tick() >= m_HealTick + (Server()->TickSpeed() / Config()->m_InfSlimeHealRate))
	{
		m_HealTick = Server()->Tick();
		m_pCharacter->IncreaseHealth(1);
	}
}

void CInfClassInfected::OnFloatingPointCollected(int Points)
{
	if(PlayerClass() != PLAYERCLASS_GHOUL)
		return;

	m_pCharacter->IncreaseOverallHp(4);
	IncreaseGhoulLevel(Points);
}

float CInfClassInfected::GetGhoulPercent() const
{
	return GetPlayer()->GetGhoulPercent();
}

void CInfClassInfected::IncreaseGhoulLevel(int Diff)
{
	GetPlayer()->IncreaseGhoulLevel(Diff);
}

int CInfClassInfected::GetGhoulLevel() const
{
	return GetPlayer()->GetGhoulLevel();
}
