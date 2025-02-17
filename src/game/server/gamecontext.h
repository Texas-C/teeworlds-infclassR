/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_GAMECONTEXT_H
#define GAME_SERVER_GAMECONTEXT_H

#include <engine/server.h>
#include <engine/storage.h>
#include <engine/console.h>
#include <engine/shared/memheap.h>

#include <game/layers.h>
#include <game/voting.h>
#include <game/server/classes.h>

#include <teeuniverses/components/localization.h>

#include "eventhandler.h"
#include "gamecontroller.h"
#include "gameworld.h"
#include "player.h"

#ifdef CONF_GEOLOCATION
#include <infclassr/geolocation.h>
#endif
#include <fstream>

/*
	Tick
		Game Context (CGameContext::tick)
			Game World (GAMEWORLD::tick)
				Reset world if requested (GAMEWORLD::reset)
				All entities in the world (ENTITY::tick)
				All entities in the world (ENTITY::tick_defered)
				Remove entities marked for deletion (GAMEWORLD::remove_entities)
			Game Controller (GAMECONTROLLER::tick)
			All players (CPlayer::tick)


	Snap
		Game Context (CGameContext::snap)
			Game World (GAMEWORLD::snap)
				All entities in the world (ENTITY::snap)
			Game Controller (GAMECONTROLLER::snap)
			Events handler (EVENT_HANDLER::snap)
			All players (CPlayer::snap)

*/

#define BROADCAST_DURATION_REALTIME (0)
#define BROADCAST_DURATION_GAMEANNOUNCE (Server()->TickSpeed()*2)

struct FunRoundConfiguration
{
	FunRoundConfiguration() = default;
	FunRoundConfiguration(int Infected, int Human)
	: InfectedClass(Infected),
	  HumanClass(Human)
	{
	}

	int InfectedClass = 0;
	int HumanClass = 0;
};

enum
{
	BROADCAST_PRIORITY_LOWEST=0,
	BROADCAST_PRIORITY_WEAPONSTATE,
	BROADCAST_PRIORITY_EFFECTSTATE,
	BROADCAST_PRIORITY_GAMEANNOUNCE,
	BROADCAST_PRIORITY_SERVERANNOUNCE,
	BROADCAST_PRIORITY_INTERFACE,
};

class CConfig;

class CGameContext : public IGameServer
{
	IServer *m_pServer;
	CConfig *m_pConfig;
	IStorage *m_pStorage;
	class IConsole *m_pConsole;
	CLayers m_Layers;
	CCollision m_Collision;
	CNetObjHandler m_NetObjHandler;
	CTuningParams m_Tuning;
	int m_TargetToKill;
	int m_TargetToKillCoolDown;
	int m_HeroGiftCooldown;

#ifdef CONF_GEOLOCATION
	Geolocation* m_pGeolocation;
#endif

	static bool ConTuneParam(IConsole::IResult *pResult, void *pUserData);
	static bool ConTuneReset(IConsole::IResult *pResult, void *pUserData);
	static bool ConTuneDump(IConsole::IResult *pResult, void *pUserData);
	static bool ConPause(IConsole::IResult *pResult, void *pUserData);
	static bool ConChangeMap(IConsole::IResult *pResult, void *pUserData);
	static bool ConSkipMap(IConsole::IResult *pResult, void *pUserData);
	static bool ConQueueMap(IConsole::IResult *pResult, void *pUserData);
	static bool ConAddMap(IConsole::IResult *pResult, void *pUserData);
	static bool ConRestart(IConsole::IResult *pResult, void *pUserData);
	static bool ConBroadcast(IConsole::IResult *pResult, void *pUserData);
	static bool ConSay(IConsole::IResult *pResult, void *pUserData);
	static bool ConSetTeam(IConsole::IResult *pResult, void *pUserData);
	static bool ConSetTeamAll(IConsole::IResult *pResult, void *pUserData);
	static bool ConAddVote(IConsole::IResult *pResult, void *pUserData);
	static bool ConRemoveVote(IConsole::IResult *pResult, void *pUserData);
	static bool ConForceVote(IConsole::IResult *pResult, void *pUserData);
	static bool ConClearVotes(IConsole::IResult *pResult, void *pUserData);
	static bool ConVote(IConsole::IResult *pResult, void *pUserData);
	static bool ConStartFunRound(IConsole::IResult *pResult, void *pUserData);
	static bool ConStartSpecialFunRound(IConsole::IResult *pResult, void *pUserData);
	static bool ConchainSpecialMotdupdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);

	CGameContext(int Resetting);
	void Construct(int Resetting);

	bool m_Resetting;

public:
	int m_ZoneHandle_icDamage;
	int m_ZoneHandle_icTeleport;
	int m_ZoneHandle_icBonus;

public:
	IServer *Server() const { return m_pServer; }
	CConfig *Config() { return m_pConfig; }
	IStorage *Storage() const { return m_pStorage; }
	class IConsole *Console() { return m_pConsole; }
	CGameWorld *GameWorld() { return &m_World; }
	CCollision *Collision() { return &m_Collision; }
	CTuningParams *Tuning() { return &m_Tuning; }
	virtual class CLayers *Layers() { return &m_Layers; }

	CGameContext();
	~CGameContext();

	void Clear();

	CEventHandler m_Events;
	CPlayer *m_apPlayers[MAX_CLIENTS];

	IGameController *m_pController;
	CGameWorld m_World;

	// helper functions
	CPlayer *GetPlayer(int ClientID) const;
	class CCharacter *GetPlayerChar(int ClientID);
	// InfClassR
	int GetActivePlayerCount() const;
	int GetZombieCount();
	int GetZombieCount(int zombie_class);

	// InfClassR fun round
	bool StartFunRound(const FunRoundConfiguration &Configuration);
	void EndFunRound();
	bool m_FunRound;
	int m_FunRoundsPassed;
	int m_DefaultTimelimit;
	std::vector<int> m_DefaultAvailabilities, m_DefaultProbabilities;
	void SetAvailabilities(std::vector<int> value);
	void SetProbabilities(std::vector<int> value);

	// voting
	void StartVote(const char *pDesc, const char *pCommand, const char *pReason);
	void EndVote();
	void SendVoteSet(int ClientID);
	void SendVoteStatus(int ClientID, int Total, int Yes, int No);
	void AbortVoteKickOnDisconnect(int ClientID);

	int m_VoteCreator;
	int64 m_VoteCloseTime;
	bool m_VoteUpdate;
	int m_VotePos;
	char m_aVoteDescription[VOTE_DESC_LENGTH];
	char m_aVoteCommand[VOTE_CMD_LENGTH];
	char m_aVoteReason[VOTE_REASON_LENGTH];
	int m_NumVoteOptions;
	int m_VoteEnforce;
	enum
	{
		VOTE_ENFORCE_UNKNOWN=0,
		VOTE_ENFORCE_NO,
		VOTE_ENFORCE_YES,
	};
	CHeap *m_pVoteOptionHeap;
	CVoteOptionServer *m_pVoteOptionFirst;
	CVoteOptionServer *m_pVoteOptionLast;

	// helper functions
	void CreateDamageInd(vec2 Pos, float AngleMod, int Amount);
	void CreateExplosion(vec2 Pos, int Owner, int Weapon, bool NoDamage, int TakeDamageMode = TAKEDAMAGEMODE_NOINFECTION, float DamageFactor = 1.0f);
	void CreateExplosionDisk(vec2 Pos, float InnerRadius, float DamageRadius, int Damage, float Force, int Owner, int Weapon, int TakeDamageMode = TAKEDAMAGEMODE_NOINFECTION);
	void CreateHammerHit(vec2 Pos);
	void CreatePlayerSpawn(vec2 Pos);
	void CreateDeath(vec2 Pos, int Who);
	void CreateSound(vec2 Pos, int Sound, int64 Mask=-1);
	void CreateSoundGlobal(int Sound, int Target=-1);

	enum
	{
		CHAT_ALL=-2,
		CHAT_SPEC=-1,
		CHAT_RED=0,
		CHAT_BLUE=1
	};

	// network
	virtual void SendChatTarget(int To, const char *pText);
	void SendChat(int ClientID, int Team, const char *pText);
	void SendEmoticon(int ClientID, int Emoticon);
	void SendWeaponPickup(int ClientID, int Weapon);
	void SendKillMessage(int Killer, int Victim, int Weapon, int ModeSpecial);

	//
	void CheckPureTuning();
	void SendTuningParams(int ClientID);

	// engine events
	virtual void OnInit();
	virtual void OnStartRound();
	virtual void OnConsoleInit();
	virtual void OnShutdown();

	virtual void OnTick();
	virtual void OnPreSnap();
	virtual void OnSnap(int ClientID);
	virtual void OnPostSnap();

	virtual void OnMessage(int MsgID, CUnpacker *pUnpacker, int ClientID);

	virtual void OnClientConnected(int ClientID);
	virtual void OnClientEnter(int ClientID);
	virtual void OnClientDrop(int ClientID, int Type, const char *pReason);
	virtual void OnClientDirectInput(int ClientID, void *pInput);
	virtual void OnClientPredictedInput(int ClientID, void *pInput);

	virtual bool IsClientReady(int ClientID);
	virtual bool IsClientPlayer(int ClientID);

	virtual const char *GameType();
	virtual const char *Version();
	virtual const char *NetVersion();
	int GetClientVersion(int ClientID);

/* INFECTION MODIFICATION START ***************************************/
public:
	int m_ChatResponseTargetID;
	int m_ChatPrintCBIndex;

private:
	static void ChatConsolePrintCallback(const char *pLine, void *pUser);

	static bool ConCredits(IConsole::IResult *pResult, void *pUserData);
	static bool ConInfo(IConsole::IResult *pResult, void *pUserData);
	static bool ConVersion(IConsole::IResult *pResult, void *pUserData);
#ifdef CONF_SQL
	static bool ConRegister(IConsole::IResult *pResult, void *pUserData);
	static bool ConLogin(IConsole::IResult *pResult, void *pUserData);
	static bool ConLogout(IConsole::IResult *pResult, void *pUserData);
	static bool ConSetEmail(IConsole::IResult *pResult, void *pUserData);
	static bool ConTop10(IConsole::IResult *pResult, void *pUserData);
	static bool ConChallenge(IConsole::IResult *pResult, void *pUserData);
	static bool ConRank(IConsole::IResult *pResult, void *pUserData);
	static bool ConGoal(IConsole::IResult *pResult, void *pUserData);
	static bool ConStats(IConsole::IResult *pResult, void *pUserData);
#endif
	static bool ConHelp(IConsole::IResult *pResult, void *pUserData);
	static bool ConCustomSkin(IConsole::IResult *pResult, void *pUserData);
	static bool ConAlwaysRandom(IConsole::IResult *pResult, void *pUserData);
	static bool ConAntiPing(IConsole::IResult *pResult, void *pUserData);
	static bool ConLanguage(IConsole::IResult *pResult, void *pUserData);
	static bool ConCmdList(IConsole::IResult *pResult, void *pUserData);
	static bool ConChangeLog(IConsole::IResult *pResult, void *pUserData);

	static bool ConClearFunRounds(IConsole::IResult *pResult, void *pUserData);
	static bool ConAddFunRound(IConsole::IResult *pResult, void *pUserData);

	bool PrivateMessage(const char* pStr, int ClientID, bool TeamChat);
	void Converse(int ClientID, const char* pStr, int team);
	void MutePlayer(const char* pStr, int ClientID);
	
	void OnCallVote(void *pRawMsg, int ClientID);
	int IsMapVote(const char *pVoteCommand);
	void GetMapNameFromCommand(char* pMapName, const char *pCommand);

	enum
	{
		SV_MAP = 1,
		CHANGE_MAP = 2,
		SKIP_MAP = 3
	};
	
public:
	virtual void OnSetAuthed(int ClientID,int Level);
	
	virtual void SendBroadcast(int To, const char *pText, int Priority, int LifeSpan);
	virtual void SendBroadcast_Localization(int To, int Priority, int LifeSpan, const char* pText, ...);
	virtual void SendBroadcast_Localization_P(int To, int Priority, int LifeSpan, int Number, const char* pText, ...);
	virtual void SendBroadcast_ClassIntro(int To, int Class);
	virtual void ClearBroadcast(int To, int Priority);
	
	virtual void SendChatTarget_Localization(int To, int Category, const char* pText, ...);
	virtual void SendChatTarget_Localization_P(int To, int Category, int Number, const char* pText, ...);
	
	virtual void SendMOTD(int To, const char* pParam);
	virtual void SendMOTD_Localization(int To, const char* pText, ...);
	
	void CreateLaserDotEvent(vec2 Pos0, vec2 Pos1, int LifeSpan);
	void CreateHammerDotEvent(vec2 Pos, int LifeSpan);
	void CreateLoveEvent(vec2 Pos);
	void SendHitSound(int ClientID);
	void SendScoreSound(int ClientID);
	void AddBroadcast(int ClientID, const char* pText, int Priority, int LifeSpan);
	void SetClientLanguage(int ClientID, const char *pLanguage);
	
private:
	int m_VoteLanguageTick[MAX_CLIENTS];
	char m_VoteLanguage[MAX_CLIENTS][16];
	int m_VoteBanClientID;
	static bool m_ClientMuted[MAX_CLIENTS][MAX_CLIENTS]; // m_ClientMuted[i][j]: i muted j
	
	class CBroadcastState
	{
	public:
		int m_NoChangeTick;
		char m_PrevMessage[1024];
		
		int m_Priority;
		char m_NextMessage[1024];
		
		int m_LifeSpanTick;
		int m_TimedPriority;
		char m_TimedMessage[1024];
	};

	static void ConList(IConsole::IResult *pResult, void *pUserData);

	
	CBroadcastState m_BroadcastStates[MAX_CLIENTS];
	
	struct LaserDotState
	{
		vec2 m_Pos0;
		vec2 m_Pos1;
		int m_LifeSpan;
		int m_SnapID;
	};
	array<LaserDotState> m_LaserDots;
	
	struct HammerDotState
	{
		vec2 m_Pos;
		int m_LifeSpan;
		int m_SnapID;
	};
	array<HammerDotState> m_HammerDots;
	
	struct LoveDotState
	{
		vec2 m_Pos;
		int m_LifeSpan;
		int m_SnapID;
	};
	array<LoveDotState> m_LoveDots;
	
	int m_aHitSoundState[MAX_CLIENTS]; //1 for hit, 2 for kill (no sounds must be sent)	

	std::vector<FunRoundConfiguration> m_FunRoundConfigurations;

public:
	virtual int GetTargetToKill();
	virtual void TargetKilled();
	virtual void EnableTargetToKill() { m_TargetToKill = (m_TargetToKill < 0 ? -1 : m_TargetToKill); }
	virtual void DisableTargetToKill() { m_TargetToKill = -2; }
	virtual int GetTargetToKillCoolDown() { return m_TargetToKillCoolDown; }
	virtual int GetHeroGiftCoolDown() { return m_HeroGiftCooldown; }
	virtual void FlagCollected(); // Triggers global gift cooldown
/* INFECTION MODIFICATION END *****************************************/
	// InfClassR begin
	void AddSpectatorCID(int ClientID);
	void RemoveSpectatorCID(int ClientID);
	bool IsSpectatorCID(int ClientID);
	bool CanJoinSpec(int ClientID);
	std::ofstream fout;
	// InfClassR end
	bool IsVersionBanned(int Version);
};

inline int64 CmaskAll() { return -1LL; }
inline int64 CmaskOne(int ClientID) { return 1LL<<ClientID; }
inline int64 CmaskAllExceptOne(int ClientID) { return CmaskAll()^CmaskOne(ClientID); }
inline bool CmaskIsSet(int64 Mask, int ClientID) { return (Mask&CmaskOne(ClientID)) != 0; }
#endif
