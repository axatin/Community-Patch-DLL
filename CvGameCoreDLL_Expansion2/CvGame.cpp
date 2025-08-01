/*	-------------------------------------------------------------------------------------------------------
	© 1991-2012 Take-Two Interactive Software and its subsidiaries.  Developed by Firaxis Games.  
	Sid Meier's Civilization V, Civ, Civilization, 2K Games, Firaxis Games, Take-Two Interactive Software 
	and their respective logos are all trademarks of Take-Two interactive Software, Inc.  
	All other marks and trademarks are the property of their respective owners.  
	All rights reserved. 
	------------------------------------------------------------------------------------------------------- */

#include "CvGameCoreDLLPCH.h"
#include "CvGameCoreUtils.h"
#include "CvInternalGameCoreUtils.h"
#include "CvGame.h"
#include "CvMap.h"
#include "CvPlot.h"
#include "CvPlayerAI.h"
#include "CvRandom.h"
#include "CvTeam.h"
#include "CvGlobals.h"
#include "CvMapGenerator.h"
#include "CvReplayMessage.h"
#include "CvInfos.h"
#include "CvReplayInfo.h"
#include "CvGameTextMgr.h"
#include "CvSiteEvaluationClasses.h"
#include "CvImprovementClasses.h"
#include "CvStartPositioner.h"
#include "CvTacticalAnalysisMap.h"
#include "CvGrandStrategyAI.h"
#include "CvMinorCivAI.h"
#include "CvDiplomacyAI.h"
#include "CvNotifications.h"
#include "CvAdvisorCounsel.h"
#include "CvAdvisorRecommender.h"
#include "CvWorldBuilderMapLoader.h"
#include "CvTypes.h"
#include "CvDllNetMessageExt.h"

#include "cvStopWatch.h"
#include "CvUnitMission.h"

#include "CvDLLUtilDefines.h"
#include "CvAchievementUnlocker.h"

// interface uses
#include "ICvDLLUserInterface.h"
#include "CvEnumSerialization.h"
#include "FStlContainerSerialization.h"
#include "CvStringUtils.h"
#include "CvBarbarians.h"
#include "CvGoodyHuts.h"

#include <sstream>

#include "CvDiplomacyRequests.h"

#include "CvDllPlot.h"
#include "FFileSystem.h"
#include "CvInfosSerializationHelper.h"
#include "CvCityManager.h"
#include "CvPlayerManager.h"
#include "CvDllContext.h"

#include "CvSpanSerialization.h"
#include "CvEnumMapSerialization.h"

//updated by pre-build hook
#include "../commit_id.inc"

// Public Functions...
// must be included after all other headers
#include "LintFree.h"

int GetNextGlobalID() { return GC.getGame().GetNextGlobalID(); }
int GetJonRand(int iRange) { return GC.getGame().getJonRandNum(iRange,"generic"); }

struct stringHash
{
	//taken from std::tr1::hash<string> but modified to look at all characters
	size_t operator()(const std::string& key) const
	{
		size_t _Val = 2166136261U;
		size_t _First = 0;
		size_t _Last = key.size();
		for(; _First < _Last; _First++)
			_Val = 16777619U * _Val ^ (size_t)key[_First];
		return (_Val);
	}
};

//------------------------------------------------------------------------------
// CvGame Version History
// Version 1 
//	 * CvGame save version reset for expansion pack 2.
//------------------------------------------------------------------------------
const int g_CurrentCvGameVersion = 1;

//some statistics
int gTactMovesCount[NUM_AI_TACTICAL_MOVES] = { 0 };
int gHomeMovesCount[NUM_AI_HOMELAND_MOVES] = { 0 };

CvGameInitialItemsOverrides::CvGameInitialItemsOverrides()
{
	//By default, all players and teams are granted all
	//free items.
	GrantInitialFreeTechsPerTeam.resize(MAX_TEAMS, true);
	GrantInitialGoldPerPlayer.resize(MAX_PLAYERS, true);
	GrantInitialCulturePerPlayer.resize(MAX_PLAYERS, true);
	ClearResearchQueuePerPlayer.resize(MAX_PLAYERS, true);
	GrantInitialUnitsPerPlayer.resize(MAX_PLAYERS, true);
}

//------------------------------------------------------------------------------
CvGame::CvGame() :
	m_jonRand("GameRng")
	, m_mapRand("PreGameRng")
	, m_endTurnTimer()
	, m_endTurnTimerSemaphore(0)
	, m_curTurnTimer()
	, m_timeSinceGameTurnStart()
	, m_fCurrentTurnTimerPauseDelta(0.f)
	, m_sentAutoMoves(false)
	, m_bForceEndingTurn(false)
	, m_pDiploResponseQuery(NULL)
	, m_bFOW(true)
#ifdef EA_EVENT_GAME_SAVE
	, m_bSavedOnce(false)
#endif
	, m_bArchaeologyTriggered(false)
	, m_bIsDesynced(false)
	, m_eObserverUIOverridePlayer(NO_PLAYER)
	, m_lastTurnAICivsProcessed(-1)
	, m_processPlayerAutoMoves(false)
	, m_cityDistancePathLength(NO_DOMAIN) //for now!
	, m_cityDistancePlots()
	, m_eCurrentVisibilityPlayer(NO_PLAYER)
{
	m_pSettlerSiteEvaluator = NULL;
	m_pStartSiteEvaluator = NULL;
	m_pStartPositioner = NULL;
	m_pGameReligions = NULL;
	m_pGameCulture = NULL;
	m_pGameLeagues = NULL;
	m_pGameTrade = NULL;

#if defined(MOD_BALANCE_CORE)
	m_pGameCorporations = NULL;
	m_pGameContracts = NULL;
#endif

	m_pAdvisorCounsel = NULL;
	m_pAdvisorRecommender = NULL;

	m_endTurnTimer.Start();
	m_endTurnTimer.Stop();

	reset(NO_HANDICAP, true);
}


//	--------------------------------------------------------------------------------
CvGame::~CvGame()
{
#ifdef VPDEBUG
	FILogFile* pLog=LOGFILEMGR.GetLog( "UnitMoveStats.txt", FILogFile::kDontTimeStamp );
	if (pLog)
	{
		pLog->Msg("#move and count @ turn %d\n",getGameTurn());
		for (int i = 0; i < NUM_AI_TACTICAL_MOVES; i++)
			pLog->Msg(CvString::format("%s:%d\n", tacticalMoveNames[i], gTactMovesCount[i]).c_str());
		for (int i = 0; i < NUM_AI_HOMELAND_MOVES; i++)
			pLog->Msg(CvString::format("%s:%d\n", homelandMoveNames[i], gHomeMovesCount[i]).c_str());
		pLog->Close();
	}
#endif

	uninit();
}

//	--------------------------------------------------------------------------------
void CvGame::init(HandicapTypes eHandicap)
{
	bool bValid = false;
	int iStartTurn = 0;
	int iEstimateEndTurn = 0;
	int iI = 0;

	//--------------------------------
	// Init saved data
	reset(eHandicap);

	m_mapRand.init(CvPreGame::mapRandomSeed() % 73637381);
	m_jonRand.init(CvPreGame::syncRandomSeed() % 52319761);

	SetClosestCityMapDirty();

	//--------------------------------
	// Verify pregame data

	//Validate game era. If we lack the era info for the current era, work backwards until we find a valid one.
	if(!GC.getEraInfo(getStartEra())){
		for(int eraIdx = ((int)CvPreGame::era())-1; eraIdx >= 0; --eraIdx){
			CvEraInfo* curEraInfo = GC.getEraInfo((EraTypes)eraIdx);
			if(curEraInfo){
				CvPreGame::setEra((EraTypes)eraIdx);
				break;
			}
		}
	}

	//--------------------------------
	// Init non-saved data

	//--------------------------------
	// Init other game data

	// Turn off all MP options if it's a single player game
	GameTypes g = CvPreGame::gameType();
	if(g == GAME_SINGLE_PLAYER)
	{
		for(iI = 0; iI < NUM_MPOPTION_TYPES; ++iI)
		{
			setMPOption((MultiplayerOptionTypes)iI, false);
		}

		setOption(GAMEOPTION_SIMULTANEOUS_TURNS, false);
		setOption(GAMEOPTION_DYNAMIC_TURNS, false);
		setOption(GAMEOPTION_PITBOSS, false);
	}

	// If this is a hot seat game, simultaneous turns is always off
	if(isHotSeat() || isPbem())
	{
		setOption(GAMEOPTION_SIMULTANEOUS_TURNS, false);
		setOption(GAMEOPTION_DYNAMIC_TURNS, false);
		setOption(GAMEOPTION_PITBOSS, false);
	}

	if(isMPOption(MPOPTION_SHUFFLE_TEAMS))
	{
		int aiTeams[MAX_CIV_PLAYERS];

		int iNumPlayers = 0;
		for(int i = 0; i < MAX_CIV_PLAYERS; i++)
		{
			if(CvPreGame::slotStatus((PlayerTypes)i) == SS_TAKEN)
			{
				aiTeams[iNumPlayers] = CvPreGame::teamType((PlayerTypes)i);
				++iNumPlayers;
			}
		}

		for(int i = 0; i < iNumPlayers; i++)
		{
			int j = (getJonRandNum(iNumPlayers - i, NULL) + i);

			if(i != j)
			{
				int iTemp = aiTeams[i];
				aiTeams[i] = aiTeams[j];
				aiTeams[j] = iTemp;
			}
		}

		iNumPlayers = 0;
		for(int i = 0; i < MAX_CIV_PLAYERS; i++)
		{
			if(CvPreGame::slotStatus((PlayerTypes)i) == SS_TAKEN)
			{
				CvPreGame::setTeamType((PlayerTypes)i, (TeamTypes)aiTeams[iNumPlayers]);
				++iNumPlayers;
			}
		}
	}

	if(isOption(GAMEOPTION_LOCK_MODS))
	{
		if(isGameMultiPlayer())
		{
			setOption(GAMEOPTION_LOCK_MODS, false);
		}
		else
		{
			static const int iPasswordSize = 8;
			char szRandomPassword[iPasswordSize];
			for(int i = 0; i < iPasswordSize-1; i++)
			{
				szRandomPassword[i] = getJonRandNum(128, "Random Keyword");
			}
			szRandomPassword[iPasswordSize-1] = 0;

			CvString strRandomPassword = szRandomPassword;
			CvPreGame::setAdminPassword(strRandomPassword);
		}
	}

	const CvGameSpeedInfo& kGameSpeedInfo = getGameSpeedInfo();
	if(getGameTurn() == 0)
	{
		iStartTurn = 0;

		for(iI = 0; iI < kGameSpeedInfo.getNumTurnIncrements(); iI++)
		{
			iStartTurn += kGameSpeedInfo.getGameTurnInfo(iI).iNumGameTurnsPerIncrement;
		}

		const CvEraInfo& kEraInfo = getStartEraInfo();

		iStartTurn *= kEraInfo.getStartPercent();
		iStartTurn /= 100;

		setGameTurn(iStartTurn);

		if (kEraInfo.isNoReligion())
		{
			CvPreGame::SetGameOption(GAMEOPTION_NO_RELIGION, true);
		}
	}

	setStartTurn(getGameTurn());

	iEstimateEndTurn = 0;

	for(iI = 0; iI < kGameSpeedInfo.getNumTurnIncrements(); iI++)
	{
		iEstimateEndTurn += kGameSpeedInfo.getGameTurnInfo(iI).iNumGameTurnsPerIncrement;
	}

	setDefaultEstimateEndTurn(iEstimateEndTurn);

	if(getMaxTurns() == 0)
	{

		setEstimateEndTurn(iEstimateEndTurn);

		if(getEstimateEndTurn() > getGameTurn())
		{
			bValid = false;

			for(iI = 0; iI < GC.getNumVictoryInfos(); iI++)
			{
				VictoryTypes eVictory = static_cast<VictoryTypes>(iI);
				CvVictoryInfo* pkVictoryInfo = GC.getVictoryInfo(eVictory);
				if(pkVictoryInfo)
				{
					if(isVictoryValid(eVictory))
					{
						if(pkVictoryInfo->isEndScore())
						{
							bValid = true;
							break;
						}
					}
				}

			}

			if(bValid)
			{
				setMaxTurns(getEstimateEndTurn() - getGameTurn());
			}
		}
	}
	else
	{
		setEstimateEndTurn(getGameTurn() + getMaxTurns());
	}

	setStartYear(/*-4000*/ GD_INT_GET(START_YEAR));

	for(iI = 0; iI < GC.getNumSpecialUnitInfos(); iI++)
	{
		SpecialUnitTypes eSpecialUnit = static_cast<SpecialUnitTypes>(iI);
		CvSpecialUnitInfo* pkSpecialUnitInfo = GC.getSpecialUnitInfo(eSpecialUnit);
		if(pkSpecialUnitInfo)
		{
			if(pkSpecialUnitInfo->isValid())
			{
				makeSpecialUnitValid(eSpecialUnit);
			}
		}
	}

	if(isOption(GAMEOPTION_QUICK_COMBAT))
	{
		CvPreGame::setQuickCombat(true);
	}

	m_bArchaeologyTriggered = false;
	CvGoodyHuts::Reset();

	doUpdateCacheOnTurn();
}

//	--------------------------------------------------------------------------------
bool CvGame::init2()
{
	InitPlayers();

	updateGlobalMedians();

	DoBarbCountdown();

	CvGameInitialItemsOverrides kItemOverrides;
	if(!InitMap(kItemOverrides))
	{
		return false;
	}

	initDiplomacy();
	setInitialItems(kItemOverrides);

	if(CvPreGame::isWBMapScript() && !CvPreGame::mapNoPlayers())
	{
		CvWorldBuilderMapLoader::SetInitialItems(false);
	}

	CheckGenerateArchaeology();

	initScoreCalculation();
	initSpyThreshold();
	setFinalInitialized(true);

#if defined(MOD_EVENTS_TERRAFORMING)
	if (MOD_EVENTS_TERRAFORMING) {
		GAMEEVENTINVOKE_HOOK(GAMEEVENT_TerraformingMap, TERRAFORMINGEVENT_LOAD, 0);
	}
#endif
#if defined(MOD_BALANCE_CORE_RESOURCE_MONOPOLIES)
	if(MOD_BALANCE_CORE_RESOURCE_MONOPOLIES)
	{
		int iNumResourceInfos= GC.getNumResourceInfos();
		for(int iResourceLoop = 0; iResourceLoop < iNumResourceInfos; iResourceLoop++)
		{
			const ResourceTypes eResource = static_cast<ResourceTypes>(iResourceLoop);
			CvResourceInfo* pkResourceInfo = GC.getResourceInfo(eResource);
			if(pkResourceInfo)
			{
				GC.getMap().setNumResources(eResource);
			}
		}
	}
#endif

	return true;
}

//------------------------------------------------------------------------------
// Lua Hooks
// These are static functions to hook into Lua and relay information to the DLL.
//------------------------------------------------------------------------------
int WorldBuilderMapLoaderAddRandomItems(lua_State* L)
{
	return CvWorldBuilderMapLoader::AddRandomItems(L);
}
//------------------------------------------------------------------------------
int WorldBuilderMapLoaderLoadModData(lua_State* L)
{
	return CvWorldBuilderMapLoader::LoadModData(L);
}
//------------------------------------------------------------------------------
int WorldBuilderMapLoaderRunPostProcessScript(lua_State* L)
{
	return CvWorldBuilderMapLoader::RunPostProcessScript(L);
}

void SetAllPlotsVisible(TeamTypes eTeam)
{
	if (eTeam != NO_TEAM)
	{
		const int iNumInvisibleInfos = NUM_INVISIBLE_TYPES;
		for (int plotID = 0; plotID < GC.getMap().numPlots(); plotID++)
		{
			CvPlot* pLoopPlot = GC.getMap().plotByIndexUnchecked(plotID);
			pLoopPlot->changeVisibilityCount(eTeam, pLoopPlot->getVisibilityCount(eTeam) + 1, NO_INVISIBLE, true, false);
			pLoopPlot->changeInvisibleVisibilityCountUnit(eTeam, pLoopPlot->getInvisibleVisibilityCountUnit(eTeam) + 1);
			for (int iJ = 0; iJ < iNumInvisibleInfos; iJ++)
			{
				pLoopPlot->changeInvisibleVisibilityCount(eTeam, ((InvisibleTypes)iJ), pLoopPlot->getInvisibleVisibilityCount(eTeam, ((InvisibleTypes)iJ)) + 1);
			}

			pLoopPlot->setRevealed(eTeam, true);
		}
	}
}

//------------------------------------------------------------------------------
bool CvGame::InitMap(CvGameInitialItemsOverrides& kGameInitialItemsOverrides)
{
	CvMap& kMap = GC.getMap();
	const bool bWBSave = CvPreGame::isWBMapScript();
	if(bWBSave)
	{
		const CvWorldBuilderMapLoaderMapInfo& kWBMapInfo =
		    CvWorldBuilderMapLoader::GetCurrentMapInfo();

		if(kWBMapInfo.uiWidth * kWBMapInfo.uiHeight != 0)
		{
			CvMapInitData kMapInitData;
			kMapInitData.m_bWrapX = kWBMapInfo.bWorldWrap;
			kMapInitData.m_bWrapY = false;
			kMapInitData.m_iGridW = kWBMapInfo.uiWidth;
			kMapInitData.m_iGridH = kWBMapInfo.uiHeight;
			kMap.init(&kMapInitData);

			CvBarbarians::MapInit(kMap.numPlots());

			CvWorldBuilderMapLoader::InitMap();
			CvWorldBuilderMapLoader::ValidateTerrain();

			ICvEngineScriptSystem1* pkScriptSystem = gDLL->GetScriptSystem();
			if(pkScriptSystem != NULL)
			{
				lua_State* L = pkScriptSystem->CreateLuaThread("WorldBuilderMapLoader");
				if(L != NULL)
				{
					lua_cpcall(L, &WorldBuilderMapLoaderAddRandomItems, 0);
					lua_cpcall(L, &WorldBuilderMapLoaderLoadModData, 0);
					lua_cpcall(L, &WorldBuilderMapLoaderRunPostProcessScript, 0);

					pkScriptSystem->FreeLuaThread(L);
				}
			}
		}
		else
		{
			// Empty map...
			ASSERT_DEBUG(0, "Empty World Builder Map!");

			// Make the map at least 1 x 1 to avoid crashes
			CvMapInitData kMapInitData;
			kMapInitData.m_bWrapX = false;
			kMapInitData.m_bWrapY = false;
			kMapInitData.m_iGridW = 1;
			kMapInitData.m_iGridH = 1;
			kMap.init(&kMapInitData);

			CvBarbarians::MapInit(kMap.numPlots());

			CvWorldBuilderMapLoader::ValidateTerrain();
		}
	}
	else
	{
		const CvString& strMapName = CvPreGame::mapScriptName();

		CvMapGenerator* pGenerator = FNEW(CvMapGenerator(strMapName.c_str()), c_eMPoolTypeGame, 0);

		CvMapInitData kData;
		if(pGenerator->GetMapInitData(kData, CvPreGame::worldSize()))
			kMap.init(&kData);
		else
			kMap.init();

		CvBarbarians::MapInit(kMap.numPlots());

		pGenerator->GenerateRandomMap();
		pGenerator->GetGameInitialItemsOverrides(kGameInitialItemsOverrides);

		delete pGenerator;
	}

	CvBarbarians::MapInit(GC.getMap().numPlots());

	// Run this for all maps because a map should never crash the game on
	// load regardless of where that map came from.  (The map scripts are mod-able after all!!!)
	CvWorldBuilderMapLoader::ValidateCoast();

	// Update some cached values
	GC.getMap().updateAdjacency();

	// Set all the observer teams to be able to see all the plots
	for(int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (CvPreGame::slotStatus((PlayerTypes)iI) == SS_OBSERVER)
		{
			CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)iI);
			SetAllPlotsVisible( kPlayer.getTeam() );
		}
	}

	return true;
}

void PrintPlayerInfo(int iIndex)
{
	const char* slotStatus[] = { "open","computer","closed","taken","observer" };
	const char* slotClaim[] = { "unassigned","reserved","assigned" };

	CvString msg = CvString::format( "Slot %d is %s and %s. Player is %s and %s. Team is %s.\n", 
		iIndex, slotStatus[ CvPreGame::slotStatus((PlayerTypes)iIndex) ], slotClaim[ CvPreGame::slotClaim((PlayerTypes)iIndex) ],
		GET_PLAYER((PlayerTypes)iIndex).isMajorCiv() ? "major" : "minor",
		GET_PLAYER((PlayerTypes)iIndex).isAlive() ? "alive" : "dead",
		GET_TEAM( GET_PLAYER((PlayerTypes)iIndex).getTeam() ).isObserver() ? "observer" : "player"
		);
	OutputDebugString(msg.c_str());
};

//------------------------------------------------------------------------------
void CvGame::InitPlayers()
{
	CivilizationTypes eBarbCiv = (CivilizationTypes)GD_INT_GET(BARBARIAN_CIVILIZATION);
	CvCivilizationInfo* pBarbarianCivilizationInfo = GC.getCivilizationInfo(eBarbCiv);
	PlayerColorTypes barbarianPlayerColor = (PlayerColorTypes)pBarbarianCivilizationInfo->getDefaultPlayerColor();
	int iNumMinors = CvPreGame::numMinorCivs();
	CivilizationTypes eMinorCiv = (CivilizationTypes)GD_INT_GET(MINOR_CIVILIZATION);
	LeaderHeadTypes eBarbLeader = (LeaderHeadTypes)GD_INT_GET(BARBARIAN_LEADER);
	HandicapTypes eAIHandicap = (HandicapTypes)GD_INT_GET(AI_HANDICAP);
	HandicapTypes eCSHandicap = (HandicapTypes)GD_INT_GET(MINOR_CIV_HANDICAP);
	bool bNoReligion = GC.getGame().isOption(GAMEOPTION_NO_RELIGION);
	bool bNoPolicies = GC.getGame().isOption(GAMEOPTION_NO_POLICIES);

	// Init teams
	for (int iI = 0; iI < MAX_TEAMS; iI++)
	{
		const TeamTypes eTeam(static_cast<TeamTypes>(iI));
		CvTeam& kTeam = GET_TEAM(eTeam);
		kTeam.init(eTeam);
	}

	// Determine player colors and don't allow duplicates.
	PlayerColorTypes aePlayerColors[MAX_MAJOR_CIVS];
	vector<PlayerColorTypes> vColorsAlreadyUsed;
	for (int iI = 0; iI < MAX_MAJOR_CIVS; iI++)
	{
		const PlayerTypes eLoopPlayer = static_cast<PlayerTypes>(iI);
		PlayerColorTypes ePlayerColor = NO_PLAYERCOLOR;
		SlotStatus eStatus = CvPreGame::slotStatus(eLoopPlayer);
		if (eStatus == SS_TAKEN || eStatus == SS_COMPUTER) // Don't set colors for unoccupied slots.
		{
			ePlayerColor = CvPreGame::playerColor(eLoopPlayer);

			// If it wasn't set in the pregame for some reason, fetch it from the database.
			if (ePlayerColor == NO_PLAYERCOLOR)
			{
				CvCivilizationInfo* pCivilizationInfo = GC.getCivilizationInfo(CvPreGame::civilization(eLoopPlayer));
				ePlayerColor = (PlayerColorTypes)pCivilizationInfo->getDefaultPlayerColor();
			}
		}

		if (ePlayerColor == NO_PLAYERCOLOR || ePlayerColor == barbarianPlayerColor)
		{
			aePlayerColors[iI] = NO_PLAYERCOLOR;
			continue;
		}

		// Check if duplicated.
		if (std::find(vColorsAlreadyUsed.begin(), vColorsAlreadyUsed.end(), ePlayerColor) == vColorsAlreadyUsed.end())
		{
			aePlayerColors[iI] = ePlayerColor;
			vColorsAlreadyUsed.push_back(ePlayerColor);
		}
		else
		{
			aePlayerColors[iI] = NO_PLAYERCOLOR;
		}
	}

	for (int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		// init Barbarian slot
		if (iI == BARBARIAN_PLAYER)
		{
			CvPreGame::setTeamType(BARBARIAN_PLAYER, BARBARIAN_TEAM);
			CvPreGame::setSlotStatus(BARBARIAN_PLAYER, SS_COMPUTER);
			CvPreGame::setNetID(BARBARIAN_PLAYER, -1);
			CvPreGame::setHandicap(BARBARIAN_PLAYER, (HandicapTypes)GD_INT_GET(BARBARIAN_HANDICAP));
			CvPreGame::setCivilization(BARBARIAN_PLAYER, eBarbCiv);
			CvPreGame::setLeaderHead(BARBARIAN_PLAYER, eBarbLeader);
			CvPreGame::setPlayerColor(BARBARIAN_PLAYER, barbarianPlayerColor);
			CvPreGame::setMinorCiv(BARBARIAN_PLAYER, false);
		}
		// Major Civs - set player color and AI Handicap
		else if (iI < MAX_MAJOR_CIVS)
		{
			const PlayerTypes eLoopPlayer = static_cast<PlayerTypes>(iI);
			PlayerColorTypes ePlayerColor = aePlayerColors[iI];
			if (ePlayerColor == NO_PLAYERCOLOR)
			{
				// search for an unused player color
				for (int iK = 0; iK < GC.GetNumPlayerColorInfos(); iK++)
				{
					if (std::find(vColorsAlreadyUsed.begin(), vColorsAlreadyUsed.end(), (PlayerColorTypes)iK) == vColorsAlreadyUsed.end())
					{
						ePlayerColor = (PlayerColorTypes)iK;
						vColorsAlreadyUsed.push_back(ePlayerColor);
						break;
					}
				}
			}
			CvPreGame::setPlayerColor(eLoopPlayer, ePlayerColor);
			// Make sure the AI has the proper handicap.
			if (CvPreGame::slotStatus(eLoopPlayer) == SS_COMPUTER)
				CvPreGame::setHandicap(eLoopPlayer, eAIHandicap);
		}
		// Minor civs
		else if (iI < MAX_CIV_PLAYERS)
		{
			const PlayerTypes eLoopPlayer = static_cast<PlayerTypes>(iI);
			CvPreGame::setNetID(eLoopPlayer, -1);
			CvPreGame::setHandicap(eLoopPlayer, eCSHandicap);
			CvPreGame::setCivilization(eLoopPlayer, eMinorCiv);
			CvPreGame::setLeaderHead(eLoopPlayer, eBarbLeader);
			CvPreGame::setMinorCiv(eLoopPlayer, true);
			MinorCivTypes minorCivType = CvPreGame::minorCivType(eLoopPlayer);
			CvMinorCivInfo* pkCityState = GC.getMinorCivInfo(minorCivType);

			// No valid City-State specified in pregame. Default to the first valid index and close the slot.
			if (minorCivType == NO_MINORCIV || !pkCityState)
			{
				CvPreGame::setSlotStatus(eLoopPlayer, SS_CLOSED);
				bool bFoundOne = false;
				for (int i = 0; i < GC.getNumMinorCivInfos(); i++)
				{
					MinorCivTypes eType = static_cast<MinorCivTypes>(i);
					pkCityState = GC.getMinorCivInfo(eType);
					if (eType != NO_MINORCIV && pkCityState)
					{
						MinorCivTraitTypes eTrait = (MinorCivTraitTypes)pkCityState->GetMinorCivTrait();
						if ((eTrait != MINOR_CIV_TRAIT_RELIGIOUS || !bNoReligion) && (eTrait != MINOR_CIV_TRAIT_CULTURED || !bNoPolicies))
						{
							CvPreGame::setPlayerColor(eLoopPlayer, (PlayerColorTypes)pkCityState->getDefaultPlayerColor());
							CvPreGame::setMinorCivType(eLoopPlayer, eType);
							bFoundOne = true;
							break;
						}
					}
				}
				if (!bFoundOne)
					CvPreGame::setPlayerColor(eLoopPlayer, NO_PLAYERCOLOR);

				continue;
			}

			// If this slot is within the range specified by the players at game start, the City-State is alive.
			// We will fill in the slot data anyway, however, as the "closed" City-States might be created as a free City-State later on.
			if (iI < MAX_MAJOR_CIVS + iNumMinors)
				CvPreGame::setSlotStatus(eLoopPlayer, SS_COMPUTER);
			else
				CvPreGame::setSlotStatus(eLoopPlayer, SS_CLOSED);

			CvPreGame::setPlayerColor(eLoopPlayer, (PlayerColorTypes)pkCityState->getDefaultPlayerColor());
		}
	}

	// Init players
	for (int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		const PlayerTypes ePlayer = static_cast<PlayerTypes>(iI);
		CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
		kPlayer.init(ePlayer);
	}

	for (int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		PrintPlayerInfo(iI);
	}
}

//	--------------------------------------------------------------------------------
//
// Set initial items (units, techs, etc...)
//
void CvGame::setInitialItems(CvGameInitialItemsOverrides& kInitialItemOverrides)
{
	initFreeState(kInitialItemOverrides);

	if(CvPreGame::isWBMapScript())
		assignStartingPlots();

	// Adjust FLAVOR_GROWTH and FLAVOR_EXPANSION based on map size
	for(int iPlayerLoop = 0; iPlayerLoop < MAX_CIV_PLAYERS; iPlayerLoop++)
	{
		CvPlayer& kPlayer = GET_PLAYER((PlayerTypes) iPlayerLoop);

		if (kPlayer.isAlive() && kPlayer.isMajorCiv())
		{
			kPlayer.GetFlavorManager()->AdjustWeightsForMap();
		}
	}

	initFreeUnits(kInitialItemOverrides);

	if (MOD_BALANCE_VP)
	{
		int iBarbReleaseTurn = getHandicapInfo().getEarliestBarbarianReleaseTurn();
		int iPlusMinus = /*2*/ GD_INT_GET(AI_TACTICAL_BARBARIAN_RELEASE_VARIATION);

		if (iBarbReleaseTurn <= 0)
			m_iEarliestBarbarianReleaseTurn = 0;
		else if (iPlusMinus == 0)
			m_iEarliestBarbarianReleaseTurn = iBarbReleaseTurn;
		else
			m_iEarliestBarbarianReleaseTurn = max(0, GC.getGame().randRangeInclusive(iBarbReleaseTurn - iPlusMinus, iBarbReleaseTurn + iPlusMinus, CvSeeder::fromRaw(0x07f63322)));
	}
	else
		m_iEarliestBarbarianReleaseTurn = max(0, getHandicapInfo().getEarliestBarbarianReleaseTurn() + GC.getGame().randRangeInclusive(0, /*15*/ GD_INT_GET(AI_TACTICAL_BARBARIAN_RELEASE_VARIATION), CvSeeder::fromRaw(0x4602dd5b)));

	UpdateGameEra();
	// What route type forms an industrial connection
	DoUpdateIndustrialRoute();

	bool bCanWorkWater = /*1*/ GD_INT_GET(CAN_WORK_WATER_FROM_GAME_START)>0;

	// Team Stuff
	for (int iTeamLoop = 0; iTeamLoop < MAX_CIV_TEAMS; iTeamLoop++)
	{
		TeamTypes eTeam = (TeamTypes) iTeamLoop;

		if(bCanWorkWater)
		{
			GET_TEAM(eTeam).changeWaterWorkCount(1);
		}

		GET_TEAM(eTeam).DoUpdateBestRoute();
	}

	// Player Stuff
	for (int iPlayerLoop = 0; iPlayerLoop < MAX_CIV_PLAYERS; iPlayerLoop++)
	{
		PlayerTypes ePlayer = (PlayerTypes) iPlayerLoop;

		if(GET_PLAYER(ePlayer).isAlive())
		{
			// Major Civ init
			if (GET_PLAYER(ePlayer).isMajorCiv())
			{
				GET_PLAYER(ePlayer).GetDiplomacyAI()->DoInitializePersonality(true);
				GET_PLAYER(ePlayer).SetMilitaryRating(GetStartingMilitaryRating());
			}
			// Minor Civ init
			else if (GET_PLAYER(ePlayer).isMinorCiv())
			{
				GET_PLAYER(ePlayer).GetMinorCivAI()->DoPickInitialItems();
			}

			// Set Policy Costs before game starts, or else it'll be 0 on the first turn and Players can get something with any amount!
			GET_PLAYER(ePlayer).DoUpdateNextPolicyCost();
		}
	}

	DoCacheMapScoreMod();

	// Diplomacy Victory
	DoInitDiploVictory();

	LogGameState();
}

void CvGame::CheckGenerateArchaeology()
{
	bool bTriggered = false;

	// See if archaeological data should be triggered
	for(int iTeamLoop = 0; iTeamLoop < MAX_CIV_TEAMS && !bTriggered; iTeamLoop++)
	{
		const TeamTypes eTeam1 = (TeamTypes)iTeamLoop;
		CvTeam& kTeam1 = GET_TEAM(eTeam1);

		if (kTeam1.isAlive() && !kTeam1.isMinorCiv())
		{
			for (int iTech = 0; iTech < GC.getNumTechInfos() && !bTriggered; iTech++)
			{
				CvTechEntry *pkTech = GC.getTechInfo((TechTypes)iTech);
				if (pkTech)
				{
					if (kTeam1.GetTeamTechs()->HasTech((TechTypes)pkTech->GetID()))
					{
						if (pkTech->IsTriggersArchaeologicalSites())
						{
							GC.getGame().TriggerArchaeologySiteCreation(false /*bCheckInitialized*/);
							bTriggered = true;
						}
					}
				}
			}
		}
	}
}

//	--------------------------------------------------------------------------------
void CvGame::regenerateMap()
{
	int iI = 0;

	if(CvPreGame::isWBMapScript())
	{
		return;
	}

	setFinalInitialized(false);

	for(iI = 0; iI < MAX_PLAYERS; iI++)
	{
		GET_PLAYER((PlayerTypes)iI).killUnits();
	}

	for(iI = 0; iI < MAX_PLAYERS; iI++)
	{
		GET_PLAYER((PlayerTypes)iI).killCities();
	}

	for(iI = 0; iI < MAX_PLAYERS; iI++)
	{
		GC.getGame().GetGameDeals().DoCancelAllDealsWithPlayer((PlayerTypes) iI);
	}

	for(iI = 0; iI < MAX_PLAYERS; iI++)
	{
		GET_PLAYER((PlayerTypes)iI).ChangeNumCitiesFounded(-GET_PLAYER((PlayerTypes)iI).GetNumCitiesFounded());
		GET_PLAYER((PlayerTypes)iI).setStartingPlot(NULL);
	}

	for(iI = 0; iI < MAX_TEAMS; iI++)
	{
		GC.getMap().setRevealedPlots(((TeamTypes)iI), false);
	}

	GC.getMap().erasePlots();

	CvGameInitialItemsOverrides kOverrides;

	setInitialItems(kOverrides);
	CheckGenerateArchaeology();
	initScoreCalculation();
	setFinalInitialized(true);

	GC.getMap().setupGraphical();
	GC.GetEngineUserInterface()->setDirty(ColoredPlots_DIRTY_BIT, true);

	GC.GetEngineUserInterface()->setCycleSelectionCounter(1);

	gDLL->AutoSave(true);
}


//	--------------------------------------------------------------------------------
void CvGame::DoGameStarted()
{
	//consistency check for map
	for(int iI = 0; iI < GC.getMap().numPlots(); iI++)
	{
		CvPlot* pLoopPlot = GC.getMap().plotByIndexUnchecked(iI);
		pLoopPlot->Validate( GC.getMap() );
	}

	for(int iFeatureLoop = 0; iFeatureLoop < GC.getNumFeatureInfos(); iFeatureLoop++)
	{
		FeatureTypes eFeature = (FeatureTypes) iFeatureLoop;
		bool bTempClearable = false;

		// Check unit build actions to see if any of them clear this feature
		for(int iBuildLoop = 0; iBuildLoop < GC.getNumBuildInfos(); iBuildLoop++)
		{
			BuildTypes eBuild = (BuildTypes) iBuildLoop;
			CvBuildInfo* pBuildInfo = GC.getBuildInfo(eBuild);

			// Feature can be removed by this build
			if(NULL != pBuildInfo && pBuildInfo->isFeatureRemove(eFeature))
			{
				GC.getFeatureInfo(eFeature)->SetClearable(true);

				bTempClearable = true;
				break;
			}

			if(bTempClearable)
				break;
		}
	}

	//performance optimization for city production AI
	int iNumBuildingInfos = GC.getNumBuildingInfos();
	for (int iI = 0; iI < iNumBuildingInfos; iI++)
	{
		BuildingTypes eBuilding = (BuildingTypes)iI;
		CvBuildingEntry* pBuilding = GC.getBuildingInfo(eBuilding);
		if (pBuilding)
			pBuilding->UpdateUnitTypesUnlocked();
	}

	GET_PLAYER(getActivePlayer()).GetUnitCycler().Rebuild();

#if defined(MOD_BALANCE_CORE)
	CvPlayerManager::Refresh(false);
#endif
}



//	--------------------------------------------------------------------------------
void CvGame::uninit()
{
	CvGoodyHuts::Uninit();
	CvBarbarians::Uninit();

	m_aiGreatestMonopolyPlayer.uninit();

	m_aiEndTurnMessagesReceived.uninit();
	m_aiRankPlayer.uninit();
	m_aiPlayerRank.uninit();
	m_aiPlayerScore.uninit();

	m_aiRankTeam.uninit();
	m_aiTeamRank.uninit();
	m_aiTeamScore.uninit();

	m_paiUnitCreatedCount.uninit();
	m_paiUnitClassCreatedCount.uninit();
	m_paiBuildingClassCreatedCount.uninit();
	m_paiProjectCreatedCount.uninit();
	m_aiVotesCast.uninit();
	m_aiPreviousVotesCast.uninit();
	m_aiNumVotesForTeam.uninit();
	m_aiTeamCompetitionWinnersScratchPad.uninit();

	m_pabSpecialUnitValid.uninit();

	if(m_ppaaiTeamVictoryRank.valid())
	{
		for(CvEnumMap<VictoryTypes, TeamTypes*>::Iterator it = m_ppaaiTeamVictoryRank.begin(); it != m_ppaaiTeamVictoryRank.end(); ++it)
		{
			SAFE_DELETE_ARRAY(*it);
		}
		m_ppaaiTeamVictoryRank.uninit();
	}
#if defined(MOD_BALANCE_CORE_JFD)
	if(m_ppaiContractUnits.valid())
	{
		for(CvEnumMap<ContractTypes, CvEnumMap<UnitTypes, int>>::Iterator it = m_ppaiContractUnits.begin(); it != m_ppaiContractUnits.end(); ++it)
		{
			it->uninit();
		}
		m_ppaiContractUnits.uninit();
	}
#endif

	m_aszDestroyedCities.clear();
	m_aszGreatPeopleBorn.clear();

	m_mapRand.uninit();
	m_jonRand.uninit();

	clearReplayMessageMap();

	m_aPlotExtraYields.clear();
	m_aPlotExtraCosts.clear();

	SAFE_DELETE(m_pDiploResponseQuery);

	SAFE_DELETE(m_pSettlerSiteEvaluator);
	SAFE_DELETE(m_pStartSiteEvaluator);
	SAFE_DELETE(m_pStartPositioner);
	SAFE_DELETE(m_pGameReligions);
	SAFE_DELETE(m_pGameCulture);
	SAFE_DELETE(m_pGameLeagues);
	SAFE_DELETE(m_pGameTrade);

#if defined(MOD_BALANCE_CORE)
	SAFE_DELETE(m_pGameCorporations);
	SAFE_DELETE(m_pGameContracts);
#endif

	SAFE_DELETE(m_pAdvisorCounsel);
	SAFE_DELETE(m_pAdvisorRecommender);

	m_bForceEndingTurn = false;

	m_lastTurnAICivsProcessed = -1;
	m_processPlayerAutoMoves = false;
	m_iEndTurnMessagesSent = 0;
	m_iElapsedGameTurns = 0;
	m_iStartTurn = 0;
	m_iWinningTurn = 0;
	m_iStartYear = 0;
	m_iEstimateEndTurn = 0;
	m_iDefaultEstimateEndTurn = 0;
	m_iTurnSlice = 0;
	m_iCutoffSlice = 0;
	m_iNumCities = 0;
	m_iTotalPopulation = 0;
	m_iTotalEconomicValue = 0;
	m_iHighestEconomicValue = 0;
	m_iMedianEconomicValue = 0;
	m_iNoNukesCount = 0;
	m_iNukesExploded = 0;
	m_iMaxPopulation = 0;
	m_iInitPopulation = 0;
	m_iInitLand = 0;
	m_iInitTech = 0;
	m_iInitWonders = 0;
	m_iAIAutoPlay = 0;
	m_iUnitedNationsCountdown = 0;
	m_iNumVictoryVotesTallied = 0;
	m_iNumVictoryVotesExpected = 0;
	m_iVotesNeededForDiploVictory = 0;
	m_iMapScoreMod = 0;
	m_iCityFoundValueReference = 0;
	m_iNumReferenceCities = 0;
	m_iNumMajorCivsAliveAtGameStart = 0;
	m_iNumMinorCivsAliveAtGameStart = 0;

	m_uiInitialTime = 0;

	m_bScoreDirty = false;
	m_bCircumnavigated = false;
	m_eTechAstronomy = NO_TECH;
	m_bDebugMode = false;
	m_bDebugModeCache = false;
	m_bFOW = true;
#ifdef EA_EVENT_GAME_SAVE
	m_bSavedOnce = false;
#endif
	m_bFinalInitialized = false;
	m_eWaitDiploPlayer = NO_PLAYER;
	m_bPbemTurnSent = false;
	m_bHotPbemBetweenTurns = false;
	m_bPlayerOptionsSent = false;
	m_bNukesValid = false;
	m_bEndGameTechResearched = false;
	m_bTunerEverConnected = false;
	m_bTutorialEverAttacked = false;
	m_bStaticTutorialActive = false;
	m_bEverRightClickMoved = false;
	m_bCombatWarned = false;
	m_bArchaeologyTriggered = false;
	m_bIsDesynced = false;
	m_eObserverUIOverridePlayer = NO_PLAYER;
	m_eCurrentVisibilityPlayer = NO_PLAYER;

	m_eHandicap = NO_HANDICAP;
	m_ePausePlayer = NO_PLAYER;
	m_eAIAutoPlayReturnPlayer = NO_PLAYER;
	m_eBestLandUnit = NO_UNIT;
	m_eWinner = NO_TEAM;
	m_eVictory = NO_VICTORY;
	m_eGameState = GAMESTATE_ON;
	m_eBestWondersPlayer = NO_PLAYER;
	m_eBestPoliciesPlayer = NO_PLAYER;
	m_eBestGreatPeoplePlayer = NO_PLAYER;
	m_eIndustrialRoute = NO_ROUTE;
	m_eGameEra = NO_ERA;
	m_eTeamThatCircumnavigated = NO_TEAM;
	m_bVictoryRandomization = false;

	m_iMedianTechsResearched = 0;
	m_iBasicNeedsMedian = 0;
	m_iGoldMedian = 0;
	m_iScienceMedian = 0;
	m_iCultureMedian = 0;

	m_iSpyThreshold = 0;

	m_iLastTurnCSSurrendered = 0;

	m_strScriptData = "";
	m_iEarliestBarbarianReleaseTurn = 0;

	m_iLastMouseoverUnitID = 0;

	CvCityManager::Shutdown();
}

//	--------------------------------------------------------------------------------
// FUNCTION: reset()
// Initializes data members that are serialized.
void CvGame::reset(HandicapTypes eHandicap, bool bConstructorCall)
{
	//--------------------------------
	// Uninit class
	uninit();

	m_fCurrentTurnTimerPauseDelta = 0.f;

	CvString strUTF8DatabasePath = gDLL->GetCacheFolderPath();
	strUTF8DatabasePath += "Civ5SavedGameDatabase.db";

	std::wstring wstrDatabasePath = CvStringUtils::FromUTF8ToUTF16(strUTF8DatabasePath);

	if(DeleteFileW(wstrDatabasePath.c_str()) == FALSE)
	{
		if(GetLastError() != ERROR_FILE_NOT_FOUND)
		{
			ASSERT_DEBUG(false, "Warning! Cannot delete existing Civ5SavedGameDatabase! Does something have it opened?");
		}
	}

	Database::Connection db;
	if(db.Open(strUTF8DatabasePath.c_str(), Database::OPEN_CREATE | Database::OPEN_READWRITE | Database::OPEN_FULLMUTEX))
	{
		db.Execute("CREATE TABLE SimpleValues(Name TEXT Primary Key, Value VARIANT)");
	}
	else
	{
		ASSERT_DEBUG(false, "Warning! Cannot create new Civ5SavedGameDatabase.");
	}


	m_eHandicap = eHandicap;

	if(!bConstructorCall)
	{
		m_aiGreatestMonopolyPlayer.init(NO_PLAYER);

		m_aiEndTurnMessagesReceived.init(0);
		m_aiRankPlayer.init(0);
		m_aiPlayerRank.init(0);
		m_aiPlayerScore.init(0);

		m_aiRankTeam.init(0);
		m_aiTeamRank.init(0);
		m_aiTeamScore.init(0);

		m_paiUnitCreatedCount.init(0);
		m_paiUnitClassCreatedCount.init(0);
		m_paiBuildingClassCreatedCount.init(0);
		m_paiProjectCreatedCount.init(0);

		//antonjs: todo: remove unused UN and voting variables and allocations
		ASSERT_DEBUG(0 < GC.getNumVoteInfos(), "GC.getNumVoteInfos() is not greater than zero in CvGame::reset");

		ASSERT_DEBUG(0 < GC.getNumVoteSourceInfos(), "GC.getNumVoteSourceInfos() is not greater than zero in CvGame::reset");
		m_aiVotesCast.init(NO_TEAM);
		m_aiPreviousVotesCast.init(NO_TEAM);
		m_aiNumVotesForTeam.init(0);
		m_aiTeamCompetitionWinnersScratchPad.init(0);
		m_pabSpecialUnitValid.init(false);

		m_ppaaiTeamVictoryRank.init();
		for(CvEnumMap<VictoryTypes, TeamTypes*>::Iterator it = m_ppaaiTeamVictoryRank.begin(); it != m_ppaaiTeamVictoryRank.end(); ++it)
		{
			*it = FNEW(TeamTypes[/*5*/ GD_INT_GET(NUM_VICTORY_POINT_AWARDS)], c_eCiv5GameplayDLL, 0);
			for(int i = 0; i < /*5*/ GD_INT_GET(NUM_VICTORY_POINT_AWARDS); ++i)
			{
				(*it)[i] = NO_TEAM;
			}
		}
#if defined(MOD_BALANCE_CORE_JFD)
		m_ppaiContractUnits.init();
		for(CvEnumMap<ContractTypes, CvEnumMap<UnitTypes, int>>::Iterator it = m_ppaiContractUnits.begin(); it != m_ppaiContractUnits.end(); ++it)
		{
			it->init(0);
		}
#endif

		ASSERT_DEBUG(m_pSettlerSiteEvaluator==NULL, "about to leak memory, CvGame::m_pSettlerSiteEvaluator");
		m_pSettlerSiteEvaluator = FNEW(CvSiteEvaluatorForSettler, c_eCiv5GameplayDLL, 0);

		ASSERT_DEBUG(m_pStartSiteEvaluator==NULL, "about to leak memory, CvGame::m_pStartSiteEvaluator");
		m_pStartSiteEvaluator = FNEW(CvCitySiteEvaluator, c_eCiv5GameplayDLL, 0);

		ASSERT_DEBUG(m_pStartPositioner==NULL, "about to leak memory, CvGame::m_pStartPositioner");
		m_pStartPositioner = new CvStartPositioner(m_pStartSiteEvaluator);

		m_kGameDeals.Init();

		ASSERT_DEBUG(m_pGameReligions==NULL, "about to leak memory, CvGame::m_pGameReligions");
		m_pGameReligions = FNEW(CvGameReligions, c_eCiv5GameplayDLL, 0);
		m_pGameReligions->Init();

#if defined(MOD_BALANCE_CORE)
		ASSERT_DEBUG(m_pGameCorporations==NULL, "about to leak memory, CvGame::m_pGameCorporations");
		m_pGameCorporations = FNEW(CvGameCorporations, c_eCiv5GameplayDLL, 0);
		m_pGameCorporations->Init();

		ASSERT_DEBUG(m_pGameContracts==NULL, "about to leak memory, CvGame::m_pGameContracts");
		m_pGameContracts = FNEW(CvGameContracts, c_eCiv5GameplayDLL, 0);
		m_pGameContracts->Init();
#endif

		ASSERT_DEBUG(m_pGameCulture==NULL, "about to leak memory, CvGame::m_pGameCulture");
		m_pGameCulture = FNEW(CvGameCulture, c_eCiv5GameplayDLL, 0);

		ASSERT_DEBUG(m_pGameLeagues==NULL, "about to leak memory, CvGame::m_pGameLeagues");
		m_pGameLeagues = FNEW(CvGameLeagues, c_eCiv5GameplayDLL, 0);
		m_pGameLeagues->Init();

		ASSERT_DEBUG(m_pGameTrade==NULL, "about to leak memory, CvGame::m_pGameTrade");
		m_pGameTrade = FNEW(CvGameTrade, c_eCiv5GameplayDLL, 0);
		m_pGameTrade->Init();

		ASSERT_DEBUG(m_pAdvisorCounsel==NULL, "about to leak memory, CvGame::m_pAdvisorCounsel");
		m_pAdvisorCounsel = FNEW(CvAdvisorCounsel, c_eCiv5GameplayDLL, 0);

		ASSERT_DEBUG(m_pAdvisorRecommender==NULL, "about to leak memory, CvGame::m_pAdvisorRecommender");
		m_pAdvisorRecommender = FNEW(CvAdvisorRecommender, c_eCiv5GameplayDLL, 0);

		m_eTechAstronomy = (TechTypes)GC.getInfoTypeForString("TECH_ASTRONOMY");
	}

	m_mapRand.reset();
	m_jonRand.reset();

	m_iNumSessions = 1;

	m_AdvisorMessagesViewed.clear();

	CvCityManager::Reset();

	m_iGlobalAssetCounterAllPreviousTurns = 1000; //0 is invalid
	m_iGlobalAssetCounterCurrentTurn = 0;
}

//	--------------------------------------------------------------------------------
/// Initial diplomacy State: right now this just has all teams meet themselves and sets them at war with the Barbs
void CvGame::initDiplomacy()
{
	for(int iI = 0; iI < MAX_TEAMS; iI++)
	{
		const TeamTypes eTeamA = static_cast<TeamTypes>(iI);
		CvTeam& kTeamA = GET_TEAM(eTeamA);
		kTeamA.meet(eTeamA, false);

		if(kTeamA.isBarbarian())
		{
			for(int iJ = 0; iJ < MAX_CIV_TEAMS; iJ++)
			{
				const TeamTypes eTeamB = static_cast<TeamTypes>(iJ);
				if(iI != iJ)
				{
					kTeamA.declareWar(eTeamB, false, kTeamA.getLeaderID());
				}
			}
		}

		if(kTeamA.isObserver())
		{
			for(int iJ = 0; iJ < MAX_CIV_TEAMS; iJ++)
			{
				const TeamTypes eTeamB = static_cast<TeamTypes>(iJ);
				if(iI != iJ)
					kTeamA.makeHasMet(eTeamB,true);
			}
		}
	}
}


//	--------------------------------------------------------------------------------
void CvGame::initFreeState(CvGameInitialItemsOverrides& kOverrides) const
{
	for(int iI = 0; iI < GC.getNumTechInfos(); iI++)
	{
		const TechTypes eTech = static_cast<TechTypes>(iI);
		CvTechEntry* pkTechInfo = GC.getTechInfo(eTech);
		if(pkTechInfo)
		{
			for(int iJ = 0; iJ < MAX_TEAMS; iJ++)
			{
				const TeamTypes eTeam = static_cast<TeamTypes>(iJ);
				const bool bGrantFreeTechs = kOverrides.GrantInitialFreeTechsPerTeam[eTeam];

				if(bGrantFreeTechs)
				{
					CvTeam& kTeam = GET_TEAM(eTeam);
					if(kTeam.isAlive())
					{
						// Skip if we already have it
						if (kTeam.GetTeamTechs()->HasTech(eTech))
						{
							continue;
						}

						bool bValid = false;

						if(!bValid)
						{
							if((getHandicapInfo().isFreeTechs(iI)) ||
							        (!(kTeam.isHuman())&& getHandicapInfo().isAIFreeTechs(iI)) ||
							        (pkTechInfo->GetEra() < getStartEra()))
							{
								bValid = true;
							}
						}

						if(!bValid)
						{
							for(int iK = 0; iK < MAX_PLAYERS; iK++)
							{
								CvPlayerAI& kPlayer = GET_PLAYER((PlayerTypes)iK);
								if(kPlayer.isAlive())
								{
									if(kPlayer.getTeam() == eTeam)
									{
										if(kPlayer.getCivilizationInfo().isCivilizationFreeTechs(iI))
										{
											bValid = true;
											break;
										}
									}
								}
							}
						}

						kTeam.setHasTech(eTech, bValid, NO_PLAYER, false, false);
						if(bValid && pkTechInfo->IsMapVisible())
						{
							GC.getMap().setRevealedPlots(eTeam, true, true);
							GC.getMap().updateDeferredFog();
						}
					}
				}
			}
		}
	}

	for(int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		const PlayerTypes ePlayer = static_cast<PlayerTypes>(iI);
		CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
		if(kPlayer.isAlive())
		{
			kPlayer.initFreeState(kOverrides);
		}
	}
}


//	--------------------------------------------------------------------------------
void CvGame::initFreeUnits(CvGameInitialItemsOverrides& kOverrides)
{
	for(int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		const PlayerTypes ePlayer = static_cast<PlayerTypes>(iI);
		CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);

		if(kOverrides.GrantInitialUnitsPerPlayer[ePlayer])
		{
			if(kPlayer.isAlive())
			{
				if(kPlayer.GetNumUnitsWithUnitAI(UNITAI_SETTLE,false) == 0 && kPlayer.getNumCities() == 0)
				{
					// if a civ has no starting plot, it has been discarded during map generation due to insufficient space
					if (kPlayer.getStartingPlot())
					{
						kPlayer.initFreeUnits();
						// count the number of major/minor civs alive at game start
						if (kPlayer.isMinorCiv())
						{
							m_iNumMinorCivsAliveAtGameStart++;
						}
						else
						{
							m_iNumMajorCivsAliveAtGameStart++;
						}
					}
					else
					{
						kPlayer.setEverAlive(false);
						kPlayer.setAlive(false);
					}
				}
			}
		}
	}
}

//	--------------------------------------------------------------------------------
void CvGame::assignStartingPlots()
{
	GetStartPositioner()->Run(GetNumMajorCivsAlive());
}

#if defined(EXTERNAL_PAUSING)
bool ExternalPause()
{
	bool bPause = false;

	// wait for an external mutex if it exists to make it easier to see what the AI is doing
	HANDLE hMutex = ::OpenMutex(SYNCHRONIZE, FALSE, "TurnByTurn");
	if (hMutex != NULL)
	{
		if (::WaitForSingleObject(hMutex, 0) == WAIT_OBJECT_0)
		{
			//we acquired the mutex, that means we can continue
			ReleaseMutex(hMutex);
			//also sleep a bit to keep the cpu requirements down if the player is not doing anything
			Sleep(10);
		}
		else
		{
			//couldn't acquire it, we should pause
			bPause = true;
			//sleep a little bit for simple rate limiting
			Sleep(200);
		}
		//close the handle in any case
		CloseHandle(hMutex);
	}

	return bPause;
}
#endif

//	---------------------------------------------------------------------------
void CvGame::update()
{
	if(IsWaitingForBlockingInput())
	{
		if(!GC.GetEngineUserInterface()->isDiploActive())
		{
			GET_PLAYER(m_eWaitDiploPlayer).doTurnPostDiplomacy();
			SetWaitingForBlockingInput(NO_PLAYER);
		}
		else
		{
			return;
		}
	}

	// Send a Lua event at the start of the update
	{
		ICvEngineScriptSystem1* pkScriptSystem = gDLL->GetScriptSystem();
		if(pkScriptSystem)
		{
			CvLuaArgsHandle args;
			bool bResult = false;
			LuaSupport::CallHook(pkScriptSystem, "GameCoreUpdateBegin", args.get(), bResult);
		}
	}

	// if the game is single player, it's ok to block all processing until
	// the user selects an extended match or quits.
	if(getGameState() == GAMESTATE_OVER && !CvPreGame::isNetworkMultiplayerGame())
	{
		testExtendedGame();
	}
	else
	{
		// allow extended games in MP without blocking processing. The game
		// may be "over" for 1 player in a match with more than 2 players,
		// when the player is defeated, for example, but not over for the
		// rest of the players. It may also be over for everyone in the match
		// but they may still have units/cities (science/cultural victories)
		if(getGameState() == GAMESTATE_OVER && CvPreGame::isNetworkMultiplayerGame())
		{
			testExtendedGame();
		}

		{
			sendPlayerOptions();

			//this creates the initial autosave
			if(getTurnSlice() == 0 && !isPaused())
			{
				gDLL->AutoSave(true);
			}

#if defined(EXTERNAL_PAUSING)
			bool bExternalPause = ExternalPause();
#else
			bool bExternalPause = false;
#endif

			// If there are no active players, move on to the AI
			if (!bExternalPause && getNumGameTurnActive() == 0)
			{
				if (gDLL->CanAdvanceTurn())
					doTurn();
			}

			if(!isPaused() && !bExternalPause) // Check for paused again, the doTurn call might have called something that paused the game and we don't want an update to sneak through
			{
				updateScore();

				updateWar();

				updateMoves();

				if(!isPaused()) // And again, the player can change after the automoves and that can pause the game
				{
					updateTimers();

					UpdatePlayers(); // slewis added!

					testAlive();

					if((getAIAutoPlay() == 0) && !(gDLL->GetAutorun()) && GAMESTATE_EXTENDED != getGameState())
					{
						if(CvPreGame::slotStatus(getActivePlayer()) != SS_OBSERVER && !GET_PLAYER(getActivePlayer()).isAlive())
						{
							setGameState(GAMESTATE_OVER);
						}
					}

					CheckPlayerTurnDeactivate();

					changeTurnSlice(1);

					if(GC.getGame().isReallyNetworkMultiPlayer() && MOD_ACTIVE_DIPLOMACY)
					{
						// JdH: humans may have been activated, check for AI diplomacy
						CvDiplomacyRequests::DoAIMPDiplomacyWithHumans();
					}

					gDLL->FlushTurnReminders();
				}
			}
		}
	}

	// Send a Lua event at the end of the update
	{
		ICvEngineScriptSystem1* pkScriptSystem = gDLL->GetScriptSystem();
		if(pkScriptSystem)
		{
			CvLuaArgsHandle args;
			bool bResult = false;
			LuaSupport::CallHook(pkScriptSystem, "GameCoreUpdateEnd", args.get(), bResult);
		}
	}
}

//	---------------------------------------------------------------------------------------------------------
//	Check to see if the player's turn should be deactivated.
//	This occurs when the player has set its EndTurn and its AutoMoves to true
//	and all activity has been completed.
void CvGame::CheckPlayerTurnDeactivate()
{
	for(int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)iI);

		if(kPlayer.isAlive() && kPlayer.isTurnActive())
		{
			if(kPlayer.isEndTurn() || (!kPlayer.isHuman() && !kPlayer.HasActiveDiplomacyRequests()))		// For some reason, AI players don't set EndTurn, why not?
			{
				if(kPlayer.hasProcessedAutoMoves())
				{
					bool bAutoMovesComplete = false;
					if(!(kPlayer.hasBusyUnitOrCity()))
					{
						bAutoMovesComplete = true;

						NET_MESSAGE_DEBUG_OSTR_ALWAYS("CheckPlayerTurnDeactivate() : auto-moves complete for " << kPlayer.getName());
					}
					else if(gDLL->HasReceivedTurnComplete(kPlayer.GetID()))
					{
						bAutoMovesComplete = true;
					}

					if(bAutoMovesComplete)
					{
						kPlayer.setTurnActive(false);

						// Activate the next player
						// This is not done if simultaneous turns is enabled (Networked MP).
						// In that case, the local human is (should be) the player we just deactivated the turn for
						// and the AI players will be activated all at once in CvGame::doTurn, once we have received
						// all the moves from the other human players
						if(!kPlayer.isSimultaneousTurns())
						{
							if((isPbem() || isHotSeat()) && kPlayer.isHuman() && countHumanPlayersAlive() > 1)
							{
								setHotPbemBetweenTurns(true);
							}

							if(isSimultaneousTeamTurns())
							{
								if(!GET_TEAM(kPlayer.getTeam()).isTurnActive())
								{
									for(int iJ = (kPlayer.getTeam() + 1); iJ < MAX_TEAMS; iJ++)
									{
										CvTeam& kTeam = GET_TEAM((TeamTypes)iJ);
										if(kTeam.isAlive() && !kTeam.isSimultaneousTurns())
										{//this team is alive and also running sequential turns.  They're up next!
											kTeam.setTurnActive(true);
											resetTurnTimer(false);
											break;
										}
									}
								}
								else
								{
									CvString logOutput;
									logOutput.Format("CheckPlayerTurnDeactivate(): Next sequential player not set TurnActive due to player(%i)'s team(%i) being TurnActive.", kPlayer.GetID(), kPlayer.getTeam());
									gDLL->netMessageDebugLog(logOutput);
								}
							}
							else
							{
								if(!GC.GetEngineUserInterface()->isDiploActive())
								{
									if(!isHotSeat() || kPlayer.isAlive() || !kPlayer.isHuman())		// If it is a hotseat game and the player is human and is dead, don't advance the player, we want them to get the defeat screen
									{
										for(int iJ = (kPlayer.GetID() + 1); iJ < MAX_PLAYERS; iJ++)
										{
											CvPlayer& kNextPlayer = GET_PLAYER((PlayerTypes)iJ);
											if(kNextPlayer.isAlive() && !kNextPlayer.isSimultaneousTurns())
											{//the player is alive and also running sequential turns.  they're up!
												if(isPbem() && kNextPlayer.isHuman())
												{
													if(!getPbemTurnSent())
													{
														gDLL->sendPbemTurn((PlayerTypes)iJ);
													}
												}
												else
												{
													kNextPlayer.setTurnActive(true);
													resetTurnTimer(false);
												}
												break;
											}
										}
									}
								}
								else
								{
									// KWG: This doesn't actually do anything other than print to the debug log
									changeNumGameTurnActive(1, std::string("Because the diplo screen is blocking I am bumping this up for player ") + getName());
								}
							}
						}
					}
				}
			}
		}
	}
}

//	---------------------------------------------------------------------------------------------------------
void CvGame::updateScore(bool bForce)
{
	if(!isScoreDirty() && !bForce)
		return;

	setScoreDirty(false);

	bool abPlayerScored[MAX_CIV_PLAYERS];
	bool abTeamScored[MAX_CIV_TEAMS];
	int iScore = 0;
	int iBestScore = 0;
	PlayerTypes eBestPlayer;
	TeamTypes eBestTeam;
	int iI = 0;
	int iJ = 0;
	int iK = 0;

	for(iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		abPlayerScored[iI] = false;
	}

	for(iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		iBestScore = MIN_INT;
		eBestPlayer = NO_PLAYER;

		for(iJ = 0; iJ < MAX_CIV_PLAYERS; iJ++)
		{
			if(!abPlayerScored[iJ])
			{
				iScore = GET_PLAYER((PlayerTypes)iJ).GetScore(false);

				if(iScore >= iBestScore)
				{
					iBestScore = iScore;
					eBestPlayer = (PlayerTypes)iJ;
				}
			}
		}

		abPlayerScored[eBestPlayer] = true;

		setRankPlayer(iI, eBestPlayer);
		setPlayerRank(eBestPlayer, iI);
		setPlayerScore(eBestPlayer, iBestScore);

		GET_PLAYER(eBestPlayer).setReplayDataValue("REPLAYDATASET_SCORE", getGameTurn(), iBestScore);
	}

	for(iI = 0; iI < MAX_CIV_TEAMS; iI++)
	{
		abTeamScored[iI] = false;
	}

	for(iI = 0; iI < MAX_CIV_TEAMS; iI++)
	{
		iBestScore = MIN_INT;
		eBestTeam = NO_TEAM;

		for(iJ = 0; iJ < MAX_CIV_TEAMS; iJ++)
		{
			if(!abTeamScored[iJ])
			{
				iScore = GET_TEAM((TeamTypes) iJ).GetScore();

				for(iK = 0; iK < MAX_CIV_PLAYERS; iK++)
				{
					if(GET_PLAYER((PlayerTypes)iK).getTeam() == iJ)
					{
						iScore += getPlayerScore((PlayerTypes)iK);
					}
				}

				if(iScore >= iBestScore)
				{
					iBestScore = iScore;
					eBestTeam = (TeamTypes)iJ;
				}
			}
		}

		abTeamScored[eBestTeam] = true;

		setRankTeam(iI, eBestTeam);
		setTeamRank(eBestTeam, iI);
		setTeamScore(eBestTeam, iBestScore);
	}
}

int CvGame::GetCityQualityReference() const
{
	//the capitals tend to be quite good so put the threshold somewhat lower
	return (54*m_iCityFoundValueReference) / (max(1,m_iNumReferenceCities)*100);
}

void CvGame::NewCapitalFounded(int iFoundValue)
{
	m_iCityFoundValueReference += iFoundValue;
	m_iNumReferenceCities++;
}


//	--------------------------------------------------------------------------------
/// How does the size of the map affect how some of the score components are weighted?
int CvGame::GetMapScoreMod() const
{
	return m_iMapScoreMod;
}

//	--------------------------------------------------------------------------------
void CvGame::DoCacheMapScoreMod()
{
	// Seed with a default value in case someone's removed the Standard worldsize (shame on you!)
	int iBaseNumTiles = 4160;

	// Calculate "base" num tiles for the average map
	WorldSizeTypes eStandardWorld = (WorldSizeTypes) GC.getInfoTypeForString("WORLDSIZE_STANDARD", true);
	if(eStandardWorld == NO_WORLDSIZE)
	{
		Database::SingleResult kResult;
		CvWorldInfo kWorldInfo;
		const bool bResult = DB.SelectAt(kResult, "Worlds", eStandardWorld);
		DEBUG_VARIABLE(bResult);
		ASSERT_DEBUG(bResult, "Cannot find world info.");
		kWorldInfo.CacheResult(kResult);

		iBaseNumTiles = kWorldInfo.getGridWidth();
		iBaseNumTiles *= kWorldInfo.getGridHeight();
	}

	int iNumTilesOnThisMap = GC.getMap().numPlots();

	int iScoreMod = 100 * iBaseNumTiles / iNumTilesOnThisMap;

	// If we're giving a bonus to score, reduce the value, so that one pop isn't worth 30 points or something crazy on really small maps
	if(iScoreMod > 100)
	{
		iScoreMod -= 100;
		iScoreMod /= 3;
		iScoreMod += 100;
	}

	m_iMapScoreMod = iScoreMod;
}


//	--------------------------------------------------------------------------------
void CvGame::updateCitySight(bool bIncrement)
{
	int iI = 0;

	for(iI = 0; iI < MAX_PLAYERS; iI++)
	{
		CvPlayerAI& kPlayer = GET_PLAYER((PlayerTypes)iI);
		if(kPlayer.isAlive())
		{
			kPlayer.updateCitySight(bIncrement);
		}
	}
}

//	--------------------------------------------------------------------------------
void CvGame::updateSelectionList()
{
	if(!GC.GetEngineUserInterface()->DoAutoUnitCycle())
	{
		return;
	}

	CvInterfacePtr<ICvUnit1> pDllHeadSelectedUnit(GC.GetEngineUserInterface()->GetHeadSelectedUnit());
	CvUnit* pkHeadSelectedUnit = GC.UnwrapUnitPointer(pDllHeadSelectedUnit.get());

	if((pkHeadSelectedUnit == NULL) || !(pkHeadSelectedUnit->ReadyToSelect()))
	{
		if(getGameTurn() == 0)
		{
			SelectSettler();	// Auto select the settler on turn 0, helps with multiple humans in the same game (Hot Seat)
		}
	}

	pDllHeadSelectedUnit.reset(GC.GetEngineUserInterface()->GetHeadSelectedUnit());
	pkHeadSelectedUnit = GC.UnwrapUnitPointer(pDllHeadSelectedUnit.get());

	if((pkHeadSelectedUnit == NULL) || !(pkHeadSelectedUnit->ReadyToSelect()))
	{
		int iOriginalPlotIndex = GC.GetEngineUserInterface()->getOriginalPlotIndex();
		CvPlot* pkOriginalPlot = (iOriginalPlotIndex != -1)? GC.getMap().plotByIndex(iOriginalPlotIndex) : NULL;

		if((pkOriginalPlot == NULL) || !(cyclePlotUnits(pkOriginalPlot, true, true, GC.GetEngineUserInterface()->getOriginalPlotCount())))
		{
			CvInterfacePtr<ICvPlot1> pSelectionPlot(GC.GetEngineUserInterface()->getSelectionPlot());
			CvPlot* pkSelectionPlot = GC.UnwrapPlotPointer(pSelectionPlot.get());
			if((pkSelectionPlot == NULL) || !(cyclePlotUnits(pkSelectionPlot, true, true)))
			{
				cycleUnits(true);
			}
		}

		pDllHeadSelectedUnit.reset(GC.GetEngineUserInterface()->GetHeadSelectedUnit());
		pkHeadSelectedUnit = GC.UnwrapUnitPointer(pDllHeadSelectedUnit.get());

		if(pkHeadSelectedUnit != NULL)
		{
			if(!(pkHeadSelectedUnit->ReadyToSelect()))
			{
				GC.GetEngineUserInterface()->ClearSelectionList();
			}
		}
	}
}

//	-----------------------------------------------------------------------------------------------
int s_unitMoveTurnSlice = 0;

bool CvGame::hasTurnTimerExpired(PlayerTypes playerID)
{//gameLoopUpdate - Indicates that we're updating the turn timer for the game loop update.  
 //					This forces the active player's turn to finish if her turn time has elapsed.
 //					We also reset the turn timer when ai processing is occurring.
 //					If false, we're simply querying the game for a player's turn timer status.
	bool gameTurnTimerExpired = false;
	bool isLocalPlayer = getActivePlayer() == playerID;
	if(isOption(GAMEOPTION_END_TURN_TIMER_ENABLED) && !isPaused() && GC.getGame().getGameState() == GAMESTATE_ON)
	{
		ICvUserInterface2* iface = GC.GetEngineUserInterface();
		if(getElapsedGameTurns() > 0)
		{
			if(isLocalPlayer && (!gDLL->allAICivsProcessedThisTurn() || !allUnitAIProcessed()))
			{//the turn timer doesn't doesn't start until all ai processing has been completed for this game turn.
				resetTurnTimer(true);

				//hold the turn timer at 0 seconds with 0% completion
				CvPreGame::setEndTurnTimerLength(0.0f);
				iface->updateEndTurnTimer(0.0f);
			}
			else
			{//turn timer is actively ticking.
				if(playerID == NO_PLAYER)
				{//can't do a turn timer check for an invalid player.
					return false;
				}
				CvPlayer& curPlayer = GET_PLAYER(playerID);

				// Has the turn expired?
				float gameTurnEnd = static_cast<float>(getMaxTurnLen());

				//NOTE:  These times exclude the time used for AI processing.
				//Time since the current player's turn started.  Used for measuring time for players in sequential turn mode.
				float timeSinceCurrentTurnStart = m_curTurnTimer.Peek() + m_fCurrentTurnTimerPauseDelta; 
				//Time since the game (year) turn started.  Used for measuring time for players in simultaneous turn mode.
				float timeSinceGameTurnStart = m_timeSinceGameTurnStart.Peek() + m_fCurrentTurnTimerPauseDelta; 
				
				float timeElapsed = (curPlayer.isSimultaneousTurns() ? timeSinceGameTurnStart : timeSinceCurrentTurnStart);
				if(curPlayer.isTurnActive())
				{//The timer is ticking for our turn
					if(timeElapsed > gameTurnEnd)
					{
						if(s_unitMoveTurnSlice == 0)
						{
							gameTurnTimerExpired = true;
						}
						else if(s_unitMoveTurnSlice + 10 < getTurnSlice())
						{
							gameTurnTimerExpired = true;
						}
					}
				}

				if((!curPlayer.isTurnActive() || gDLL->HasReceivedTurnComplete(playerID)) //Active player has finished their turn.
					&& getNumSequentialHumans() > 1)	//or sequential turn mode
				{//It's not our turn and there are sequential turn human players in the game.

					//In this case, the turn timer shows progress in terms of the max possible time until our next turn.
					//As such, timeElapsed has to be adjusted to be a value in terms of the max possible time.

					//determine number of players in the sequential turn sequence, not counting the active player.
					int playersInSeq = getNumSequentialHumans(playerID);

					//The max turn length is multiplied by the number of other human players in the sequential turn sequence.
					gameTurnEnd *= playersInSeq;

					float timePerPlayer = gameTurnEnd / playersInSeq; //time limit per human
					//count how many human players are left until us in the sequence.
					int humanTurnsUntilMe = countSeqHumanTurnsUntilPlayerTurn(playerID);
					int humanTurnsCompleted = playersInSeq - humanTurnsUntilMe;

					if(humanTurnsUntilMe)
					{//We're waiting on other sequential players
						timeElapsed =  timeSinceCurrentTurnStart + humanTurnsCompleted*timePerPlayer;
					}
					else
					{//All the other sequential players have finished.
					 //Either we're waiting on turn processing or on players who are playing simultaneous turns.

						//scale time to be that of the remaining possible time for the simultaneous players.
						//From the player's perspective, the timer will simply creep down for the remaining simultaneous turn time
						//rather than skipping straight to zero like it would by just tracking the sequential players' turn time.
						timeElapsed = timeSinceGameTurnStart + (humanTurnsCompleted-1)*timePerPlayer;
					}
				}

				if(isLocalPlayer)
				{//update the local end turn timer.
					CvPreGame::setEndTurnTimerLength(gameTurnEnd);
					iface->updateEndTurnTimer(timeElapsed / gameTurnEnd);
				}
			}
		}
		else if(isLocalPlayer){
			//hold the turn timer at 0 seconds with 0% completion
			CvPreGame::setEndTurnTimerLength(0.0f);

#if defined(MOD_EVENTS_RED_TURN)
			if (MOD_EVENTS_RED_TURN)
			// RED <<<<<
			{
				ICvEngineScriptSystem1* pkScriptSystem = gDLL->GetScriptSystem();
				if(pkScriptSystem)
				{	
					CvLuaArgsHandle args;

					args->Push(getActivePlayer());

					bool bResult = false;
					LuaSupport::CallHook(pkScriptSystem, "TurnComplete", args.get(), bResult);
				}
			}
			// RED >>>>>
#endif

			iface->updateEndTurnTimer(0.0f);
		}
	}

	return gameTurnTimerExpired;
}

//	-----------------------------------------------------------------------------------------------
void CvGame::TurnTimerSync(float fCurTurnTime, float fTurnStartTime)
{
	m_curTurnTimer.StartWithOffset(fCurTurnTime);
	m_timeSinceGameTurnStart.StartWithOffset(fTurnStartTime);
}

//	-----------------------------------------------------------------------------------------------
void CvGame::GetTurnTimerData(float& fCurTurnTime, float& fTurnStartTime)
{
	fCurTurnTime = m_curTurnTimer.Peek();
	fTurnStartTime = m_timeSinceGameTurnStart.Peek();
}

//	-----------------------------------------------------------------------------------------------
void CvGame::updateTestEndTurn()
{
	PlayerTypes activePlayerID = getActivePlayer();
	CvPlayer& activePlayer = GET_PLAYER(activePlayerID);

	ICvUserInterface2* pkIface = GC.GetEngineUserInterface();
	if(pkIface != NULL)
	{
		bool automaticallyEndTurns = (isGameMultiPlayer())? pkIface->IsMPAutoEndTurnEnabled() : pkIface->IsSPAutoEndTurnEnabled();
		if(automaticallyEndTurns && s_unitMoveTurnSlice != 0)
			automaticallyEndTurns = s_unitMoveTurnSlice + 10 < getTurnSlice();

		if(automaticallyEndTurns)
		{
			bool hasSelection = false;

			CvInterfacePtr<ICvUnit1> pDllHeadSelectedUnit(pkIface->GetHeadSelectedUnit());
			if(pDllHeadSelectedUnit)
			{
				hasSelection = pDllHeadSelectedUnit->IsSelected();
			}

			if(!hasSelection && !pkIface->waitingForRemotePlayers())
			{
				if(m_endTurnTimerSemaphore < 1)
				{
					if(pkIface->canEndTurn() && gDLL->allAICivsProcessedThisTurn() && allUnitAIProcessed() && !gDLL->HasSentTurnComplete())
					{
						if (MOD_API_ACHIEVEMENTS)
							activePlayer.GetPlayerAchievements().EndTurn();

						if (MOD_EVENTS_RED_TURN)
						// RED <<<<<
						{
							ICvEngineScriptSystem1* pkScriptSystem = gDLL->GetScriptSystem();
							if(pkScriptSystem)
							{	
								CvLuaArgsHandle args;

								args->Push(getActivePlayer());

								bool bResult = false;
								LuaSupport::CallHook(pkScriptSystem, "TurnComplete", args.get(), bResult);
							}
						}
						// RED >>>>>

						gDLL->sendTurnComplete();

						if (MOD_API_ACHIEVEMENTS)
							CvAchievementUnlocker::EndTurn();

						m_endTurnTimer.Start();
					}
				}
			}
		}
	}

	if(activePlayer.isTurnActive())
	{
		// check notifications
		EndTurnBlockingTypes eEndTurnBlockingType = NO_ENDTURN_BLOCKING_TYPE;
		int iNotificationIndex = -1;
		activePlayer.GetNotifications()->GetEndTurnBlockedType(eEndTurnBlockingType, iNotificationIndex);

		if(eEndTurnBlockingType == NO_ENDTURN_BLOCKING_TYPE)
		{
			// No notifications are blocking, check units/cities
			if(activePlayer.hasPromotableUnit() && !GC.getGame().isOption(GAMEOPTION_PROMOTION_SAVING))
			{
				eEndTurnBlockingType = ENDTURN_BLOCKING_UNIT_PROMOTION;
			}
			else if(activePlayer.hasReadyUnit())
			{
				const CvUnit* pUnit = activePlayer.GetFirstReadyUnit();
				ASSERT_DEBUG(pUnit, "GetFirstReadyUnit is returning null");
				if(pUnit)
				{
					if(!pUnit->canHold(pUnit->plot()))
					{
						eEndTurnBlockingType = ENDTURN_BLOCKING_STACKED_UNITS;
					}
					else
					{
						eEndTurnBlockingType = ENDTURN_BLOCKING_UNITS;
					}
				}
			}
		}

		if(eEndTurnBlockingType == NO_ENDTURN_BLOCKING_TYPE)
		{
			if(!(activePlayer.hasBusyUnitOrCity()) && !(activePlayer.hasReadyUnit()))
			{
				// JAR  - Looks like popups are pretty much disabled at this point, this check will break
				// multiplayer games. Look at revision #27 to resurrect the old popup check code if/when
				// they are implemented again.
				if(!isGameMultiPlayer())
				{
					if((activePlayer.isOption(PLAYEROPTION_WAIT_END_TURN) && !isGameMultiPlayer()) || !(GC.GetEngineUserInterface()->isHasMovedUnit()) || isHotSeat() || isPbem())
					{
						GC.GetEngineUserInterface()->setCanEndTurn(true);
					}
				}
				else
				{
					if(activePlayer.hasAutoUnit() && !m_sentAutoMoves)
					{
						if(!(gDLL->shiftKey()))
						{
							gDLL->sendAutoMoves();
							m_sentAutoMoves = true;
						}
					}
					else
					{
						if((activePlayer.isOption(PLAYEROPTION_WAIT_END_TURN) && !isGameMultiPlayer()) || !(GC.GetEngineUserInterface()->isHasMovedUnit()) || isHotSeat() || isPbem())
						{
							GC.GetEngineUserInterface()->setCanEndTurn(true);
						}
						else
						{
							if(GC.GetEngineUserInterface()->getEndTurnCounter() > 0)
							{
								GC.GetEngineUserInterface()->changeEndTurnCounter(-1);
							}
							else
							{
								if(!gDLL->HasSentTurnComplete() && gDLL->allAICivsProcessedThisTurn() && allUnitAIProcessed() && pkIface && pkIface->IsMPAutoEndTurnEnabled())
								{
									if (MOD_API_ACHIEVEMENTS)
										activePlayer.GetPlayerAchievements().EndTurn();

									if (MOD_EVENTS_RED_TURN)
									// RED <<<<<
									{
										ICvEngineScriptSystem1* pkScriptSystem = gDLL->GetScriptSystem();
										if(pkScriptSystem)
										{	
											CvLuaArgsHandle args;

											args->Push(getActivePlayer());

											bool bResult = false;
											LuaSupport::CallHook(pkScriptSystem, "TurnComplete", args.get(), bResult);
										}
									}
									// RED >>>>>

									gDLL->sendTurnComplete();

									if (MOD_API_ACHIEVEMENTS)
										CvAchievementUnlocker::EndTurn();
								}

								GC.GetEngineUserInterface()->setEndTurnCounter(3); // XXX
								if(isGameMultiPlayer())
								{
									GC.GetEngineUserInterface()->setCanEndTurn(true);
									m_endTurnTimer.Start();
								}
							}
						}
					}
				}
			}
		}

		activePlayer.SetEndTurnBlocking(eEndTurnBlockingType, iNotificationIndex);
	}
}

//	--------------------------------------------------------------------------------
void CvGame::testExtendedGame()
{
	int iI = 0;

	if(getGameState() != GAMESTATE_OVER)
	{
		return;
	}

	for(iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if(GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if(GET_PLAYER((PlayerTypes)iI).isHuman())
			{
				if(GET_PLAYER((PlayerTypes)iI).isExtendedGame())
				{
					setGameState(GAMESTATE_EXTENDED);
					break;
				}
			}
		}
	}
}


//	--------------------------------------------------------------------------------
CvUnit* CvGame::getPlotUnit(CvPlot* pPlot, int iIndex)
{
	IDInfo* pUnitNode1 = NULL;
	IDInfo* pUnitNode2 = NULL;
	CvUnit* pLoopUnit1 = NULL;
	CvUnit* pLoopUnit2 = NULL;
	int iCount = 0;
	int iPass = 0;
	PlayerTypes activePlayer = getActivePlayer();
	TeamTypes activeTeam = getActiveTeam();

	if(pPlot != NULL)
	{
		iCount = 0;

		for(iPass = 0; iPass < 2; iPass++)
		{
			pUnitNode1 = pPlot->headUnitNode();

			while(pUnitNode1 != NULL)
			{
				pLoopUnit1 = ::GetPlayerUnit(*pUnitNode1);
				pUnitNode1 = pPlot->nextUnitNode(pUnitNode1);

				if(!(pLoopUnit1->isInvisible(activeTeam, true)))
				{
					if(!(pLoopUnit1->isCargo()))
					{
						if((pLoopUnit1->getOwner() == activePlayer) == (iPass == 0))
						{
							if(iCount == iIndex)
							{
								return pLoopUnit1;
							}

							iCount++;

							if(pLoopUnit1->hasCargo())
							{
								pUnitNode2 = pPlot->headUnitNode();

								while(pUnitNode2 != NULL)
								{
									pLoopUnit2 = ::GetPlayerUnit(*pUnitNode2);
									pUnitNode2 = pPlot->nextUnitNode(pUnitNode2);

									if(!(pLoopUnit2->isInvisible(activeTeam, true)))
									{
										if(pLoopUnit2->getTransportUnit() == pLoopUnit1)
										{
											if(iCount == iIndex)
											{
												return pLoopUnit2;
											}

											iCount++;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return NULL;
}

//	--------------------------------------------------------------------------------
void CvGame::getPlotUnits(CvPlot* pPlot, std::vector<CvUnit*>& plotUnits)
{
	plotUnits.erase(plotUnits.begin(), plotUnits.end());

	IDInfo* pUnitNode1 = NULL;
	IDInfo* pUnitNode2 = NULL;
	CvUnit* pLoopUnit1 = NULL;
	CvUnit* pLoopUnit2 = NULL;
	int iPass = 0;
	PlayerTypes activePlayer = getActivePlayer();
	TeamTypes activeTeam = getActiveTeam();

	if(pPlot != NULL)
	{
		for(iPass = 0; iPass < 2; iPass++)
		{
			pUnitNode1 = pPlot->headUnitNode();

			while(pUnitNode1 != NULL)
			{
				pLoopUnit1 = ::GetPlayerUnit(*pUnitNode1);
				pUnitNode1 = pPlot->nextUnitNode(pUnitNode1);

				if(!(pLoopUnit1->isInvisible(activeTeam, true)))
				{
					if(!(pLoopUnit1->isCargo()))
					{
						if((pLoopUnit1->getOwner() == activePlayer) == (iPass == 0))
						{
							plotUnits.push_back(pLoopUnit1);

							if(pLoopUnit1->hasCargo())
							{
								pUnitNode2 = pPlot->headUnitNode();

								while(pUnitNode2 != NULL)
								{
									pLoopUnit2 = ::GetPlayerUnit(*pUnitNode2);
									pUnitNode2 = pPlot->nextUnitNode(pUnitNode2);

									if(!(pLoopUnit2->isInvisible(activeTeam, true)))
									{
										if(pLoopUnit2->getTransportUnit() == pLoopUnit1)
										{
											plotUnits.push_back(pLoopUnit2);
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

//	--------------------------------------------------------------------------------
void CvGame::cycleCities(bool bForward, bool bAdd)
{
	CvCity* pSelectCity = NULL;

	CvInterfacePtr<ICvCity1> pHeadSelectedCity(GC.GetEngineUserInterface()->getHeadSelectedCity());

	CvCity* pkHeadSelectedCity = GC.UnwrapCityPointer(pHeadSelectedCity.get());

	if((pkHeadSelectedCity != NULL) && ((pkHeadSelectedCity->getTeam() == getActiveTeam()) || isDebugMode()))
	{
		CvCity* pLoopCity = pkHeadSelectedCity;
		do
		{
			pLoopCity = GET_PLAYER(pLoopCity->getOwner()).nextCity(pLoopCity, !bForward);

			if(pLoopCity == NULL)
			{
				int iLoop = 0;
				pLoopCity = GET_PLAYER(pkHeadSelectedCity->getOwner()).firstCity(&iLoop, !bForward);
			}

			// we don't want the player to be able to cycle to puppeted cities - it kind of defeats the whole purpose - except venice!
			bool bCanControlCity = !pLoopCity->IsPuppet() || GET_PLAYER(pLoopCity->getOwner()).GetPlayerTraits()->IsNoAnnexing();
			if((pLoopCity != NULL) && (pLoopCity != pkHeadSelectedCity) && bCanControlCity)  
			{
				pSelectCity = pLoopCity;
			}

		}
		while((pLoopCity != pkHeadSelectedCity) && !pSelectCity);

	}
	else
	{
		int iLoop = 0;
		pSelectCity = GET_PLAYER(getActivePlayer()).firstCity(&iLoop, !bForward);
	}

	if(pSelectCity != NULL)
	{
		CvInterfacePtr<ICvCity1> pDllSelectedCity = GC.WrapCityPointer(pSelectCity);
		if(bAdd)
		{
			GC.GetEngineUserInterface()->clearSelectedCities();
			GC.GetEngineUserInterface()->addSelectedCity(pDllSelectedCity.get());
		}
		else
		{
			GC.GetEngineUserInterface()->selectCity(pDllSelectedCity.get());
		}
	}
}

//	--------------------------------------------------------------------------------
void CvGame::cycleUnits(bool bClear, bool bForward, bool bWorkers)
{
	CvUnit* pNextUnit = NULL;
	CvCity* pCycleCity = NULL;
	bool bWrap = false;
	bool bProcessed = false;
	PlayerTypes eActivePlayer = getActivePlayer();
	ICvUserInterface2* pUI = GC.GetEngineUserInterface();
	CvPlayerAI& theActivePlayer = GET_PLAYER(eActivePlayer);

	CvInterfacePtr<ICvUnit1> pDllSelectedUnit(pUI->GetHeadSelectedUnit());
	CvUnit* pCycleUnit = GC.UnwrapUnitPointer(pDllSelectedUnit.get());

	if(pCycleUnit != NULL)
	{
		if(pCycleUnit->getOwner() != eActivePlayer)
		{
			pCycleUnit = NULL;
		}

		pNextUnit = theActivePlayer.GetUnitCycler().Cycle(pCycleUnit, bForward, bWorkers, &bWrap);

		if(bWrap)
		{
			if(theActivePlayer.hasAutoUnit())
			{
				gDLL->sendAutoMoves();
			}
		}
	}
	else
	{
		pNextUnit = GC.getMap().findUnit(0, 0, eActivePlayer, true, bWorkers);
	}

	if(pNextUnit != NULL && !bWrap)
	{
		ASSERT_DEBUG(pNextUnit->getOwner() == eActivePlayer);
		selectUnit(pNextUnit, bClear);
		bProcessed = true;
	}

	if(pNextUnit != NULL /*&& bWrap */&& !pCycleCity && !bProcessed)
	{
		ASSERT_DEBUG(pNextUnit->getOwner() == eActivePlayer);
		selectUnit(pNextUnit, bClear);
	}

	if(pNextUnit == NULL && pCycleUnit != NULL && pCycleUnit->getOwner() == eActivePlayer)
	{
		pUI->ClearSelectionList();
		pCycleUnit->plot()->updateCenterUnit();
	}

	pDllSelectedUnit.reset(pUI->GetHeadSelectedUnit());
	CvUnit* pCurrentSelectedUnit = GC.UnwrapUnitPointer(pDllSelectedUnit.get());
	if((pCycleUnit != pCurrentSelectedUnit) || ((pCycleUnit != NULL) && pCycleUnit->ReadyToSelect()) || pCycleCity)
	{
		pUI->lookAtSelectionPlot();
	}
}

//	--------------------------------------------------------------------------------
// Returns true if unit was cycled...
bool CvGame::cyclePlotUnits(CvPlot* pPlot, bool bForward, bool bAuto, int iCount)
{
	IDInfo* pUnitNode = NULL;
	CvUnit* pSelectedUnit = NULL;
	CvUnit* pLoopUnit = NULL;

	ASSERT_DEBUG(iCount >= -1, "iCount expected to be >= -1");

	if(iCount == -1)
	{
		pUnitNode = pPlot->headUnitNode();

		while(pUnitNode != NULL)
		{
			pLoopUnit = ::GetPlayerUnit(*pUnitNode);

			if(NULL != pLoopUnit && pLoopUnit->IsSelected())
			{
				break;
			}

			pUnitNode = pPlot->nextUnitNode(pUnitNode);
		}
	}
	else
	{
		pUnitNode = pPlot->headUnitNode();

		while(pUnitNode != NULL)
		{
			pLoopUnit = ::GetPlayerUnit(*pUnitNode);

			if((iCount - 1) == 0)
			{
				break;
			}

			if(iCount > 0)
			{
				iCount--;
			}

			pUnitNode = pPlot->nextUnitNode(pUnitNode);
		}

		if(pUnitNode == NULL)
		{
			pUnitNode = pPlot->tailUnitNode();

			if(pUnitNode != NULL)
			{
				pLoopUnit = ::GetPlayerUnit(*pUnitNode);
			}
		}
	}

	if(pUnitNode != NULL)
	{
		pSelectedUnit = pLoopUnit;

		while(true)
		{
			if(bForward)
			{
				pUnitNode = pPlot->nextUnitNode(pUnitNode);
				if(pUnitNode == NULL)
				{
					pUnitNode = pPlot->headUnitNode();
				}
			}
			else
			{
				pUnitNode = pPlot->prevUnitNode(pUnitNode);
				if(pUnitNode == NULL)
				{
					pUnitNode = pPlot->tailUnitNode();
				}
			}

			pLoopUnit = ::GetPlayerUnit(*pUnitNode);

			if(iCount == -1)
			{
				if(pLoopUnit == pSelectedUnit)
				{
					break;
				}
			}

			if(NULL != pLoopUnit && pLoopUnit->getOwner() == getActivePlayer())
			{
				if(bAuto)
				{
					if(pLoopUnit->ReadyToSelect())
					{
						/*GC.GetEngineUserInterface()->*/selectUnit(pLoopUnit, true);
						return true;
					}
				}
				else
				{
					CvInterfacePtr<ICvUnit1> pDllLoopUnit = GC.WrapUnitPointer(pLoopUnit);
					GC.GetEngineUserInterface()->InsertIntoSelectionList(pDllLoopUnit.get(), true, false);
					return true;
				}
			}

			if(pLoopUnit == pSelectedUnit)
			{
				break;
			}
		}
	}

	return false;
}


//	--------------------------------------------------------------------------------
void CvGame::selectionListMove(CvPlot* pPlot, bool bShift)
{
	if(pPlot == NULL)
	{
		return;
	}

	CvInterfacePtr<ICvUnit1> pSelectedUnit(GC.GetEngineUserInterface()->GetHeadSelectedUnit());
	CvUnit* pkSelectedUnit = GC.UnwrapUnitPointer(pSelectedUnit.get());

	if((pkSelectedUnit == NULL) || (pkSelectedUnit->getOwner() != getActivePlayer()))
	{
		return;
	}

	//need to be careful, potential deadlock
	if (GET_PLAYER(pkSelectedUnit->getOwner()).isTurnActive())
	{
		if(pkSelectedUnit->CanSwapWithUnitHere(*pPlot))
		{
			selectionListGameNetMessage(GAMEMESSAGE_SWAP_UNITS, CvTypes::getMISSION_SWAP_UNITS(), pPlot->getX(), pPlot->getY(), 0, false, bShift);
		}
		else
		{
			selectionListGameNetMessage(GAMEMESSAGE_PUSH_MISSION, CvTypes::getMISSION_MOVE_TO(), pPlot->getX(), pPlot->getY(), 0, false, bShift);
		}
	}
}


//	--------------------------------------------------------------------------------
void CvGame::selectionListGameNetMessage(int eMessage, int iData2, int iData3, int iData4, int iFlags, bool bAlt, bool bShift)
{
	CvInterfacePtr<ICvUnit1> pSelectedUnit(GC.GetEngineUserInterface()->GetHeadSelectedUnit());
	CvUnit* pkSelectedUnit = GC.UnwrapUnitPointer(pSelectedUnit.get());

	if(pkSelectedUnit != NULL)
	{
		if(pkSelectedUnit->getOwner() == getActivePlayer() && !pSelectedUnit->IsBusy())
		{
			if(eMessage == GAMEMESSAGE_DO_COMMAND)
			{
				gDLL->sendDoCommand(pkSelectedUnit->GetID(), ((CommandTypes)iData2), iData3, iData4, bAlt);
			}
			else if((eMessage == GAMEMESSAGE_PUSH_MISSION) || (eMessage == GAMEMESSAGE_AUTO_MISSION))
			{
				if(eMessage == GAMEMESSAGE_PUSH_MISSION)
				{
					MissionTypes eMission = (MissionTypes)iData2;
					CvPlot* pPlot = GC.getMap().plot(iData3, iData4);
					if(pPlot && pkSelectedUnit->CanSwapWithUnitHere(*pPlot) && eMission != CvTypes::getMISSION_ROUTE_TO())
					{
						gDLL->sendSwapUnits(pkSelectedUnit->GetID(), ((MissionTypes)iData2), iData3, iData4, iFlags, bShift);
					}
					else
					{
#if defined(MOD_EVENTS_RED_COMBAT_MISSION)
						if (MOD_EVENTS_RED_COMBAT_MISSION)
						// RED <<<<<
						{
							ICvEngineScriptSystem1* pkScriptSystem = gDLL->GetScriptSystem();
							if(pkScriptSystem && pPlot)
							{						
								CvLuaArgsHandle args;

								args->Push(pkSelectedUnit->getOwner());
								args->Push(pkSelectedUnit->GetID());
								args->Push(pPlot->getX());
								args->Push(pPlot->getY());
								args->Push(iData2);

								bool bResult = false;
								LuaSupport::CallHook(pkScriptSystem, "PushingMissionTo", args.get(), bResult);
							}
						}
						// RED >>>>>
#endif

						gDLL->sendPushMission(pkSelectedUnit->GetID(), ((MissionTypes)iData2), iData3, iData4, iFlags, bShift);
					}
				}
				else
				{
					gDLL->sendAutoMission(pkSelectedUnit->GetID());
				}
			}
			else if(eMessage == GAMEMESSAGE_SWAP_UNITS)
			{
				gDLL->sendSwapUnits(pkSelectedUnit->GetID(), ((MissionTypes)iData2), iData3, iData4, iFlags, bShift);
			}
			else
			{
				ASSERT_DEBUG(false);
			}
		}
	}
}


//	--------------------------------------------------------------------------------
void CvGame::selectedCitiesGameNetMessage(int eMessage, int iData2, int iData3, int iData4, bool bOption, bool bAlt, bool bShift, bool bCtrl)
{
	const IDInfo* pSelectedCityNode = NULL;
	CvCity* pSelectedCity = NULL;

	pSelectedCityNode = GC.GetEngineUserInterface()->headSelectedCitiesNode();

	while(pSelectedCityNode != NULL)
	{
		pSelectedCity = ::GetPlayerCity(*pSelectedCityNode);
		pSelectedCityNode = GC.GetEngineUserInterface()->nextSelectedCitiesNode(pSelectedCityNode);
		ASSERT_DEBUG(pSelectedCity);

		if(pSelectedCity != NULL)
		{
			if(pSelectedCity->getOwner() == getActivePlayer())
			{
				switch(eMessage)
				{
				case GAMEMESSAGE_PUSH_ORDER:
					cityPushOrder(pSelectedCity, ((OrderTypes)iData2), iData3, bAlt, bShift, bCtrl);
					break;

				case GAMEMESSAGE_POP_ORDER:
					if(pSelectedCity->getOrderQueueLength() >= 1)
					{
						gDLL->sendPopOrder(pSelectedCity->GetID(), iData2);
					}
					break;

				case GAMEMESSAGE_SWAP_ORDER:
					if(pSelectedCity->getOrderQueueLength() >= 2)
					{
						gDLL->sendSwapOrder(pSelectedCity->GetID(), iData2);
					}
					break;

				case GAMEMESSAGE_DO_TASK:
					gDLL->sendDoTask(pSelectedCity->GetID(), ((TaskTypes)iData2), iData3, iData4, bOption, bAlt, bShift, bCtrl);
					break;

				default:
					ASSERT_DEBUG(false);
					break;
				}
			}
		}
	}
}


//	--------------------------------------------------------------------------------
void CvGame::cityPushOrder(CvCity* pCity, OrderTypes eOrder, int iData, bool bAlt, bool bShift, bool bCtrl)
{
	ASSERT_DEBUG(pCity);
	if(!pCity) return;
	gDLL->sendPushOrder(pCity->GetID(), eOrder, iData, bAlt, bShift, bCtrl);
}

//	--------------------------------------------------------------------------------
void CvGame::CityPurchase(CvCity* pCity, UnitTypes eUnitType, BuildingTypes eBuildingType, ProjectTypes eProjectType, YieldTypes ePurchaseYield)
{
	ASSERT_DEBUG(pCity);
	if(!pCity) return;

	// we're trying to buy a unit
	if(eUnitType >= 0)
	{
		// if there's a unit of the same type in the tile, BAIL!
		if(!pCity->CanPlaceUnitHere(eUnitType))
		{
			return;
		}
	}

	gDLL->sendPurchase(pCity->GetID(), eUnitType, eBuildingType, eProjectType, ePurchaseYield);
}

//	--------------------------------------------------------------------------------
void CvGame::selectUnit(CvUnit* pUnit, bool bClear, bool bToggle, bool bSound)
{
	CvInterfacePtr<ICvUnit1> pOldSelectedUnit(GC.GetEngineUserInterface()->GetHeadSelectedUnit());
	CvUnit* pkOldSelectedUnit = GC.UnwrapUnitPointer(pOldSelectedUnit.get());

	GC.GetEngineUserInterface()->clearSelectedCities();

	bool bGroup = true;
	if(bClear)
	{
		GC.GetEngineUserInterface()->ClearSelectionList();
		bGroup = false;
	}

	pUnit->IncrementFirstTimeSelected();

	CvInterfacePtr<ICvUnit1> pDllUnit = GC.WrapUnitPointer(pUnit);
	GC.GetEngineUserInterface()->InsertIntoSelectionList(pDllUnit.get(), true, bToggle, bGroup, bSound);

	gDLL->GameplayMinimapUnitSelect(pUnit->getX(), pUnit->getY());

	GC.GetEngineUserInterface()->makeSelectionListDirty();

	bool bHighlightTrade = false;
	int iRouteIndex = -1;
	if(GC.getGame().getActivePlayer() == pUnit->getOwner())
	{
		CvMap& theMap = GC.getMap();
		theMap.updateDeferredFog();
		if(pkOldSelectedUnit)
		{
			pkOldSelectedUnit->plot()->updateCenterUnit();
		}
		pUnit->plot()->updateCenterUnit();

		if (pUnit->isTrade())
		{
			iRouteIndex = GC.getGame().GetGameTrade()->GetIndexFromUnitID( pUnit->GetID(), pUnit->getOwner() );
			if (iRouteIndex != -1)
				bHighlightTrade = true;
		}
	}
	
	if (bHighlightTrade)
		gDLL->TradeVisuals_ActivatePopupRoute(iRouteIndex);
	else
		gDLL->TradeVisuals_DeactivatePopupRoute();
}
//	--------------------------------------------------------------------------------
static void IfTradeUnit_DisplayPopupTradeRoute(CvUnit *pUnit)
{
	int iRouteIndex = 0;

	iRouteIndex = -1;
	if (pUnit && pUnit->isTrade())
	{
		iRouteIndex = GC.getGame().GetGameTrade()->GetIndexFromUnitID(pUnit->GetID(),pUnit->getOwner());
		if (iRouteIndex != -1) {
			gDLL->TradeVisuals_ActivatePopupRoute(iRouteIndex);
		}
	}	
}
void CvGame::mouseoverUnit(CvUnit *pUnit, bool bEnter)
{
	CvUnit *pkSelectedUnit = NULL;

	if (pUnit)
	{
		if (bEnter) {
			IfTradeUnit_DisplayPopupTradeRoute(pUnit);
			m_iLastMouseoverUnitID = pUnit->GetID();
		} else {
			if (pUnit->GetID() == m_iLastMouseoverUnitID) {
				gDLL->TradeVisuals_DeactivatePopupRoute();

				//reactivate selected unit's popup route
				CvInterfacePtr<ICvUnit1> pSelectedUnit(GC.GetEngineUserInterface()->GetHeadSelectedUnit());
				pkSelectedUnit = GC.UnwrapUnitPointer(pSelectedUnit.get());
				IfTradeUnit_DisplayPopupTradeRoute(pkSelectedUnit);
			}
		}
	}
}

//	--------------------------------------------------------------------------------
void CvGame::selectGroup(CvUnit* pUnit, bool bShift, bool bCtrl, bool bAlt) const
{
	IDInfo* pUnitNode = NULL;
	CvPlot* pUnitPlot = NULL;
	bool bGroup = false;

	ASSERT_DEBUG(pUnit != NULL, "pUnit == NULL unexpectedly");

	if(bAlt || bCtrl)
	{
		GC.GetEngineUserInterface()->clearSelectedCities();

		if(!bShift)
		{
			GC.GetEngineUserInterface()->ClearSelectionList();
			bGroup = true;
		}
		else
		{
			bGroup = true;
		}

		if(!pUnit) return;

		pUnitPlot = pUnit->plot();

		pUnitNode = pUnitPlot->headUnitNode();

		while(pUnitNode != NULL)
		{
			CvUnit* pLoopUnit = ::GetPlayerUnit(*pUnitNode);
			pUnitNode = pUnitPlot->nextUnitNode(pUnitNode);

			if(NULL != pLoopUnit && pLoopUnit->getOwner() == getActivePlayer())
			{
				if(pLoopUnit->canMove())
				{
					CvPlayerAI* pOwnerPlayer = &(GET_PLAYER(pLoopUnit->getOwner()));
					if( !pOwnerPlayer->isSimultaneousTurns() || getGameTurn() - pLoopUnit->getLastMoveTurn() >= 1)
					{
						if(bAlt || (pLoopUnit->getUnitType() == pUnit->getUnitType()))
						{
							CvInterfacePtr<ICvUnit1> pDllLoopUnit = GC.WrapUnitPointer(pLoopUnit);
							GC.GetEngineUserInterface()->InsertIntoSelectionList(pDllLoopUnit.get(), true, false, bGroup, false, true);
						}
					}
				}
			}
		}
	}
	else
	{
		CvInterfacePtr<ICvUnit1> pDllUnit = GC.WrapUnitPointer(pUnit);
		GC.GetEngineUserInterface()->selectUnit(pDllUnit.get(), !bShift, bShift, true);
	}
}


//	--------------------------------------------------------------------------------
void CvGame::selectAll(CvPlot* pPlot) const
{
	CvUnit* pSelectUnit = NULL;

	if(pPlot != NULL)
	{
		CvUnit* pCenterUnit = pPlot->getCenterUnit();

		if((pCenterUnit != NULL) && (pCenterUnit->getOwner() == getActivePlayer()))
		{
			pSelectUnit = pCenterUnit;
		}
	}

	if(pSelectUnit != NULL)
	{
		CvInterfacePtr<ICvUnit1> pDllSelectUnit = GC.WrapUnitPointer(pSelectUnit);
		GC.GetEngineUserInterface()->selectGroup(pDllSelectUnit.get(), false, false, true);
	}
}

//	--------------------------------------------------------------------------------
void CvGame::SelectSettler(void)
{
	CvUnit* pSettlerUnit = NULL;
	CvPlayerAI* pActivePlayer = &(GET_PLAYER(getActivePlayer()));

	CvUnit* pLoopUnit = NULL;
	int iUnitIndex = 0;
	for(pLoopUnit = pActivePlayer->firstUnit(&iUnitIndex); pLoopUnit != NULL; pLoopUnit = pActivePlayer->nextUnit(&iUnitIndex))
	{
		if(pLoopUnit->isFound())
		{
			pSettlerUnit = pLoopUnit;
			break;
		}
	}

	if(pSettlerUnit && pSettlerUnit->ReadyToSelect())
	{
		selectUnit(pSettlerUnit, true, false, true);
	}
}


//	--------------------------------------------------------------------------------
bool CvGame::selectionListIgnoreBuildingDefense()
{
	bool bIgnoreBuilding = false;
	bool bAttackLandUnit = false;
	CvInterfacePtr<ICvUnit1> pSelectedUnit(GC.GetEngineUserInterface()->GetHeadSelectedUnit());
	CvUnit* pkSelectedUnit = GC.UnwrapUnitPointer(pSelectedUnit.get());

	if(pkSelectedUnit != NULL)
	{
		if(pkSelectedUnit->ignoreBuildingDefense())
		{
			bIgnoreBuilding = true;
		}

		if((pkSelectedUnit->getDomainType() == DOMAIN_LAND) && pkSelectedUnit->IsCanAttack())
		{
			bAttackLandUnit = true;
		}
	}

	if(!bIgnoreBuilding && !bAttackLandUnit)
	{
		const UnitTypes eBestLandUnit = getBestLandUnit();
		if(eBestLandUnit != NO_UNIT)
		{
			CvUnitEntry* pkUnitInfo = GC.getUnitInfo(eBestLandUnit);
			if(pkUnitInfo)
			{
				bIgnoreBuilding = pkUnitInfo->IsIgnoreBuildingDefense();
			}
		}
	}

	return bIgnoreBuilding;
}


//	--------------------------------------------------------------------------------
bool CvGame::canHandleAction(int iAction, CvPlot* pPlot, bool bTestVisible)
{
	CvPlot* pMissionPlot = NULL;
	bool bShift = gDLL->shiftKey();

	CvActionInfo* pActionInfo = GC.getActionInfo(iAction);
	ASSERT_DEBUG(pActionInfo != NULL);
	if(!pActionInfo) return false;

	if(pActionInfo->getControlType() != NO_CONTROL)
	{
		if(canDoControl((ControlTypes)(GC.getActionInfo(iAction)->getControlType())))
		{
			return true;
		}
	}

	if(GC.GetEngineUserInterface()->isCitySelection())
	{
		return false; // XXX hack!
	}

	CvInterfacePtr<ICvUnit1> pHeadSelectedUnit(GC.GetEngineUserInterface()->GetHeadSelectedUnit());
	CvUnit* pkHeadSelectedUnit = GC.UnwrapUnitPointer(pHeadSelectedUnit.get());

	if(pkHeadSelectedUnit != NULL)
	{
		if(pkHeadSelectedUnit->getOwner() == getActivePlayer())
		{
			if(GET_PLAYER(pkHeadSelectedUnit->getOwner()).isSimultaneousTurns() || GET_PLAYER(pkHeadSelectedUnit->getOwner()).isTurnActive())
			{
				if(GC.getActionInfo(iAction)->getMissionType() != NO_MISSION)
				{
					if(pPlot != NULL)
					{
						pMissionPlot = pPlot;
					}
					else if(bShift)
					{
						pMissionPlot = pkHeadSelectedUnit->LastMissionPlot();
					}
					else
					{
						pMissionPlot = NULL;
					}

					if((pMissionPlot == NULL) || !(pMissionPlot->isVisible(pkHeadSelectedUnit->getTeam())))
					{
						pMissionPlot = pkHeadSelectedUnit->plot();
					}

					if(pkHeadSelectedUnit->CanStartMission(pActionInfo->getMissionType(), pActionInfo->getMissionData(), -1, pMissionPlot, bTestVisible))
					{
						return true;
					}
				}

				if(GC.getActionInfo(iAction)->getCommandType() != NO_COMMAND)
				{
					if(pkHeadSelectedUnit->canDoCommand(((CommandTypes)(pActionInfo->getCommandType())), pActionInfo->getCommandData(), -1, bTestVisible))
					{
						return true;
					}
				}

				if(GC.GetEngineUserInterface()->CanDoInterfaceMode((InterfaceModeTypes)pActionInfo->getInterfaceModeType(), bTestVisible))
				{
					return true;
				}
			}
		}
	}

	return false;
}

//	--------------------------------------------------------------------------------
//	Handle an action initiated by the local human player
void CvGame::handleAction(int iAction)
{
	bool bAlt = false;
	bool bShift = false;
	bool bSkip = false;

	bAlt = gDLL->altKey();
	bShift = false;//gDLL->shiftKey();

	CvInterfacePtr<ICvUnit1> pHeadSelectedUnit(GC.GetEngineUserInterface()->GetHeadSelectedUnit());

	if(!(canHandleAction(iAction)))
	{
		return;
	}

	// Control
	CvActionInfo* pkActionInfo = GC.getActionInfo(iAction);
	if(pkActionInfo->getControlType() != NO_CONTROL)
	{
		doControl((ControlTypes)(pkActionInfo->getControlType()));
	}

	// Interface Mode
	if(GC.GetEngineUserInterface()->CanDoInterfaceMode((InterfaceModeTypes)pkActionInfo->getInterfaceModeType()))
	{
		if(pHeadSelectedUnit)
		{
			if(GC.getInterfaceModeInfo((InterfaceModeTypes)pkActionInfo->getInterfaceModeType())->getSelectAll())
			{
				GC.GetEngineUserInterface()->selectGroup(pHeadSelectedUnit.get(), false, false, true);
			}
			else if(GC.getInterfaceModeInfo((InterfaceModeTypes)pkActionInfo->getInterfaceModeType())->getSelectType())
			{
				GC.GetEngineUserInterface()->selectGroup(pHeadSelectedUnit.get(), false, true, false);
			}
		}

		GC.GetEngineUserInterface()->setInterfaceMode((InterfaceModeTypes)pkActionInfo->getInterfaceModeType());
	}

	// Mission
	int iMissionType = pkActionInfo->getMissionType();
	if(iMissionType != NO_MISSION)
	{
		bool bSkipMissionAdd = false;

		if(iMissionType == CvTypes::getMISSION_BUILD() || iMissionType == CvTypes::getMISSION_FOUND())
		{
			bool bShowConfirmPopup = false;
			// If we're going to build over an existing Improvement/Route, ask the player to confirm
			if(iMissionType == CvTypes::getMISSION_BUILD())
			{
				int iBuild = pkActionInfo->getMissionData();
				CvBuildInfo* pBuildInfo = GC.getBuildInfo((BuildTypes)iBuild);

				// Let them Repair, it's fine
				if(!pBuildInfo->isRepair())
				{
					// Let them build a Route, it's fine
					if(pBuildInfo->getRoute() == NO_ROUTE)
					{
						// Let them build over an improvement if the new improvement REQUIRES the old one
						ImprovementTypes eImprovement = (ImprovementTypes)pBuildInfo->getImprovement();
						if (eImprovement == NO_IMPROVEMENT || !GC.getImprovementInfo(eImprovement)->IsRequiresImprovement())
						{
							if(pHeadSelectedUnit)
							{
								CvUnit* pkHeadSelectedUnit = GC.UnwrapUnitPointer(pHeadSelectedUnit.get());
								CvPlot* pPlot = pkHeadSelectedUnit->plot();
								if(pPlot != NULL)
								{
									// don't ask for confirmation if the old improvement doesn't connect the resource on the plot but the new improvement does
									ResourceTypes eResource = pPlot->getResourceType(pkHeadSelectedUnit->getTeam());
									if(pPlot->getImprovementType() != NO_IMPROVEMENT && (pPlot->getFeatureType() != FEATURE_FALLOUT || !pBuildInfo->isFeatureRemove(FEATURE_FALLOUT)) && (eResource == NO_RESOURCE || GC.getImprovementInfo(pPlot->getImprovementType())->IsConnectsResource(eResource) || !GC.getImprovementInfo(eImprovement)->IsConnectsResource(eResource) || GC.getImprovementInfo(pPlot->getImprovementType())->IsCreatedByGreatPerson()))
									{
										bShowConfirmPopup = true;
									}
								}
							}
						}
					}
				}
			}
			else
			if(iMissionType == CvTypes::getMISSION_FOUND())
			{
				if(pHeadSelectedUnit)
				{
					CvUnit* pkHeadSelectedUnit = GC.UnwrapUnitPointer(pHeadSelectedUnit.get());
					CvPlot* pPlot = pkHeadSelectedUnit->plot();
					if(pPlot != NULL)
					{
						ResourceTypes eArtifactResourceType = static_cast<ResourceTypes>(GD_INT_GET(ARTIFACT_RESOURCE));
						ResourceTypes eHiddenArtifactResourceType = static_cast<ResourceTypes>(GD_INT_GET(ARTIFACT_RESOURCE));
						if (pPlot->getResourceType() == eArtifactResourceType || pPlot->getResourceType() == eHiddenArtifactResourceType)
						{
							bShowConfirmPopup = true;
						}
					}
				}
			}

			if(bShowConfirmPopup)
			{
				int iBuild = pkActionInfo->getMissionData();
				CvPopupInfo kPopupInfo(BUTTONPOPUP_CONFIRM_IMPROVEMENT_REBUILD, iAction, iBuild);
				kPopupInfo.bOption1 = bAlt;
				GC.GetEngineUserInterface()->AddPopup(kPopupInfo);
				bSkipMissionAdd = true;		// Skip the mission add, the popup will do it
			}
		}
		else
		if (iMissionType == CvTypes::getMISSION_ESTABLISH_TRADE_ROUTE())
		{
			if(pHeadSelectedUnit)
			{
				CvUnit* pkHeadSelectedUnit = GC.UnwrapUnitPointer(pHeadSelectedUnit.get());
				if (pkHeadSelectedUnit)
				{
					GC.getGame().GetGameTrade()->InvalidateTradePathCache(pkHeadSelectedUnit->getOwner());
					CvPopupInfo kPopup(BUTTONPOPUP_CHOOSE_INTERNATIONAL_TRADE_ROUTE, pkHeadSelectedUnit->getOwner());
					kPopup.iData2 = pkHeadSelectedUnit->GetID();
					GC.GetEngineUserInterface()->AddPopup(kPopup);
				}
			}
			bSkipMissionAdd = true;	// Skip no matter what, if there is no unit, there is no mission
		}
		else
		if (iMissionType == CvTypes::getMISSION_CHANGE_TRADE_UNIT_HOME_CITY())
		{
			if(pHeadSelectedUnit)
			{
				CvUnit* pkHeadSelectedUnit = GC.UnwrapUnitPointer(pHeadSelectedUnit.get());
				if (pkHeadSelectedUnit)
				{
					CvPopupInfo kPopup(BUTTONPOPUP_CHOOSE_TRADE_UNIT_NEW_HOME);
					kPopup.iData1 = pkHeadSelectedUnit->GetID();
					GC.GetEngineUserInterface()->AddPopup(kPopup);
				}
			}
			bSkipMissionAdd = true;	// Skip no matter what, if there is no unit, there is no mission
		}
		if (iMissionType == CvTypes::getMISSION_CHANGE_ADMIRAL_PORT())
		{
			if(pHeadSelectedUnit)
			{
				CvUnit* pkHeadSelectedUnit = GC.UnwrapUnitPointer(pHeadSelectedUnit.get());
				if (pkHeadSelectedUnit)
				{
					CvPopupInfo kPopup(BUTTONPOPUP_CHOOSE_ADMIRAL_PORT);
					kPopup.iData1 = pkHeadSelectedUnit->GetID();
					GC.GetEngineUserInterface()->AddPopup(kPopup);
				}
			}
			bSkipMissionAdd = true;	// Skip no matter what, if there is no unit, there is no mission
		}

		if (!bSkipMissionAdd)
		{
			selectionListGameNetMessage(GAMEMESSAGE_PUSH_MISSION, iMissionType, pkActionInfo->getMissionData(), -1, 0, false, bShift);
			GC.GetEngineUserInterface()->setInterfaceMode(INTERFACEMODE_SELECTION);
		}
	}

	// Command
	if(pkActionInfo->getCommandType() != NO_COMMAND)
	{
		bSkip = false;

		if(!bSkip)
		{
			if(pkActionInfo->isConfirmCommand())
			{
				CvPopupInfo kPopupInfo(BUTTONPOPUP_CONFIRMCOMMAND, iAction);
				kPopupInfo.bOption1 = bAlt;
				GC.GetEngineUserInterface()->AddPopup(kPopupInfo);
			}
			else
			{
				selectionListGameNetMessage(GAMEMESSAGE_DO_COMMAND, pkActionInfo->getCommandType(), pkActionInfo->getCommandData(), -1, 0, bAlt);
			}
		}
	}
}


//	--------------------------------------------------------------------------------
bool CvGame::canDoControl(ControlTypes eControl)
{
	switch(eControl)
	{
	case CONTROL_SELECTYUNITTYPE:
	case CONTROL_SELECTYUNITALL:
	case CONTROL_SELECT_HEALTHY:
	case CONTROL_SELECTCITY:
	case CONTROL_SELECTCAPITAL:
	case CONTROL_NEXTUNIT:
	case CONTROL_PREVUNIT:
	case CONTROL_CYCLEUNIT:
	case CONTROL_CYCLEUNIT_ALT:
	case CONTROL_CYCLEWORKER:
	case CONTROL_LASTUNIT:
	case CONTROL_FORCEENDTURN:
	case CONTROL_AUTOMOVES:
	case CONTROL_SAVE_GROUP:
	case CONTROL_QUICK_SAVE:
	case CONTROL_QUICK_LOAD:
	case CONTROL_TURN_LOG:
		if(!GC.GetEngineUserInterface()->isFocused())
		{
			return true;
		}
		break;

	case CONTROL_PING:
	case CONTROL_YIELDS:
	case CONTROL_RESOURCE_ALL:
	case CONTROL_UNIT_ICONS:
	case CONTROL_SCORES:
	case CONTROL_OPTIONS_SCREEN:
	case CONTROL_DOMESTIC_SCREEN:
	case CONTROL_CIVILOPEDIA:
	case CONTROL_POLICIES_SCREEN:
	case CONTROL_FOREIGN_SCREEN:
	case CONTROL_MILITARY_SCREEN:
	case CONTROL_TECH_CHOOSER:
	case CONTROL_INFO:
	case CONTROL_SAVE_NORMAL:
	case CONTROL_ADVISOR_COUNSEL:
	case CONTROL_NEXTCITY:
	case CONTROL_PREVCITY:
	case CONTROL_RELIGION_OVERVIEW:
	case CONTROL_ESPIONAGE_OVERVIEW:
	case CONTROL_TOGGLE_OBSERVER_MODE:
	case CONTROL_TOGGLE_AI_TAKEOVER:
	case CONTROL_SWITCH_TO_NEXT_PLAYER:
		return true;
		break;

	case CONTROL_VICTORY_SCREEN:
		if(getGameState() == GAMESTATE_ON)
		{
			return true;
		}
		break;


	case CONTROL_CENTERONSELECTION:
	{
		CvInterfacePtr<ICvPlot1> pSelectionPlot(GC.GetEngineUserInterface()->getSelectionPlot());
		if(pSelectionPlot)
		{
			return true;
		}
	}
	break;

	case CONTROL_LOAD_GAME:
		if(!(isNetworkMultiPlayer()))
		{
			return true;
		}
		break;

	case CONTROL_RETIRE:
		if((getGameState() == GAMESTATE_ON) || isGameMultiPlayer())
		{
			if(GET_PLAYER(getActivePlayer()).isAlive())
			{
				if(isPbem() || isHotSeat())
				{
					if(!GET_PLAYER(getActivePlayer()).isEndTurn())
					{
						return true;
					}
				}
				else
				{
					return true;
				}
			}
		}
		break;

	case CONTROL_ENDTURN:
	case CONTROL_ENDTURN_ALT:
		if(GC.GetEngineUserInterface()->canEndTurn() && !GC.GetEngineUserInterface()->isFocused())
		{
			return true;
		}
		break;

	case CONTROL_TOGGLE_STRATEGIC_VIEW:
		GC.GetEngineUserInterface()->ToggleStrategicView();
		break;

	case CONTROL_RESTART_GAME:
		{
			if(!isGameMultiPlayer() && getGameTurn() == getStartTurn())
			{
				return true;
			}
		}
		break;

	default:
		ASSERT_DEBUG(false, "eControl did not match any valid options");
		break;
	}

	return false;
}


//	--------------------------------------------------------------------------------
void CvGame::doControl(ControlTypes eControl)
{
	if(!canDoControl(eControl))
	{
		return;
	}

	switch(eControl)
	{
	case CONTROL_CENTERONSELECTION:
		GC.GetEngineUserInterface()->lookAtSelectionPlot();
		break;

	case CONTROL_SELECTYUNITTYPE:
	{
		CvInterfacePtr<ICvUnit1> pHeadSelectedUnit(GC.GetEngineUserInterface()->GetHeadSelectedUnit());
		if(pHeadSelectedUnit)
		{
			GC.GetEngineUserInterface()->selectGroup(pHeadSelectedUnit.get(), false, true, false);
		}
	}
	break;

	case CONTROL_SELECTYUNITALL:
	{
		CvInterfacePtr<ICvUnit1> pHeadSelectedUnit(GC.GetEngineUserInterface()->GetHeadSelectedUnit());
		if(pHeadSelectedUnit)
		{
			GC.GetEngineUserInterface()->selectGroup(pHeadSelectedUnit.get(), false, false, true);
		}
	}
	break;

	case CONTROL_SELECT_HEALTHY:
	{
		CvInterfacePtr<ICvUnit1> pHeadSelectedUnit(GC.GetEngineUserInterface()->GetHeadSelectedUnit());
		CvUnit* pkHeadSelectedUnit = GC.UnwrapUnitPointer(pHeadSelectedUnit.get());
		GC.GetEngineUserInterface()->ClearSelectionList();
		if(pkHeadSelectedUnit != NULL)
		{
			CvPlot* pHeadPlot = pkHeadSelectedUnit->plot();
			std::vector<CvUnit*> plotUnits;
			getPlotUnits(pHeadPlot, plotUnits);
			for(int iI = 0; iI < (int) plotUnits.size(); iI++)
			{
				CvUnit* pUnit = plotUnits[iI];

				if(pUnit->getOwner() == getActivePlayer())
				{
					if(!GET_PLAYER(pUnit->getOwner()).isSimultaneousTurns() || getGameTurn() - pUnit->getLastMoveTurn() >= 1)
					{
						if(pUnit->IsHurt())
						{
							CvInterfacePtr<ICvUnit1> pDllUnit = GC.WrapUnitPointer(pUnit);
							GC.GetEngineUserInterface()->InsertIntoSelectionList(pDllUnit.get(), true, false, true, true, true);
						}
					}
				}
			}
		}
	}
	break;

	case CONTROL_SELECTCITY:
		if(GC.GetEngineUserInterface()->isCityScreenUp())
		{
			cycleCities();
		}
		else
		{
			GC.GetEngineUserInterface()->selectLookAtCity();
		}
		GC.GetEngineUserInterface()->lookAtSelectionPlot();
		break;

	case CONTROL_SELECTCAPITAL:
	{
		CvCity* pCapitalCity = GET_PLAYER(getActivePlayer()).getCapitalCity();
		if(pCapitalCity != NULL)
		{
			CvInterfacePtr<ICvCity1> pDllCapitalCity = GC.WrapCityPointer(pCapitalCity);
			GC.GetEngineUserInterface()->selectCity(pDllCapitalCity.get());
		}
		GC.GetEngineUserInterface()->lookAtSelectionPlot();
	}
	break;

	case CONTROL_NEXTCITY:
		if(GC.GetEngineUserInterface()->isCitySelection())
		{
			cycleCities(true, !(GC.GetEngineUserInterface()->isCityScreenUp()));
		}
		else
		{
			GC.GetEngineUserInterface()->selectLookAtCity(true);
		}
		GC.GetEngineUserInterface()->lookAtSelectionPlot();
		break;

	case CONTROL_PREVCITY:
		if(GC.GetEngineUserInterface()->isCitySelection())
		{
			cycleCities(false, !(GC.GetEngineUserInterface()->isCityScreenUp()));
		}
		else
		{
			GC.GetEngineUserInterface()->selectLookAtCity(true);
		}
		GC.GetEngineUserInterface()->lookAtSelectionPlot();
		break;

	case CONTROL_NEXTUNIT:
	{
		CvInterfacePtr<ICvPlot1> pSelectionPlot(GC.GetEngineUserInterface()->getSelectionPlot());
		CvPlot* pkSelectionPlot = GC.UnwrapPlotPointer(pSelectionPlot.get());
		if(pkSelectionPlot != NULL)
		{
			cyclePlotUnits(pkSelectionPlot);
		}
		break;
	}

	case CONTROL_PREVUNIT:
	{
		CvInterfacePtr<ICvPlot1> pSelectionPlot(GC.GetEngineUserInterface()->getSelectionPlot());
		CvPlot* pkSelectionPlot = GC.UnwrapPlotPointer(pSelectionPlot.get());
		if(pkSelectionPlot != NULL)
		{
			cyclePlotUnits(pkSelectionPlot, false);
		}
		break;
	}

	case CONTROL_CYCLEUNIT:
	case CONTROL_CYCLEUNIT_ALT:
		cycleUnits(true);
		break;

	case CONTROL_CYCLEWORKER:
		cycleUnits(true, true, true);
		break;

	case CONTROL_LASTUNIT:
	{
		ICvUserInterface2* UI = GC.GetEngineUserInterface();
		CvInterfacePtr<ICvUnit1> pUnit(UI->getLastSelectedUnit());

		if(pUnit)
		{
			UI->selectUnit(pUnit.get(), true);
			UI->lookAtSelectionPlot();
		}
		else
		{
			cycleUnits(true, false);
		}

		UI->setLastSelectedUnit(NULL);
	}
	break;

	case CONTROL_ENDTURN:
	case CONTROL_ENDTURN_ALT:
		if(GC.GetEngineUserInterface()->canEndTurn() && gDLL->allAICivsProcessedThisTurn() && allUnitAIProcessed())
		{
			CvPlayerAI& kActivePlayer = GET_PLAYER(getActivePlayer());

			if (MOD_EVENTS_RED_TURN)
			// RED <<<<<
			{
				ICvEngineScriptSystem1* pkScriptSystem = gDLL->GetScriptSystem();
				if(pkScriptSystem)
				{	
					CvLuaArgsHandle args;

					args->Push(getActivePlayer());

					bool bResult = false;
					LuaSupport::CallHook(pkScriptSystem, "TurnComplete", args.get(), bResult);
				}
			}
			// RED >>>>>

			if (MOD_API_ACHIEVEMENTS)
				kActivePlayer.GetPlayerAchievements().EndTurn();

			gDLL->sendTurnComplete();

			if (MOD_API_ACHIEVEMENTS)
				CvAchievementUnlocker::EndTurn();

			GC.GetEngineUserInterface()->setInterfaceMode(INTERFACEMODE_SELECTION);
		}
		break;

	case CONTROL_FORCEENDTURN:
	{
		EndTurnBlockingTypes eBlock = GET_PLAYER(getActivePlayer()).GetEndTurnBlockingType();
		if(gDLL->allAICivsProcessedThisTurn() && allUnitAIProcessed() && (eBlock == NO_ENDTURN_BLOCKING_TYPE || eBlock == ENDTURN_BLOCKING_UNITS))
		{
			if (MOD_API_ACHIEVEMENTS)
			{
				CvPlayerAI& kActivePlayer = GET_PLAYER(getActivePlayer());
				kActivePlayer.GetPlayerAchievements().EndTurn();
			}

			if (MOD_EVENTS_RED_TURN)
			// RED <<<<<
			{
				ICvEngineScriptSystem1* pkScriptSystem = gDLL->GetScriptSystem();
				if(pkScriptSystem)
				{	
					CvLuaArgsHandle args;

					args->Push(getActivePlayer());

					bool bResult = false;
					LuaSupport::CallHook(pkScriptSystem, "TurnComplete", args.get(), bResult);
				}
			}
			// RED >>>>>

			gDLL->sendTurnComplete();

			if (MOD_API_ACHIEVEMENTS)
				CvAchievementUnlocker::EndTurn();

			SetForceEndingTurn(true);
			GC.GetEngineUserInterface()->setInterfaceMode(INTERFACEMODE_SELECTION);
		}
		break;
	}

	case CONTROL_AUTOMOVES:
		gDLL->sendAutoMoves();
		break;

	case CONTROL_PING:
		GC.GetEngineUserInterface()->setInterfaceMode(INTERFACEMODE_PING);
		break;

	case CONTROL_YIELDS:
		GC.GetEngineUserInterface()->toggleYieldVisibleMode();
		break;

	case CONTROL_RESOURCE_ALL:
		GC.GetEngineUserInterface()->toggleResourceVisibleMode();
		break;

	case CONTROL_UNIT_ICONS:
		break;

	case CONTROL_SCORES:
		break;

	case CONTROL_LOAD_GAME:
		gDLL->LoadGame();
		break;

	case CONTROL_OPTIONS_SCREEN:
		gDLL->GameplayOpenOptionsScreen();
		break;

	case CONTROL_RETIRE:
		if(!isGameMultiPlayer() || countHumanPlayersAlive() == 1)
		{
			setGameState(GAMESTATE_OVER);
			GC.GetEngineUserInterface()->setDirty(Soundtrack_DIRTY_BIT, true);
		}
		else
		{
			if(isNetworkMultiPlayer())
			{
				GC.GetEngineUserInterface()->exitingToMainMenu();
			}
		}
		break;

	case CONTROL_SAVE_GROUP:
		gDLL->SaveGame(SAVEGAME_GROUP);
		break;

	case CONTROL_SAVE_NORMAL:
		gDLL->SaveGame(SAVEGAME_NORMAL);
		break;

	case CONTROL_TOGGLE_OBSERVER_MODE:
	{
		if (!(isNetworkMultiPlayer()))	// SP only!
		{
			// use this control only if ObserverUIOverridePlayer not set
			if (getObserverUIOverridePlayer() == NO_PLAYER) 
			{
				PlayerTypes eActivePlayer = getActivePlayer();
				// already in observer mode? exit it
				if (GET_PLAYER(eActivePlayer).isObserver())
				{
					PlayerTypes eReturnPlayer = NO_PLAYER;

					// return as the currently active player
					for (int i = 0; i < MAX_MAJOR_CIVS; ++i)
					{
						CvPlayer& kCurrentPlayer = GET_PLAYER((PlayerTypes)i);
						if (kCurrentPlayer.isAlive() && kCurrentPlayer.isTurnActive())
						{
							eReturnPlayer = (PlayerTypes)i;
							break;
						}
					}

					if (eReturnPlayer != NO_PLAYER)
					{
						setAIAutoPlay(0, eReturnPlayer);
					}
				}
				else
				{
					// enter observer mode
					setAIAutoPlay(GD_INT_GET(MAX_TURNS_OBSERVER_MODE) > 0 ? GD_INT_GET(MAX_TURNS_OBSERVER_MODE) : INT_MAX, eActivePlayer);
				}
			}
		}
		break;
	}

	case CONTROL_TOGGLE_AI_TAKEOVER:
	{
		if (!(isNetworkMultiPlayer()))	// SP only!
		{
			if (!GET_PLAYER(getActivePlayer()).isObserver())
			{
				// if we're not in observer mode, we set ObserverUIOverridePlayer to the active player and enter observer mode
				setObserverUIOverridePlayer(GC.getGame().getActivePlayer());
				setAIAutoPlay(GD_INT_GET(MAX_TURNS_OBSERVER_MODE) > 0 ? GD_INT_GET(MAX_TURNS_OBSERVER_MODE) : INT_MAX, GC.getGame().getActivePlayer());
			}
			else
			{				
				// if in observer mode, we give control back to the original player
				if (getObserverUIOverridePlayer() != NO_PLAYER)
				{
					PlayerTypes eReturnPlayer = getObserverUIOverridePlayer();
					setAIAutoPlay(0, eReturnPlayer);
					setObserverUIOverridePlayer(NO_PLAYER);
				}
			}
		}
	}
	break;

	case CONTROL_SWITCH_TO_NEXT_PLAYER:
	{
		// give the human control over the next AI player
		if (!(isNetworkMultiPlayer()))	// SP only!
		{
			PlayerTypes eActivePlayer = getActivePlayer();
			// only allowed if it's a human player's turn
			if (GET_PLAYER(eActivePlayer).isTurnActive() && !GET_PLAYER(eActivePlayer).isObserver())
			{
				PlayerTypes eNextPlayer = NO_PLAYER;
				for (int iJ = (GET_PLAYER(eActivePlayer).GetID() + 1); iJ < MAX_PLAYERS; iJ++)
				{
					CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iJ);
					if (kLoopPlayer.isAlive() && kLoopPlayer.isMajorCiv())
					{
						eNextPlayer = (PlayerTypes)iJ;
						break;
					}
				}
				if (eNextPlayer == NO_PLAYER)
				{
					for (int iJ = 0; iJ < GET_PLAYER(eActivePlayer).GetID(); iJ++)
					{
						CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iJ);
						if (kLoopPlayer.isAlive() && kLoopPlayer.isMajorCiv())
						{
							eNextPlayer = (PlayerTypes)iJ;
							break;
						}
					}
				}
				if (eNextPlayer != NO_PLAYER)
				{
					PlayerTypes ePreviousPlayer = CvPreGame::activePlayer();
					CvPreGame::setHandicap(eNextPlayer, CvPreGame::handicap(eActivePlayer));
					CvPreGame::setSlotStatus(ePreviousPlayer, SS_COMPUTER);
					CvPreGame::VerifyHandicap(ePreviousPlayer);
					CvPreGame::setSlotStatus(eNextPlayer, SS_TAKEN);
					GC.getGame().setActivePlayer(eNextPlayer, false /*bForceHotSeat*/, true /*bAutoplaySwitch*/);
					GET_PLAYER(ePreviousPlayer).SetEndTurnBlocking(NO_ENDTURN_BLOCKING_TYPE, -1);
					GET_PLAYER(ePreviousPlayer).GetDiplomacyAI()->SlotStateChange();
					GET_PLAYER(eNextPlayer).GetDiplomacyAI()->SlotStateChange();
				}
			}
		}
	}
	break;

	case CONTROL_QUICK_SAVE:
		if(!(isNetworkMultiPlayer()))	// SP only!
		{
			gDLL->QuickSave();
		}
		break;

	case CONTROL_QUICK_LOAD:
		if(!(isNetworkMultiPlayer()))	// SP only!
		{
			gDLL->QuickLoad();
		}
		break;

	case CONTROL_CIVILOPEDIA:
		gDLL->GameplaySearchForPediaEntry("OPEN_VIA_HOTKEY");
		break;

	case CONTROL_POLICIES_SCREEN:
	{
		CvPopupInfo kPopup(BUTTONPOPUP_CHOOSEPOLICY, getActivePlayer());
		kPopup.iData1 = 1;
		GC.GetEngineUserInterface()->AddPopup(kPopup);
	}
	break;

	case CONTROL_FOREIGN_SCREEN:
	{
		CvPopupInfo kPopup(BUTTONPOPUP_DIPLOMATIC_OVERVIEW, getActivePlayer());
		kPopup.iData1 = 1;
		GC.GetEngineUserInterface()->AddPopup(kPopup);
	}
	break;

	case CONTROL_MILITARY_SCREEN:
	{
		CvPopupInfo kPopup(BUTTONPOPUP_MILITARY_OVERVIEW, getActivePlayer());
		kPopup.iData1 = 1;
		GC.GetEngineUserInterface()->AddPopup(kPopup);
	}
	break;

	case CONTROL_TECH_CHOOSER:
	{
		CvPopupInfo kPopup(BUTTONPOPUP_TECH_TREE, getActivePlayer());

		// If the popup queue is empty, just show the tech tree, don't queue it up - otherwise, if we, say, go into the Pedia from here, it'll end up BEHIND the tech tree
		if(!GC.GetEngineUserInterface()->IsPopupQueueEmpty())
			kPopup.iData1 = 1;

		GC.GetEngineUserInterface()->AddPopup(kPopup);
	}
	break;

	case CONTROL_TURN_LOG:
	{
		CvPopupInfo kPopup(BUTTONPOPUP_NOTIFICATION_LOG, getActivePlayer());
		kPopup.iData1 = 1;
		GC.GetEngineUserInterface()->AddPopup(kPopup);
	}
	break;

	case CONTROL_DOMESTIC_SCREEN:
	{
		CvPopupInfo kPopup(BUTTONPOPUP_ECONOMIC_OVERVIEW, getActivePlayer());
		kPopup.iData1 = 1;
		GC.GetEngineUserInterface()->AddPopup(kPopup);
	}
	break;

	case CONTROL_VICTORY_SCREEN:
	{
		CvPopupInfo kPopup(BUTTONPOPUP_VICTORY_INFO, getActivePlayer());
		kPopup.iData1 = 1;
		GC.GetEngineUserInterface()->AddPopup(kPopup);
	}
	break;

	case CONTROL_INFO:
	{
		CvPopupInfo kPopup(BUTTONPOPUP_DEMOGRAPHICS, getActivePlayer());
		kPopup.iData1 = 1;
		GC.GetEngineUserInterface()->AddPopup(kPopup);
	}
	break;

	case CONTROL_ADVISOR_COUNSEL:
	{
		CvPopupInfo kPopup(BUTTONPOPUP_ADVISOR_COUNSEL);
		kPopup.iData1 = 1;
		GC.GetEngineUserInterface()->AddPopup(kPopup);
	}
	break;

	case CONTROL_ESPIONAGE_OVERVIEW:
	{
		CvPopupInfo kPopup(BUTTONPOPUP_ESPIONAGE_OVERVIEW, getActivePlayer());
		kPopup.iData1 = 1;
		GC.GetEngineUserInterface()->AddPopup(kPopup);
	}
	break;

	case CONTROL_RELIGION_OVERVIEW:
	{
		CvPopupInfo kPopup(BUTTONPOPUP_RELIGION_OVERVIEW, getActivePlayer());
		kPopup.iData1 = 1;
		GC.GetEngineUserInterface()->AddPopup(kPopup);
	}
	break;

	case CONTROL_RESTART_GAME:
	{
		gDLL->RestartGame();
	}
	break;

	default:
		ASSERT_DEBUG(false, "eControl did not match any valid options");
		break;
	}
}

//	--------------------------------------------------------------------------------
bool CvGame::IsForceEndingTurn() const
{
	return m_bForceEndingTurn;
}

//	--------------------------------------------------------------------------------
void CvGame::SetForceEndingTurn(bool bValue)
{
	m_bForceEndingTurn = bValue;
}

//	--------------------------------------------------------------------------------
int CvGame::getAdjustedPopulationPercent(VictoryTypes eVictory) const
{
	int iPopulation = 0;
	int iBestPopulation = 0;
	int iNextBestPopulation = 0;
	int iI = 0;

	CvVictoryInfo* pkVictoryInfo = GC.getVictoryInfo(eVictory);
	if(pkVictoryInfo == NULL)
	{
		return 0;
	}

	if(pkVictoryInfo->getPopulationPercentLead() == 0)
	{
		return 0;
	}

	if(getTotalPopulation() == 0)
	{
		return 100;
	}

	iBestPopulation = 0;
	iNextBestPopulation = 0;

	for(iI = 0; iI < MAX_CIV_TEAMS; iI++)
	{
		if(GET_TEAM((TeamTypes)iI).isAlive())
		{
			iPopulation = GET_TEAM((TeamTypes)iI).getTotalPopulation();

			if(iPopulation > iBestPopulation)
			{
				iNextBestPopulation = iBestPopulation;
				iBestPopulation = iPopulation;
			}
			else if(iPopulation > iNextBestPopulation)
			{
				iNextBestPopulation = iPopulation;
			}
		}
	}

	return std::min(100, (((iNextBestPopulation * 100) / getTotalPopulation()) + pkVictoryInfo->getPopulationPercentLead()));
}

//	--------------------------------------------------------------------------------
int CvGame::getAdjustedLandPercent(VictoryTypes eVictory) const
{
	CvVictoryInfo* pkVictoryInfo = GC.getVictoryInfo(eVictory);
	if(pkVictoryInfo == NULL)
		return 0;

	if(pkVictoryInfo->getLandPercent() == 0)
	{
		return 0;
	}

	int iPercent = pkVictoryInfo->getLandPercent();

	iPercent -= (countCivTeamsEverAlive() * 2);

	return std::max(iPercent, pkVictoryInfo->getMinLandPercent());
}

//	--------------------------------------------------------------------------------
int CvGame::GetNumMajorCivsEver(bool bOnlyStart) const
{
	if (bOnlyStart)
		return m_iNumMajorCivsAliveAtGameStart;

	int iCount = 0;
	for (int iMajorLoop = 0; iMajorLoop < MAX_MAJOR_CIVS; iMajorLoop++)
	{
		if (GET_PLAYER((PlayerTypes)iMajorLoop).isEverAlive())
			iCount++;
	}

	return iCount;
}

//	--------------------------------------------------------------------------------
int CvGame::GetNumMajorCivsAlive() const
{
	int iCount = 0;
	for (int iMajorLoop = 0; iMajorLoop < MAX_MAJOR_CIVS; iMajorLoop++)
	{
		if (GET_PLAYER((PlayerTypes)iMajorLoop).isAlive())
			iCount++;
	}

	return iCount;
}

//	--------------------------------------------------------------------------------
int CvGame::GetNumMinorCivsEver(bool bOnlyStart) const
{
	if (bOnlyStart)
		return m_iNumMinorCivsAliveAtGameStart;

	int iCount = 0;
	for (int iMinorLoop = MAX_MAJOR_CIVS; iMinorLoop < MAX_CIV_PLAYERS; iMinorLoop++)
	{
		if (GET_PLAYER((PlayerTypes)iMinorLoop).isEverAlive())
			iCount++;
	}

	return iCount;
}

//	--------------------------------------------------------------------------------
int CvGame::GetNumMinorCivsAlive() const
{
	int iCount = 0;
	for (int iMinorLoop = MAX_MAJOR_CIVS; iMinorLoop < MAX_CIV_PLAYERS; iMinorLoop++)
	{
		if (GET_PLAYER((PlayerTypes)iMinorLoop).isAlive())
			iCount++;
	}

	return iCount;
}

//	--------------------------------------------------------------------------------
int CvGame::countCivPlayersAlive() const
{
	int iCount = 0;
	int iI = 0;

	iCount = 0;

	for(iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if(GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			iCount++;
		}
	}

	return iCount;
}


//	--------------------------------------------------------------------------------
int CvGame::countCivPlayersEverAlive() const
{
	int iCount = 0;
	int iI = 0;

	iCount = 0;

	for(iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if(GET_PLAYER((PlayerTypes)iI).isEverAlive())
		{
			iCount++;
		}
	}

	return iCount;
}


//	--------------------------------------------------------------------------------
int CvGame::countCivTeamsAlive() const
{
	int iCount = 0;
	int iI = 0;

	iCount = 0;

	for(iI = 0; iI < MAX_CIV_TEAMS; iI++)
	{
		if(GET_TEAM((TeamTypes)iI).isAlive())
		{
			iCount++;
		}
	}

	return iCount;
}


//	--------------------------------------------------------------------------------
int CvGame::countCivTeamsEverAlive() const
{
	int iCount = 0;
	int iI = 0;

	iCount = 0;

	for(iI = 0; iI < MAX_CIV_TEAMS; iI++)
	{
		if(GET_TEAM((TeamTypes)iI).isEverAlive())
		{
			iCount++;
		}
	}

	return iCount;
}


//	--------------------------------------------------------------------------------
int CvGame::countHumanPlayersAlive() const
{
	int iCount = 0;
	int iI = 0;

	iCount = 0;

	for(iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if(GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if(GET_PLAYER((PlayerTypes)iI).isHuman())
			{
				iCount++;
			}
		}
	}

	return iCount;
}


//	--------------------------------------------------------------------------------
int CvGame::countHumanPlayersEverAlive() const
{
	int iCount = 0;
	int iI = 0;

	iCount = 0;

	for(iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if(GET_PLAYER((PlayerTypes)iI).isEverAlive())
		{
			if(GET_PLAYER((PlayerTypes)iI).isHuman())
			{
				iCount++;
			}
		}
	}

	return iCount;
}

//	--------------------------------------------------------------------------------
int CvGame::countSeqHumanTurnsUntilPlayerTurn( PlayerTypes playerID ) const
{
	//This function counts the number of sequential human player turns that remain before this player's turn.
	int humanTurnsUntilMe = 0;
	bool startCountingPlayers = false;
	CvPlayer& targetPlayer = GET_PLAYER(playerID);
	if(targetPlayer.isSimultaneousTurns())
	{
		//target player is playing simultaneous turns and is not actually in the sequential turn sequence.
		//Count every human player in sequential turn mode who is taking or hasn't taken their turn.
		for(int i = 0; i < MAX_PLAYERS; ++i)
		{
			CvPlayer& kCurrentPlayer = GET_PLAYER((PlayerTypes)i);
			if(kCurrentPlayer.isHuman() 
				&& kCurrentPlayer.isAlive() 
				&& !kCurrentPlayer.isSimultaneousTurns())
			{
				//another human player who is playing sequential turns.
				if(kCurrentPlayer.isTurnActive())
				{
					//This player is currently playing their turn. Start counting human players after this point.
					startCountingPlayers = true;
					humanTurnsUntilMe++;
				}
				else if(startCountingPlayers)
				{
					//This is a human player who's before us in line.
					humanTurnsUntilMe++;
				}
			}
		}	
	}
	else
	{
		//target player is playing sequential turns.  
		//Our next turn will begin after every sequential player has finished this turn 
		//AND everyone ahead of us in the sequence has finished their turn for the NEXT turn.

		//Starting after us, count every player who's playing sequential turns.
		startCountingPlayers = false;
		int curPlayerIdx = (targetPlayer.GetID()+1)%MAX_PLAYERS;
		for(int i = 0; i < MAX_PLAYERS; curPlayerIdx = ++curPlayerIdx%MAX_PLAYERS, ++i)
		{
			CvPlayer& kCurrentPlayer = GET_PLAYER((PlayerTypes)curPlayerIdx);
			if(kCurrentPlayer.GetID() == targetPlayer.GetID())
			{
				//This is us.  We've looped back to ourself.  We're done.
				break;
			}
			else if(kCurrentPlayer.isHuman() 
				&& kCurrentPlayer.isAlive() 
				&& !kCurrentPlayer.isSimultaneousTurns())
			{
				//another human player who is playing sequential turns.
				if(kCurrentPlayer.isTurnActive())
				{
					//This player is currently playing their turn. Start counting human players after this point.
					startCountingPlayers = true;
					humanTurnsUntilMe++;
				}
				else if(startCountingPlayers)
				{
					//This is a human player who's before us in line.
					humanTurnsUntilMe++;
				}
			}
		}	
	}

	return humanTurnsUntilMe;
}

//	--------------------------------------------------------------------------------
int CvGame::countTotalCivPower()
{
	int iCount = 0;
	int iI = 0;

	iCount = 0;

	for(iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if(GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			iCount += GET_PLAYER((PlayerTypes)iI).getPower();
		}
	}

	return iCount;
}


//	--------------------------------------------------------------------------------
int CvGame::countTotalNukeUnits()
{
	int iCount = 0;
	int iI = 0;

	iCount = 0;

	for(iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if(GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			iCount += GET_PLAYER((PlayerTypes)iI).getNumNukeUnits();
		}
	}

	return iCount;
}


//	--------------------------------------------------------------------------------
int CvGame::countKnownTechNumTeams(TechTypes eTech)
{
	int iCount = 0;
	int iI = 0;

	iCount = 0;

	for(iI = 0; iI < MAX_TEAMS; iI++)
	{
		if(GET_TEAM((TeamTypes)iI).isEverAlive())
		{
			if(GET_TEAM((TeamTypes)iI).GetTeamTechs()->HasTech(eTech))
			{
				iCount++;
			}
		}
	}

	return iCount;
}

//	--------------------------------------------------------------------------------
int CvGame::goldenAgeLength(int iManualLength) const
{
	int iLength = 0;
	
	// Sometimes we need to alter a manual number of golden age turns by the game speed
	if (iManualLength >= 0)
	{
		iLength = iManualLength;
	}
	else
	{
		iLength = /*10*/ GD_INT_GET(GOLDEN_AGE_LENGTH);
	}

	iLength *= getGameSpeedInfo().getGoldenAgePercent();
	iLength /= 100;

	return iLength;
}

//	--------------------------------------------------------------------------------
int CvGame::victoryDelay(VictoryTypes eVictory) const
{
	ASSERT_DEBUG(eVictory >= 0 && eVictory < GC.getNumVictoryInfos());

	CvVictoryInfo* pkVictoryInfo = GC.getVictoryInfo(eVictory);
	ASSERT_DEBUG(pkVictoryInfo);

	if(pkVictoryInfo == NULL)
		return 0;

	int iLength = pkVictoryInfo->getVictoryDelayTurns();

	iLength *= getGameSpeedInfo().getVictoryDelayPercent();
	iLength /= 100;

	return iLength;
}


//	--------------------------------------------------------------------------------
int CvGame::getImprovementUpgradeTimeMod(ImprovementTypes eImprovement, const CvPlot* pPlot) const
{
	int iTime = 100;

	CvImprovementEntry* pkImprovementInfo = GC.getImprovementInfo(eImprovement);
	if(pPlot != NULL && NULL != pkImprovementInfo)
	{
		if(pPlot->isRiver())
		{
			if(pkImprovementInfo->GetRiverSideUpgradeMod() > 0)
			{
				iTime *= pkImprovementInfo->GetRiverSideUpgradeMod();
				iTime /= 100;
			}
		}

		if(pPlot->isCoastalLand())
		{
			if(pkImprovementInfo->GetCoastalLandUpgradeMod() > 0)
			{
				iTime *= pkImprovementInfo->GetCoastalLandUpgradeMod();
				iTime /= 100;
			}
		}

		if(pPlot->isHills())
		{
			if(pkImprovementInfo->GetHillsUpgradeMod() > 0)
			{
				iTime *= pkImprovementInfo->GetHillsUpgradeMod();
				iTime /= 100;
			}
		}
	}

	return iTime;
}

//	--------------------------------------------------------------------------------
int CvGame::getImprovementUpgradeTime(ImprovementTypes eImprovement, const CvPlot* pPlot) const
{
	CvImprovementEntry* pkImprovementInfo = GC.getImprovementInfo(eImprovement);
	ASSERT_DEBUG(pkImprovementInfo);

	if(pkImprovementInfo == NULL)
		return 0;

	int iTime = pkImprovementInfo->GetUpgradeTime();

	iTime *= getImprovementUpgradeTimeMod(eImprovement, pPlot);
	iTime /= 100;

	iTime *= getGameSpeedInfo().getImprovementPercent();
	iTime /= 100;

	iTime *= getStartEraInfo().getImprovementPercent();
	iTime /= 100;

	return iTime;
}



//	--------------------------------------------------------------------------------
bool CvGame::canTrainNukes() const
{
	for(int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		const PlayerTypes ePlayer = static_cast<PlayerTypes>(iI);
		if(GET_PLAYER(ePlayer).isAlive())
		{
			for(int iJ = 0; iJ < GC.getNumUnitInfos(); iJ++)
			{
				const UnitTypes eUnit = static_cast<UnitTypes>(iJ);
				CvUnitEntry* pkUnitInfo = GC.getUnitInfo(eUnit);
				if(pkUnitInfo)
				{
					if(pkUnitInfo->GetNukeDamageLevel() > 0)
					{
						if(GET_PLAYER(ePlayer).canTrainUnit(eUnit))
						{
							return true;
						}
					}
				}
			}
		}
	}

	return false;
}


//	--------------------------------------------------------------------------------
EraTypes CvGame::getCurrentEra() const
{
	return m_eGameEra;
}

void CvGame::UpdateGameEra()
{
	float fEra = 0;
	int iCount = 0;
	for (int iI = 0; iI < MAX_TEAMS; iI++)
	{
		if (GET_TEAM((TeamTypes)iI).isAlive() && GET_TEAM((TeamTypes)iI).isMajorCiv())
		{
			fEra += static_cast<float>(GET_TEAM((TeamTypes)iI).GetCurrentEra());
			iCount++;
		}
	}
	if (iCount >= 0)
	{
		int iRoundedEra = int(fEra / (max(1, iCount)) + 0.5f);
		m_eGameEra = (EraTypes)iRoundedEra;
	}
	else
	{
		m_eGameEra = NO_ERA;
	}
}


//	--------------------------------------------------------------------------------
TeamTypes CvGame::getActiveTeam() const
{
	PlayerTypes eActivePlayer = getActivePlayer();
	if(eActivePlayer == NO_PLAYER)
	{
		return NO_TEAM;
	}
	else
	{
		return (TeamTypes)GET_PLAYER(eActivePlayer).getTeam();
	}
}


//	--------------------------------------------------------------------------------
CivilizationTypes CvGame::getActiveCivilizationType() const
{
	PlayerTypes eActivePlayer = getActivePlayer();
	if(eActivePlayer == NO_PLAYER)
	{
		return NO_CIVILIZATION;
	}
	else
	{
		return (CivilizationTypes)GET_PLAYER(eActivePlayer).getCivilizationType();
	}
}

//	--------------------------------------------------------------------------------
bool CvGame::isReallyNetworkMultiPlayer() const
{
	return CvPreGame::isReallyNetworkMultiPlayer();
}

//	--------------------------------------------------------------------------------
bool CvGame::isNetworkMultiPlayer() const
{
	return CvPreGame::isNetworkMultiplayerGame();
}


//	--------------------------------------------------------------------------------
bool CvGame::isGameMultiPlayer() const
{
	return (isNetworkMultiPlayer() || isPbem() || isHotSeat());
}


//	--------------------------------------------------------------------------------
bool CvGame::isTeamGame() const
{
	ASSERT_DEBUG(countCivPlayersAlive() >= countCivTeamsAlive());
	return (countCivPlayersAlive() > countCivTeamsAlive());
}

//	--------------------------------------------------------------------------------
/// Return control to the user after an autoplay
void CvGame::ReviveActivePlayer()
{
	if(!(GET_PLAYER(getActivePlayer()).isAlive()))
	{
		setAIAutoPlay(0, m_eAIAutoPlayReturnPlayer);
	}
}

PlayerTypes CvGame::getFirstAlivePlayer() const
{
	for (int iMajorLoop = 0; iMajorLoop < MAX_MAJOR_CIVS; iMajorLoop++)
	{
		if (GET_PLAYER((PlayerTypes)iMajorLoop).isAlive())
			return (PlayerTypes)iMajorLoop;
	}

	return NO_PLAYER;
}

//	--------------------------------------------------------------------------------
int CvGame::getNumHumanPlayers()
{
	return CvPreGame::numHumans();
}

//	--------------------------------------------------------------------------------
int CvGame::getNumHumansInHumanWars(PlayerTypes ignorePlayer)
{//returns the number of human players who are currently at war with other human players.
	int humansWarringHumans = 0;
	for(int i = 0; i < MAX_CIV_PLAYERS; ++i)
	{
		const CvPlayer& curPlayer = GET_PLAYER((PlayerTypes)i);
		if(curPlayer.isAlive() 
			&& curPlayer.isHuman() 
			&& (ignorePlayer == NO_PLAYER || curPlayer.GetID() != ignorePlayer)  //ignore the ignore player
			&& GET_TEAM(curPlayer.getTeam()).isAtWarWithHumans())
		{
			++humansWarringHumans;
		}
	}
	return humansWarringHumans;
}

//	--------------------------------------------------------------------------------
int CvGame::getNumSequentialHumans(PlayerTypes ignorePlayer)
{//returns the number of human players who are playing sequential turns.
	int seqHumans = 0;
	for(int i = 0; i < MAX_CIV_PLAYERS; ++i)
	{
		const CvPlayer& curPlayer = GET_PLAYER((PlayerTypes)i);
		if(curPlayer.isAlive() 
			&& curPlayer.isHuman() 
			&& !curPlayer.isSimultaneousTurns()
			&& (ignorePlayer == NO_PLAYER || curPlayer.GetID() != ignorePlayer))  //ignore the ignore player
		{
			++seqHumans;
		}
	}
	return seqHumans;
}

//	------------------------------------------------------------------------------------------------
int CvGame::getGameTurn() const
{
	return CvPreGame::gameTurn();
}

//	------------------------------------------------------------------------------------------------
void CvGame::setGameTurn(int iNewValue)
{
	if(getGameTurn() != iNewValue)
	{
		std::string turnMessage = std::string("Game Turn ") + FSerialization::toString(iNewValue) + std::string("\n");
		gDLL->netMessageDebugLog(turnMessage);

		CvPreGame::setGameTurn(iNewValue);
		ASSERT_DEBUG(getGameTurn() >= 0);

		setScoreDirty(true);

		GC.GetEngineUserInterface()->setDirty(TurnTimer_DIRTY_BIT, true);
		GC.GetEngineUserInterface()->setDirty(GameData_DIRTY_BIT, true);
		m_sentAutoMoves = false;
		gDLL->GameplayTurnChanged(iNewValue);
		endTurnTimerReset();
	}
}

//	------------------------------------------------------------------------------------------------
void CvGame::incrementGameTurn()
{
	setGameTurn(getGameTurn() + 1);
}


//	--------------------------------------------------------------------------------
int CvGame::getTurnYear(int iGameTurn) const
{
	// moved the body of this method to Game Core Utils so we have access for other games than the current one (replay screen in HOF)
	return getTurnYearForGame(iGameTurn, getStartYear(), getCalendar(), getGameSpeedType());
}


//	--------------------------------------------------------------------------------
int CvGame::getGameTurnYear()
{
	return getTurnYear(getGameTurn());
}


//	--------------------------------------------------------------------------------
int CvGame::getElapsedGameTurns() const
{
	return m_iElapsedGameTurns;
}


//	--------------------------------------------------------------------------------
void CvGame::incrementElapsedGameTurns()
{
	m_iElapsedGameTurns++;
}


//	--------------------------------------------------------------------------------
int CvGame::getMaxTurns() const
{
	return CvPreGame::maxTurns();
}


//	--------------------------------------------------------------------------------
void CvGame::setMaxTurns(int iNewValue) const
{
	CvPreGame::setMaxTurns(iNewValue);
	ASSERT_DEBUG(getMaxTurns() >= 0);
}


//	--------------------------------------------------------------------------------
void CvGame::changeMaxTurns(int iChange)
{
	setMaxTurns(getMaxTurns() + iChange);
}


//	--------------------------------------------------------------------------------
int CvGame::getMaxCityElimination() const
{
	return CvPreGame::maxCityElimination();
}


//	--------------------------------------------------------------------------------
void CvGame::setMaxCityElimination(int iNewValue) const
{
	CvPreGame::setMaxCityElimination(iNewValue);
	ASSERT_DEBUG(getMaxCityElimination() >= 0);
}

//	--------------------------------------------------------------------------------
int CvGame::getStartTurn() const
{
	return m_iStartTurn;
}

//	--------------------------------------------------------------------------------
void CvGame::setStartTurn(int iNewValue)
{
	m_iStartTurn = iNewValue;
}

//	--------------------------------------------------------------------------------
int CvGame::GetWinningTurn() const
{
	return m_iWinningTurn;
}

//	--------------------------------------------------------------------------------
void CvGame::SetWinningTurn(int iNewValue)
{
	m_iWinningTurn = iNewValue;
}


//	--------------------------------------------------------------------------------
int CvGame::getStartYear() const
{
	return m_iStartYear;
}


//	--------------------------------------------------------------------------------
void CvGame::setStartYear(int iNewValue)
{
	m_iStartYear = iNewValue;
}


//	--------------------------------------------------------------------------------
int CvGame::getEstimateEndTurn() const
{
	return m_iEstimateEndTurn;
}


//	--------------------------------------------------------------------------------
void CvGame::setEstimateEndTurn(int iNewValue)
{
	m_iEstimateEndTurn = iNewValue;
}

//	--------------------------------------------------------------------------------
int CvGame::getDefaultEstimateEndTurn() const
{
	return m_iDefaultEstimateEndTurn;
}


//	--------------------------------------------------------------------------------
void CvGame::setDefaultEstimateEndTurn(int iNewValue)
{
	m_iDefaultEstimateEndTurn = iNewValue;
}


//	--------------------------------------------------------------------------------
int CvGame::getTurnSlice() const
{
	return m_iTurnSlice;
}


//	--------------------------------------------------------------------------------
int CvGame::getMinutesPlayed() const
{
	return (getTurnSlice() / gDLL->getTurnsPerMinute());
}


//	--------------------------------------------------------------------------------
void CvGame::setTurnSlice(int iNewValue)
{
	m_iTurnSlice = iNewValue;
}


//	--------------------------------------------------------------------------------
void CvGame::changeTurnSlice(int iChange)
{
	setTurnSlice(getTurnSlice() + iChange);
}

//	--------------------------------------------------------------------------------
void CvGame::resetTurnTimer(bool resetGameTurnStart)
{
	m_curTurnTimer.Start();
	m_fCurrentTurnTimerPauseDelta = 0;
	if(resetGameTurnStart)
	{
		m_timeSinceGameTurnStart.Start();
	}
}

//	--------------------------------------------------------------------------------
int CvGame::getMaxTurnLen() const
{//returns the amount of time players are being given for this turn.
	if(getPitbossTurnTime() != 0)
	{//manually set turn time.
		if(isPitboss())
		{// Turn time is in hours
			return (getPitbossTurnTime() * 3600);
		}
		else
		{
			return getPitbossTurnTime();
		}
	}
	else
	{
		int iMaxUnits = 0;
		int iMaxCities = 0;

		// Find out who has the most units and who has the most cities
		// Calculate the max turn time based on the max number of units and cities
		for(int i = 0; i < MAX_CIV_PLAYERS; ++i)
		{
			if(GET_PLAYER((PlayerTypes)i).isAlive())
			{
				if(GET_PLAYER((PlayerTypes)i).getNumUnits() > iMaxUnits)
				{
					iMaxUnits = GET_PLAYER((PlayerTypes)i).getNumUnits();
				}
				if(GET_PLAYER((PlayerTypes)i).getNumCities() > iMaxCities)
				{
					iMaxCities = GET_PLAYER((PlayerTypes)i).getNumCities();
				}
			}
		}

		// Now return turn len based on base len and unit and city resources
		const CvTurnTimerInfo& kTurnTimer = CvPreGame::turnTimerInfo();
		int baseTurnTime = (kTurnTimer.getBaseTime() +
		        (kTurnTimer.getCityResource() * iMaxCities) +
		        (kTurnTimer.getUnitResource() * iMaxUnits));
		
		return baseTurnTime;
	}
}

//	--------------------------------------------------------------------------------
bool CvGame::IsStaticTutorialActive() const
{
	return m_bStaticTutorialActive;
}

//	--------------------------------------------------------------------------------
void CvGame::SetStaticTutorialActive(bool bStaticTutorialActive)
{
	m_bStaticTutorialActive = bStaticTutorialActive;
}

//	--------------------------------------------------------------------------------
bool CvGame::HasAdvisorMessageBeenSeen(const char* szAdvisorMessageName)
{
	stringHash hasher;
	std::tr1::unordered_set<size_t>::iterator it = m_AdvisorMessagesViewed.find( hasher(szAdvisorMessageName) );
	return it != m_AdvisorMessagesViewed.end();
}

//	--------------------------------------------------------------------------------
void CvGame::SetAdvisorMessageHasBeenSeen(const char* szAdvisorMessageName, bool bSeen)
{
	stringHash hasher;
	if(bSeen)
	{
		m_AdvisorMessagesViewed.insert( hasher(szAdvisorMessageName) );
	}
	else
	{
		m_AdvisorMessagesViewed.erase( hasher(szAdvisorMessageName) );
	}
}

//	--------------------------------------------------------------------------------
bool CvGame::IsCityScreenBlocked()
{
	return CvPreGame::IsCityScreenBlocked();
}

//	--------------------------------------------------------------------------------
bool CvGame::CanOpenCityScreen(PlayerTypes eOpener, CvCity* pCity)
{
	if(GET_PLAYER(eOpener).getTeam() == pCity->getTeam())
	{
		return true;
	}
	else if (GET_PLAYER(eOpener).isObserver())
	{
		return true;
	}
	else if (!GET_PLAYER(pCity->getOwner()).isMinorCiv())
	{
		if (!MOD_BALANCE_VP && (GET_PLAYER(eOpener).GetEspionage()->HasEstablishedSurveillanceInCity(pCity) || GET_PLAYER(eOpener).GetEspionage()->IsAnySchmoozing(pCity)))
		{
			return true;
		}
		else if (MOD_BALANCE_VP && pCity->GetCityEspionage()->GetRevealCityScreen(eOpener))
		{
			return true;
		}
	}

	return false;
}

//	--------------------------------------------------------------------------------
int CvGame::getTargetScore() const
{
	return CvPreGame::targetScore();
}


//	--------------------------------------------------------------------------------
void CvGame::setTargetScore(int iNewValue) const
{
	CvPreGame::setTargetScore(iNewValue);
	ASSERT_DEBUG(getTargetScore() >= 0);
}


//	--------------------------------------------------------------------------------
int CvGame::getNumGameTurnActive()
{
	int numActive = 0;
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		if(GET_PLAYER((PlayerTypes)i).isAlive() && GET_PLAYER((PlayerTypes)i).isTurnActive())
		{
			++numActive;
		}
	}
	return numActive;
}


//	--------------------------------------------------------------------------------
int CvGame::countNumHumanGameTurnActive()
{
	int iCount = 0;
	int iI = 0;

	iCount = 0;

	for(iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if(GET_PLAYER((PlayerTypes)iI).isHuman())
		{
			if(GET_PLAYER((PlayerTypes)iI).isTurnActive())
			{
				iCount++;
			}
		}
	}

	return iCount;
}


//	--------------------------------------------------------------------------------
void CvGame::changeNumGameTurnActive(int iChange, const std::string& why)
{
	char changeBuf[8] = {0};
	char activeBuf[8] = {0};
	sprintf_s(changeBuf, "%d", iChange);
	sprintf_s(activeBuf, "%d", getNumGameTurnActive());
	std::string output = "changeNumGameTurnActive(";
	output += changeBuf;
	output += ") m_iNumActive=";
	output += activeBuf;
	output += " : " + why;
	gDLL->netMessageDebugLog(output);
	ASSERT_DEBUG(getNumGameTurnActive() >= 0);
}


//	--------------------------------------------------------------------------------
int CvGame::getNumCities() const
{
	return m_iNumCities;
}


//	--------------------------------------------------------------------------------
int CvGame::getNumCivCities() const
{
	return (getNumCities() - GET_PLAYER(BARBARIAN_PLAYER).getNumCities());
}


//	--------------------------------------------------------------------------------
void CvGame::changeNumCities(int iChange)
{
	m_iNumCities = (m_iNumCities + iChange);
	ASSERT_DEBUG(getNumCities() >= 0);
}


//	--------------------------------------------------------------------------------
int CvGame::getTotalPopulation() const
{
	return m_iTotalPopulation;
}


//	--------------------------------------------------------------------------------
void CvGame::changeTotalPopulation(int iChange)
{
	m_iTotalPopulation = (m_iTotalPopulation + iChange);
	ASSERT_DEBUG(getTotalPopulation() >= 0);
}

//	--------------------------------------------------------------------------------
int CvGame::getTotalEconomicValue() const
{
	return m_iTotalEconomicValue;
}


//	--------------------------------------------------------------------------------
void CvGame::setTotalEconomicValue(int iChange)
{
	m_iTotalEconomicValue = iChange;
	ASSERT_DEBUG(getTotalEconomicValue() >= 0);
}

//	--------------------------------------------------------------------------------
int CvGame::getHighestEconomicValue() const
{
	return m_iHighestEconomicValue;
}


//	--------------------------------------------------------------------------------
void CvGame::setHighestEconomicValue(int iChange)
{
	m_iHighestEconomicValue = iChange;
	ASSERT_DEBUG(getHighestEconomicValue() >= 0);
}

//	--------------------------------------------------------------------------------
int CvGame::getMedianEconomicValue() const
{
	return m_iMedianEconomicValue;
}

//	--------------------------------------------------------------------------------
void CvGame::setMedianEconomicValue(int iChange)
{
	m_iMedianEconomicValue = iChange;
	ASSERT_DEBUG(getMedianEconomicValue() >= 0);
}

//	--------------------------------------------------------------------------------
int CvGame::getNoNukesCount() const
{
	return m_iNoNukesCount;
}


//	--------------------------------------------------------------------------------
bool CvGame::isNoNukes() const
{
	return (getNoNukesCount() > 0);
}


//	--------------------------------------------------------------------------------
void CvGame::changeNoNukesCount(int iChange)
{
	m_iNoNukesCount = (m_iNoNukesCount + iChange);
	ASSERT_DEBUG(getNoNukesCount() >= 0);
}

//	--------------------------------------------------------------------------------
int CvGame::getNukesExploded() const
{
	return m_iNukesExploded;
}


//	--------------------------------------------------------------------------------
void CvGame::changeNukesExploded(int iChange)
{
	m_iNukesExploded = (m_iNukesExploded + iChange);
}


//	--------------------------------------------------------------------------------
int CvGame::getMaxPopulation() const
{
	return m_iMaxPopulation;
}


//	--------------------------------------------------------------------------------
int CvGame::getInitPopulation() const
{
	return m_iInitPopulation;
}


//	--------------------------------------------------------------------------------
int CvGame::getInitLand() const
{
	return m_iInitLand;
}


//	--------------------------------------------------------------------------------
int CvGame::getInitTech() const
{
	return m_iInitTech;
}


//	--------------------------------------------------------------------------------
int CvGame::getInitWonders() const
{
	return m_iInitWonders;
}


//	--------------------------------------------------------------------------------
void CvGame::initScoreCalculation()
{
	// initialize score calculation
	int iMaxFood = 0;
	for(int i = 0; i < GC.getMap().numPlots(); i++)
	{
		CvPlot* pPlot = GC.getMap().plotByIndexUnchecked(i);
		if(!pPlot->isWater() || pPlot->isAdjacentToLand(false))
		{
			iMaxFood += pPlot->calculateBestNatureYield(YIELD_FOOD, NO_PLAYER);
		}
	}
	m_iMaxPopulation = getPopulationScore(iMaxFood / std::max(1, /*2*/ GD_INT_GET(FOOD_CONSUMPTION_PER_POPULATION)));
	if(NO_ERA != getStartEra())
	{
		const CvEraInfo& kStartEra = getStartEraInfo();
		int iNumSettlers = kStartEra.getStartingUnitMultiplier();
		m_iInitPopulation = getPopulationScore(iNumSettlers * (kStartEra.getFreePopulation() + 1));

		m_iInitLand = getLandPlotsScore(iNumSettlers *  AVG_CITY_PLOTS);
	}
	else
	{
		m_iInitPopulation = 0;
		m_iInitLand = 0;
	}

	m_iInitTech = 0;
	for(int i = 0; i < GC.getNumTechInfos(); i++)
	{
		const TechTypes eTech = static_cast<TechTypes>(i);
		CvTechEntry* pkTechInfo = GC.getTechInfo(eTech);
		if(pkTechInfo)
		{
			if(pkTechInfo->GetEra() < getStartEra())
			{
				m_iInitTech += getTechScore(eTech);
			}
			else
			{
				// count all possible free techs as initial to lower the score from immediate retirement
				for(int iCiv = 0; iCiv < GC.getNumCivilizationInfos(); iCiv++)
				{
					const CivilizationTypes eCivilization = static_cast<CivilizationTypes>(iCiv);
					CvCivilizationInfo* pkCivilizationInfo = GC.getCivilizationInfo(eCivilization);
					if(pkCivilizationInfo)
					{
						if(pkCivilizationInfo->isPlayable())
						{
							if(pkCivilizationInfo->isCivilizationFreeTechs(i))
							{
								m_iInitTech += getTechScore(eTech);
								break;
							}
						}
					}
				}
			}
		}
	}
	m_iInitWonders = 0;
}


//	--------------------------------------------------------------------------------
int CvGame::getAIAutoPlay() const
{
	return m_iAIAutoPlay;
}

//	--------------------------------------------------------------------------------
void CvGame::setAIAutoPlay(int iNewValue, PlayerTypes eReturnAsPlayer)
{
	int iOldValue = getAIAutoPlay();

	if(iOldValue != iNewValue)
	{
		m_iAIAutoPlay = std::max(0, iNewValue);
		m_eAIAutoPlayReturnPlayer = eReturnAsPlayer;

		//activating
		if ((iOldValue == 0) && (getAIAutoPlay() > 0))
		{
			int iObserver = NO_PLAYER;
			PlayerTypes activePlayer = CvPreGame::activePlayer();

			// Is active player already in an observer slot?
			if (CvPreGame::slotStatus(activePlayer) == SS_OBSERVER)
				return;

			for (int iI = 0; iI < MAX_PLAYERS; iI++)
				PrintPlayerInfo(iI);

			// Observer has to be in a major slot, the "higher" players don't get notifications etc, leads to a crash (see CvPlayer::Init())
			for (int iI = 0; iI < MAX_MAJOR_CIVS; iI++)
			{
				// Found an observer slot
				if (CvPreGame::slotStatus((PlayerTypes)iI) == SS_OBSERVER && (CvPreGame::slotClaim((PlayerTypes)iI) == SLOTCLAIM_UNASSIGNED || CvPreGame::slotClaim((PlayerTypes)iI) == SLOTCLAIM_RESERVED))
				{
					// Set current active player to a computer player
					CvPreGame::setSlotStatus(CvPreGame::activePlayer(), SS_COMPUTER);
					CvPreGame::VerifyHandicap(CvPreGame::activePlayer());
					iObserver = iI;
					break;
				}
			}

			if (iObserver == NO_PLAYER)
			{
				//try to find a closed slot. start from the end, so we reduce the chance 
				//to hit a killed player which might be revived. that would lead to problems.
				for (int iI = MAX_MAJOR_CIVS - 1; iI > 0; iI--)
				{
					//open up a slot if required - it seems Really Advanced Setup does not leave observer slots but sets them all to closed
					if (CvPreGame::slotStatus((PlayerTypes)iI) == SS_CLOSED)
					{
						iObserver = iI;
						break;
					}
				}
			}

			//we didn't find a slot...
			if (iObserver == NO_PLAYER)
			{
				m_iAIAutoPlay = 0;
				return;
			}


			CvPreGame::setSlotStatus((PlayerTypes)iObserver, SS_OBSERVER);
			GET_PLAYER((PlayerTypes)iObserver).init((PlayerTypes)iObserver);
			//make sure the team flags are set correctly 
			GET_TEAM(GET_PLAYER((PlayerTypes)iObserver).getTeam()).updateTeamStatus();
			// Set current active player to a computer player
			PlayerTypes eNewAIPlayer = CvPreGame::activePlayer();
			CvPreGame::setSlotStatus(eNewAIPlayer, SS_COMPUTER);
			CvPreGame::VerifyHandicap(eNewAIPlayer);

			// trigger some AI decisions for the new computer player
			GET_PLAYER(eNewAIPlayer).GetDiplomacyAI()->SlotStateChange();
			int iLoop = 0;
			CvCity* pLoopCity = NULL;
			for (pLoopCity = GET_PLAYER(eNewAIPlayer).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(eNewAIPlayer).nextCity(&iLoop))
			{
				if (!pLoopCity->isProduction() || pLoopCity->isProductionProcess())
				{
					pLoopCity->AI_chooseProduction(false /*bInterruptWonders*/, false);
				}
			}
			GET_PLAYER(eNewAIPlayer).AI_chooseResearch();
			GET_PLAYER(eNewAIPlayer).GetPlayerPolicies()->DoPolicyAI();
			GET_PLAYER(eNewAIPlayer).SetEndTurnBlocking(NO_ENDTURN_BLOCKING_TYPE, -1);

			if (getObserverUIOverridePlayer() == NO_PLAYER)
			{
				SetAllPlotsVisible(GET_PLAYER((PlayerTypes)iObserver).getTeam());
			}
			else
			{
				// for all plots we have to copy visibility/revealed status from the team of eTurnActivePlayer to the team of getActivePlayer
				TeamTypes eObserverTeam = GET_PLAYER((PlayerTypes)iObserver).getTeam();
				TeamTypes ePlayerTeam = GET_PLAYER(getObserverUIOverridePlayer()).getTeam();
				const int iNumInvisibleInfos = NUM_INVISIBLE_TYPES;
				for (int plotID = 0; plotID < GC.getMap().numPlots(); plotID++)
				{
					CvPlot* pLoopPlot = GC.getMap().plotByIndexUnchecked(plotID);
					pLoopPlot->changeVisibilityCount(eObserverTeam, pLoopPlot->getVisibilityCount(ePlayerTeam) - pLoopPlot->getVisibilityCount(eObserverTeam), NO_INVISIBLE, true, false);
					pLoopPlot->flipVisibility(eObserverTeam);
					pLoopPlot->changeInvisibleVisibilityCountUnit(eObserverTeam, pLoopPlot->getInvisibleVisibilityCountUnit(ePlayerTeam) - pLoopPlot->getInvisibleVisibilityCountUnit(eObserverTeam));
					for (int iJ = 0; iJ < iNumInvisibleInfos; iJ++)
					{
						pLoopPlot->changeInvisibleVisibilityCount(eObserverTeam, ((InvisibleTypes)iJ), pLoopPlot->getInvisibleVisibilityCount(ePlayerTeam, ((InvisibleTypes)iJ)) - pLoopPlot->getInvisibleVisibilityCount(eObserverTeam, ((InvisibleTypes)iJ)));
					}
					pLoopPlot->setRevealed(eObserverTeam, pLoopPlot->isRevealed(ePlayerTeam));
				}
			}

			CvPreGame::setSlotClaim((PlayerTypes)iObserver, SLOTCLAIM_ASSIGNED);
			setActivePlayer((PlayerTypes)iObserver, false /*bForceHotSeat*/, true /*bAutoplaySwitch*/);
		}
		else if (iOldValue > 0 && getAIAutoPlay() == 0)
		{
			// If no player specified, returning as an observer
			if (eReturnAsPlayer == NO_PLAYER || !GET_PLAYER(eReturnAsPlayer).isAlive())
			{
				CvPreGame::setSlotClaim(getActivePlayer(), SLOTCLAIM_ASSIGNED);
				CvPreGame::setSlotStatus(getActivePlayer(), SS_OBSERVER);
			}

			// Want to return as a specific player
			else
			{
				// Reset observer slot
				CvPreGame::setSlotClaim(getActivePlayer(), SLOTCLAIM_UNASSIGNED);
				CvPreGame::setSlotStatus(getActivePlayer(), SS_OBSERVER);


				// Reset observer slot
				CvPreGame::setSlotClaim(getActivePlayer(), SLOTCLAIM_UNASSIGNED);
				CvPreGame::setSlotStatus(getActivePlayer(), SS_OBSERVER);

				// Move the active player to the desired slot
				CvPreGame::setSlotStatus(eReturnAsPlayer, SS_TAKEN);
				CvPreGame::VerifyHandicap(eReturnAsPlayer, true);
				setActivePlayer(eReturnAsPlayer, false /*bForceHotSeat*/, true /*bAutoplaySwitch*/);
				GET_PLAYER(eReturnAsPlayer).GetDiplomacyAI()->SlotStateChange();
			}
		}
	}
}


//	--------------------------------------------------------------------------------
void CvGame::changeAIAutoPlay(int iChange)
{
	setAIAutoPlay(getAIAutoPlay() + iChange, m_eAIAutoPlayReturnPlayer);
}


//	--------------------------------------------------------------------------------
unsigned int CvGame::getInitialTime() const
{
	return m_uiInitialTime;
}


//	--------------------------------------------------------------------------------
void CvGame::setInitialTime(unsigned int uiNewValue)
{
	m_uiInitialTime = uiNewValue;
}


//	--------------------------------------------------------------------------------
bool CvGame::isScoreDirty() const
{
	return m_bScoreDirty;
}


//	--------------------------------------------------------------------------------
void CvGame::setScoreDirty(bool bNewValue)
{
	m_bScoreDirty = bNewValue;
}

TechTypes CvGame::getOceanPassableTech() const
{
	return m_eTechAstronomy;
}

//	--------------------------------------------------------------------------------
bool CvGame::isCircumnavigated() const
{
	return m_bCircumnavigated;
}


//	--------------------------------------------------------------------------------
void CvGame::makeCircumnavigated()
{
	m_bCircumnavigated = true;
}

//	---------------------------------------------------------------------------
bool CvGame::circumnavigationAvailable() const
{
	if(isCircumnavigated())
	{
		return false;
	}

	CvMap& kMap = GC.getMap();

	if(!(kMap.isWrapX()) && !(kMap.isWrapY()))
	{
		return false;
	}

	if(kMap.getLandPlots() > ((kMap.numPlots() * 2) / 3))
	{
		return false;
	}

	return true;
}

//	---------------------------------------------------------------------------
/// Message from UI to gameplay about something that should happen with regards to diplomacy
void CvGame::DoFromUIDiploEvent(FromUIDiploEventTypes eEvent, PlayerTypes eAIPlayer, int iArg1, int iArg2)
{
#if defined(MOD_EVENTS_DIPLO_EVENTS)
	if (MOD_EVENTS_DIPLO_EVENTS) {
		GAMEEVENTINVOKE_HOOK(GAMEEVENT_UiDiploEvent, eEvent, eAIPlayer, iArg1, iArg2);
	} else {
#endif
	ICvEngineScriptSystem1* pkScriptSystem = gDLL->GetScriptSystem();
	if(pkScriptSystem)
	{
		CvLuaArgsHandle args;
		args->Push(eEvent);
		args->Push(eAIPlayer);
		args->Push(iArg1);
		args->Push(iArg2);

		bool bResult = false;
		LuaSupport::CallHook(pkScriptSystem, "UiDiploEvent", args.get(), bResult);
	}
#if defined(MOD_EVENTS_DIPLO_EVENTS)
	}
#endif

	gDLL->sendFromUIDiploEvent(eAIPlayer, eEvent, iArg1, iArg2);
}


//	--------------------------------------------------------------------------------
/// Set up diplo victory parameters
void CvGame::DoInitDiploVictory()
{
	DoUpdateDiploVictory();
}

//	--------------------------------------------------------------------------------
/// Update diplo victory parameters, such as how many votes are needed to win
void CvGame::DoUpdateDiploVictory()
{
	int iVotesForHost = 1;
	int iVotesPerCiv = 1;
	int iVotesPerCityState = 1;
	for (int i = 0; i < GC.getNumLeagueSpecialSessionInfos(); i++)
	{
		LeagueSpecialSessionTypes e = (LeagueSpecialSessionTypes)i;
		CvLeagueSpecialSessionEntry* pInfo = GC.getLeagueSpecialSessionInfo(e);
		ASSERT_DEBUG(pInfo != NULL);
		if (pInfo != NULL)
		{
			if (pInfo->IsUnitedNations())
			{
				iVotesForHost = pInfo->GetHostDelegates();
				iVotesPerCiv = pInfo->GetCivDelegates();
				iVotesPerCityState = pInfo->GetCityStateDelegates();
			}
		}
	}

	float fCivsToCount = 0.0f;
	float fCityStatesToCount = 0.0f;
	for (uint i = 0; i < MAX_CIV_PLAYERS; i++)
	{
		PlayerTypes e = (PlayerTypes)i;
		CvPlayer* pPlayer = &GET_PLAYER(e);
		if (pPlayer != NULL && pPlayer->isEverAlive())
		{
			// Minor civ
			if (pPlayer->isMinorCiv())
			{
				// Bought out does not count (they are no longer in the pool of teams, cannot be liberated, etc.)
				if (!pPlayer->GetMinorCivAI()->IsBoughtOut())
				{
					if (pPlayer->isAlive())
					{
						if(MOD_BALANCE_CORE_VICTORY_GAME_CHANGES)
						{
							fCityStatesToCount += 1.34f;
						}
						else
						{
							fCityStatesToCount += 1.0f;
						}
					}
					else
					{
						if(MOD_BALANCE_CORE_VICTORY_GAME_CHANGES)
						{
							fCityStatesToCount += 0.75f;
						}
						else
						{
							fCityStatesToCount += 0.5f;
						}
					}
				}
			}
			// Major civ
			else
			{
				if (pPlayer->isAlive())
				{
					if(MOD_BALANCE_CORE_VICTORY_GAME_CHANGES)
					{
						fCivsToCount += 1.5f;
					}
					else
					{
						fCivsToCount += 1.0f;
					}
				}
				else
				{
					if(MOD_BALANCE_CORE_VICTORY_GAME_CHANGES)
					{
						fCivsToCount += 0.75f;
					}
					else
					{
						fCivsToCount += 0.5f;
					}
				}
			}
		}
	}

	// Number of delegates needed to win increases the more civs and city-states there are in the game,
	// but these two scale differently since civs' delegates are harder to secure. These functions 
	// are based on a logarithmic regression.
	float fCivVotesPortion = (/*1.443f*/ GD_FLOAT_GET(DIPLO_VICTORY_CIV_DELEGATES_COEFFICIENT) * log(fCivsToCount)) + /*7.000f*/ GD_FLOAT_GET(DIPLO_VICTORY_CIV_DELEGATES_CONSTANT);
	if (fCivVotesPortion < 0.0f)
	{
		fCivVotesPortion = 0.0f;
	}
	float fCityStateVotesPortion = (/*16.023f*/ GD_FLOAT_GET(DIPLO_VICTORY_CS_DELEGATES_COEFFICIENT) * log(fCityStatesToCount)) + /*-13.758f*/ GD_FLOAT_GET(DIPLO_VICTORY_CS_DELEGATES_CONSTANT);
	if (fCityStateVotesPortion < 0.0f)
	{
		fCityStateVotesPortion = 0.0f;
	}

	int iVotesToWin = (int)floor(fCivVotesPortion + fCityStateVotesPortion);
	iVotesToWin = MAX(iVotesForHost + iVotesPerCiv + 1, iVotesToWin);
	iVotesToWin = MIN(iVotesForHost + (iVotesPerCiv * (int)fCivsToCount) + (iVotesPerCityState * (int)fCityStatesToCount), iVotesToWin);

	SetVotesNeededForDiploVictory(iVotesToWin);
	GC.GetEngineUserInterface()->setDirty(LeagueScreen_DIRTY_BIT, true);
}

//	--------------------------------------------------------------------------------
/// How many votes are needed to win?
int CvGame::GetVotesNeededForDiploVictory() const
{
	return m_iVotesNeededForDiploVictory;
}

//	--------------------------------------------------------------------------------
/// How many votes are needed to win?
void CvGame::SetVotesNeededForDiploVictory(int iValue)
{
	if(iValue != GetVotesNeededForDiploVictory())
	{
		m_iVotesNeededForDiploVictory = iValue;
	}
}

//	--------------------------------------------------------------------------------
/// Is the UN active? (Diplo Victory)
bool CvGame::IsUnitedNationsActive()
{
	if (!GC.getGame().isOption(GAMEOPTION_NO_LEAGUES))
	{
		if (GetGameLeagues()->GetNumActiveLeagues() > 0)
		{
			CvLeague* pLeague = GetGameLeagues()->GetActiveLeague();
			if (pLeague != NULL)
			{
				if (pLeague->IsUnitedNations())
				{
					return true;
				}
			}
		}
	}

	return false;
}

//	--------------------------------------------------------------------------------
/// United Nations diplo victory countdown
int CvGame::GetUnitedNationsCountdown() const
{
	return m_iUnitedNationsCountdown;
}

//	--------------------------------------------------------------------------------
/// United Nations diplo victory countdown
void CvGame::SetUnitedNationsCountdown(int iValue)
{
	if(iValue != GetUnitedNationsCountdown())
	{
		m_iUnitedNationsCountdown = iValue;
	}
}

//	--------------------------------------------------------------------------------
/// United Nations diplo victory countdown
void CvGame::ChangeUnitedNationsCountdown(int iChange)
{
	if(iChange != 0)
	{
		SetUnitedNationsCountdown(GetUnitedNationsCountdown() + iChange);
	}
}

//	--------------------------------------------------------------------------------
/// How many diplo votes tallied
int CvGame::GetNumVictoryVotesTallied() const
{
	return m_iNumVictoryVotesTallied;
}

//	--------------------------------------------------------------------------------
/// How many diplo votes tallied
/// Preliminary votes will be counted, but will not trigger victory
void CvGame::SetNumVictoryVotesTallied(int iValue, bool /*bPreliminaryVote*/)
{
	m_iNumVictoryVotesTallied = iValue;

	// Vote completed?
	if(iValue > 0)
	{
		if(iValue == GetNumVictoryVotesExpected())
		{
		}
	}
}

//	--------------------------------------------------------------------------------
/// How many diplo votes tallied
/// Preliminary votes will be counted, but will not trigger victory
void CvGame::ChangeNumVictoryVotesTallied(int iChange, bool bPreliminaryVote)
{
	SetNumVictoryVotesTallied(GetNumVictoryVotesTallied() + iChange, bPreliminaryVote);
}

//	--------------------------------------------------------------------------------
/// How many diplo votes Expected
int CvGame::GetNumVictoryVotesExpected() const
{
	return m_iNumVictoryVotesExpected;
}

//	--------------------------------------------------------------------------------
/// How many diplo votes Expected
void CvGame::SetNumVictoryVotesExpected(int iValue)
{
	m_iNumVictoryVotesExpected = iValue;
}

//	--------------------------------------------------------------------------------
TeamTypes CvGame::GetVoteCast(TeamTypes eVotingTeam) const
{
	ASSERT_DEBUG(eVotingTeam >= 0, "eMajor is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(eVotingTeam < MAX_CIV_TEAMS, "eMajor is expected to be within maximum bounds (invalid Index)");

	return (TeamTypes) m_aiVotesCast[eVotingTeam];
}

//	--------------------------------------------------------------------------------
/// Have eVotingTeam cast its UN vote for eVote
/// Preliminary votes will be counted, but will not trigger victory
void CvGame::SetVoteCast(TeamTypes eVotingTeam, TeamTypes eVote, bool bPreliminaryVote)
{
	ASSERT_DEBUG(eVotingTeam >= 0, "eMajor is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(eVotingTeam < MAX_CIV_TEAMS, "eMajor is expected to be within maximum bounds (invalid Index)");

	if(eVote != GetVoteCast(eVotingTeam))
	{
		m_aiVotesCast[eVotingTeam] = eVote;

		// Increment counts (unless we're resetting)
		if(eVote != NO_TEAM)
		{
			int iNumVotes = 1;

			ChangeNumVotesForTeam(eVote, iNumVotes);

			// Must call this last, as when enough votes are tallied the election ends immediately
			ChangeNumVictoryVotesTallied(1, bPreliminaryVote);
		}
	}
}

//	--------------------------------------------------------------------------------
/// Get the last vote this team made, for record keeping
TeamTypes CvGame::GetPreviousVoteCast(TeamTypes eVotingTeam) const
{
	ASSERT_DEBUG(eVotingTeam >= 0, "eVotingTeam is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(eVotingTeam < MAX_CIV_TEAMS, "eVotingTeam is expected to be within maximum bounds (invalid Index)");
	if (eVotingTeam < 0 || eVotingTeam >= MAX_CIV_TEAMS) return NO_TEAM;

	return (TeamTypes) m_aiPreviousVotesCast[eVotingTeam];
}

//	--------------------------------------------------------------------------------
/// Set the last vote this team made, for record keeping
void CvGame::SetPreviousVoteCast(TeamTypes eVotingTeam, TeamTypes eVotingTarget)
{
	ASSERT_DEBUG(eVotingTeam >= 0, "eVotingTeam is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(eVotingTeam < MAX_CIV_TEAMS, "eVotingTeam is expected to be within maximum bounds (invalid Index)");
	if (eVotingTeam < 0 || eVotingTeam >= MAX_CIV_TEAMS) return;

	if (eVotingTarget != GetPreviousVoteCast(eVotingTeam))
	{
		m_aiPreviousVotesCast[eVotingTeam] = eVotingTarget;
	}
}

//	--------------------------------------------------------------------------------
int CvGame::GetNumVotesForTeam(TeamTypes eTeam) const
{
	ASSERT_DEBUG(eTeam >= 0, "eMajor is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(eTeam < MAX_CIV_TEAMS, "eMajor is expected to be within maximum bounds (invalid Index)");

	return m_aiNumVotesForTeam[eTeam];
}

//	--------------------------------------------------------------------------------
void CvGame::SetNumVotesForTeam(TeamTypes eTeam, int iValue)
{
	ASSERT_DEBUG(eTeam >= 0, "eMajor is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(eTeam < MAX_CIV_TEAMS, "eMajor is expected to be within maximum bounds (invalid Index)");

	if(iValue != GetNumVotesForTeam(eTeam))
	{
		m_aiNumVotesForTeam[eTeam] = iValue;
	}
}

//	--------------------------------------------------------------------------------
void CvGame::ChangeNumVotesForTeam(TeamTypes eTeam, int iChange)
{
	SetNumVotesForTeam(eTeam, GetNumVotesForTeam(eTeam) + iChange);
}


//	--------------------------------------------------------------------------------
Localization::String CvGame::GetDiploResponse(const char* szLeader, const char* szResponse, const Localization::String& strOptionalKey1, const Localization::String& strOptionalKey2)
{
	//cvStopWatch stopWatch("GetDiploResponse");

	Localization::String response;

	if(m_pDiploResponseQuery == NULL)
	{
		//Directly reference Language_en_US is safe here since we're just looking for the tag
		//and not the actual text.
		const char* szSQL = "select Tag, Bias from Diplomacy_Responses, Language_en_US where (LeaderType = ? or LeaderType = 'GENERIC') and ResponseType = ? and Tag like Response";
		m_pDiploResponseQuery = new Database::Results();
		if(!GC.GetGameDatabase()->Execute(*m_pDiploResponseQuery, szSQL, strlen(szSQL)))
		{
			ASSERT_DEBUG(false, "Failed to generate diplo response query.");
		}
	}

	//This is not the fastest thing out there...
	//The original design was to select a random text key (of uniform probability) from the set generated by the above SQL statement
	//(with the random selection actually part of the SQL statement)
	//
	//Jon had later decided that he wanted to specify a bias for each diplomacy response such that certain ones would
	//appear more frequently than others.
	//To implement this quickly, I convert the discrete distribution into a uniform distribution and select from that.
	//This implementation generates a ton of strings to store the text keys though and would benefit greatly from a "stack_string"
	//implementation.  For now though, it works, and the code is called so infrequently that it shouldn't be noticeable.
	//NOTE: Profiled on my machine to take 0.006965 seconds on average to complete.
	std::vector<string> probabilities;
	probabilities.reserve(512);
	std::vector<int> biasList;
	int totbias = 0;

	// Get only leader-specific responses first
	if (MOD_NO_RANDOM_TEXT_CIVS)
	{
		Database::Results* tempDatabase = NULL;
		const char* callTemp = "select Tag, Bias from Diplomacy_Responses, Language_en_US where LeaderType = ? and ResponseType = ? and Tag like Response";
		tempDatabase = new Database::Results();
		if (!GC.GetGameDatabase()->Execute(*tempDatabase, callTemp, strlen(callTemp)))
		{
			ASSERT_DEBUG(false, "Failed to generate diplo response query.");
		}

		tempDatabase->Bind(1, szLeader);
		tempDatabase->Bind(2, szResponse);

		while (tempDatabase->Step())
		{
			const char* szTag = tempDatabase->GetText(0);
			int bias = tempDatabase->GetInt(1);
			totbias += bias;
			biasList.push_back(totbias);
			probabilities.push_back(szTag);
		}

		tempDatabase->Reset();

		if (!probabilities.empty())
		{
			int tempRand = randRangeExclusive(0, totbias, CvSeeder(probabilities.size()));
			for (uint choice = 0; choice < biasList.size(); choice++)
			{
				if (tempRand < biasList[choice])
				{
					response = Localization::Lookup(probabilities[choice].c_str());
					response << strOptionalKey1 << strOptionalKey2;
					return response;
				}
			}
		}
	}

	ASSERT_DEBUG(totbias == 0);
	ASSERT_DEBUG(biasList.empty());
	ASSERT_DEBUG(probabilities.empty());

	m_pDiploResponseQuery->Bind(1, szLeader);
	m_pDiploResponseQuery->Bind(2, szResponse);
	while (m_pDiploResponseQuery->Step())
	{
		const char* szTag = m_pDiploResponseQuery->GetText(0);
		int bias = m_pDiploResponseQuery->GetInt(1);
		totbias += bias;
		biasList.push_back(totbias); 
		probabilities.push_back(szTag);
	}

	m_pDiploResponseQuery->Reset();

	if (!probabilities.empty())
	{
		int tempRand = randRangeExclusive(0, totbias, CvSeeder(probabilities.size()));
		for (uint choice = 0; choice < biasList.size(); choice++)
		{
			if(tempRand < biasList[choice])
			{
				response = Localization::Lookup(probabilities[choice].c_str());
				response << strOptionalKey1 << strOptionalKey2;
				return response;
			}
		}
	}

	char szMessage[256];
	sprintf_s(szMessage, "Could not find diplomacy response. Leader - %s, Response - %s", szLeader, szResponse);
	ASSERT_DEBUG(false, szMessage);

	// Shouldn't be here
	return response;
}


//	--------------------------------------------------------------------------------
bool CvGame::isDebugMode() const
{
	return m_bDebugModeCache;
}

//	--------------------------------------------------------------------------------
void CvGame::setFOW(bool bMode)
{
	m_bFOW = bMode;
}

//	--------------------------------------------------------------------------------
bool CvGame::getFOW() const
{
	return m_bFOW;
}

//	--------------------------------------------------------------------------------
void CvGame::setDebugMode(bool bDebugMode)
{
	if(m_bDebugMode != bDebugMode)
		toggleDebugMode();
}

//	--------------------------------------------------------------------------------
void CvGame::toggleDebugMode()
{
	m_bDebugMode = (!(m_bDebugMode));
	updateDebugModeCache();

	GC.getMap().updateVisibility();

	GC.GetEngineUserInterface()->setDirty(GameData_DIRTY_BIT, true);
	GC.GetEngineUserInterface()->setDirty(Score_DIRTY_BIT, true);
	GC.GetEngineUserInterface()->setDirty(MinimapSection_DIRTY_BIT, true);
	GC.GetEngineUserInterface()->setDirty(UnitInfo_DIRTY_BIT, true);
	GC.GetEngineUserInterface()->setDirty(CityInfo_DIRTY_BIT, true);
}

//	--------------------------------------------------------------------------------
void CvGame::updateDebugModeCache()
{
	m_bDebugModeCache = m_bDebugMode;
}

//	--------------------------------------------------------------------------------
int CvGame::getPitbossTurnTime() const
{
	return CvPreGame::pitBossTurnTime();
}

//	--------------------------------------------------------------------------------
void CvGame::setPitbossTurnTime(int iHours)
{
	CvPreGame::setPitBossTurnTime(iHours);
}


//	--------------------------------------------------------------------------------
bool CvGame::isHotSeat() const
{
	return CvPreGame::isHotSeat();
}

//	--------------------------------------------------------------------------------
bool CvGame::isPbem() const
{
	return CvPreGame::isPlayByEmail();
}

//	--------------------------------------------------------------------------------
bool CvGame::isPitboss() const
{
	return CvPreGame::isPitBoss();
}

//	--------------------------------------------------------------------------------
bool CvGame::isSimultaneousTeamTurns() const
{//When players are taking sequential turns, do they take them simultaneous with every member of their team?
 //WARNING:  This function doesn't indicate if a player is taking sequential turns or not.
	//		 Use CvPlayer::isSimultaneousTurns() to determine that.
	if(!isNetworkMultiPlayer())
	{
		return false;
	}

	if(!isOption(GAMEOPTION_DYNAMIC_TURNS) && isOption(GAMEOPTION_SIMULTANEOUS_TURNS))
	{//truely simultaneous turn mode doesn't do this.
		return false;
	}

	return true;
}


bool CvGame::isDesynced() const
{
	if (!isNetworkMultiPlayer())
		return false;

	return m_bIsDesynced;
}
void CvGame::setDesynced(bool bNewValue)
{
	if (!isNetworkMultiPlayer())
		return;

	m_bIsDesynced = bNewValue;
}

//	--------------------------------------------------------------------------------
bool CvGame::isFinalInitialized() const
{
	return m_bFinalInitialized;
}


//	--------------------------------------------------------------------------------
void CvGame::setFinalInitialized(bool bNewValue)
{
	if(isFinalInitialized() != bNewValue)
	{
		m_bFinalInitialized = bNewValue;
	}
}


//	--------------------------------------------------------------------------------
bool CvGame::getPbemTurnSent() const
{
	return m_bPbemTurnSent;
}


//	--------------------------------------------------------------------------------
void CvGame::setPbemTurnSent(bool bNewValue)
{
	m_bPbemTurnSent = bNewValue;
}


//	--------------------------------------------------------------------------------
bool CvGame::getHotPbemBetweenTurns() const
{
	return m_bHotPbemBetweenTurns;
}


//	--------------------------------------------------------------------------------
void CvGame::setHotPbemBetweenTurns(bool bNewValue)
{
	m_bHotPbemBetweenTurns = bNewValue;
}


//	--------------------------------------------------------------------------------
bool CvGame::isPlayerOptionsSent() const
{
	return m_bPlayerOptionsSent;
}


//	--------------------------------------------------------------------------------
void CvGame::sendPlayerOptions(bool bForce)
{
	if(getActivePlayer() == NO_PLAYER)
	{
		return;
	}

	if(!isPlayerOptionsSent() || bForce)
	{
		m_bPlayerOptionsSent = true;

		gDLL->BeginSendBundle();
		for(int iI = 0; iI < GC.getNumPlayerOptionInfos(); iI++)
		{
			const PlayerOptionTypes eOption = static_cast<PlayerOptionTypes>(iI);

			CvPlayerOptionInfo* pkInfo = GC.getPlayerOptionInfo(eOption);
			if (pkInfo)
			{
				uint uiID = FStringHash( pkInfo->GetType() );
				gDLL->sendPlayerOption(static_cast<PlayerOptionTypes>(uiID), gDLL->getPlayerOption(static_cast<PlayerOptionTypes>(uiID)));
			}
		}
		gDLL->EndSendBundle();
	}
}


//	--------------------------------------------------------------------------------
PlayerTypes CvGame::getActivePlayer() const
{
	return CvPreGame::activePlayer();
}


//	--------------------------------------------------------------------------------
void CvGame::setActivePlayer(PlayerTypes eNewValue, bool bForceHotSeat, bool bAutoplaySwitch)
{
	PlayerTypes eOldActivePlayer = getActivePlayer();
	if(eOldActivePlayer != eNewValue)
	{
		int iActiveNetId = ((NO_PLAYER != eOldActivePlayer) ? GET_PLAYER(eOldActivePlayer).getNetID() : -1);
		CvPreGame::setActivePlayer(eNewValue);
		gDLL->GameplayActivePlayerChanged(eNewValue);

		if(GET_PLAYER(eNewValue).isHuman() && (isHotSeat() || isPbem() || bForceHotSeat))
		{
			if(isHotSeat())
			{
				GC.GetEngineUserInterface()->setDirty(TurnTimer_DIRTY_BIT, true);
				GC.GetEngineUserInterface()->setDirty(GameData_DIRTY_BIT, true);
				m_sentAutoMoves = false;
				resetTurnTimer();
				endTurnTimerReset();
			}

			gDLL->getPassword(eNewValue);
			setHotPbemBetweenTurns(false);

			if(NO_PLAYER != eOldActivePlayer)
			{
				int iInactiveNetId = GET_PLAYER(eNewValue).getNetID();
				GET_PLAYER(eNewValue).setNetID(iActiveNetId);
				GET_PLAYER(eOldActivePlayer).setNetID(iInactiveNetId);
			}

			if(countHumanPlayersAlive() == 1 && isPbem())
			{
				// Nobody else left alive
				CvPreGame::setGameType(GAME_HOTSEAT_MULTIPLAYER);
			}
		}

		if(isHotSeat() || bForceHotSeat || bAutoplaySwitch)
		{
			sendPlayerOptions(true);
		}

		if(GC.IsGraphicsInitialized())
		{
			// Publish the player change first
			// Messages will be sent out by the updating of the fog and they do not indicate
			// the player the update is for, so listeners will want to get the player change message first
			gDLL->PublishActivePlayer(eNewValue, eOldActivePlayer);

			CvMap& theMap = GC.getMap();
			theMap.updateFog();
			theMap.updateVisibility();

			ICvUserInterface2* theUI = GC.GetEngineUserInterface();
			theUI->setCanEndTurn(false);

			theUI->clearSelectedCities();
			theUI->ClearSelectionList();

			theUI->setDirty(PercentButtons_DIRTY_BIT, true);
			theUI->setDirty(ResearchButtons_DIRTY_BIT, true);
			theUI->setDirty(GameData_DIRTY_BIT, true);
			theUI->setDirty(MinimapSection_DIRTY_BIT, true);
			theUI->setDirty(CityInfo_DIRTY_BIT, true);
			theUI->setDirty(UnitInfo_DIRTY_BIT, true);

			//theUI->setDirty(NationalBorders_DIRTY_BIT, true);
			theUI->setDirty(BlockadedPlots_DIRTY_BIT, true);
		}
	}
}

PlayerTypes CvGame::getObserverUIOverridePlayer() const
{
	return m_eObserverUIOverridePlayer;
}

void CvGame::setObserverUIOverridePlayer(PlayerTypes ePlayer)
{
	m_eObserverUIOverridePlayer = ePlayer;
}

//	--------------------------------------------------------------------------------
const CvHandicapInfo& CvGame::getHandicapInfo() const
{
	static CvHandicapInfo emptyResult;
	CvHandicapInfo* pkHandicapInfo = GC.getHandicapInfo(getHandicapType());
	if(pkHandicapInfo == NULL)
	{
		const char* szError = "ERROR: Game does not contain valid handicap!!";
		GC.LogMessage(szError);
		ASSERT_DEBUG(false, szError);
		return emptyResult;
	}
	else
		return *pkHandicapInfo;
}

HandicapTypes CvGame::getHandicapType() const
{
	return m_eHandicap;
}

void CvGame::setHandicapType(HandicapTypes eHandicap)
{
	m_eHandicap = eHandicap;
}

//	-----------------------------------------------------------------------------------------------
PlayerTypes CvGame::getPausePlayer()
{
	return m_ePausePlayer;
}

//	-----------------------------------------------------------------------------------------------
bool CvGame::isPaused()
{
	return (getPausePlayer() != NO_PLAYER);
}

//	-----------------------------------------------------------------------------------------------
void CvGame::setPausePlayer(PlayerTypes eNewValue)
{
	if(!isNetworkMultiPlayer())
	{
		// If we're not in Network MP, if the game is paused the turn timer is too.
		if(isOption(GAMEOPTION_END_TURN_TIMER_ENABLED))
		{
			if(eNewValue != NO_PLAYER && m_ePausePlayer == NO_PLAYER)
			{
				m_fCurrentTurnTimerPauseDelta += m_curTurnTimer.Stop();
				m_timeSinceGameTurnStart.Stop();
			}
			else if(eNewValue == NO_PLAYER && m_ePausePlayer != NO_PLAYER)
			{
				m_timeSinceGameTurnStart.Start();
				m_curTurnTimer.Start();
			}
		}
	}

	m_ePausePlayer = eNewValue;
}

//	-----------------------------------------------------------------------------------------------
int CvGame::GetStartingMilitaryRating() const
{
	return getStartEra() > 0 ? (getStartEra() * /*1000*/ GD_INT_GET(MILITARY_RATING_PER_ADVANCED_START_ERA)) + /*1000*/ GD_INT_GET(MILITARY_RATING_STARTING_VALUE) : GD_INT_GET(MILITARY_RATING_STARTING_VALUE);
}

int CvGame::GetMinimumHumanMilitaryRating() const
{
	return GetStartingMilitaryRating() * /*80*/ range(GD_INT_GET(MILITARY_RATING_HUMAN_BUFFER_VALUE_PERCENT), 0, 100) / 100;
}

//	-----------------------------------------------------------------------------------------------
/// Modify military strength perception based on skill rating
int CvGame::ComputeRatingStrengthAdjustment(PlayerTypes ePlayer, PlayerTypes ePerceivingPlayer) const
{
	if (!GET_PLAYER(ePlayer).isMajorCiv())
		return 100;
	
	int iCivRating = GET_PLAYER(ePlayer).GetMilitaryRating();
	int iAverageRating = ComputeAverageMajorMilitaryRating(ePerceivingPlayer, /*eExcludedPlayer*/ ePlayer);

	// Don't adjust
	if (iAverageRating == -1)
		return 100;

	// Calculate the percentage difference from the average
	int iPercentageDifference = (iCivRating * 100 - iAverageRating * 100) / max(iAverageRating, 1);
	if (iPercentageDifference < 0)
		iPercentageDifference *= -1; // need the absolute value

	int iRtnValue = 100;

	// If above average, apply the % difference as a positive modifier to strength, cap above at +100%
	if (iCivRating > iAverageRating)
	{
		iRtnValue = min(100 + iPercentageDifference, max(GD_INT_GET(MILITARY_RATING_MAXIMUM_BONUS), 0) + 100);
	}
	// If below average, apply the % difference as a negative modifier to strength, cap below at -50%
	else if (iCivRating < iAverageRating)
	{
		iRtnValue = max(100 - iPercentageDifference, max(GD_INT_GET(MILITARY_RATING_MAXIMUM_PENALTY), -100) + 100);
	}

	return iRtnValue;
}

/// What is the average (living) major civ's military rating (only counting players that we know)?
int CvGame::ComputeAverageMajorMilitaryRating(PlayerTypes ePerceivingPlayer, PlayerTypes eExcludedPlayer /* = NO_PLAYER */) const
{
	int iTotalRating = 0;
	int iNumCivs = 0;
	
	for (int iPlayerLoop = 0; iPlayerLoop < MAX_MAJOR_CIVS; iPlayerLoop++)
	{
		PlayerTypes eLoopPlayer = (PlayerTypes) iPlayerLoop;
		
		if (eLoopPlayer == eExcludedPlayer)
			continue;

		if (ePerceivingPlayer != NO_PLAYER && GET_TEAM(GET_PLAYER(ePerceivingPlayer).getTeam()).isHasMet(GET_PLAYER(eLoopPlayer).getTeam()))
			continue;
		
		if (GET_PLAYER(eLoopPlayer).isAlive() && GET_PLAYER(eLoopPlayer).isMajorCiv() && GET_PLAYER(eLoopPlayer).getNumCities() > 0)
		{
			iTotalRating += GET_PLAYER(eLoopPlayer).GetMilitaryRating();
			iNumCivs++;
		}
	}

	// No other civs - don't adjust
	if (iNumCivs == 0)
		return -1;

	// If there's only one other civ that the perceiving player has met, return -1 so the AI will adjust its opponent's strength perception, but not its own
	// This avoids double counting when there isn't any other civ to factor into the global average
	if (iNumCivs == 1 && ePerceivingPlayer == eExcludedPlayer && ePerceivingPlayer != NO_PLAYER)
		return -1;
	
	return (iTotalRating / iNumCivs);
}

///	-----------------------------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////////////////
/// Diplomacy AI Options - All except the first two are configurable in DiploAIOptions.sql
/// Also consolidates some checks from various game options, for simplicity.

/// Disable Victory Competition
bool CvGame::IsVictoryCompetitionEnabled() const
{
	if (MOD_BALANCE_VP && isOption(GAMEOPTION_DISABLE_VICTORY_COMPETITION))
		return false;

	// Victory competition automatically ends when someone wins the game.
	return getWinner() == NO_TEAM;
}

/// Disable Endgame Aggression Boost
bool CvGame::IsEndgameAggressionEnabled() const
{
	if (MOD_BALANCE_VP && isOption(GAMEOPTION_DISABLE_ENDGAME_AGGRESSION))
		return false;

	// Automatically disabled if victory competition is disabled.
	return IsVictoryCompetitionEnabled();
}

/// Limit Victory Pursuit Randomization
bool CvGame::IsNoPrimaryVictoryPursuitRandomization() const
{
	return GD_INT_GET(DIPLOAI_LIMIT_VICTORY_PURSUIT_RANDOMIZATION) > 0;
}

bool CvGame::IsNoSecondaryVictoryPursuitRandomization() const
{
	return GD_INT_GET(DIPLOAI_LIMIT_VICTORY_PURSUIT_RANDOMIZATION) > 1;
}

/// Enable Nuclear Gandhi
/// NOTE: Only affects his extra personality changes, not his NUKE or USE_NUKE flavors.
bool CvGame::IsNuclearGandhiEnabled() const
{
	if (isNoNukes())
		return false;

	if (isOption(GAMEOPTION_RANDOM_PERSONALITIES) && GD_INT_GET(DIPLOAI_ENABLE_NUCLEAR_GANDHI) < 2)
		return false;

	return GD_INT_GET(DIPLOAI_ENABLE_NUCLEAR_GANDHI) > 0;
}

/// Disable War Bribes
/// NOTE: Does not affect coop war requests.
bool CvGame::IsAllWarBribesDisabled() const
{
	return GD_INT_GET(DIPLOAI_DISABLE_WAR_BRIBES) > 1;
}

bool CvGame::IsAIWarBribesDisabled() const
{
	return GD_INT_GET(DIPLOAI_DISABLE_WAR_BRIBES) > 0;
}

/// Disable City Trading
bool CvGame::IsAICityTradingHumanOnly() const
{
	return GD_INT_GET(DIPLOAI_DISABLE_CITY_TRADING) == 1;
}

bool CvGame::IsAICityTradingDisabled() const
{
	return GD_INT_GET(DIPLOAI_DISABLE_CITY_TRADING) > 1;
}

bool CvGame::IsAllCityTradingDisabled() const
{
	return GD_INT_GET(DIPLOAI_DISABLE_CITY_TRADING) > 2;
}

/// Disable Insult Messages
/// Only affects human players, and only applies to insulting messages sent by the AI on their turn.
bool CvGame::IsInsultMessagesDisabled() const
{
	return GD_INT_GET(DIPLOAI_DISABLE_INSULT_MESSAGES) > 0;
}

/// Disable Compliment Messages
/// Only affects human players, and only applies to friendly messages sent by the AI on their turn.
bool CvGame::IsComplimentMessagesDisabled() const
{
	return GD_INT_GET(DIPLOAI_DISABLE_COMPLIMENT_MESSAGES) > 0;
}

/// No Fake Modifiers
/// This controls whether the AI is able to fake having no disputes (no contested borders, etc.) in the opinion table.
/// Does not prevent displaying a false Approach or Approach hint (i.e. "They desire friendly relations with our empire.")
bool CvGame::IsNoFakeOpinionModifiers() const
{
	return GD_INT_GET(DIPLOAI_NO_FAKE_OPINION_MODIFIERS) > 0;
}

/// Show All Opinion Modifiers
/// This controls whether the AI should always display its full list of Opinion modifiers, even when it is FRIENDLY or otherwise might want to hide something.
bool CvGame::IsShowHiddenOpinionModifiers() const
{
	if (IsDiploDebugModeEnabled() || (MOD_BALANCE_VP && isOption(GAMEOPTION_TRANSPARENT_DIPLOMACY)))
		return true;

	return GD_INT_GET(DIPLOAI_SHOW_HIDDEN_OPINION_MODIFIERS) > 0;
}

/// Show Opinion Values
/// This controls whether the AI should display the number value of each Opinion modifier in its table of modifiers.
bool CvGame::IsShowAllOpinionValues() const
{
	if (IsDiploDebugModeEnabled() || (MOD_BALANCE_VP && isOption(GAMEOPTION_TRANSPARENT_DIPLOMACY)))
		return true;

	return GD_INT_GET(DIPLOAI_SHOW_ALL_OPINION_VALUES) > 0;
}

/// Show Base Human Opinion
/// CvDiplomacyAI::GetBaseOpinionScore()
bool CvGame::IsShowBaseHumanOpinion() const
{
	if (IsDiploDebugModeEnabled())
		return true;

	return GD_INT_GET(DIPLOAI_SHOW_BASE_HUMAN_OPINION) > 0;
}

/// Hide Opinion Table
/// Overrides Transparent Diplomacy, Show All Opinion Modifiers, and Show All Opinion Values.
bool CvGame::IsHideOpinionTable() const
{
	if (IsDiploDebugModeEnabled())
		return false;

	return GD_INT_GET(DIPLOAI_HIDE_OPINION_TABLE) > 0;
}

/// Enables human permanent items (e.g., lump Gold) to be traded for AI temporary items (e.g., resources), but not the other way around
bool CvGame::IsHumanPermanentForAITemporaryTradingAllowed() const
{
	return GD_INT_GET(DIPLOAI_TEMPORARY_FOR_PERMANENT_TRADING_SETTING) > 0;
}

/// Removes restrictions on permanent-for-temporary item trading
bool CvGame::IsPermanentForTemporaryTradingAllowed() const
{
	return GD_INT_GET(DIPLOAI_TEMPORARY_FOR_PERMANENT_TRADING_SETTING) > 1;
}

/// Disable Friendship Requests
/// Only affects human players, and only affects requests sent by the AI on their turn.
bool CvGame::IsFriendshipRequestsDisabled() const
{
	return GD_INT_GET(DIPLOAI_DISABLE_FRIENDSHIP_REQUESTS) > 0;
}

/// Disable Gift Offers
/// Only affects human players, and only affects gift offers sent by the AI on their turn.
bool CvGame::IsGiftOffersDisabled() const
{
	return GD_INT_GET(DIPLOAI_DISABLE_GIFT_OFFERS) > 0;
}

/// Disable Coop War Requests
/// Only affects human players, and only affects coop war requests sent by the AI on their turn.
bool CvGame::IsCoopWarRequestsWithHumansDisabled() const
{
	return GD_INT_GET(DIPLOAI_DISABLE_COOP_WAR_REQUESTS) > 0;
}

/// Only affects coop war requests sent by the AI on their turn.
bool CvGame::IsCoopWarRequestsDisabled() const
{
	return GD_INT_GET(DIPLOAI_DISABLE_COOP_WAR_REQUESTS) > 1;
}

/// Disable Help Requests
/// Only affects human players, and only affects help requests sent by the AI on their own turn.
bool CvGame::IsHelpRequestsDisabled() const
{
	return GD_INT_GET(DIPLOAI_DISABLE_HELP_REQUESTS) > 0;
}

/// Disable Trade Offers
/// Only affects human players, and only affects trade offers sent by the AI on their turn. Does not affect peace offers.
bool CvGame::IsTradeOffersDisabled(bool bIncludeRenewals) const
{
	if (bIncludeRenewals)
		return GD_INT_GET(DIPLOAI_DISABLE_TRADE_OFFERS) > 1;

	return GD_INT_GET(DIPLOAI_DISABLE_TRADE_OFFERS) > 0;
}

/// Disable Peace Offers
/// Only affects human players, and only affects peace offers sent by the AI on their turn.
bool CvGame::IsPeaceOffersDisabled() const
{
	return GD_INT_GET(DIPLOAI_DISABLE_PEACE_OFFERS) > 0;
}

/// Disable Demands
/// Only affects human players, and only affects demands sent by the AI on their turn.
bool CvGame::IsDemandsDisabled() const
{
	return GD_INT_GET(DIPLOAI_DISABLE_DEMANDS) > 0;
}

/// Disable Independence Requests
/// Only affects human players, and only affects independence requests sent by the AI on their turn.
bool CvGame::IsIndependenceRequestsDisabled() const
{
	return GD_INT_GET(DIPLOAI_DISABLE_INDEPENDENCE_REQUESTS) > 0;
}

/// Disable All Statements
/// Only affects human players. Affects statements sent by the AI on their turn as well as popup messages (e.g. from returning civilians or stealing territory).
bool CvGame::IsAllDiploStatementsDisabled() const
{
	return GD_INT_GET(DIPLOAI_DISABLE_ALL_STATEMENTS) > 0;
}

/// Passive Mode (towards all players)
bool CvGame::IsAIPassiveMode() const
{
	if (isOption(GAMEOPTION_ALWAYS_PEACE))
		return true;

	return GD_INT_GET(DIPLOAI_PASSIVE_MODE) > 1;
}

/// Passive Mode (towards humans)
bool CvGame::IsAIPassiveTowardsHumans() const
{
	if (IsAIPassiveMode())
		return true;

	return GD_INT_GET(DIPLOAI_PASSIVE_MODE) == 1;
}

/// Helper function to determine if a given player can attempt a Domination Victory (returns true if it is currently possible for them to win one)
/// Can also pass in eMakePeacePlayer to determine if making peace with a player would lock a player out of attempting a Domination Victory
bool CvGame::CanPlayerAttemptDominationVictory(PlayerTypes ePlayer, PlayerTypes eMakePeacePlayer, bool bCheckEliminationPossible) const
{
	bool bDominationVictoryEnabled = isVictoryValid((VictoryTypes) GC.getInfoTypeForString("VICTORY_DOMINATION", true));
	if (!bDominationVictoryEnabled && !bCheckEliminationPossible)
		return false;

	if ((!GET_PLAYER(ePlayer).isHuman() && IsAIPassiveMode()) || isOption(GAMEOPTION_ALWAYS_PEACE) || isOption(GAMEOPTION_NO_CHANGING_WAR_PEACE))
	{
		// Loop through all major civs
		for (int iPlayerLoop = 0; iPlayerLoop < MAX_MAJOR_CIVS; iPlayerLoop++)
		{
			CvPlayerAI& kPlayer = GET_PLAYER((PlayerTypes)iPlayerLoop);
			// Ignore this player and their teammates
			if (kPlayer.getTeam() == GET_PLAYER(ePlayer).getTeam())
				continue;

			// If testing elimination, ignore capitals and just look at war status
			if (bCheckEliminationPossible)
			{
				if (!kPlayer.isAlive())
					continue;

				// Not already at war?
				if (!kPlayer.IsAtWarWith(ePlayer))
					return false;

				// Already at war, but making peace with this player would stop us from eliminating everyone?
				if (eMakePeacePlayer != NO_PLAYER && eMakePeacePlayer == kPlayer.GetID())
					return false;
			}
			else
			{
				// Ignore players who never founded an original capital
				CvPlot* pCapitalPlot = GC.getMap().plot(kPlayer.GetOriginalCapitalX(), kPlayer.GetOriginalCapitalY());
				if (pCapitalPlot == NULL || !pCapitalPlot->isCity())
					continue;

				PlayerTypes eCapitalOwner = pCapitalPlot->getPlotCity()->GetOwnerForDominationVictory();

				// Not already at war?
				if (!GET_PLAYER(eCapitalOwner).IsAtWarWith(ePlayer))
				{
					return false;
				}

				// Already at war, but making peace with this player would block us from achieving Domination Victory?
				if (eMakePeacePlayer != NO_PLAYER)
				{
					if (GET_PLAYER(eCapitalOwner).getTeam() == GET_PLAYER(eMakePeacePlayer).getTeam())
					{
						return false;
					}
				}
			}
		}
	}
	else if (!GET_PLAYER(ePlayer).isHuman() && IsAIPassiveTowardsHumans())
	{
		// Loop through all major civs
		for (int iPlayerLoop = 0; iPlayerLoop < MAX_MAJOR_CIVS; iPlayerLoop++)
		{
			CvPlayerAI& kPlayer = GET_PLAYER((PlayerTypes)iPlayerLoop);
			// Ignore this player and their teammates
			if (kPlayer.getTeam() == GET_PLAYER(ePlayer).getTeam())
				continue;

			// If testing elimination, ignore capitals and just look at war status
			if (bCheckEliminationPossible)
			{
				if (!kPlayer.isAlive())
					continue;

				// It's only humans we can't make peace with ...
				if (!kPlayer.isHuman())
					continue;

				// Not already at war?
				if (!kPlayer.IsAtWarWith(ePlayer))
					return false;

				// Already at war, but making peace with this player would stop us from eliminating everyone?
				if (eMakePeacePlayer != NO_PLAYER && eMakePeacePlayer == kPlayer.GetID())
					return false;
			}
			else
			{
				// Ignore players who never founded an original capital
				CvPlot* pCapitalPlot = GC.getMap().plot(kPlayer.GetOriginalCapitalX(), kPlayer.GetOriginalCapitalY());
				if (pCapitalPlot == NULL || !pCapitalPlot->isCity())
					continue;

				PlayerTypes eCapitalOwner = pCapitalPlot->getPlotCity()->GetOwnerForDominationVictory();

				// It's only humans we can't make peace with ...
				if (!GET_PLAYER(eCapitalOwner).isHuman())
					continue;

				// Not already at war?
				if (!GET_PLAYER(eCapitalOwner).IsAtWarWith(ePlayer))
				{
					return false;
				}

				// Already at war, but making peace with this player would block us from achieving Domination Victory?
				if (eMakePeacePlayer != NO_PLAYER)
				{
					if (GET_PLAYER(eCapitalOwner).getTeam() == GET_PLAYER(eMakePeacePlayer).getTeam())
					{
						return false;
					}
				}
			}
		}
	}

	return true;
}

/// Would making peace with this player prevent us from achieving a Domination Victory?
bool CvGame::WouldMakingPeacePreventDominationVictory(PlayerTypes ePlayer, PlayerTypes eMakePeacePlayer) const
{
	if (!GET_PLAYER(ePlayer).isMajorCiv())
		return false;

	return CanPlayerAttemptDominationVictory(ePlayer, NO_PLAYER, areNoVictoriesValid()) && !CanPlayerAttemptDominationVictory(ePlayer, eMakePeacePlayer, areNoVictoriesValid());
}

/// Aggressive Mode (towards all players)
bool CvGame::IsAIAggressiveMode() const
{
	if (IsAIPassiveMode())
		return false;

	if (GD_INT_GET(DIPLOAI_AGGRESSIVE_MODE) == 2)
		return true;

	if (GD_INT_GET(DIPLOAI_DISABLE_DOMINATION_ONLY_AGGRESSION) > 0)
		return false;

	bool bDiplomaticVictoryEnabled = isVictoryValid((VictoryTypes) GC.getInfoTypeForString("VICTORY_DIPLOMATIC", true));
	bool bScienceVictoryEnabled = isVictoryValid((VictoryTypes) GC.getInfoTypeForString("VICTORY_SPACE_RACE", true));
	bool bCultureVictoryEnabled = isVictoryValid((VictoryTypes) GC.getInfoTypeForString("VICTORY_CULTURAL", true));
	return !bDiplomaticVictoryEnabled && !bScienceVictoryEnabled && !bCultureVictoryEnabled;
}

/// Aggressive Mode (towards humans)
bool CvGame::IsAIAggressiveTowardsHumans() const
{
	if (IsAIPassiveTowardsHumans())
		return false;

	if (IsAIAggressiveMode())
		return true;

	return GD_INT_GET(DIPLOAI_AGGRESSIVE_MODE) > 0;
}

/// Diplomacy AI Debug Mode
/// Enables the debug mode
bool CvGame::IsDiploDebugModeEnabled() const
{
	return GD_INT_GET(DIPLOAI_ENABLE_DEBUG_MODE) > 0;
}

/// Forces the AI to accept all Discuss requests from human players
bool CvGame::IsAIMustAcceptHumanDiscussRequests() const
{
	return GD_INT_GET(DIPLOAI_ENABLE_DEBUG_MODE) == 2;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///	-----------------------------------------------------------------------------------------------
UnitTypes CvGame::getBestLandUnit()
{
	return m_eBestLandUnit;
}

//	--------------------------------------------------------------------------------
int CvGame::getBestLandUnitCombat()
{
	const UnitTypes eBestLandUnit = getBestLandUnit();
	CvUnitEntry* pkUnitInfo = GC.getUnitInfo(eBestLandUnit);
	if(pkUnitInfo)
	{
		return std::max(1, pkUnitInfo->GetCombat());
	}

	return 1;
}


//	--------------------------------------------------------------------------------
void CvGame::setBestLandUnit(UnitTypes eNewValue)
{
	if(getBestLandUnit() != eNewValue)
	{
		m_eBestLandUnit = eNewValue;

		GC.GetEngineUserInterface()->setDirty(UnitInfo_DIRTY_BIT, true);
	}
}


//	--------------------------------------------------------------------------------
int CvGame::GetFaithCost(CvUnitEntry *pkUnit) const
{
	int iRtnValue = pkUnit->GetFaithCost();
	iRtnValue *= getGameSpeedInfo().getTrainPercent();
	iRtnValue /= 1000;   // Normalize costs like purchase system does
	iRtnValue *= 10;

	return iRtnValue;
}

//	--------------------------------------------------------------------------------
TeamTypes CvGame::getWinner() const
{
	return m_eWinner;
}

//	--------------------------------------------------------------------------------
VictoryTypes CvGame::getVictory() const
{
	return m_eVictory;
}
#if defined(MOD_BALANCE_CORE)
bool CvGame::isVictoryRandomizationDone() const
{
	return m_bVictoryRandomization;
}
void CvGame::setVictoryRandomizationDone(bool bValue)
{
	m_bVictoryRandomization = bValue;
}
#endif


//	--------------------------------------------------------------------------------
void CvGame::setWinner(TeamTypes eNewWinner, VictoryTypes eNewVictory)
{
	if((getWinner() != eNewWinner) || (getVictory() != eNewVictory))
	{
		m_eWinner = eNewWinner;
		m_eVictory = eNewVictory;
		SetWinningTurn(getElapsedGameTurns());

		// Reset UN countdown if necessary
		SetUnitedNationsCountdown(0);

		if(getVictory() != NO_VICTORY && !IsStaticTutorialActive())
		{
			CvVictoryInfo* pkVictoryInfo = GC.getVictoryInfo(getVictory());
			ASSERT_DEBUG(pkVictoryInfo);
			if(pkVictoryInfo == NULL)
				return;

			const char* szVictoryTextKey = pkVictoryInfo->GetTextKey();

			if (getWinner() != NO_TEAM)
			{
				const PlayerTypes winningTeamLeaderID = GET_TEAM(getWinner()).getLeaderID();
				CvPlayerAI& kWinningTeamLeader = GET_PLAYER(winningTeamLeaderID);
				const char* szWinningTeamLeaderNameKey = kWinningTeamLeader.getNameKey();

				Localization::String localizedText = Localization::Lookup("TXT_KEY_GAME_WON");
				localizedText << GET_TEAM(getWinner()).getName().GetCString() << szVictoryTextKey;
				addReplayMessage(REPLAY_MESSAGE_MAJOR_EVENT, winningTeamLeaderID, localizedText.toUTF8(), -1, -1);

				//Notify everyone of the victory
				localizedText = Localization::Lookup("TXT_KEY_NOTIFICATION_VICTORY_WINNER");
				localizedText << szWinningTeamLeaderNameKey << szVictoryTextKey;

				Localization::String localizedSummary = Localization::Lookup("TXT_KEY_NOTIFICATION_SUMMARY_VICTORY_WINNER");
				localizedSummary << szWinningTeamLeaderNameKey;

				for (int iNotifyLoop = 0; iNotifyLoop < MAX_MAJOR_CIVS; ++iNotifyLoop)
				{
					PlayerTypes eNotifyPlayer = (PlayerTypes) iNotifyLoop;
					CvPlayerAI& kCurNotifyPlayer = GET_PLAYER(eNotifyPlayer);
					CvNotifications* pNotifications = kCurNotifyPlayer.GetNotifications();
					if (pNotifications)
						pNotifications->Add(NOTIFICATION_VICTORY, localizedText.toUTF8(), localizedSummary.toUTF8(), -1, -1, -1);
				}

				if (pkVictoryInfo)
					LogGameResult(pkVictoryInfo->GetText(), kWinningTeamLeader.getCivilizationShortDescription());

				//--Start Achievements
				//--Don't allow most in multiplayer so friends can't achieve-whore it up together
				if (MOD_API_ACHIEVEMENTS && !GC.getGame().isGameMultiPlayer() && kWinningTeamLeader.isHuman() && kWinningTeamLeader.isLocalPlayer())
				{
					const bool bUsingDLC1Scenario = gDLL->IsModActivated(CIV5_DLC_01_SCENARIO_MODID);
					const bool bUsingDLC2Scenario = gDLL->IsModActivated(CIV5_DLC_02_SCENARIO_MODID) || gDLL->IsModActivated(CIV5_COMPLETE_SCENARIO1_MODID);
					const bool bUsingDLC3Scenario = gDLL->IsModActivated(CIV5_DLC_03_SCENARIO_MODID);
					const bool bUsingDLC4Scenario = gDLL->IsModActivated(CIV5_DLC_04_SCENARIO_MODID);
					const bool bUsingDLC5Scenario = gDLL->IsModActivated(CIV5_DLC_05_SCENARIO_MODID);
					const bool bUsingDLC6Scenario = gDLL->IsModActivated(CIV5_DLC_06_SCENARIO_MODID);

					const bool bUsingXP1Scenario1 = gDLL->IsModActivated(CIV5_XP1_SCENARIO1_MODID);
					const bool bUsingXP1Scenario2 = gDLL->IsModActivated(CIV5_XP1_SCENARIO2_MODID);
					const bool bUsingXP1Scenario3 = gDLL->IsModActivated(CIV5_XP1_SCENARIO3_MODID);

					const bool bUsingXP2Scenario1 = gDLL->IsModActivated(CIV5_XP2_SCENARIO1_MODID);
					const bool bUsingXP2Scenario2 = gDLL->IsModActivated(CIV5_XP2_SCENARIO2_MODID);

					const VictoryTypes eVictorySpaceRace = static_cast<VictoryTypes>(1);
					const VictoryTypes eVictoryDomination = static_cast<VictoryTypes>(2);
					const VictoryTypes eVictoryCultural = static_cast<VictoryTypes>(3);
					const VictoryTypes eVictoryDiplomatic = static_cast<VictoryTypes>(4);

					const PolicyBranchTypes ePolicyBranchFreedom = static_cast<PolicyBranchTypes>(9);
					const PolicyBranchTypes ePolicyBranchOrder = static_cast<PolicyBranchTypes>(10);
					const PolicyBranchTypes ePolicyBranchAutocracy = static_cast<PolicyBranchTypes>(11);

					const HandicapTypes eHandicapDeity = static_cast<HandicapTypes>(7);

					//Games Won Stat
					gDLL->IncrementSteamStat(ESTEAMSTAT_TOTAL_WINS);

					//Victory on Map Sizes
					WorldSizeTypes	winnerMapSize = GC.getMap().getWorldSize();
					switch(winnerMapSize)
					{
					case WORLDSIZE_DUEL:
						gDLL->UnlockAchievement(ACHIEVEMENT_MAPSIZE_DUEL);
						break;
					case WORLDSIZE_TINY:
						gDLL->UnlockAchievement(ACHIEVEMENT_MAPSIZE_TINY);
						break;
					case WORLDSIZE_SMALL:
						gDLL->UnlockAchievement(ACHIEVEMENT_MAPSIZE_SMALL);
						break;
					case WORLDSIZE_STANDARD:
						if(!bUsingXP2Scenario2 && !bUsingXP2Scenario1)
						{
							gDLL->UnlockAchievement(ACHIEVEMENT_MAPSIZE_STANDARD);
						}
						break;
					case WORLDSIZE_LARGE:
						gDLL->UnlockAchievement(ACHIEVEMENT_MAPSIZE_LARGE);
						break;
					case WORLDSIZE_HUGE:
						gDLL->UnlockAchievement(ACHIEVEMENT_MAPSIZE_HUGE);
						break;
					default:
						OutputDebugString("Playing on some other kind of world size.");
					}

					//Victory on Map Types
					const CvString& winnerMapName = CvPreGame::mapScriptName();
					//OutputDebugString(winnerMapName);
					//OutputDebugString("\n");

					if(winnerMapName == "Assets\\Maps\\Continents.lua")
						gDLL->UnlockAchievement(ACHIEVEMENT_MAPTYPE_CONTINENTS);
					else if(winnerMapName == "Assets\\Maps\\Pangaea.lua")
						gDLL->UnlockAchievement(ACHIEVEMENT_MAPTYPE_PANGAEA);
					else if(winnerMapName == "Assets\\Maps\\Archipelago.lua")
						gDLL->UnlockAchievement(ACHIEVEMENT_MAPTYPE_ARCHIPELAGO);
					else if(winnerMapName == "Assets\\Maps\\Earth_Duel.Civ5Map" || winnerMapName == "Assets\\Maps\\Earth_Huge.Civ5Map"
						|| winnerMapName == "Assets\\Maps\\Earth_Large.Civ5Map" || winnerMapName == "Assets\\Maps\\Earth_Small.Civ5Map"
						|| winnerMapName == "Assets\\Maps\\Earth_Standard.Civ5Map" || winnerMapName == "Assets\\Maps\\Earth_Tiny.Civ5Map")
						gDLL->UnlockAchievement(ACHIEVEMENT_MAPTYPE_EARTH);
					else
						OutputDebugString("\n Playing some other map. \n\n");


					//Victory on Difficulty Levels
					HandicapTypes winnerHandicapType = getHandicapType();
					if(!bUsingXP2Scenario1 && !bUsingXP2Scenario2) 
					{
						switch(winnerHandicapType)
						{
						case 0:
							gDLL->UnlockAchievement(ACHIEVEMENT_DIFLEVEL_SETTLER);
							break;
						case 1:
							gDLL->UnlockAchievement(ACHIEVEMENT_DIFLEVEL_CHIEFTAIN);
							break;
						case 2:
							gDLL->UnlockAchievement(ACHIEVEMENT_DIFLEVEL_WARLORD);
							break;
						case 3:
							gDLL->UnlockAchievement(ACHIEVEMENT_DIFLEVEL_PRINCE);
							break;
						case 4:
							gDLL->UnlockAchievement(ACHIEVEMENT_DIFLEVEL_KING);
							break;
						case 5:
							gDLL->UnlockAchievement(ACHIEVEMENT_DIFLEVEL_EMPEROR);
							break;
						case 6:
							gDLL->UnlockAchievement(ACHIEVEMENT_DIFLEVEL_IMMORTAL);
							break;
						case 7:
							gDLL->UnlockAchievement(ACHIEVEMENT_DIFLEVEL_DEITY);
							break;
						default:
							OutputDebugString("Playing on some non-existant dif level.");
						}
					}

					//Different Victory Win Types
					if(!bUsingXP2Scenario1 && !bUsingXP2Scenario2)
					{
						switch(eNewVictory)
						{
						case 0:
							OutputDebugString("No current Achievement for a time victory");
							break;
						case 1:
							gDLL->UnlockAchievement(ACHIEVEMENT_VICTORY_SPACE);
							break;
						case 2:
							{
								if(!bUsingXP1Scenario1)
								{
									gDLL->UnlockAchievement(ACHIEVEMENT_VICTORY_DOMINATION);
								}
							}
							break;
						case 3:
							gDLL->UnlockAchievement(ACHIEVEMENT_VICTORY_CULTURE);
							break;
						case 4:
							gDLL->UnlockAchievement(ACHIEVEMENT_VICTORY_DIPLO);
							break;
						default:
							OutputDebugString("Your l33t victory skills allowed you to win in some other way.");
						}
					}


					//Victory with Specific Leaders
					CvString pLeader =  kWinningTeamLeader.getLeaderTypeKey();

					if(!bUsingDLC6Scenario && pLeader == "LEADER_ALEXANDER")
						gDLL->UnlockAchievement(ACHIEVEMENT_WIN_ALEXANDER);
					else if(pLeader == "LEADER_WASHINGTON" && !bUsingXP2Scenario1)
						gDLL->UnlockAchievement(ACHIEVEMENT_WIN_WASHINGTON);
					else if(!bUsingXP1Scenario3 && !bUsingDLC4Scenario && !bUsingXP2Scenario1 && pLeader == "LEADER_ELIZABETH")
						gDLL->UnlockAchievement(ACHIEVEMENT_WIN_ELIZABETH);
					else if(!bUsingXP1Scenario2 && !bUsingXP1Scenario3 && !bUsingDLC4Scenario && pLeader == "LEADER_NAPOLEON")
						gDLL->UnlockAchievement(ACHIEVEMENT_WIN_NAPOLEON);
					else if(!bUsingXP1Scenario2 && !bUsingXP1Scenario3 && !bUsingDLC4Scenario && pLeader == "LEADER_BISMARCK")
						gDLL->UnlockAchievement(ACHIEVEMENT_WIN_BISMARCK);
					else if(!bUsingXP1Scenario3 && pLeader == "LEADER_CATHERINE")
					{
						gDLL->UnlockAchievement(ACHIEVEMENT_WIN_CATHERINE);

						if(bUsingXP1Scenario1 && winnerHandicapType >= 5)
						{
							gDLL->UnlockAchievement(ACHIEVEMENT_XP1_42);
						}
					}
					else if(!bUsingXP1Scenario2 && !bUsingXP2Scenario2 && pLeader == "LEADER_AUGUSTUS")
						gDLL->UnlockAchievement(ACHIEVEMENT_WIN_CAESAR);
					else if(!bUsingDLC6Scenario && pLeader == "LEADER_RAMESSES")
						gDLL->UnlockAchievement(ACHIEVEMENT_WIN_RAMESSES);
					else if(!bUsingXP1Scenario2 && !bUsingXP1Scenario1 && pLeader == "LEADER_ASKIA")
						gDLL->UnlockAchievement(ACHIEVEMENT_WIN_ASKIA);
					else if(!bUsingXP1Scenario1 && !bUsingDLC6Scenario && pLeader == "LEADER_HARUN_AL_RASHID")
						gDLL->UnlockAchievement(ACHIEVEMENT_WIN_HARUN);
					else if(!bUsingDLC6Scenario && pLeader == "LEADER_DARIUS")
						gDLL->UnlockAchievement(ACHIEVEMENT_WIN_DARIUS);
					else if(!bUsingDLC3Scenario && pLeader == "LEADER_GANDHI")
					{
						gDLL->UnlockAchievement(ACHIEVEMENT_WIN_GANDHI);
						if(eNewVictory == 3 && kWinningTeamLeader.getNumCities() <= 3) //Bollywood
							gDLL->UnlockAchievement(ACHIEVEMENT_SPECIAL_BOLLYWOOD);
					}
					else if(pLeader == "LEADER_RAMKHAMHAENG")
						gDLL->UnlockAchievement(ACHIEVEMENT_WIN_RAMKHAMHAENG);
					else if(!bUsingDLC5Scenario && pLeader == "LEADER_WU_ZETIAN")
						gDLL->UnlockAchievement(ACHIEVEMENT_WIN_WU);
					else if(!bUsingDLC5Scenario && pLeader == "LEADER_ODA_NOBUNAGA")
						gDLL->UnlockAchievement(ACHIEVEMENT_WIN_ODA);
					else if(!bUsingDLC3Scenario && pLeader == "LEADER_HIAWATHA")
						gDLL->UnlockAchievement(ACHIEVEMENT_WIN_HIAWATHA);
					else if(!bUsingDLC3Scenario && pLeader == "LEADER_MONTEZUMA")
						gDLL->UnlockAchievement(ACHIEVEMENT_WIN_MONTEZUMA);
					else if(!bUsingXP1Scenario1 && !bUsingDLC6Scenario && pLeader == "LEADER_SULEIMAN")
						gDLL->UnlockAchievement(ACHIEVEMENT_WIN_SULEIMAN);
					else if(pLeader == "LEADER_NEBUCHADNEZZAR")
						gDLL->UnlockAchievement(ACHIEVEMENT_WIN_NEBUCHADNEZZAR);
					else if(!bUsingDLC5Scenario && pLeader == "LEADER_GENGHIS_KHAN")
					{
						gDLL->UnlockAchievement(ACHIEVEMENT_WIN_GENGHIS);
					}
					else if(pLeader == "LEADER_ISABELLA")
						gDLL->UnlockAchievement(ACHIEVEMENT_WIN_ISABELLA);
					else if(pLeader == "LEADER_PACHACUTI")
						gDLL->UnlockAchievement(ACHIEVEMENT_WIN_PACHACUTI);
					else if(!bUsingDLC3Scenario && pLeader == "LEADER_KAMEHAMEHA")
						gDLL->UnlockAchievement(ACHIEVEMENT_WIN_KAMEHAMEHA);
					else if(pLeader == "LEADER_HARALD")
						gDLL->UnlockAchievement(ACHIEVEMENT_WIN_BLUETOOTH);
					else if(!bUsingDLC5Scenario && pLeader == "LEADER_SEJONG")
						gDLL->UnlockAchievement(ACHIEVEMENT_WIN_SEJONG);
					else if(pLeader == "LEADER_MARIA")
						gDLL->UnlockAchievement(ACHIEVEMENT_XP1_01);
					else if(!bUsingXP1Scenario2 && pLeader == "LEADER_THEODORA")
						gDLL->UnlockAchievement(ACHIEVEMENT_XP1_02);
					else if(pLeader == "LEADER_DIDO")
						gDLL->UnlockAchievement(ACHIEVEMENT_XP1_03);
					else if(pLeader == "LEADER_BOUDICCA")
					{
						gDLL->UnlockAchievement(ACHIEVEMENT_XP1_04);

						if(bUsingXP1Scenario1 && winnerHandicapType >= 5)
						{
							gDLL->UnlockAchievement(ACHIEVEMENT_XP1_41);
						}
					}
					else if(pLeader == "LEADER_SELASSIE")
						gDLL->UnlockAchievement(ACHIEVEMENT_XP1_05);
					else if(pLeader == "LEADER_ATTILA")
						gDLL->UnlockAchievement(ACHIEVEMENT_XP1_06);
					else if(pLeader == "LEADER_PACAL")
						gDLL->UnlockAchievement(ACHIEVEMENT_XP1_07);
					else if(pLeader == "LEADER_WILLIAM")
						gDLL->UnlockAchievement(ACHIEVEMENT_XP1_08);
					else if(!bUsingXP1Scenario3 && pLeader == "LEADER_GUSTAVUS_ADOLPHUS")
						gDLL->UnlockAchievement(ACHIEVEMENT_XP1_09);
					else if(pLeader == "LEADER_ASHURBANIPAL" )
						gDLL->UnlockAchievement(ACHIEVEMENT_XP2_01);
					else if(pLeader == "LEADER_PEDRO")
					{
						gDLL->UnlockAchievement(ACHIEVEMENT_XP2_02);

						//Diplomatic Victory
						if(eNewVictory == eVictoryDiplomatic)
						{
							gDLL->UnlockAchievement(ACHIEVEMENT_XP2_20);
						}
					}
					else if(pLeader == "LEADER_GAJAH_MADA" )
						gDLL->UnlockAchievement(ACHIEVEMENT_XP2_03);
					else if(pLeader == "LEADER_AHMAD_ALMANSUR")
						gDLL->UnlockAchievement(ACHIEVEMENT_XP2_04);
					else if(pLeader == "LEADER_CASIMIR")
					{
						gDLL->UnlockAchievement(ACHIEVEMENT_XP2_05);

						//Space Victory
						if(eNewVictory == eVictorySpaceRace)
						{
							gDLL->UnlockAchievement(ACHIEVEMENT_XP2_23);
						}
					}
					else if(pLeader == "LEADER_MARIA_I")
						gDLL->UnlockAchievement(ACHIEVEMENT_XP2_06);
					else if(pLeader == "LEADER_POCATELLO")
						gDLL->UnlockAchievement(ACHIEVEMENT_XP2_07);
					else if(pLeader == "LEADER_ENRICO_DANDOLO")
						gDLL->UnlockAchievement(ACHIEVEMENT_XP2_08);
					else if(pLeader == "LEADER_SHAKA")
						gDLL->UnlockAchievement(ACHIEVEMENT_XP2_09);
					else
						OutputDebugString("\nPlaying with a non-standard leader.\n");

					//One City
					if(kWinningTeamLeader.getNumCities() == 1)
					{
						gDLL->UnlockAchievement(ACHIEVEMENT_ONECITY);
					}

					//Uber Achievements for unlocking other achievements
					if(gDLL->IsAchievementUnlocked(ACHIEVEMENT_MAPSIZE_DUEL) &&  gDLL->IsAchievementUnlocked(ACHIEVEMENT_MAPSIZE_TINY) &&  gDLL->IsAchievementUnlocked(ACHIEVEMENT_MAPSIZE_SMALL) &&  gDLL->IsAchievementUnlocked(ACHIEVEMENT_MAPSIZE_STANDARD) &&  gDLL->IsAchievementUnlocked(ACHIEVEMENT_MAPSIZE_LARGE) &&  gDLL->IsAchievementUnlocked(ACHIEVEMENT_MAPSIZE_HUGE) &&  gDLL->IsAchievementUnlocked(ACHIEVEMENT_MAPTYPE_ARCHIPELAGO) &&  gDLL->IsAchievementUnlocked(ACHIEVEMENT_MAPTYPE_CONTINENTS) &&  gDLL->IsAchievementUnlocked(ACHIEVEMENT_MAPTYPE_EARTH) &&  gDLL->IsAchievementUnlocked(ACHIEVEMENT_MAPTYPE_PANGAEA))
					{
						gDLL->UnlockAchievement(ACHIEVEMENT_MAPS_ALL);
					}
					if(gDLL->IsAchievementUnlocked(ACHIEVEMENT_VICTORY_CULTURE) && gDLL->IsAchievementUnlocked(ACHIEVEMENT_VICTORY_SPACE) && gDLL->IsAchievementUnlocked(ACHIEVEMENT_VICTORY_DIPLO) && gDLL->IsAchievementUnlocked(ACHIEVEMENT_VICTORY_DOMINATION))
					{
						gDLL->UnlockAchievement(ACHIEVEMENT_VICTORY_ALL);
					}
					if(gDLL->IsAchievementUnlocked(ACHIEVEMENT_WIN_WASHINGTON) && gDLL->IsAchievementUnlocked(ACHIEVEMENT_WIN_ELIZABETH) && gDLL->IsAchievementUnlocked(ACHIEVEMENT_WIN_NAPOLEON)&& gDLL->IsAchievementUnlocked(ACHIEVEMENT_WIN_BISMARCK)&& gDLL->IsAchievementUnlocked(ACHIEVEMENT_WIN_CATHERINE)&& gDLL->IsAchievementUnlocked(ACHIEVEMENT_WIN_CAESAR)&& gDLL->IsAchievementUnlocked(ACHIEVEMENT_WIN_ALEXANDER)&& gDLL->IsAchievementUnlocked(ACHIEVEMENT_WIN_RAMESSES)&& gDLL->IsAchievementUnlocked(ACHIEVEMENT_WIN_ASKIA)&& gDLL->IsAchievementUnlocked(ACHIEVEMENT_WIN_HARUN)&& gDLL->IsAchievementUnlocked(ACHIEVEMENT_WIN_DARIUS)&& gDLL->IsAchievementUnlocked(ACHIEVEMENT_WIN_GANDHI)&& gDLL->IsAchievementUnlocked(ACHIEVEMENT_WIN_RAMKHAMHAENG)&& gDLL->IsAchievementUnlocked(ACHIEVEMENT_WIN_WU)&& gDLL->IsAchievementUnlocked(ACHIEVEMENT_WIN_ODA)&& gDLL->IsAchievementUnlocked(ACHIEVEMENT_WIN_HIAWATHA)&& gDLL->IsAchievementUnlocked(ACHIEVEMENT_WIN_MONTEZUMA)&& gDLL->IsAchievementUnlocked(ACHIEVEMENT_WIN_SULEIMAN))
					{
						gDLL->UnlockAchievement(ACHIEVEMENT_WIN_ALLBASELEADERS);
					}

					//Victory/Ideology Achievements
					const PolicyBranchTypes eBranch =kWinningTeamLeader.GetPlayerPolicies()->GetLateGamePolicyTree();
					if(eBranch != NO_POLICY_BRANCH_TYPE)
					{
						if(eNewVictory == eVictorySpaceRace)
						{
							if(eBranch == ePolicyBranchFreedom)
							{
								gDLL->UnlockAchievement(ACHIEVEMENT_XP2_10);
							}
							else if(eBranch == ePolicyBranchOrder)
							{
								gDLL->UnlockAchievement(ACHIEVEMENT_XP2_11);
							}
						}
						else if(eNewVictory == eVictoryDomination)
						{
							if(eBranch == ePolicyBranchOrder)
							{
								gDLL->UnlockAchievement(ACHIEVEMENT_XP2_17);
							}
							else if(eBranch == ePolicyBranchAutocracy)
							{
								gDLL->UnlockAchievement(ACHIEVEMENT_XP2_18);
							}
						}
						else if(eNewVictory == eVictoryCultural)
						{
							if(eBranch == ePolicyBranchFreedom)
							{
								gDLL->UnlockAchievement(ACHIEVEMENT_XP2_12);
							}
							else if(eBranch == ePolicyBranchOrder)
							{
								gDLL->UnlockAchievement(ACHIEVEMENT_XP2_13);
							}
							else if(eBranch == ePolicyBranchAutocracy)
							{
								gDLL->UnlockAchievement(ACHIEVEMENT_XP2_14);
							}
						}
						else if(eNewVictory == eVictoryDiplomatic)
						{
							if(eBranch == ePolicyBranchFreedom)
							{
								gDLL->UnlockAchievement(ACHIEVEMENT_XP2_15);
							}
							else if(eBranch == ePolicyBranchAutocracy)
							{
								gDLL->UnlockAchievement(ACHIEVEMENT_XP2_16);
							}
						}
					}

					// World Congress related Achievements
					if (GetGameLeagues()->GetNumActiveLeagues() > 0)
					{
						CvLeague* pLeague = GetGameLeagues()->GetActiveLeague();
						if (pLeague)
						{
							// Diplomatic Victory without ever being host
							if (eNewVictory == eVictoryDiplomatic)
							{
								if (!pLeague->HasMemberEverBeenHost(kWinningTeamLeader.GetID()))
								{
									gDLL->UnlockAchievement(ACHIEVEMENT_XP2_43);
								}
							}
						}
					}

					//Check for PSG
					CvAchievementUnlocker::Check_PSG();

					//DLC1 Scenario Win Achievements
					if(bUsingDLC1Scenario)
					{
						if(eNewVictory == 2)	//Only win by domination victory
						{
							CvString strHandicapType = this->getHandicapInfo().GetType();

							//All easier difficulty level achievements are unlocked when you beat it on a harder difficulty level.
							bool bBeatOnHarderDifficulty = false;

							if(strHandicapType == "HANDICAP_DEITY")
							{
								gDLL->UnlockAchievement(ACHIEVEMENT_WIN_SCENARIO_01_DEITY);
								bBeatOnHarderDifficulty = true;
							}

							if(bBeatOnHarderDifficulty || strHandicapType == "HANDICAP_IMMORTAL")
							{
								gDLL->UnlockAchievement(ACHIEVEMENT_WIN_SCENARIO_01_IMMORTAL);
								bBeatOnHarderDifficulty = true;
							}

							if(bBeatOnHarderDifficulty || strHandicapType == "HANDICAP_EMPEROR")
							{
								gDLL->UnlockAchievement(ACHIEVEMENT_WIN_SCENARIO_01_EMPEROR);
								bBeatOnHarderDifficulty = true;
							}

							if(bBeatOnHarderDifficulty || strHandicapType == "HANDICAP_KING")
							{
								gDLL->UnlockAchievement(ACHIEVEMENT_WIN_SCENARIO_01_KING);
								bBeatOnHarderDifficulty = true;
							}

							//Despite it's name, this achievement is for any difficulty.
							gDLL->UnlockAchievement(ACHIEVEMENT_WIN_SCENARIO_01_PRINCE_OR_BELOW);
						}
					}

					//DLC2 Scenario Win Achievements
					if(bUsingDLC2Scenario)
					{
						CvString strCivType = kWinningTeamLeader.getCivilizationInfo().GetType();
						if(strCivType == "CIVILIZATION_SPAIN")
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_02_WIN_SPAIN);
						else if(strCivType == "CIVILIZATION_FRANCE")
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_02_WIN_FRANCE);
						else if(strCivType == "CIVILIZATION_ENGLAND")
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_02_WIN_ENGLAND);
						else if(strCivType == "CIVILIZATION_INCA")
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_02_WIN_INCA);
						else if(strCivType == "CIVILIZATION_AZTEC")
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_02_WIN_AZTECS);
						else if(strCivType == "CIVILIZATION_IROQUOIS")
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_02_WIN_IROQUOIS);
					}

					//DLC3 Scenario Win Achievements
					if(bUsingDLC3Scenario)
					{
						CvString strCivType = kWinningTeamLeader.getCivilizationInfo().GetType();
						if(strCivType == "CIVILIZATION_POLYNESIA")
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_03_WIN_HIVA);
						else if(strCivType == "CIVILIZATION_IROQUOIS")
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_03_WIN_TAHITI);
						else if(strCivType == "CIVILIZATION_INDIA")
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_03_WIN_SAMOA);
						else if(strCivType == "CIVILIZATION_AZTEC")
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_03_WIN_TONGA);
					}

					//DLC4 Scenario Win Achievements
					if(bUsingDLC4Scenario)
					{
						CvString strCivType = kWinningTeamLeader.getCivilizationInfo().GetType();
						if(strCivType == "CIVILIZATION_DENMARK")
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_04_WIN_DENMARK);
						else if(strCivType == "CIVILIZATION_ENGLAND")
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_04_WIN_ENGLAND);
						else if(strCivType == "CIVILIZATION_GERMANY")
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_04_WIN_NORWAY);
						else if(strCivType == "CIVILIZATION_FRANCE")
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_04_NORMANDY);

						switch(winnerHandicapType)
						{
						case 5:	//	Win scenario on Emperor (any civ)  YOU! The Conqueror
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_04_WIN_EMPEROR);
							break;
						case 6:	//	Win scenario on Immortal (any civ)  Surviving Domesday
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_04_WIN_IMMORTAL);
							break;
						case 7:	//	Win scenario on Deity (any civ)  Surviving Ragnarok
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_04_WIN_DEITY);
							break;
						default:
							break;
						}
					}

					//DLC5 Scenario Win Achievements
					if(bUsingDLC5Scenario)
					{
						// Civilization
						CvString strCivType = kWinningTeamLeader.getCivilizationInfo().GetType();
						if(strCivType == "CIVILIZATION_JAPAN")
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_05_WIN_JAPAN);
						else if(strCivType == "CIVILIZATION_KOREA")
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_05_WIN_KOREA);
						else if(strCivType == "CIVILIZATION_CHINA")
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_05_WIN_CHINA);
						else if(strCivType == "CIVILIZATION_MONGOL")
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_05_WIN_MANCHU);

						// Difficulty
						switch(winnerHandicapType)
						{
						case 5: // Emperor
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_05_WIN_EMPEROR);
							break;
						case 6: // Immortal
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_05_WIN_IMMORTAL);
							break;
						case 7: // Deity
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_05_WIN_DEITY);
							break;
						default:
							break;
						}

						// Win in less than 100 turns
						if(getGameTurn() >= 0 && getGameTurn() < 100)
						{
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_05_WIN_100TURNS);
						}

					}

					//DLC6 Scenario Win Achievements
					if(bUsingDLC6Scenario)
					{
						// Civilization
						CvString strCivType = kWinningTeamLeader.getCivilizationInfo().GetType();
						if(strCivType == "CIVILIZATION_OTTOMAN")
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_06_WIN_HITTITES);
						else if(strCivType == "CIVILIZATION_GREECE")
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_06_WIN_GREECE);
						else if(strCivType == "CIVILIZATION_ARABIA")
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_06_WIN_SUMER);
						else if(strCivType == "CIVILIZATION_EGYPT")
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_06_WIN_EGYPT);
						else if(strCivType == "CIVILIZATION_PERSIA")
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_06_WIN_PERSIA);

						// Difficulty
						switch(winnerHandicapType)
						{
						case 3: // Prince
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_06_WIN_PRINCE);
							break;
						case 4: // King
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_06_WIN_KING);
							break;
						case 5:	// Emperor
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_06_WIN_EMPEROR);
							break;
						case 6:	// Immortal
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_06_WIN_IMMORTAL);
							break;
						case 7:	// Deity
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_06_WIN_DEITY);
							break;
						default:
							break;
						}

					}

					if(bUsingXP1Scenario1)
					{
						gDLL->UnlockAchievement(ACHIEVEMENT_XP1_21);

						if(pLeader == "LEADER_HARUN_AL_RASHID" && winnerHandicapType >= 5)
						{
							gDLL->UnlockAchievement(ACHIEVEMENT_XP1_43);
						}
					}

					if(bUsingXP1Scenario2)
					{
						gDLL->UnlockAchievement(ACHIEVEMENT_XP1_22);

						if(winnerHandicapType >= 7) //Deity or higher
						{
							if(pLeader == "LEADER_THEODORA")
							{
								typedef std::pair<int,int> Location;
								typedef std::tr1::array<Location, 27> OriginalCitiesArray;
								OriginalCitiesArray OriginalCities = {
									Location(65,6), // Alexandria
									Location(48,7), // Cyrene
									Location(71,9),  // Caesarea
									Location(79,13), // Palmyra
									Location(74,14), // Damascus
									Location(49,18), // Mistra
									Location(68,20), // Seleucia
									Location(73,20), // Antioch
									Location(63,21), // Attalia
									Location(76,21), // Edessa
									Location(51,22), // Athens
									Location(47,23), // Nicopolis
									Location(57,23), // Ephesus
									Location(67,24), // Iconium
									Location(75,26), // Melitene
									Location(63,27), // Dorylaeum
									Location(50,28), // Thessalonica
									Location(58,29), // Constantinople
									Location(45,30), // Dyrrachium
									Location(71,30), // Amasia
									Location(54,31), // Adrianople
									Location(64,32), // Amastris
									Location(78,32), // Theodosiopolis
									Location(49,34), // Naissus
									Location(69,34), // Sinope
									Location(75,34), // Trebizond
									Location(45,35), // Sirmium	

								};					

								CvMap& kMap = GC.getMap();
								const PlayerTypes eActivePlayer = GC.getGame().getActivePlayer();
								bool bHasAllCities = true;

								//Test if we still own each city.
								for(OriginalCitiesArray::iterator it = OriginalCities.begin(); it != OriginalCities.end(); ++it)
								{
									CvPlot* pkPlot = kMap.plot(it->first, it->second);
									if(pkPlot != NULL)
									{
										CvCity* pkCity = pkPlot->getPlotCity();
										if(pkCity != NULL)
										{
											if(pkCity->getOwner() == eActivePlayer)
											{
												continue;
											}
										}
									}

									bHasAllCities = false;
								}

								if(bHasAllCities)
								{
									gDLL->UnlockAchievement(ACHIEVEMENT_XP1_50);
								}
							}
							else if(pLeader == "LEADER_AUGUSTUS")
							{
								typedef std::pair<int,int> Location;
								typedef std::tr1::array<Location, 29> OriginalCitiesArray;
								OriginalCitiesArray OriginalCities = {
									Location(10,16), // Russadir
									Location(15,18), // Iol Caesarea
									Location(30,18), // Carthage
									Location(37,19), // Syracuse
									Location(4,23),  // Gades
									Location(13,25), // Carthago Nova
									Location(37,27), // Neapoli
									Location(41,27), // Brundisium
									Location(1,29),  // Olisipo
									Location(6,29),  // Emerita Augusta
									Location(15,30), // Tarraco
									Location(33,31), // Rome
									Location(14,34), // Caesaraugusta
									Location(20,36), // Narbo
									Location(24,36), // Massilia
									Location(40,36), // Salonae
									Location(28,37), // Genua
									Location(34,37), // Ravenna
									Location(3,39),  // Brigantium
									Location(30,40), // Milan
									Location(16,42), // Mediolanum
									Location(22,42), // Lundunum
									Location(32,45), // Casta Regina
									Location(41,45), // Carnutum
									Location(14,47), // Portus Namnetum
									Location(21,48), // Lutetia
									Location(26,49), // Trier
									Location(17,52), // Coriallum
									Location(22,53)  // Gesoriacum
								};

								CvMap& kMap = GC.getMap();
								const PlayerTypes eActivePlayer = GC.getGame().getActivePlayer();
								bool bHasAllCities = true;

								//Test if we still own each city.
								for(OriginalCitiesArray::iterator it = OriginalCities.begin(); it != OriginalCities.end(); ++it)
								{
									CvPlot* pkPlot = kMap.plot(it->first, it->second);
									if(pkPlot != NULL)
									{
										CvCity* pkCity = pkPlot->getPlotCity();
										if(pkCity != NULL)
										{
											if(pkCity->getOwner() == eActivePlayer)
											{
												continue;
											}
										}
									}

									bHasAllCities = false;
								}

								if(bHasAllCities)
								{
									gDLL->UnlockAchievement(ACHIEVEMENT_XP1_50);
								}
							}
						}
					}

					if(bUsingXP1Scenario3)
					{
						gDLL->UnlockAchievement(ACHIEVEMENT_XP1_23);
					}


					if(bUsingXP2Scenario1)
					{
						CvString strCivType = kWinningTeamLeader.getCivilizationInfo().GetType();
						if(strCivType == "CIVILIZATION_AMERICA")
						{
							gDLL->UnlockAchievement(ACHIEVEMENT_XP2_56);
						}
						else
						{
							gDLL->UnlockAchievement(ACHIEVEMENT_XP2_57);
						}

						// Difficulty
						if (winnerHandicapType == eHandicapDeity)
						{
							gDLL->UnlockAchievement(ACHIEVEMENT_XP2_58);
						}
					}

					if(bUsingXP2Scenario2)
					{
						gDLL->UnlockAchievement(ACHIEVEMENT_XP2_49);

						// Difficulty
						if (winnerHandicapType == eHandicapDeity)
						{
							CvString strCivType = kWinningTeamLeader.getCivilizationInfo().GetType();
							if(strCivType == "CIVILIZATION_AMERICA")
							{
								gDLL->UnlockAchievement(ACHIEVEMENT_XP2_50);
							}
							else if(strCivType == "CIVILIZATION_OTTOMAN")
							{
								gDLL->UnlockAchievement(ACHIEVEMENT_XP2_51);
							}
						}
					}
				}
				//Win any multiplayer game
				if (MOD_API_ACHIEVEMENTS && GC.getGame().isGameMultiPlayer() && kWinningTeamLeader.isHuman() && (GET_PLAYER(GC.getGame().getActivePlayer()).GetID() == kWinningTeamLeader.GetID()))
				{
					gDLL->UnlockAchievement(ACHIEVEMENT_WIN_MULTIPLAYER);
				}

				if (getAIAutoPlay() > 0)
				{
					setAIAutoPlay(0, kWinningTeamLeader.GetID());
					setGameState(GAMESTATE_EXTENDED);
				}
				else if (gDLL->GetAutorun())
				{
					setGameState(GAMESTATE_EXTENDED);
				}
				else
				{
					setGameState(GAMESTATE_OVER);
				}
			}
		}

		GC.GetEngineUserInterface()->setDirty(Center_DIRTY_BIT, true);
		GC.GetEngineUserInterface()->setDirty(Soundtrack_DIRTY_BIT, true);
	}
}

void CvGame::LogTurnScores() const
{
	if (GC.getLogging() && GC.getAILogging())
	{
		static bool bFirstRun = true;
		bool bBuildHeader = false;
		CvString strHeader;
		if (bFirstRun)
		{
			bFirstRun = false;
			bBuildHeader = true;
		}

		CvString header = "Turn";
		CvString rowOutput;
		CvString strTemp;

		CvString strLogName = "Score_Log.csv";
		FILogFile* pLog = LOGFILEMGR.GetLog(strLogName, FILogFile::kDontTimeStamp);

		rowOutput.Format("%03d", getElapsedGameTurns());

		for (int iPlayerLoop = 0; iPlayerLoop < MAX_MAJOR_CIVS; iPlayerLoop++)
		{
			PlayerTypes eLoopPlayer = (PlayerTypes)iPlayerLoop;
			CvPlayer& eLoopCvPlayer = GET_PLAYER(eLoopPlayer);
			if (eLoopPlayer != NO_PLAYER && eLoopCvPlayer.isEverAlive() && !eLoopCvPlayer.isMinorCiv() && !eLoopCvPlayer.isBarbarian())
			{
				strTemp = eLoopCvPlayer.getCivilizationShortDescription();
				header += ", " + strTemp;

				strTemp.Format("%5d", GET_TEAM(eLoopCvPlayer.getTeam()).GetScore());
				rowOutput += ", " + strTemp;
			}
		}

		if (bBuildHeader)
		{
			pLog->Msg(header);
		}
		pLog->Msg(rowOutput);
	}
}

void CvGame::LogGameResult(const char* victoryTypeText, const char* victoryCivText) const
{
	if (GC.getLogging() && GC.getAILogging())
	{
		CvString header = "Turn, VictoryType, VictoryCiv";
		CvString rowOutput;
		CvString strTemp;

		CvString strLogName = "GameResult_Log.csv";
		FILogFile* pLog = LOGFILEMGR.GetLog(strLogName, FILogFile::kDontTimeStamp);

		CvString victoryType = victoryTypeText;
		CvString victoryCiv = victoryCivText;

		rowOutput.Format("%03d", m_iWinningTurn);
		rowOutput += ", " + victoryType;
		rowOutput += ", " + victoryCiv;

		for (int iPlayerLoop = 0; iPlayerLoop < MAX_MAJOR_CIVS; iPlayerLoop++)
		{
			PlayerTypes eLoopPlayer = (PlayerTypes)iPlayerLoop;
			CvPlayer& eLoopCvPlayer = GET_PLAYER(eLoopPlayer);
			if (eLoopPlayer != NO_PLAYER && eLoopCvPlayer.isEverAlive() && !eLoopCvPlayer.isMinorCiv() && !eLoopCvPlayer.isBarbarian())
			{
				strTemp = eLoopCvPlayer.getCivilizationShortDescription();
				header += ", " + strTemp;

				strTemp.Format("%05d", GET_TEAM(eLoopCvPlayer.getTeam()).GetScore());
				rowOutput += ", " + strTemp;
			}
		}

		pLog->Msg(header);
		pLog->Msg(rowOutput);
	}
}

//	--------------------------------------------------------------------------------
// Check last slot to see if there is still a Victory slot open
bool CvGame::isVictoryAvailable(VictoryTypes eVictory) const
{
	if(getTeamVictoryRank(eVictory, /*5*/ GD_INT_GET(NUM_VICTORY_POINT_AWARDS) - 1) == NO_TEAM)
	{
		return true;
	}

	return false;
}

//	--------------------------------------------------------------------------------
/// What's the next victory slot available to be won?
int CvGame::GetNextAvailableVictoryCompetitionRank(VictoryTypes eVictory) const
{
	for(int iRankLoop = 0; iRankLoop < /*5*/ GD_INT_GET(NUM_VICTORY_POINT_AWARDS); iRankLoop++)
	{
		if(getTeamVictoryRank(eVictory, iRankLoop) == NO_TEAM)
		{
			return iRankLoop;
		}
	}

	return -1;
}


//	--------------------------------------------------------------------------------
void CvGame::DoPlaceTeamInVictoryCompetition(VictoryTypes eNewVictory, TeamTypes eTeam)
{
	CvVictoryInfo* pkVictoryInfo = GC.getVictoryInfo(eNewVictory);
	if(pkVictoryInfo == NULL)
		return;

	// Is there a slot open for this new Team?
	if(isVictoryAvailable(eNewVictory))
	{
		CvTeam& kTeam = GET_TEAM(eTeam);

		// Loop through and find first available slot
		for(int iSlotLoop = 0; iSlotLoop < /*5*/ GD_INT_GET(NUM_VICTORY_POINT_AWARDS); iSlotLoop++)
		{
			if(getTeamVictoryRank(eNewVictory, iSlotLoop) == NO_TEAM)
			{
				setTeamVictoryRank(eNewVictory, iSlotLoop, eTeam);
				int iNumPoints = pkVictoryInfo->GetVictoryPointAward(iSlotLoop);
				kTeam.changeVictoryPoints(iNumPoints);
				kTeam.setVictoryAchieved(eNewVictory, true);

				Localization::String youWonInfo = Localization::Lookup("TXT_KEY_NOTIFICATION_VICTORY_RACE_WON_YOU");
				Localization::String youWonSummary = Localization::Lookup("TXT_KEY_NOTIFICATION_SUMMARY_VICTORY_RACE_WON_YOU");
				Localization::String someoneWonInfo = Localization::Lookup("TXT_KEY_NOTIFICATION_VICTORY_RACE_WON_SOMEBODY");
				Localization::String someoneWonSummary = Localization::Lookup("TXT_KEY_NOTIFICATION_SUMMARY_VICTORY_RACE_WON_SOMEBODY");
				Localization::String unmetWonInfo = Localization::Lookup("TXT_KEY_NOTIFICATION_VICTORY_RACE_WON_UNMET");
				Localization::String unmetWonSummary = Localization::Lookup("TXT_KEY_NOTIFICATION_SUMMARY_VICTORY_RACE_WON_UNMET");

				for(int iNotifyLoop = 0; iNotifyLoop < MAX_MAJOR_CIVS; ++iNotifyLoop){
					PlayerTypes eNotifyPlayer = (PlayerTypes) iNotifyLoop;
					CvPlayerAI& kCurNotifyPlayer = GET_PLAYER(eNotifyPlayer);
					CvNotifications* pNotification = kCurNotifyPlayer.GetNotifications();
					if(pNotification)
					{
						Localization::String localizedText;
						Localization::String localizedSummary;

						const char* szVictoryTextKey = pkVictoryInfo->GetTextKey();
						// Active Team
						if(eTeam == kCurNotifyPlayer.getTeam())
						{
							localizedText = youWonInfo;
							localizedText << iSlotLoop+1 << szVictoryTextKey << iNumPoints;
							localizedSummary = youWonSummary;
							localizedSummary << iSlotLoop+1 << szVictoryTextKey;
						}
						// Met Team
						else if(GET_TEAM(kCurNotifyPlayer.getTeam()).isHasMet(eTeam))
						{
							const char* szTeamLeaderNameKey = GET_PLAYER(kTeam.getLeaderID()).getNameKey();

							localizedText = someoneWonInfo;
							localizedText << szTeamLeaderNameKey << iSlotLoop+1 << szVictoryTextKey << iNumPoints;
							localizedSummary = someoneWonSummary;
							localizedSummary << szTeamLeaderNameKey << iSlotLoop+1 << szVictoryTextKey;
						}
						// Unmet Team
						else
						{
							localizedText = unmetWonInfo;
							localizedText << iSlotLoop+1 << szVictoryTextKey << iNumPoints;
							localizedSummary = unmetWonSummary;
							localizedSummary << iSlotLoop+1 << szVictoryTextKey;
						}
						pNotification->Add(NOTIFICATION_VICTORY, localizedText.toUTF8(), localizedSummary.toUTF8(), -1, -1, -1);
					}
				}

				break;
			}
		}
	}
}

//	--------------------------------------------------------------------------------
TeamTypes CvGame::getTeamVictoryRank(VictoryTypes eNewVictory, int iRank) const
{
	ASSERT_DEBUG(iRank >= 0);
	ASSERT_DEBUG(iRank < /*5*/ GD_INT_GET(NUM_VICTORY_POINT_AWARDS));

	return m_ppaaiTeamVictoryRank[eNewVictory][iRank];
}


//	--------------------------------------------------------------------------------
void CvGame::setTeamVictoryRank(VictoryTypes eNewVictory, int iRank, TeamTypes eTeam)
{
	ASSERT_DEBUG(iRank >= 0);
	ASSERT_DEBUG(iRank < /*5*/ GD_INT_GET(NUM_VICTORY_POINT_AWARDS));

	m_ppaaiTeamVictoryRank[eNewVictory][iRank] = eTeam;
}

//	--------------------------------------------------------------------------------
/// Returns the Average Military Might of all Players in the game
int CvGame::GetWorldMilitaryStrengthAverage(PlayerTypes ePlayer, bool bIncludeMe, bool bIncludeOnlyKnown)
{
	int iWorldMilitaryStrength = 0;
	int iNumAlivePlayers = 0;

	// Look at our military strength relative to everyone else in the world
	for(int iMajorLoop = 0; iMajorLoop < MAX_MAJOR_CIVS; iMajorLoop++)
	{
		if(GET_PLAYER((PlayerTypes) iMajorLoop).isAlive())
		{
			if(bIncludeMe || iMajorLoop != ePlayer)
			{
				if(!bIncludeOnlyKnown || (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isHasMet(GET_PLAYER((PlayerTypes) iMajorLoop).getTeam())))
				{
					iNumAlivePlayers++;
					iWorldMilitaryStrength += GET_PLAYER((PlayerTypes) iMajorLoop).GetMilitaryMight();
				}
			}
		}
	}

	if(iNumAlivePlayers > 0)
	{
		iWorldMilitaryStrength /= iNumAlivePlayers;
	}

	return iWorldMilitaryStrength;
}


//	--------------------------------------------------------------------------------
/// Recursive function to see how much Research is left to get to a Tech
int CvGame::GetResearchLeftToTech(TeamTypes eTeam, TechTypes eTech)
{
	CvTechEntry* pkTechInfo = GC.getTechInfo(eTech);

	//THIS SHOULD NEVER HAPPEN!
	if(pkTechInfo == NULL)
		return 0;

	// Base Case - first Prereq AND Tech does not exist
	if(pkTechInfo->GetPrereqAndTechs(0) == NO_TECH)
	{
		return pkTechInfo->GetResearchCost();
	}

	// Another base case! - Team already has tech
	if(eTeam != NO_TEAM && GET_TEAM(eTeam).GetTeamTechs()->HasTech(eTech))
	{
		return 0;
	}

	int iPrereqTechCost = 0;
	TechTypes ePreReq;

	for(int i = 0; i < /*6*/ GD_INT_GET(NUM_AND_TECH_PREREQS); i++)
	{
		ePreReq = (TechTypes) pkTechInfo->GetPrereqAndTechs(i);

		if(ePreReq != NO_TECH)
		{
			iPrereqTechCost += GetResearchLeftToTech(eTeam, ePreReq);
		}
	}

	return pkTechInfo->GetResearchCost() + iPrereqTechCost;
}

//	--------------------------------------------------------------------------------
GameStateTypes CvGame::getGameState()
{
	return m_eGameState;
}


//	--------------------------------------------------------------------------------
void CvGame::setGameState(GameStateTypes eNewValue)
{
	if(getGameState() != eNewValue)
	{
		m_eGameState = eNewValue;

		if (eNewValue == GAMESTATE_OVER || eNewValue == GAMESTATE_EXTENDED)
		{
			if (MOD_API_ACHIEVEMENTS && !isGameMultiPlayer())
			{
				if (GetGameLeagues()->GetNumActiveLeagues() > 0)
				{
					CvLeague* pLeague = GetGameLeagues()->GetActiveLeague();
					if (pLeague)
					{
						if (pLeague->HasMemberAlwaysBeenHost(getActivePlayer()))
						{
							gDLL->UnlockAchievement(ACHIEVEMENT_XP2_41);
						}
					}
				}
			}
		}

		if(eNewValue == GAMESTATE_OVER)
		{
			if (MOD_API_ACHIEVEMENTS && !isGameMultiPlayer())
			{
				bool bLocalPlayerLost = true;

				const TeamTypes eWinningTeam = getWinner();
				if(eWinningTeam != NO_TEAM)
				{
					CvPlayerAI& kLocalPlayer = GET_PLAYER(getActivePlayer());
					if(eWinningTeam == kLocalPlayer.getTeam())
						bLocalPlayerLost = false;
				}
				if(bLocalPlayerLost)
				{
					//Handle any local losing achievements here.

					//Are we playing DLC_01 - Mongol Scenario??
					bool bUsingDLC1MongolScenario = gDLL->IsModActivated(CIV5_DLC_01_SCENARIO_MODID);
					if(bUsingDLC1MongolScenario && getGameTurn() == 100)
					{
						gDLL->UnlockAchievement(ACHIEVEMENT_LOSE_SCENARIO_01);
					}

					//DLC_05 - Achievement for losing Invasion of Korea Scenario as Korea
					bool bUsingDLC5Scenario = gDLL->IsModActivated(CIV5_DLC_05_SCENARIO_MODID);
					if(bUsingDLC5Scenario) // && getGameTurn() == 100)
					{
						CvPlayerAI& kLocalPlayer = GET_PLAYER(getActivePlayer());
						CvString strCivType = kLocalPlayer.getCivilizationInfo().GetType();
						if(strCivType == "CIVILIZATION_KOREA")
						{
							gDLL->UnlockAchievement(ACHIEVEMENT_SCENARIO_05_LOSE_KOREA);
						}
					}
				}
			}

			//Write out time spent playing.
			int iHours = getMinutesPlayed() / 60;
			int iMinutes = getMinutesPlayed() % 60;

			for(int iI = 0; iI < MAX_CIV_PLAYERS; iI++)
			{
				CvPlayer& player = GET_PLAYER((PlayerTypes)iI);
				if(player.isHuman())
				{
					addReplayMessage(REPLAY_MESSAGE_MAJOR_EVENT, (PlayerTypes)iI, GetLocalizedText("TXT_KEY_MISC_TIME_SPENT", iHours, iMinutes));

				}
			}

			saveReplay();
			showEndGameSequence();
		}

		GC.GetEngineUserInterface()->setDirty(Cursor_DIRTY_BIT, true);
		GC.GetEngineUserInterface()->setDirty(GameData_DIRTY_BIT, true);
	}
}

//	--------------------------------------------------------------------------------
const CvGameSpeedInfo& CvGame::getGameSpeedInfo() const
{
	static CvGameSpeedInfo emptyResult;
	CvGameSpeedInfo* pkGameSpeedInfo = GC.getGameSpeedInfo(getGameSpeedType());
	if(pkGameSpeedInfo == NULL)
	{
		const char* szError = "ERROR: Game does not contain valid game speed!!";
		GC.LogMessage(szError);
		ASSERT_DEBUG(false, szError);

		return emptyResult;
	}
	else
		return *pkGameSpeedInfo;
}

//	--------------------------------------------------------------------------------
GameSpeedTypes CvGame::getGameSpeedType() const
{
	return CvPreGame::gameSpeed();
}

//	--------------------------------------------------------------------------------
const CvEraInfo& CvGame::getStartEraInfo() const
{
	static CvEraInfo emptyResult;

	CvEraInfo* pkStartEraInfo = GC.getEraInfo(getStartEra());
	if(pkStartEraInfo == NULL)
	{
		const char* szError = "ERROR: Game does not contain valid start era!!";
		GC.LogMessage(szError);
		ASSERT_DEBUG(false, szError);

		return emptyResult;
	}
	else
		return *pkStartEraInfo;
}

//	--------------------------------------------------------------------------------
EraTypes CvGame::getStartEra() const
{
	return CvPreGame::era();
}

//	--------------------------------------------------------------------------------
CalendarTypes CvGame::getCalendar() const
{
	return CvPreGame::calendar();
}


//	--------------------------------------------------------------------------------
int CvGame::getEndTurnMessagesReceived(int iIndex)
{
	ASSERT_DEBUG(iIndex >= 0, "iIndex is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(iIndex < MAX_PLAYERS, "iIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiEndTurnMessagesReceived[iIndex];
}


//	--------------------------------------------------------------------------------
void CvGame::incrementEndTurnMessagesReceived(int iIndex)
{
	ASSERT_DEBUG(iIndex >= 0, "iIndex is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(iIndex < MAX_PLAYERS, "iIndex is expected to be within maximum bounds (invalid Index)");
	m_aiEndTurnMessagesReceived[iIndex]++;
}


//	--------------------------------------------------------------------------------
PlayerTypes CvGame::getRankPlayer(int iRank)
{
	ASSERT_DEBUG(iRank >= 0, "iRank is expected to be non-negative (invalid Rank)");
	ASSERT_DEBUG(iRank < MAX_PLAYERS, "iRank is expected to be within maximum bounds (invalid Rank)");
	return (PlayerTypes)m_aiRankPlayer[iRank];
}


//	--------------------------------------------------------------------------------
void CvGame::setRankPlayer(int iRank, PlayerTypes ePlayer)
{
	ASSERT_DEBUG(iRank >= 0, "iRank is expected to be non-negative (invalid Rank)");
	ASSERT_DEBUG(iRank < MAX_PLAYERS, "iRank is expected to be within maximum bounds (invalid Rank)");

	if(getRankPlayer(iRank) != ePlayer)
	{
		m_aiRankPlayer[iRank] = ePlayer;
	}
}


//	--------------------------------------------------------------------------------
int CvGame::getPlayerRank(PlayerTypes ePlayer)
{
	ASSERT_DEBUG(ePlayer >= 0, "eIndex is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(ePlayer < MAX_PLAYERS, "ePlayer is expected to be within maximum bounds (invalid Index)");
	return m_aiPlayerRank[ePlayer];
}


//	--------------------------------------------------------------------------------
void CvGame::setPlayerRank(PlayerTypes ePlayer, int iRank)
{
	ASSERT_DEBUG(ePlayer >= 0, "eIndex is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(ePlayer < MAX_PLAYERS, "ePlayer is expected to be within maximum bounds (invalid Index)");
	m_aiPlayerRank[ePlayer] = iRank;
	ASSERT_DEBUG(getPlayerRank(ePlayer) >= 0);
}


//	--------------------------------------------------------------------------------
int CvGame::getPlayerScore(PlayerTypes ePlayer)
{
	ASSERT_DEBUG(ePlayer >= 0, "eIndex is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(ePlayer < MAX_PLAYERS, "ePlayer is expected to be within maximum bounds (invalid Index)");
	return m_aiPlayerScore[ePlayer];
}


//	--------------------------------------------------------------------------------
void CvGame::setPlayerScore(PlayerTypes ePlayer, int iScore)
{
	ASSERT_DEBUG(ePlayer >= 0, "eIndex is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(ePlayer < MAX_PLAYERS, "ePlayer is expected to be within maximum bounds (invalid Index)");

	if(getPlayerScore(ePlayer) != iScore)
	{
		m_aiPlayerScore[ePlayer] = iScore;
		ASSERT_DEBUG(getPlayerScore(ePlayer) >= 0);

		GC.GetEngineUserInterface()->setDirty(Score_DIRTY_BIT, true);
	}
}


//	--------------------------------------------------------------------------------
TeamTypes CvGame::getRankTeam(int iRank)
{
	ASSERT_DEBUG(iRank >= 0, "iRank is expected to be non-negative (invalid Rank)");
	ASSERT_DEBUG(iRank < MAX_TEAMS, "iRank is expected to be within maximum bounds (invalid Index)");
	return (TeamTypes)m_aiRankTeam[iRank];
}


//	--------------------------------------------------------------------------------
void CvGame::setRankTeam(int iRank, TeamTypes eTeam)
{
	ASSERT_DEBUG(iRank >= 0, "iRank is expected to be non-negative (invalid Rank)");
	ASSERT_DEBUG(iRank < MAX_TEAMS, "iRank is expected to be within maximum bounds (invalid Index)");

	if(getRankTeam(iRank) != eTeam)
	{
		m_aiRankTeam[iRank] = eTeam;
	}
}


//	--------------------------------------------------------------------------------
int CvGame::getTeamRank(TeamTypes eTeam)
{
	ASSERT_DEBUG(eTeam >= 0, "eTeam is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(eTeam < MAX_TEAMS, "eTeam is expected to be within maximum bounds (invalid Index)");
	return m_aiTeamRank[eTeam];
}


//	--------------------------------------------------------------------------------
void CvGame::setTeamRank(TeamTypes eTeam, int iRank)
{
	ASSERT_DEBUG(eTeam >= 0, "eTeam is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(eTeam < MAX_TEAMS, "eTeam is expected to be within maximum bounds (invalid Index)");
	m_aiTeamRank[eTeam] = iRank;
	ASSERT_DEBUG(getTeamRank(eTeam) >= 0);
}


//	--------------------------------------------------------------------------------
int CvGame::getTeamScore(TeamTypes eTeam) const
{
	ASSERT_DEBUG(eTeam >= 0, "eTeam is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(eTeam < MAX_TEAMS, "eTeam is expected to be within maximum bounds (invalid Index)");
	return m_aiTeamScore[eTeam];
}


//	--------------------------------------------------------------------------------
void CvGame::setTeamScore(TeamTypes eTeam, int iScore)
{
	ASSERT_DEBUG(eTeam >= 0, "eTeam is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(eTeam < MAX_TEAMS, "eTeam is expected to be within maximum bounds (invalid Index)");
	m_aiTeamScore[eTeam] = iScore;
	ASSERT_DEBUG(getTeamScore(eTeam) >= 0);
}


//	--------------------------------------------------------------------------------
bool CvGame::isOption(GameOptionTypes eIndex) const
{
	int i = 0;
	CvPreGame::GetGameOption(eIndex, i);

	return (i == 1);
}

//	--------------------------------------------------------------------------------
bool CvGame::isOption(const char* pszOption) const
{
	int i = 0;
	CvPreGame::GetGameOption(pszOption, i);

	return (i == 1);
}

//	--------------------------------------------------------------------------------
void CvGame::setOption(GameOptionTypes eIndex, bool bEnabled)
{
	CvPreGame::SetGameOption(eIndex, (int)bEnabled);
}

//	--------------------------------------------------------------------------------
void CvGame::setOption(const char* pszOption, bool bEnabled)
{
	CvPreGame::SetGameOption(pszOption, (int)bEnabled);
}


//	--------------------------------------------------------------------------------
bool CvGame::isMPOption(MultiplayerOptionTypes eIndex) const
{
	return CvPreGame::multiplayerOptionEnabled(eIndex);
}


//	--------------------------------------------------------------------------------
void CvGame::setMPOption(MultiplayerOptionTypes eIndex, bool bEnabled)
{
	CvPreGame::setMultiplayerOption(eIndex, bEnabled);
}

//	--------------------------------------------------------------------------------
int CvGame::getUnitCreatedCount(UnitTypes eIndex)
{
	ASSERT_DEBUG(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(eIndex < GC.getNumUnitInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiUnitCreatedCount[eIndex];
}


//	--------------------------------------------------------------------------------
void CvGame::incrementUnitCreatedCount(UnitTypes eIndex)
{
	ASSERT_DEBUG(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(eIndex < GC.getNumUnitInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_paiUnitCreatedCount[eIndex]++;
}


//	--------------------------------------------------------------------------------
int CvGame::getUnitClassCreatedCount(UnitClassTypes eIndex)
{
	ASSERT_DEBUG(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(eIndex < GC.getNumUnitClassInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiUnitClassCreatedCount[eIndex];
}


//	--------------------------------------------------------------------------------
bool CvGame::isUnitClassMaxedOut(UnitClassTypes eIndex, int iExtra)
{
	ASSERT_DEBUG(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(eIndex < GC.getNumUnitClassInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	CvUnitClassInfo* pkUnitClassInfo = GC.getUnitClassInfo(eIndex);
	if(pkUnitClassInfo == NULL)
	{
		return false;
	}

	if(!isWorldUnitClass(eIndex))
	{
		return false;
	}

	ASSERT_DEBUG(getUnitClassCreatedCount(eIndex) <= pkUnitClassInfo->getMaxGlobalInstances(), "Index is expected to be within maximum bounds (invalid Index)");

	return ((getUnitClassCreatedCount(eIndex) + iExtra) >= pkUnitClassInfo->getMaxGlobalInstances());
}


//	--------------------------------------------------------------------------------
void CvGame::incrementUnitClassCreatedCount(UnitClassTypes eIndex)
{
	ASSERT_DEBUG(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(eIndex < GC.getNumUnitClassInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_paiUnitClassCreatedCount[eIndex]++;
}


//	--------------------------------------------------------------------------------
int CvGame::getBuildingClassCreatedCount(BuildingClassTypes eIndex)
{
	ASSERT_DEBUG(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(eIndex < GC.getNumBuildingClassInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiBuildingClassCreatedCount[eIndex];
}


//	--------------------------------------------------------------------------------
bool CvGame::isBuildingClassMaxedOut(BuildingClassTypes eIndex, int iExtra)
{
	ASSERT_DEBUG(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(eIndex < GC.getNumBuildingClassInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	CvBuildingClassInfo* pkBuildingClassInfo = GC.getBuildingClassInfo(eIndex);
	if(pkBuildingClassInfo == NULL)
	{
		ASSERT_DEBUG(false, "BuildingClassInfo does not exist for type.  NOT GOOD");
		return false;
	}


	if(!isWorldWonderClass(*pkBuildingClassInfo))
	{
		return false;
	}

	ASSERT_DEBUG(getBuildingClassCreatedCount(eIndex) <= pkBuildingClassInfo->getMaxGlobalInstances(), "Index is expected to be within maximum bounds (invalid Index)");

	return ((getBuildingClassCreatedCount(eIndex) + iExtra) >= pkBuildingClassInfo->getMaxGlobalInstances());
}


//	--------------------------------------------------------------------------------
void CvGame::incrementBuildingClassCreatedCount(BuildingClassTypes eIndex)
{
	ASSERT_DEBUG(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(eIndex < GC.getNumBuildingClassInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_paiBuildingClassCreatedCount[eIndex]++;
}

//	--------------------------------------------------------------------------------
void CvGame::decrementBuildingClassCreatedCount(BuildingClassTypes eIndex)
{
	ASSERT_DEBUG(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(eIndex < GC.getNumBuildingClassInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_paiBuildingClassCreatedCount[eIndex]--;
}


//	--------------------------------------------------------------------------------
int CvGame::getProjectCreatedCount(ProjectTypes eIndex)
{
	ASSERT_DEBUG(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(eIndex < GC.getNumProjectInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiProjectCreatedCount[eIndex];
}


//	--------------------------------------------------------------------------------
bool CvGame::isProjectMaxedOut(ProjectTypes eIndex, int iExtra)
{
	ASSERT_DEBUG(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(eIndex < GC.getNumProjectInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if(!isWorldProject(eIndex))
	{
		return false;
	}

	ASSERT_DEBUG(getProjectCreatedCount(eIndex) <= GC.getProjectInfo(eIndex)->GetMaxGlobalInstances(), "Index is expected to be within maximum bounds (invalid Index)");

	return ((getProjectCreatedCount(eIndex) + iExtra) >= GC.getProjectInfo(eIndex)->GetMaxGlobalInstances());
}


//	--------------------------------------------------------------------------------
void CvGame::incrementProjectCreatedCount(ProjectTypes eIndex, int iExtra)
{
	ASSERT_DEBUG(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(eIndex < GC.getNumProjectInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_paiProjectCreatedCount[eIndex] += iExtra;
}

//	--------------------------------------------------------------------------------
bool CvGame::isVictoryValid(VictoryTypes eIndex) const
{
	ASSERT_DEBUG(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(eIndex < GC.getNumVictoryInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return CvPreGame::isVictory(eIndex);
}

//	--------------------------------------------------------------------------------
void CvGame::setVictoryValid(VictoryTypes eIndex, bool bValid)
{
	ASSERT_DEBUG(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(eIndex < GC.getNumVictoryInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	CvPreGame::setVictory(eIndex, bValid);
}

//	--------------------------------------------------------------------------------
bool CvGame::areNoVictoriesValid() const
{
	bool bRtnValue = true;

	for(int iI = 0; iI < GC.getNumVictoryInfos(); iI++)
	{
		VictoryTypes eVictory = static_cast<VictoryTypes>(iI);
		CvVictoryInfo* pkVictoryInfo = GC.getVictoryInfo(eVictory);
		if(pkVictoryInfo)
		{
			if(isVictoryValid(eVictory))
			{
				bRtnValue = false;
				break;
			}
		}
	}

	return bRtnValue;
}

//	--------------------------------------------------------------------------------
bool CvGame::isSpecialUnitValid(SpecialUnitTypes eIndex)
{
	ASSERT_DEBUG(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(eIndex < GC.getNumSpecialUnitInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_pabSpecialUnitValid[eIndex];
}


//	--------------------------------------------------------------------------------
void CvGame::makeSpecialUnitValid(SpecialUnitTypes eIndex)
{
	ASSERT_DEBUG(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(eIndex < GC.getNumSpecialUnitInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_pabSpecialUnitValid[eIndex] = true;
}


//	--------------------------------------------------------------------------------
bool CvGame::isNukesValid() const
{
	return m_bNukesValid;
}


//	--------------------------------------------------------------------------------
void CvGame::makeNukesValid(bool bValid)
{
	m_bNukesValid = bValid;
}

//	--------------------------------------------------------------------------------
const CvString& CvGame::getName()
{
	return CvPreGame::gameName();
}


//	--------------------------------------------------------------------------------
void CvGame::setName(const char* szName)
{
	CvPreGame::setGameName(szName);
}

//	--------------------------------------------------------------------------------
bool CvGame::isDestroyedCityName(CvString& szName) const
{
	stringHash hasher;
	std::vector<size_t>::const_iterator it = std::find(m_aszDestroyedCities.begin(), m_aszDestroyedCities.end(), hasher(szName.c_str()));
	return it != m_aszDestroyedCities.end();
}

//	--------------------------------------------------------------------------------
void CvGame::addDestroyedCityName(const CvString& szName)
{
	stringHash hasher;
	m_aszDestroyedCities.push_back( hasher(szName.c_str()) );
}

//	--------------------------------------------------------------------------------
bool CvGame::isGreatPersonBorn(CvString& szName) const
{
	stringHash hasher;
	std::vector<size_t>::const_iterator it = std::find(m_aszGreatPeopleBorn.begin(), m_aszGreatPeopleBorn.end(), hasher(szName.c_str()));
	return it != m_aszGreatPeopleBorn.end();
}

//	--------------------------------------------------------------------------------
void CvGame::addGreatPersonBornName(const CvString& szName)
{
	stringHash hasher;
	m_aszGreatPeopleBorn.push_back( hasher(szName.c_str()) );
}

//	--------------------------------------------------------------------------------
void CvGame::removeGreatPersonBornName(const CvString& szName)
{
	stringHash hasher;
	m_aszGreatPeopleBorn.erase(std::remove(m_aszGreatPeopleBorn.begin(), m_aszGreatPeopleBorn.end(), hasher(szName.c_str())), m_aszGreatPeopleBorn.end());
}

// Protected Functions...

//	--------------------------------------------------------------------------------
void CvGame::doTurn()
{
	if (MOD_UNIT_KILL_STATS)
		GC.getMap().DoKillCountDecay();

	// END OF TURN

	//We reset the turn timer now so that we know that the turn timer has been reset at least once for
	//this turn.  CvGameController::Update() will continue to reset the timer if there is prolonged ai processing.
	resetTurnTimer(true);

	m_processPlayerAutoMoves = false; // Starts out as false and gets set to true in updateMoves but was never getting set back to false if MP simultaneous turns with no AI.

	// If player unit cycling has been canceled for this turn, set it back to normal for the next
	GC.GetEngineUserInterface()->setNoSelectionListCycle(false);

	gDLL->DoTurn();

#if defined(MOD_ACTIVE_DIPLOMACY)
	// Dodgy business to cleanup all the completed requests from last turn. Any still here should just be ones that were processed on other clients anyway.
	if (MOD_ACTIVE_DIPLOMACY)
	{
		bool isNetworkMultiPlayer = GC.getGame().isNetworkMultiPlayer();
		for (int iI = 0; iI < MAX_MAJOR_CIVS; iI++)
		{
			CvPlayerAI& kPlayer = GET_PLAYER((PlayerTypes)iI);
			ASSERT_DEBUG((kPlayer.isLocalPlayer() && !kPlayer.GetDiplomacyRequests()->HasPendingRequests()) || !kPlayer.isLocalPlayer(), "Clearing requests, expected local player to be empty.");
			kPlayer.GetDiplomacyRequests()->ClearAllRequests();

			if (isNetworkMultiPlayer && !kPlayer.isHuman()) {
				// in the beginning of turn - remove all the proposed deals from/to this player
				GC.getGame().GetGameDeals().DoCancelAllProposedMPDealsWithPlayer((PlayerTypes)iI, DIPLO_ALL_PLAYERS);
			}
		}
	}
#endif

	doUpdateCacheOnTurn();

	updateScore();

	LogTurnScores();

	m_kGameDeals.DoTurn();

	for(int iI = 0; iI < MAX_TEAMS; iI++)
	{
		if(GET_TEAM((TeamTypes)iI).isAlive())
		{
			GET_TEAM((TeamTypes)iI).doTurn();
		}
	}

	GC.getMap().doTurn();

	GC.GetEngineUserInterface()->doTurn();

	GetGameReligions()->DoTurn();
	GetGameTrade()->DoTurn();
	GetGameLeagues()->DoTurn();
	GetGameCulture()->DoTurn();

#if defined(MOD_BALANCE_CORE)
	GetGameCorporations()->DoTurn();
	GetGameContracts()->DoTurn();

	for (int iLoop = 0; iLoop < GC.getNumResourceInfos(); iLoop++)
	{
		const ResourceTypes eResource = static_cast<ResourceTypes>(iLoop);
		CvResourceInfo* pkResource = GC.getResourceInfo(eResource);
		if (pkResource && pkResource->isMonopoly())
		{
			UpdateGreatestPlayerResourceMonopoly(eResource);
			for (int iI = 0; iI < MAX_MAJOR_CIVS; iI++)
			{
				if (GET_PLAYER((PlayerTypes)iI).isAlive())
				{
					GET_PLAYER((PlayerTypes)iI).CheckForMonopoly(eResource);
				}
			}
		}
	}
	UpdateGreatestPlayerResourceMonopoly();
#endif


	updateGlobalMedians();
	updateEconomicTotal();
	DoBarbCountdown();
	GC.GetEngineUserInterface()->setCanEndTurn(false);
	GC.GetEngineUserInterface()->setHasMovedUnit(false);

	if(getAIAutoPlay() > 0)
	{
		changeAIAutoPlay(-1);

		if(getAIAutoPlay() == 0)
		{
			ReviveActivePlayer();
		}
	}

	m_kGameDeals.DoTurnPost();

	RollOverAssetCounter();

	//-------------------------------------------------------------
	// old turn ends here, new turn starts
	//-------------------------------------------------------------

	OutputDebugString(CvString::format("Turn\t%03i\tTime\t%012u\tThread\t%d\n", getGameTurn(), GetTickCount(), GetCurrentThreadId()));
	incrementGameTurn();
	incrementElapsedGameTurns();
	gDLL->PublishNewGameTurn(getGameTurn());

	if(isOption(GAMEOPTION_DYNAMIC_TURNS))
	{// update turn mode for dynamic turn mode.
		for(int teamIdx = 0; teamIdx < MAX_TEAMS; ++teamIdx)
		{
			CvTeam& curTeam = GET_TEAM((TeamTypes)teamIdx);
			curTeam.setDynamicTurnsSimultMode(!curTeam.isHuman() || !curTeam.isAtWarWithHumans());
		}
	}

	// Configure turn active status for the beginning of the new turn.
	if(isOption(GAMEOPTION_DYNAMIC_TURNS) || isOption(GAMEOPTION_SIMULTANEOUS_TURNS))
	{// In multi-player with simultaneous turns, we activate all of the AI players
	 // at the same time.  The human players who are playing simultaneous turns will be activated in updateMoves after all
	 // the AI players are processed.
		int aiShuffle[MAX_PLAYERS];
		for(int iI = 0; iI < MAX_PLAYERS; iI++)
			aiShuffle[iI] = iI;

		//use the pre-game RNG here
		shuffleArray(aiShuffle, MAX_PLAYERS, getMapRand());

		for(int iI = 0; iI < MAX_PLAYERS; iI++)
		{
			int iLoopPlayer = aiShuffle[iI];
			CvPlayer& player = GET_PLAYER((PlayerTypes)iLoopPlayer);
			// activate AI here, when they are done, activate human players in
			// updateMoves
			if(player.isAlive() && !player.isHuman())
			{
				player.setTurnActive(true);
			}
		}
	}

	if(isSimultaneousTeamTurns())
	{//We're doing simultaneous team turns, activate the first team in sequence.
		for(int iI = 0; iI < MAX_TEAMS; iI++)
		{
			CvTeam& kTeam = GET_TEAM((TeamTypes)iI);
			if(kTeam.isAlive() && !kTeam.isSimultaneousTurns()) 
			{
				kTeam.setTurnActive(true);
				break;
			}
		}
	}
	else if(!isOption(GAMEOPTION_SIMULTANEOUS_TURNS))
	{// player sequential turns.
		// Sequential turns.  Activate the first player we find from the start, human or AI, who wants a sequential turn.
		for(int iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if(GET_PLAYER((PlayerTypes)iI).isAlive() 
				&& !GET_PLAYER((PlayerTypes)iI).isSimultaneousTurns()) //we don't want to be a person who's doing a simultaneous turn for dynamic turn mode.
			{
				if(isPbem() && GET_PLAYER((PlayerTypes)iI).isHuman())
				{
					if(iI == getActivePlayer())
					{
						// Nobody else left alive
						CvPreGame::setGameType(GAME_HOTSEAT_MULTIPLAYER);
						GET_PLAYER((PlayerTypes)iI).setTurnActive(true);
					}
					else if(!getPbemTurnSent())
					{
						gDLL->sendPbemTurn((PlayerTypes)iI);
					}
				}
				else
				{
					GET_PLAYER((PlayerTypes)iI).setTurnActive(true);
					ASSERT_DEBUG(getNumGameTurnActive() == 1);
				}

				break;
			}
		}
	}

#if defined(MOD_BALANCE_CORE)
	if (isOption(GAMEOPTION_RANDOM_VICTORY))
	{
		doVictoryRandomization();
	}
#endif
	// Victory stuff
	testVictory();

#if defined(MOD_ACTIVE_DIPLOMACY)
	if(GC.getGame().isReallyNetworkMultiPlayer() && MOD_ACTIVE_DIPLOMACY)
	{
		// JdH: humans may have been activated, check for AI diplomacy
		CvDiplomacyRequests::DoAIMPDiplomacyWithHumans();
	}
#endif

	// Who's Winning
	if(GET_PLAYER(getActivePlayer()).isAlive() && !IsStaticTutorialActive())
	{
		// Don't show this stuff in MP
		if(!isReallyNetworkMultiPlayer() && !isPbem() && !isHotSeat())
		{
			int iTurnFrequency = /*25*/ GD_INT_GET(PROGRESS_POPUP_TURN_FREQUENCY);

			// This isn't exactly appropriate, but it'll do
			iTurnFrequency *= getGameSpeedInfo().getTrainPercent();
			iTurnFrequency /= 100;

			if(getElapsedGameTurns() % iTurnFrequency == 0)
			{
				// This popup his the sync rand, so beware
				CvPopupInfo kPopupInfo(BUTTONPOPUP_WHOS_WINNING);
				GC.GetEngineUserInterface()->AddPopup(kPopupInfo);
			}
		}
	}

	LogGameState();

	if (isNetworkMultiPlayer())
	{
		//autosave after doing a turn
		gDLL->AutoSave(false, false);

		// send desync warning, if applicable
		if (isDesynced())
		{
			setDesynced(false);
			CvString strWarningText = GetLocalizedText("TXT_KEY_VP_MP_WARNING_DESYNC");
			gGlobals.getDLLIFace()->sendChat(strWarningText, CHATTARGET_ALL, NO_PLAYER);
		}
	}
}

//	--------------------------------------------------------------------------------
ImprovementTypes CvGame::GetBarbarianCampImprovementType()
{
	return (ImprovementTypes)GD_INT_GET(BARBARIAN_CAMP_IMPROVEMENT);
}

int CvGame::GetBarbarianReleaseTurn() const
{ 
	return (m_iEarliestBarbarianReleaseTurn * GC.getGame().getGameSpeedInfo().getTrainPercent()) / 100;
}

//	--------------------------------------------------------------------------------
void CvGame::SetBarbarianReleaseTurn(int iValue)
{
	m_iEarliestBarbarianReleaseTurn = iValue;
}


void DumpUnitInfo(CvUnitEntry* pkUnitInfo)
{
	int iUnitEra = 0;
	TechTypes ePrereqTech = (TechTypes)pkUnitInfo->GetPrereqAndTech();
	if (ePrereqTech != NO_TECH)
	{
		CvTechEntry* pkTechInfo = GC.getTechInfo(ePrereqTech);
		if (pkTechInfo)
			iUnitEra = pkTechInfo->GetEra();
	}

	CvString strAIType;
	getUnitAIString(strAIType,pkUnitInfo->GetDefaultUnitAIType());

	OutputDebugString( CvString::format("Unit,%d,Name,%s,AIType,%d,%s,era,%d,cs,%d,rcs,%d,moves,%d,cost,%d\n",
		pkUnitInfo->GetID(),pkUnitInfo->GetType(),pkUnitInfo->GetDefaultUnitAIType(),strAIType.c_str(),
		iUnitEra,pkUnitInfo->GetCombat(),pkUnitInfo->GetRangedCombat(),pkUnitInfo->GetMoves(),pkUnitInfo->GetProductionCost()).c_str() );
}

//	--------------------------------------------------------------------------------
UnitTypes CvGame::GetRandomUniqueUnitType(bool bIncludeCivsInGame, bool bIncludeStartEra, bool bIncludeOldEras, bool bIncludeRanged, bool bCoastal, int iPlotX, int iPlotY)
{
	// Find the unique units that have already been assigned
	CvSeeder randomSeed;
	std::set<UnitTypes> setUniquesAlreadyAssigned;
	for(int iMinorLoop = MAX_MAJOR_CIVS; iMinorLoop < MAX_CIV_PLAYERS; iMinorLoop++)
	{
		PlayerTypes eMinorLoop = (PlayerTypes) iMinorLoop;
		CvPlayer* pMinorLoop = &GET_PLAYER(eMinorLoop);
		if(pMinorLoop && pMinorLoop->isEverAlive())
		{
			randomSeed.mixAssign(pMinorLoop->GetID()); //needed later
			UnitTypes eUniqueUnit = pMinorLoop->GetMinorCivAI()->GetUniqueUnit();
			if(eUniqueUnit != NO_UNIT)
			{
				setUniquesAlreadyAssigned.insert(eUniqueUnit);
			}
		}
	}
	
	vector<OptionWithScore<UnitTypes>> veUnitRankings;
	// Loop through all Unit Classes twice; once to find UUs that won't exist in game,
	// but if our list of candidates is empty, include all possible UUs the second time around
	for(int iTrialLoop = 0; iTrialLoop < 2; iTrialLoop++)
	{
		for(int iUnitLoop = 0; iUnitLoop < GC.getNumUnitInfos(); iUnitLoop++)
		{
			const UnitTypes eLoopUnit = (UnitTypes) iUnitLoop;
			CvUnitEntry* pkUnitInfo = GC.getUnitInfo(eLoopUnit);
			if(pkUnitInfo == NULL)
				continue;

			bool bValid = (pkUnitInfo->GetCombat() > 0 || pkUnitInfo->GetRangedCombat() > 0);

			// Unit has combat strength, make sure it isn't only defensive (and with no ranged combat ability)
			if(bValid && pkUnitInfo->GetRange() == 0)
			{
				for(int iPromotionLoop = 0; iPromotionLoop < GC.getNumPromotionInfos(); iPromotionLoop++)
				{
					const PromotionTypes ePromotion = (PromotionTypes) iPromotionLoop;
					CvPromotionEntry* pkPromotionInfo = GC.getPromotionInfo(ePromotion);
					if(pkPromotionInfo)
					{
						if(pkUnitInfo->GetFreePromotions(iPromotionLoop))
						{
							if(pkPromotionInfo->IsOnlyDefensive())
							{
								bValid = false;
								break;
							}
						}
					}
				}
			}
			if(!bValid)
				continue;

			//Not valid?
			if (MOD_GLOBAL_EXCLUDE_FROM_GIFTS && pkUnitInfo->IsNoMinorGifts())
				continue;
			if (pkUnitInfo->IsInvalidMinorCivGift())
				continue;
			
			// Compare to the units of other civs, of the same unitclass
			UnitClassTypes eLoopUnitClass = (UnitClassTypes) pkUnitInfo->GetUnitClassType();
			CvUnitClassInfo* pkUnitClassInfo = GC.getUnitClassInfo(eLoopUnitClass);

			if(pkUnitClassInfo == NULL)
			{
				ASSERT_DEBUG(false, "UnitClassInfo is NULL.");
				continue;
			}
			
			// We only want unique units that are not in the game already, or are explicitly Minor Civ Gifts
			if( !MOD_BALANCE_CORE_MINOR_CIV_GIFT || !pkUnitInfo->IsMinorCivGift() )
			{
				if(eLoopUnit == pkUnitClassInfo->getDefaultUnitIndex())
					continue;
				
				if (!bIncludeCivsInGame)
				{
					for(int iMajorLoop = 0; iMajorLoop < MAX_PLAYERS; iMajorLoop++)  // MAX_PLAYERS so that we look at Barbarian UUs (ie. Brute) as well
					{
						PlayerTypes eMajorLoop = (PlayerTypes) iMajorLoop;
						if(GET_PLAYER(eMajorLoop).isAlive())
						{
							UnitTypes eUniqueUnitInGame = (UnitTypes) GET_PLAYER(eMajorLoop).getCivilizationInfo().getCivilizationUnits(eLoopUnitClass);
							if(eLoopUnit == eUniqueUnitInGame)
							{
								bValid = false;
								break;
							}
						}
					}
				}
			}
			if(!bValid)
				continue;
			
			// Avoid Recon units
			if(pkUnitInfo->GetDefaultUnitAIType() == UNITAI_EXPLORE)
				continue;

			// No Ranged units?
			if(!bIncludeRanged && pkUnitInfo->GetRangedCombat() > 0)
				continue;

			// Must be land Unit?
			if(!bCoastal && pkUnitInfo->GetDomainType() != DOMAIN_LAND)
					continue;

			if(pkUnitInfo->GetDomainType() == DOMAIN_AIR)
				continue;

			// Technology level
			TechTypes ePrereqTech = (TechTypes) pkUnitInfo->GetPrereqAndTech();
			EraTypes ePrereqEra = NO_ERA;
			if (ePrereqTech != NO_TECH)
			{
				CvTechEntry* pkTechInfo = GC.getTechInfo(ePrereqTech);
				ASSERT_DEBUG(pkTechInfo, "Tech info not found when picking unique unit for minor civ.");
				if (pkTechInfo)
				{
					ePrereqEra = (EraTypes) pkTechInfo->GetEra();
				}
			}

			if ( ePrereqEra == getStartEra() && !bIncludeStartEra )
				continue;
			else if ( ePrereqEra < getStartEra() && !bIncludeOldEras ) // Assumption: NO_ERA < 0
				continue;

			// Is this Unit already assigned to another minor civ?
			if (setUniquesAlreadyAssigned.count(eLoopUnit) > 0)
				continue;

			int iRandom = randRangeExclusive(0, 300, CvSeeder(iPlotX).mix(iPlotY).mix(iUnitLoop));

			//Weight minor civ gift units higher, so they're more likely to spawn each game (Careful, +50 caused Minor Civs Gifts to take up more than 50% of Military UU slots).
			if (pkUnitInfo->IsMinorCivGift())
				iRandom += 5;

			veUnitRankings.push_back( OptionWithScore<UnitTypes>(eLoopUnit, iRandom));
		}
		// we didn't find any candidates! try again with all UUs
		if (veUnitRankings.size() <= 0)
			bIncludeCivsInGame = true;
		else
			break;
	}

	return PseudoRandomChoiceByWeight(veUnitRankings, NO_UNIT, /*5*/ GD_INT_GET(UNIT_SPAWN_NUM_CHOICES), randomSeed);
}

//	--------------------------------------------------------------------------------
void CvGame::updateWar() const
{
	int iI = 0;
	int iJ = 0;

	if(isOption(GAMEOPTION_ALWAYS_WAR))
	{
		for(iI = 0; iI < MAX_TEAMS; iI++)
		{
			CvTeam& teamI = GET_TEAM((TeamTypes)iI);
			if(teamI.isHuman())
			{
				if(teamI.isAlive())
				{
					for(iJ = 0; iJ < MAX_TEAMS; iJ++)
					{
						CvTeam& teamJ = GET_TEAM((TeamTypes)iJ);
						if(!(teamJ.isHuman()))
						{
							if(teamJ.isAlive())
							{
								if(iI != iJ)
								{
									if(teamI.isHasMet((TeamTypes)iJ))
									{
										if(!atWar(((TeamTypes)iI), ((TeamTypes)iJ)))
										{
											teamI.declareWar(((TeamTypes)iJ), false, teamI.getLeaderID());
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

//	-----------------------------------------------------------------------------------------------
void CvGame::updateMoves()
{
	CvUnit* pLoopUnit = NULL;
	int iLoop = 0;
	int iI = 0;

	// Process all AI first, then process players.
	// Processing of the AI 'first' only occurs when the AI are activated first
	// in doTurn, when MPSIMULTANEOUS_TURNS is set.  If the turns are sequential,
	// only one human or AI is active at one time and this will process them in order.
	vector<PlayerTypes> playersToProcess;

	for(iI = 0; iI < MAX_PLAYERS; iI++)
	{
		CvPlayer& player = GET_PLAYER((PlayerTypes)iI);
		if(player.isAlive() && player.isTurnActive() && !player.isHuman())
		{
			playersToProcess.push_back(static_cast<PlayerTypes>(iI));
			m_processPlayerAutoMoves = false;
			// Notice the break.  Even if there is more than one AI with an active turn, we do them sequentially.
			break;
		}
	}


	int currentTurn = getGameTurn();
	bool activatePlayers = playersToProcess.empty() && m_lastTurnAICivsProcessed != currentTurn;

	m_firstActivationOfPlayersAfterLoad = activatePlayers && m_lastTurnAICivsProcessed == -1;

	// If no AI with an active turn, check humans.
	if(playersToProcess.empty())
	{
		SetLastTurnAICivsProcessed();
		if(gDLL->allAICivsProcessedThisTurn())
		{//everyone is finished processing the AI civs.
			PlayerTypes eActivePlayer = getActivePlayer();
			if(eActivePlayer != NO_PLAYER && CvPreGame::slotStatus(eActivePlayer) == SS_OBSERVER)
			{//if the active player is an observer, send a turn complete so we don't hold up the game.
				//We wait until allAICivsProcessedThisTurn to prevent a race condition where an observer could send turn complete,
				//before all clients have cleared the netbarrier locally.
				if (MOD_API_ACHIEVEMENTS)
				{
					CvPlayer& kActivePlayer = GET_PLAYER(eActivePlayer);
					kActivePlayer.GetPlayerAchievements().EndTurn();
				}

				gDLL->sendTurnComplete();

				if (MOD_API_ACHIEVEMENTS)
					CvAchievementUnlocker::EndTurn();
			}

			if(!m_processPlayerAutoMoves)
			{
				if(!GC.getGame().isOption(GAMEOPTION_DYNAMIC_TURNS) && GC.getGame().isOption(GAMEOPTION_SIMULTANEOUS_TURNS))
				{//fully simultaneous turns.
					// All humans must be ready for auto moves
					bool readyForAutoMoves = true;
					for(iI = 0; iI < MAX_PLAYERS; iI++)
					{
						CvPlayer& player = GET_PLAYER((PlayerTypes)iI);
						if(player.isHuman() && !player.isObserver() && !player.isAutoMoves())
							readyForAutoMoves = false;
					}
					m_processPlayerAutoMoves = readyForAutoMoves;
				}
				else
					m_processPlayerAutoMoves = true;
			}

			for(iI = 0; iI < MAX_PLAYERS; iI++)
			{
				CvPlayer& player = GET_PLAYER((PlayerTypes)iI);

				player.checkInitialTurnAIProcessed();
				if(player.isTurnActive() && player.isHuman())
				{
					playersToProcess.push_back(static_cast<PlayerTypes>(iI));
				}
			}
		}
	}

	vector<PlayerTypes>::const_iterator i;

	for(i = playersToProcess.begin(); i != playersToProcess.end(); ++i)
	{
		CvPlayer& player = GET_PLAYER((PlayerTypes)*i);

		int iReadyUnitsBeforeMoves = player.GetCountReadyUnits();

		if(player.isAlive())
		{
			if (player.isHuman())
			{
				if (GC.getGame().getActivePlayer() == player.GetID())
				{
					GC.getGame().SetCurrentVisibilityPlayer(player.GetID());
				}
			}
			else
			{
				GC.getGame().SetCurrentVisibilityPlayer(player.GetID());
			}

			bool bAutomatedUnitNeedsUpdate = player.hasUnitsThatNeedAIUpdate();
			bool bHomelandAINeedsUpdate = player.GetHomelandAI()->NeedsUpdate();
			if(player.isTurnActive() || bAutomatedUnitNeedsUpdate || bHomelandAINeedsUpdate)
			{
				if(!(player.isAutoMoves()) || bAutomatedUnitNeedsUpdate || bHomelandAINeedsUpdate)
				{
					if(bAutomatedUnitNeedsUpdate || bHomelandAINeedsUpdate || !player.isHuman())
					{
					// ------- this is where the important stuff happens! --------------
						player.AI_unitUpdate(bHomelandAINeedsUpdate);
						NET_MESSAGE_DEBUG_OSTR_ALWAYS("UpdateMoves() : player.AI_unitUpdate() called for player " << player.GetID() << " " << player.getName()); 
					}

					int iReadyUnitsNow = player.GetCountReadyUnits();

					// Was a move completed, if so save off which turn slice this was
					if(iReadyUnitsNow < iReadyUnitsBeforeMoves)
					{
						player.SetLastSliceMoved(m_iTurnSlice);
					}

					if(!(player.isHuman()) && !(player.hasBusyUnitOrCity()))
					{
						if(iReadyUnitsNow == 0)
						{
							player.setAutoMoves(true);
							NET_MESSAGE_DEBUG_OSTR_ALWAYS("UpdateMoves() : player.setAutoMoves(true) called for player " << player.GetID() << " " << player.getName()); 
						}
						else
						{
							const CvUnit* pReadyUnit = player.GetFirstReadyUnit();
							if (pReadyUnit) //there was a hang with a queued attack on autoplay so setAutoMoves(true) was never called
							{
								int iWaitTime = 1000;
								if(!isNetworkMultiPlayer())
								{
									iWaitTime = 10;
								}
								if(m_iTurnSlice - player.GetLastSliceMoved() > iWaitTime)
								{
									CvUnitEntry* entry = GC.getUnitInfo(pReadyUnit->getUnitType());
									if(entry)
									{
										CvString strTemp = entry->GetDescription();
										CvString szAssertMessage;
										szAssertMessage.Format(
										    "GAME HANG - Stuck units will have their turn ended so game can advance. [DETAILS: Player %i %s. First stuck unit is %s at (%d, %d)]",
										    player.GetID(), player.getName(), strTemp.GetCString(), pReadyUnit->getX(), pReadyUnit->getY());
										ASSERT_DEBUG(false, szAssertMessage);
										NET_MESSAGE_DEBUG_OSTR_ALWAYS(szAssertMessage);
									}
									player.EndTurnsForReadyUnits();
								}
							}
						}
					}
				}

				if(player.isAutoMoves() && (!player.isHuman() || m_processPlayerAutoMoves))
				{
					bool bRepeatAutomoves = false;
					int iRepeatPassCount = 2;	// Prevent getting stuck in a loop
					do
					{
						for(pLoopUnit = player.firstUnit(&iLoop); pLoopUnit; pLoopUnit = player.nextUnit(&iLoop))
						{
							CvString tempString;
							getMissionAIString(tempString, pLoopUnit->GetMissionAIType());
							NET_MESSAGE_DEBUG_OSTR_ALWAYS("UpdateMoves() : player " << player.GetID() << " " << player.getName()
																							<< " running AutoMission (" << tempString << ") on " 
																							<< pLoopUnit->getName() << " id=" << pLoopUnit->GetID());

							pLoopUnit->AutoMission();

							// Does the unit still have movement points left over?
							if(player.isHuman() && CvUnitMission::HasCompletedMoveMission(pLoopUnit) && pLoopUnit->canMove() && !pLoopUnit->IsDoingPartialMove() && !pLoopUnit->IsAutomated())
							{
								if(player.isEndTurn())
								{
									bRepeatAutomoves = true;	// Do another pass.
									NET_MESSAGE_DEBUG_OSTR_ALWAYS("UpdateMoves() : player " << player.GetID() << " " << player.getName()
																									<< " AutoMission did not use up all movement points for " 
																									<< pLoopUnit->getName() << " id=" << pLoopUnit->GetID());

									if(player.isLocalPlayer() && gDLL->sendTurnUnready())
										player.setEndTurn(false);
								}
							}

							// slewis sez:

							// This is a short-term solution to a problem where a unit with an auto-mission (a queued, multi-turn) move order cannot reach its destination, but
							//  does not re-enter the "need order" list because this code is processed at the end of turns. The result is that the player could easily "miss" moving
							//  the unit this turn because it displays "next turn" rather than "skip unit turn" and the unit is not added to the "needs orders" list.
							// To correctly fix this problem, we would need some way to determine if any of the auto-missions are invalid before the player can end the turn and
							//  activate the units that have a problem.
							// The problem with evaluating this is that, with one unit per tile, we don't know what is a valid move until other units have moved.
							// (For example, if one unit was to follow another, we would want the unit in the lead to move first and then have the following unit move, in order
							//  to prevent the following unit from constantly waking up because it can't move into the next tile. This is currently not supported.)

							// This short-term solution will reactivate a unit after the player clicks "next turn". It will appear strange, because the player will be asked to move
							// a unit after they clicked "next turn", but it is to give the player a chance to move all of their units.

							// jrandall sez: In MP matches, let's not OOS or stall the game.
							if(!isNetworkMultiPlayer() && !isOption(GAMEOPTION_END_TURN_TIMER_ENABLED))
							{
								if(pLoopUnit && player.isEndTurn() && pLoopUnit->GetLengthMissionQueue() == 0 && pLoopUnit->GetActivityType() == ACTIVITY_AWAKE && pLoopUnit->canMove() && !pLoopUnit->IsDoingPartialMove() && !pLoopUnit->IsAutomated())
								{
									if(IsForceEndingTurn())
									{
										SetForceEndingTurn(false);
									}
									else
									{
										ASSERT_DEBUG(GC.getGame().getActivePlayer() == player.GetID(), "slewis - We should not need to resolve ambiguous end turns for the AI or remotely.");
										if(player.isLocalPlayer() && gDLL->sendTurnUnready())
											player.setEndTurn(false);
									}
								}
							}
						}
					}
					while(bRepeatAutomoves && iRepeatPassCount--);

					// slewis - I changed this to only be the AI because human players should have the tools to deal with this now
					if(!player.isHuman())
					{
						for(pLoopUnit = player.firstUnit(&iLoop); pLoopUnit; pLoopUnit = player.nextUnit(&iLoop))
						{
							if (pLoopUnit->isDelayedDeath() || pLoopUnit->plot() == NULL)
								continue;

							bool bMoveMe  = false;
							IDInfo* pUnitNodeInner = pLoopUnit->plot()->headUnitNode();
							while(pUnitNodeInner != NULL && !bMoveMe)
							{
								CvUnit* pLoopUnitInner = ::GetPlayerUnit(*pUnitNodeInner);
								if(pLoopUnitInner && pLoopUnit != pLoopUnitInner)
								{
									if(pLoopUnit->getOwner() == pLoopUnitInner->getOwner())	// Could be a dying Unit from another player here
									{
										if (!pLoopUnit->canEndTurnAtPlot(pLoopUnit->plot()))
										{
											if(pLoopUnitInner->IsFortified() && !pLoopUnit->IsFortified())
											{
												bMoveMe = true;
											}
										}
									}
								}
								pUnitNodeInner = pLoopUnit->plot()->nextUnitNode(pUnitNodeInner);
							}

							if(bMoveMe)
							{
								if (!pLoopUnit->jumpToNearestValidPlotWithinRange(1))
									pLoopUnit->kill(false);
								break;
							}
						}
					}

					// If we completed the processing of the auto-moves, flag it.
					if(player.isEndTurn() || !player.isHuman())
					{
						player.setProcessedAutoMoves(true);
					}
				}

				// KWG: This code should go into CheckPlayerTurnDeactivate
				if(!player.isEndTurn() && gDLL->HasReceivedTurnComplete(player.GetID()) && player.isHuman() /* && (isNetworkMultiPlayer() || (!isNetworkMultiPlayer() && player.GetID() != getActivePlayer())) */)
				{
					if(!player.hasBusyUnitOrCity())
					{
						player.setEndTurn(true);
						if(player.isEndTurn())
						{//If the player's turn ended, indicate it in the log.  We only do so when the end turn state has changed to prevent useless log spamming in multiplayer. 
							NET_MESSAGE_DEBUG_OSTR_ALWAYS("UpdateMoves() : player.setEndTurn(true) called for player " << player.GetID() << " " << player.getName());
						}
					}
					else
					{
						if(!player.hasBusyUnitUpdatesRemaining())
						{
							NET_MESSAGE_DEBUG_OSTR_ALWAYS("Received turn complete for player "  << player.GetID() << " " << player.getName() << " but there is a busy unit. Forcing the turn to advance");
							player.setEndTurn(true);
						}
					}
				}
			}
		}
	}

	if(activatePlayers)
	{
		if (isOption(GAMEOPTION_DYNAMIC_TURNS) || isOption(GAMEOPTION_SIMULTANEOUS_TURNS))
		{//Activate human players who are playing simultaneous turns now that we've finished moves for the AI.
			// KWG: This code should go into CheckPlayerTurnDeactivate
			for(iI = 0; iI < MAX_PLAYERS; iI++)
			{
				CvPlayer& player = GET_PLAYER((PlayerTypes)iI);
				if(!player.isTurnActive() && player.isHuman() && player.isAlive() && player.isSimultaneousTurns())
				{
					player.setTurnActive(true);
				}
			}
		}
	}
}

//	-----------------------------------------------------------------------------------------------
void CvGame::updateTimers() const
{
	for(int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)iI);
		if(kPlayer.isAlive())
		{
			kPlayer.updateTimers();
		}
	}
#if defined(MOD_ACTIVE_DIPLOMACY)
	if(!GC.getGame().isReallyNetworkMultiPlayer() || !MOD_ACTIVE_DIPLOMACY)
	{
		if(isHotSeat())
		{
			// For Hot Seat, all the AIs will get a chance to do diplomacy with the active human player
			PlayerTypes eActivePlayer = getActivePlayer();
			if(eActivePlayer != NO_PLAYER)
			{
				CvPlayer& kActivePlayer = GET_PLAYER(eActivePlayer);
				if(kActivePlayer.isAlive() && kActivePlayer.isHuman() && kActivePlayer.isTurnActive())
					CvDiplomacyRequests::DoAIDiplomacy(eActivePlayer);
			}
		}
	}
#endif
}

//	-----------------------------------------------------------------------------------------------
void CvGame::UpdatePlayers()
{
	int numActive = 0;
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)i);
		// slewis - should it check for active turn?
		if(kPlayer.isAlive() && kPlayer.isTurnActive())
		{
			kPlayer.UpdateNotifications();
			++numActive;
		}
	}
	ASSERT_DEBUG(numActive == getNumGameTurnActive());
}

//	-----------------------------------------------------------------------------------------------
void CvGame::testAlive()
{
	for(int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		GET_PLAYER((PlayerTypes)iI).verifyAlive();
	}
}

bool CvGame::testVictory(VictoryTypes eVictory, TeamTypes eTeam, bool* pbEndScore) const
{
	ASSERT_DEBUG(eVictory >= 0 && eVictory < GC.getNumVictoryInfos());
	ASSERT_DEBUG(eTeam >=0 && eTeam < MAX_CIV_TEAMS);
	ASSERT_DEBUG(GET_TEAM(eTeam).isAlive());

	CvVictoryInfo* pkVictoryInfo = GC.getVictoryInfo(eVictory);
	if(pkVictoryInfo == NULL)
	{
		return false;
	}

	// Has the player already achieved this victory?
	if(GET_TEAM(eTeam).isVictoryAchieved(eVictory))
	{
		return false;
	}

	bool bValid = isVictoryValid(eVictory);
	if(pbEndScore)
	{
		*pbEndScore = false;
	}

	// Can't end the game unless a certain number of turns has already passed (ignore this on Debug Micro Map because it's only for testing)
	if(getElapsedGameTurns() <= /*10*/ GD_INT_GET(MIN_GAME_TURNS_ELAPSED_TO_TEST_VICTORY) && (GC.getMap().getWorldSize() != WORLDSIZE_DEBUG))
	{
		return false;
	}

	// End Score
	if(bValid)
	{
		if(pkVictoryInfo->isEndScore())
		{
			if(pbEndScore)
			{
				*pbEndScore = true;
			}

			if(getMaxTurns() == 0)
			{
				bValid = false;
			}
			else if(getElapsedGameTurns() < getMaxTurns())
			{
				bValid = false;
			}
			else
			{
				bool bFound = false;

				for(int iK = 0; iK < MAX_CIV_TEAMS; iK++)
				{
					if(GET_TEAM((TeamTypes)iK).isAlive())
					{
						if(iK != eTeam)
						{
							if(getTeamScore((TeamTypes)iK) >= getTeamScore(eTeam))
							{
								bFound = true;
								break;
							}
						}
					}
				}

				if(bFound)
				{
					bValid = false;
				}
			}
		}
	}

	// Target Score
	if(bValid)
	{
		if(pkVictoryInfo->isTargetScore())
		{
			if(getTargetScore() == 0)
			{
				bValid = false;
			}
			else if(getTeamScore(eTeam) < getTargetScore())
			{
				bValid = false;
			}
			else
			{
				bool bFound = false;

				for(int iK = 0; iK < MAX_CIV_TEAMS; iK++)
				{
					if(GET_TEAM((TeamTypes)iK).isAlive())
					{
						if(iK != eTeam)
						{
							if(getTeamScore((TeamTypes)iK) >= getTeamScore(eTeam))
							{
								bFound = true;
								break;
							}
						}
					}
				}

				if(bFound)
				{
					bValid = false;
				}
			}
		}
	}

	// Conquest
	if(bValid)
	{
		if(pkVictoryInfo->isConquest())
		{
			if(GET_TEAM(eTeam).getNumCities() == 0)
			{
				bValid = false;
			}
			else
			{
				bool bFound = false;

				for(int iK = 0; iK < MAX_CIV_TEAMS; iK++)
				{
					if(GET_TEAM((TeamTypes)iK).isAlive())
					{
						if(iK != eTeam)
						{
							if(GET_TEAM((TeamTypes)iK).getNumCities() > 0)
							{
								bFound = true;
								break;
							}
						}
					}
				}

				if(bFound)
				{
					bValid = false;
				}
			}
		}
	}

	// Diplomacy Victory
	if(bValid)
	{
		if(pkVictoryInfo->isDiploVote())
		{
			bValid = false;
			for (int iPlayerLoop = 0; iPlayerLoop < MAX_PLAYERS; iPlayerLoop++)
			{
				PlayerTypes ePlayerLoop = (PlayerTypes) iPlayerLoop;
				if (GET_PLAYER(ePlayerLoop).getTeam() == eTeam)
				{
					if (m_pGameLeagues->GetDiplomaticVictor() == ePlayerLoop)
					{
						bValid = true;
					}
				}
			}
		}
	}

	// Culture victory
	if(bValid)
	{
		if(pkVictoryInfo->isInfluential())
		{
			// See if all players on this team have influential culture with all other players (still alive)
			bValid = false;
			for(int iPlayerLoop = 0; iPlayerLoop < MAX_PLAYERS; iPlayerLoop++)
			{
				CvPlayer &kPlayer = GET_PLAYER((PlayerTypes)iPlayerLoop);
				if (kPlayer.isAlive())
				{
#if defined(MOD_BALANCE_CORE_VICTORY_GAME_CHANGES)
					if(MOD_BALANCE_CORE_VICTORY_GAME_CHANGES)
					{
						if(kPlayer.GetPlayerPolicies()->GetLateGamePolicyTree() == NO_POLICY_BRANCH_TYPE)
							continue;

						if(kPlayer.GetPlayerPolicies()->GetLateGamePolicyTree() != NO_POLICY_BRANCH_TYPE)
						{
							if(kPlayer.GetCulture()->GetPublicOpinionType() > PUBLIC_OPINION_CONTENT)
								continue;
						}

						ProjectTypes eUtopia = (ProjectTypes)GC.getInfoTypeForString("PROJECT_UTOPIA_PROJECT", true);
						if (eUtopia != NO_PROJECT)
						{
							if (GET_TEAM(kPlayer.getTeam()).getProjectCount(eUtopia) <= 0)
								continue;
						}
					}
#endif
					if (kPlayer.getTeam() == eTeam)
					{
						if (kPlayer.GetCulture()->GetNumCivsInfluentialOn() >= m_pGameCulture->GetNumCivsInfluentialForWin())
						{
							// Not enough civs for a win
							bValid = true;
							break;
						}
					}
				}
			}		
		}
	}

	// Religion in all Cities
	if(bValid)
	{
		if(pkVictoryInfo->IsReligionInAllCities())
		{
			bool bReligionInAllCities = true;

			CvCity* pLoopCity = NULL;
			int iLoop = 0;

			PlayerTypes eLoopPlayer;

			// See if all players on this team have their State Religion in their Cities
			for(int iPlayerLoop = 0; iPlayerLoop < MAX_PLAYERS; iPlayerLoop++)
			{
				eLoopPlayer = (PlayerTypes) iPlayerLoop;

				if(GET_PLAYER(eLoopPlayer).isAlive())
				{
					if(GET_PLAYER(eLoopPlayer).getTeam() == eTeam)
					{
						for(pLoopCity = GET_PLAYER(eLoopPlayer).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(eLoopPlayer).nextCity(&iLoop))
						{
							// Any Cities WITHOUT State Religion?
							if(pLoopCity->GetPlayersReligion() != eLoopPlayer)
							{
								bReligionInAllCities = false;
								break;
							}
						}

						if(!bReligionInAllCities)
						{
							break;
						}
					}
				}
			}

			if(!bReligionInAllCities)
			{
				bValid = false;
			}
		}
	}

	// FindAllNaturalWonders
	if(bValid)
	{
		if(pkVictoryInfo->IsFindAllNaturalWonders())
		{
			int iWorldNumNaturalWonders = GC.getMap().GetNumNaturalWonders();

			if(iWorldNumNaturalWonders == 0 || GET_TEAM(eTeam).GetNumNaturalWondersDiscovered() < iWorldNumNaturalWonders)
			{
				bValid = false;
			}
		}
	}

	// Population Percent
	if(bValid)
	{
		if(getAdjustedPopulationPercent(eVictory) > 0)
		{
			if(100 * GET_TEAM(eTeam).getTotalPopulation() < getTotalPopulation() * getAdjustedPopulationPercent(eVictory))
			{
				bValid = false;
			}
		}
	}

	// Land Percent
	if(bValid)
	{
		if(getAdjustedLandPercent(eVictory) > 0)
		{
			if(100 * GET_TEAM(eTeam).getTotalLand() < GC.getMap().getLandPlots() * getAdjustedLandPercent(eVictory))
			{
				bValid = false;
			}
		}
	}

	// Buildings
	if(bValid)
	{
		for(int iK = 0; iK < GC.getNumBuildingClassInfos(); iK++)
		{
			BuildingClassTypes eBuildingClass = static_cast<BuildingClassTypes>(iK);
			CvBuildingClassInfo* pkBuildingClassInfo = GC.getBuildingClassInfo(eBuildingClass);
			if(pkBuildingClassInfo)
			{
				if(pkBuildingClassInfo->getVictoryThreshold(eVictory) > GET_TEAM(eTeam).getBuildingClassCount(eBuildingClass))
				{
					bValid = false;
					break;
				}
			}

		}
	}

	// Projects
	if(bValid)
	{
		for(int iK = 0; iK < GC.getNumProjectInfos(); iK++)
		{
			const ProjectTypes eProject = static_cast<ProjectTypes>(iK);
			CvProjectEntry* pkProjectInfo = GC.getProjectInfo(eProject);
			if(pkProjectInfo)
			{
				if(pkProjectInfo->GetVictoryMinThreshold(eVictory) > GET_TEAM(eTeam).getProjectCount(eProject))
				{
					bValid = false;
					break;
				}
			}
		}
	}

	return bValid;
}

//	---------------------------------------------------------------------------
void CvGame::testVictory()
{
	bool bEndScore = false;

	// Send a game event to allow a Lua script to set the victory state
	ICvEngineScriptSystem1* pkScriptSystem = gDLL->GetScriptSystem();
	if(pkScriptSystem)
	{
		CvLuaArgsHandle args;
		bool bResult = false;
		LuaSupport::CallHook(pkScriptSystem, "GameCoreTestVictory", args.get(), bResult);
	}

	if(getVictory() != NO_VICTORY)
	{
		return;
	}

	if(getGameState() == GAMESTATE_EXTENDED)
	{
		return;
	}

	updateScore();

	bool bEndGame = false;
#if defined(MOD_BALANCE_CORE)
	bool bIsDomination = false;
	bool bIsScore = false;
#endif

	std::vector<std::vector<int> > aaiGameWinners;
	int iTeamLoop = 0;
	int iVictoryLoop = 0;

	int iNumCompetitionWinners = 0;
	for(iTeamLoop = 0; iTeamLoop < MAX_CIV_TEAMS; iTeamLoop++)
	{
		m_aiTeamCompetitionWinnersScratchPad[iTeamLoop] = 0;
	}

	// Look at each Victory Competition
	for(iVictoryLoop = 0; iVictoryLoop < GC.getNumVictoryInfos(); iVictoryLoop++)
	{
		const VictoryTypes eVictory = static_cast<VictoryTypes>(iVictoryLoop);
		CvVictoryInfo* pkVictoryInfo = GC.getVictoryInfo(eVictory);
		if(pkVictoryInfo == NULL)
			continue;

		for(iTeamLoop = 0; iTeamLoop < MAX_CIV_TEAMS; iTeamLoop++)
		{
			CvTeam& kLoopTeam = GET_TEAM((TeamTypes)iTeamLoop);
			if(kLoopTeam.isAlive())
			{
				if(!(kLoopTeam.isMinorCiv()))
				{
					if(testVictory(eVictory, (TeamTypes)iTeamLoop, &bEndScore))
					{
						// Some Victories win the game for the player who accomplishes it.  If this happens for multiple teams in the same turn we have to break the tie
						if(pkVictoryInfo->IsWinsGame())
						{
							std::vector<int> aWinner;
							aWinner.push_back(iTeamLoop);
							aWinner.push_back(iVictoryLoop);
							aaiGameWinners.push_back(aWinner);

							bEndGame = true;
#if defined(MOD_BALANCE_CORE)
							if (pkVictoryInfo->isConquest())
							{
								bIsDomination = true;
							}
#endif
						}
						// Non game-ending Competition winner placement
						else
						{
							m_aiTeamCompetitionWinnersScratchPad[iNumCompetitionWinners] = iTeamLoop;
							iNumCompetitionWinners++;
						}
					}
				}
			}
		}

		// Any (non game-ending) Competition placers?
		if(iNumCompetitionWinners > 0)
		{
			int iRand = 0;

			do
			{
				if(isVictoryAvailable(eVictory))
				{
					iRand = GC.getGame().randRangeExclusive(0, iNumCompetitionWinners, CvSeeder(iNumCompetitionWinners).mix(iVictoryLoop));
					iTeamLoop = m_aiTeamCompetitionWinnersScratchPad[iRand];

					DoPlaceTeamInVictoryCompetition(eVictory, (TeamTypes) iTeamLoop);

					iNumCompetitionWinners--;
				}
				else
				{
					iNumCompetitionWinners = 0;
				}
			}
			while(iNumCompetitionWinners > 0);
		}
	}

	// Game could have been set to ending already by an insta-win victory, or we might have hit the end of time manually
	if(!bEndGame)
	{
		bEndGame = IsEndGameTechResearched();

		if(bEndGame)
		{
			VictoryTypes eScoreVictory = NO_VICTORY;
			for(iVictoryLoop = 0; iVictoryLoop < GC.getNumVictoryInfos(); iVictoryLoop++)
			{
				VictoryTypes eVictory = static_cast<VictoryTypes>(iVictoryLoop);
				CvVictoryInfo* pkVictoryInfo = GC.getVictoryInfo(eVictory);
				if(pkVictoryInfo)
				{
					if(pkVictoryInfo->isTargetScore())
					{
						eScoreVictory = eVictory;
						break;
					}
				}
			}

			aaiGameWinners.clear();

			// Find out who is in the lead with VPs
			int iBestVPNum = 0;
			int iVPs = 0;
			for(iTeamLoop = 0; iTeamLoop < MAX_CIV_TEAMS; iTeamLoop++)
			{
				iVPs = GET_TEAM((TeamTypes) iTeamLoop).getVictoryPoints();

				if(iVPs > iBestVPNum)
				{
					iBestVPNum = iVPs;
				}
			}

			// Now that we know what the highest is, see if any players are tied
			for(iTeamLoop = 0; iTeamLoop < MAX_CIV_TEAMS; iTeamLoop++)
			{
				iVPs = GET_TEAM((TeamTypes) iTeamLoop).getVictoryPoints();

				if(iVPs == iBestVPNum)
				{
					std::vector<int> aWinner;
					aWinner.push_back(iTeamLoop);
					aWinner.push_back(eScoreVictory);
					aaiGameWinners.push_back(aWinner);
				}
			}
		}
	}

	if(!bEndGame)
	{
		bEndGame = (getMaxTurns() > 0 && getElapsedGameTurns() >= getMaxTurns());

		if(bEndGame)
		{
			VictoryTypes eScoreVictory = NO_VICTORY;
			for(iVictoryLoop = 0; iVictoryLoop < GC.getNumVictoryInfos(); iVictoryLoop++)
			{
				VictoryTypes eVictory = static_cast<VictoryTypes>(iVictoryLoop);
				CvVictoryInfo* pkVictoryInfo = GC.getVictoryInfo(eVictory);
				if(pkVictoryInfo)
				{
					if(pkVictoryInfo->isEndScore())
					{
						eScoreVictory = eVictory;
#if defined(MOD_BALANCE_CORE)
						bIsScore = true;
#endif
						break;
					}
				}
			}

			aaiGameWinners.clear();

			// Find out who is in the lead with VPs
			int iBestVPNum = 0;
			int iVPs = 0;
			for(iTeamLoop = 0; iTeamLoop < MAX_CIV_TEAMS; iTeamLoop++)
			{
				iVPs = GET_TEAM((TeamTypes) iTeamLoop).GetScore();

				if(iVPs > iBestVPNum)
				{
					iBestVPNum = iVPs;
				}
			}

			// Now that we know what the highest is, see if any players are tied
			for(iTeamLoop = 0; iTeamLoop < MAX_CIV_TEAMS; iTeamLoop++)
			{
				iVPs = GET_TEAM((TeamTypes) iTeamLoop).GetScore();

				if(iVPs == iBestVPNum)
				{
					std::vector<int> aWinner;
					aWinner.push_back(iTeamLoop);
					aWinner.push_back(eScoreVictory);
					aaiGameWinners.push_back(aWinner);
				}
			}
		}
	}

#if defined(MOD_BALANCE_CORE)
	//Not already defined? Skip!
	if (isOption(GAMEOPTION_RANDOM_VICTORY) && !isVictoryRandomizationDone() && !bIsDomination && !bIsScore)
	{
		aaiGameWinners.clear();
		bEndGame = false;
	}
#endif

	// Two things can set this to true: either someone has finished an insta-win victory, or the game-ending tech has been researched and we're now tallying VPs
	if(bEndGame && !aaiGameWinners.empty())
	{
		uint uWinner = urandLimitExclusive(aaiGameWinners.size(), CvSeeder(aaiGameWinners.size()));
		CUSTOMLOG("Calling setWinner from testVictory: %i, %i", aaiGameWinners[uWinner][0], aaiGameWinners[uWinner][1]);
		setWinner(((TeamTypes)aaiGameWinners[uWinner][0]), ((VictoryTypes)aaiGameWinners[uWinner][1]));
	}

	if(getVictory() == NO_VICTORY)
	{
		if(getMaxTurns() > 0)
		{
			if(getElapsedGameTurns() >= getMaxTurns())
			{
				if(!bEndScore)
				{
					if((getAIAutoPlay() > 0) || gDLL->GetAutorun())
					{
						setGameState(GAMESTATE_EXTENDED);
					}
					else
					{
						setGameState(GAMESTATE_OVER);
					}
				}
			}
		}
	}
}

#if defined(MOD_BALANCE_CORE)
void CvGame::doVictoryRandomization()
{
	//Already done?
	if (isVictoryRandomizationDone())
		return;

	//no one in atomic era?
	bool bFinalEra = false;
	for (int iPlayerLoop = 0; iPlayerLoop < MAX_MAJOR_CIVS; iPlayerLoop++)
	{
		PlayerTypes eLoopPlayer = (PlayerTypes)iPlayerLoop;
		if (eLoopPlayer != NO_PLAYER && GET_PLAYER(eLoopPlayer).isAlive() && !GET_PLAYER(eLoopPlayer).isMinorCiv() && !GET_PLAYER(eLoopPlayer).isBarbarian())
		{
			if (GET_PLAYER(eLoopPlayer).GetCurrentEra() >= (EraTypes) GC.getInfoTypeForString("ERA_POSTMODERN"))
			{
				bFinalEra = true;
			}
		}
	}
	if (!bFinalEra)
	{
		return;
	}
	//Okay, let's do this!

	VictoryTypes eBestVictory = NO_VICTORY;
	int iBestVictoryScore = 0;
	// Look at each Victory Competition
	for (int iVictoryLoop = 0; iVictoryLoop < GC.getNumVictoryInfos(); iVictoryLoop++)
	{
		const VictoryTypes eVictory = static_cast<VictoryTypes>(iVictoryLoop);
		CvVictoryInfo* pkVictoryInfo = GC.getVictoryInfo(eVictory);
		if (pkVictoryInfo == NULL)
			continue;

		//Skip score, always valid.
		if (pkVictoryInfo->isEndScore())
			continue;

		//Skip conquest, always valid.
		if (pkVictoryInfo->isConquest())
			continue;

		int iScore = randRangeExclusive(0, 100, CvSeeder(GC.getGame().GetGameCulture()->GetNumGreatWorks()).mix(iVictoryLoop));
		
		if (pkVictoryInfo->isDiploVote())
		{
			iScore += GC.getGame().GetNumMinorCivsAlive() * 3;
		}
		else if (pkVictoryInfo->isInfluential())
		{
			iScore += GC.getGame().GetGameCulture()->GetNumGreatWorks() / 5;
		}
		else if (eVictory == (VictoryTypes)GC.getInfoTypeForString("VICTORY_SPACE_RACE", true))
		{
			for (int iTeamLoop = 0; iTeamLoop < MAX_CIV_TEAMS; iTeamLoop++)
			{
				TeamTypes eTeam = (TeamTypes)iTeamLoop;
				if (eTeam == NO_TEAM)
					continue;

				if (GET_TEAM(eTeam).isMinorCiv() || GET_TEAM(eTeam).isBarbarian())
					continue;

				iScore += (GET_TEAM(eTeam).GetTeamTechs()->GetNumTechsKnown() / 20);
			}
		}
		else
		{
			continue;
		}
		if (iScore > iBestVictoryScore)
		{
			iBestVictoryScore = iScore;
			eBestVictory = eVictory;
		}
	}

	if (eBestVictory != NO_VICTORY)
	{
		CvVictoryInfo* pkBestVictoryInfo = GC.getVictoryInfo(eBestVictory);
		if (pkBestVictoryInfo == NULL)
			return;

		//Enable good victories
		setVictoryValid(eBestVictory, true);

		if (GC.getLogging() && GC.getAILogging())
		{
			CvString strOutput;

			CvString playerName;
			CvString otherPlayerName;
			CvString strMinorString;
			CvString strDesc;
			CvString strLogName;
			CvString strTemp;

			strLogName = "RandomVictory_Log.csv";

			FILogFile* pLog = NULL;
			pLog = LOGFILEMGR.GetLog(strLogName, FILogFile::kDontTimeStamp);

			// Get the leading info for this line
			strOutput.Format("%03d", GC.getGame().getElapsedGameTurns());

			strTemp.Format("Random Victory Chosen: %s", pkBestVictoryInfo->GetDescription());
			strOutput += ", " + strTemp;

			pLog->Msg(strOutput);
		}

		// Look at each Victory Competition
		for (int iVictoryLoop = 0; iVictoryLoop < GC.getNumVictoryInfos(); iVictoryLoop++)
		{
			const VictoryTypes eVictory = static_cast<VictoryTypes>(iVictoryLoop);
			CvVictoryInfo* pkVictoryInfo = GC.getVictoryInfo(eVictory);
			if (pkVictoryInfo == NULL)
				continue;

			//Skip conquest, always valid.
			if (pkVictoryInfo->isConquest())
				continue;

			//Skip score, always valid.
			if (pkVictoryInfo->isEndScore())
				continue;

			//Skip our best selection.
			if (eBestVictory == eVictory)
				continue;

			//Disable all other victories
			setVictoryValid(eVictory, false);
		}

		for (int iPlayerLoop = 0; iPlayerLoop < MAX_MAJOR_CIVS; iPlayerLoop++)
		{
			PlayerTypes ePlayer = (PlayerTypes)iPlayerLoop;
			if (ePlayer == NO_PLAYER)
				continue;

			if (GET_PLAYER(ePlayer).isHuman() && GC.getGame().getActivePlayer() == ePlayer)
			{
				if (pkBestVictoryInfo->isDiploVote())
				{
					CvPopupInfo kPopupInfo(BUTTONPOPUP_MODDER_4, 1);
					DLLUI->AddPopup(kPopupInfo);
				}
				else if (pkBestVictoryInfo->isInfluential())
				{
					CvPopupInfo kPopupInfo(BUTTONPOPUP_MODDER_4, 2);
					DLLUI->AddPopup(kPopupInfo);
				}
				else if (eBestVictory == (VictoryTypes)GC.getInfoTypeForString("VICTORY_SPACE_RACE", true))
				{
					CvPopupInfo kPopupInfo(BUTTONPOPUP_MODDER_4, 3);
					DLLUI->AddPopup(kPopupInfo);
				}
			}
		}

		setVictoryRandomizationDone(true);
	}
	
}
#endif

//	--------------------------------------------------------------------------------
CvRandom& CvGame::getMapRand()
{
	return m_mapRand;
}


//	--------------------------------------------------------------------------------
int CvGame::getMapRandNum(int iNum, const char* pszLog)
{
	if (iNum > 0)
		return m_mapRand.get(iNum, pszLog);

	return (int)m_mapRand.get(-iNum, pszLog)*(-1);
}


//	--------------------------------------------------------------------------------
CvRandom& CvGame::getJonRand()
{
	return m_jonRand;
}
//	--------------------------------------------------------------------------------
/// Get a synchronous random number in the range of 0...iNum-1
/// Allows for logging.
int CvGame::getJonRandNum(int iNum, const char* pszLog)
{
	if (iNum > 0)
		return m_jonRand.get(iNum, pszLog);

	return (int)m_jonRand.get(-iNum, pszLog)*(-1);
}

//	--------------------------------------------------------------------------------
/// Get a synchronous random number in the range of 0...iNum-1
/// Allows for logging with variable arguments.
/// This method allows logging with formatted strings similar to printf.
/// Unfortunately we need to name the method differently so that the non-va one can still exist without
/// causing ambiguous call errors.  The non VA one is needed for use as a delegate.
int CvGame::getJonRandNumVA(int iNum, const char* pszLog, ...)
{
	if (pszLog)
	{
		const size_t uiOutputSize = 512; // Buffer size for formatted log message
		char szOutput[uiOutputSize]; // Buffer to hold the formatted log message

		va_list vl; // Declare variable argument list
		va_start(vl, pszLog); // Initialize the variable argument list
		vsprintf_s(szOutput, uiOutputSize, pszLog, vl); // Format the log message into the buffer
		va_end(vl); // Clean up the variable argument list

		return getJonRandNum(iNum, szOutput); // Call the non-variable argument method with the formatted log message
	}
	else
		return getJonRandNum(iNum, NULL); // Call the non-variable argument method with no log message
}

//	--------------------------------------------------------------------------------
/// Get an asynchronous random number in the range of 0...iNum-1
/// This should only be called by operations that will not affect gameplay!
int CvGame::getAsyncRandNum(int iNum, const char* pszLog)
{
	if (iNum > 0)
		return GC.getASyncRand().get(iNum, pszLog);

	return (int)GC.getASyncRand().get(-iNum, pszLog)*(-1);
}
//	--------------------------------------------------------------------------------
uint CvGame::randCore(const CvSeeder& extraSeed) const
{
	const CvSeeder mapSeed = CvSeeder::fromRaw(CvPreGame::mapRandomSeed());
	const CvSeeder gameSeed = CvSeeder(getGameTurn());
	return mapSeed.mix(gameSeed).mix(extraSeed);
}

uint CvGame::urandLimitExclusive(uint limit, const CvSeeder& extraSeed) const
{
	ASSERT_DEBUG(limit != 0);
	const uint rand = randCore(extraSeed);
	return rand % limit;
}

uint CvGame::urandLimitInclusive(uint limit, const CvSeeder& extraSeed) const
{
	const uint rand = randCore(extraSeed);
	if (limit == UINT_MAX)
	{
		return rand;
	}
	else
	{
		return rand % (limit + 1);
	}
}

uint CvGame::urandRangeExclusive(uint min, uint max, const CvSeeder& extraSeed) const
{
	ASSERT_DEBUG(min < max);
	return urandLimitExclusive(max - min, extraSeed) + min;
}

uint CvGame::urandRangeInclusive(uint min, uint max, const CvSeeder& extraSeed) const
{
	ASSERT_DEBUG(min <= max);
	return urandLimitInclusive(max - min, extraSeed) + min;
}

int CvGame::randRangeExclusive(int min, int max, const CvSeeder& extraSeed) const
{
	ASSERT_DEBUG(min < max);
	return static_cast<int>(urandLimitExclusive(static_cast<uint>(max) - static_cast<uint>(min), extraSeed) + static_cast<uint>(min));
}

int CvGame::randRangeInclusive(int min, int max, const CvSeeder& extraSeed) const
{
	ASSERT_DEBUG(min <= max);
	return static_cast<int>(urandLimitInclusive(static_cast<uint>(max) - static_cast<uint>(min), extraSeed) + static_cast<uint>(min));
}


//	--------------------------------------------------------------------------------
int CvGame::calculateSyncChecksum()
{
	CvUnit* pLoopUnit = NULL;
	unsigned int iMultiplier = 0;
	unsigned long long iValue = 0;
	int iLoop = 0;
	int iI = 0;
	int iJ = 0;

	iValue = 0;

	iValue += getMapRand().getSeed();
	iValue += getJonRand().getSeed();

	iValue += getNumCities();
	iValue += getTotalPopulation();

	iValue += GC.getMap().getOwnedPlots();
	iValue += GC.getMap().getNumAreas();

	for(iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if(GET_PLAYER((PlayerTypes)iI).isEverAlive())
		{
			iMultiplier = getPlayerScore((PlayerTypes)iI);

			switch(getTurnSlice() % 4)
			{
			case 0:
				iMultiplier += (GET_PLAYER((PlayerTypes)iI).getTotalPopulation() * 543271);
				iMultiplier += (GET_PLAYER((PlayerTypes)iI).getTotalLand() * 327382);
				iMultiplier += (GET_PLAYER((PlayerTypes)iI).GetTreasury()->GetGold() * 107564);
				iMultiplier += (GET_PLAYER((PlayerTypes)iI).getPower() * 135647);
				iMultiplier += (GET_PLAYER((PlayerTypes)iI).getNumCities() * 436432);
				iMultiplier += (GET_PLAYER((PlayerTypes)iI).getNumUnits() * 324111);
				break;

			case 1:
				for(iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
				{
					iMultiplier += (GET_PLAYER((PlayerTypes)iI).calculateTotalYield((YieldTypes)iJ) * 432754);
				}
				break;

			case 2:
				for(iJ = 0; iJ < GC.getNumImprovementInfos(); iJ++)
				{
					iMultiplier += (GET_PLAYER((PlayerTypes)iI).getImprovementCount((ImprovementTypes)iJ) * 883422);
				}

				for(iJ = 0; iJ < GC.getNumBuildingClassInfos(); iJ++)
				{
					CvBuildingClassInfo* pkBuildingClassInfo = GC.getBuildingClassInfo((BuildingClassTypes)iJ);
					if(!pkBuildingClassInfo)
					{
						continue;
					}

					iMultiplier += (GET_PLAYER((PlayerTypes)iI).getBuildingClassCountPlusMaking((BuildingClassTypes)iJ) * 954531);
				}

				for(iJ = 0; iJ < GC.getNumUnitClassInfos(); iJ++)
				{
					CvUnitClassInfo* pkUnitClassInfo = GC.getUnitClassInfo((UnitClassTypes)iJ);
					if(!pkUnitClassInfo)
					{
						continue;
					}

					iMultiplier += (GET_PLAYER((PlayerTypes)iI).getUnitClassCountPlusMaking((UnitClassTypes)iJ) * 754843);
				}
				break;

			case 3:
				for(pLoopUnit = GET_PLAYER((PlayerTypes)iI).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER((PlayerTypes)iI).nextUnit(&iLoop))
				{
					iMultiplier += (pLoopUnit->getX() * 876543);
					iMultiplier += (pLoopUnit->getY() * 985310);
					iMultiplier += (pLoopUnit->getDamage() * 736373);
					iMultiplier += ((pLoopUnit->getExperienceTimes100() / 100) * 820622);
					iMultiplier += (pLoopUnit->getLevel() * 367291);
				}
				break;
			}

			if(iMultiplier != 0)
			{
				iValue *= iMultiplier;
			}
		}
	}

#if defined(MOD_CORE_DEBUGGING)
	debugSyncChecksum();
#endif

	return iValue & 0xFFFFFFFF;
}

#if defined(MOD_BALANCE_CORE)
void CvGame::debugSyncChecksum()
{
	CvString fname = CvString::format( "AssetsTurn%03d.txt", GC.getGame().getGameTurn() );
	FILogFile* pLog=LOGFILEMGR.GetLog( fname.c_str(), FILogFile::kDontTimeStamp );
	if (!pLog)
		return;

	pLog->Msg( "----global-----\n" );
	pLog->Msg( CvString::format("MapRandSeed: %I64u\n",getMapRand().getSeed()).c_str() );
	pLog->Msg( CvString::format("GameRandSeed: %I64u\n",getJonRand().getSeed()).c_str() );
	pLog->Msg( CvString::format("NCities: %d\n",getNumCities()).c_str() );
	pLog->Msg( CvString::format("TotalPop: %d\n",getTotalPopulation()).c_str() );
	pLog->Msg( CvString::format("OwnedPlots: %d\n",GC.getMap().getOwnedPlots()).c_str() );
	pLog->Msg( CvString::format("MapAreas: %d\n",GC.getMap().getNumAreas()).c_str() );

	for(int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if(GET_PLAYER((PlayerTypes)iI).isEverAlive())
		{
			pLog->Msg( CvString::format("------ Player %02d : %s\n",iI,GET_PLAYER((PlayerTypes)iI).getCivilizationAdjective()).c_str() );

			pLog->Msg( CvString::format("Score: %d\n",getPlayerScore((PlayerTypes)iI)).c_str() );

			pLog->Msg( CvString::format("pop: %d\n",GET_PLAYER((PlayerTypes)iI).getTotalPopulation()).c_str() );
			pLog->Msg( CvString::format("land: %d\n",GET_PLAYER((PlayerTypes)iI).getTotalLand()).c_str() );
			pLog->Msg( CvString::format("gold: %d\n",GET_PLAYER((PlayerTypes)iI).GetTreasury()->GetGold()).c_str() );
			pLog->Msg( CvString::format("power: %d\n",GET_PLAYER((PlayerTypes)iI).getPower()).c_str() );
			pLog->Msg( CvString::format("cities: %d\n",GET_PLAYER((PlayerTypes)iI).getNumCities()).c_str() );
			pLog->Msg( CvString::format("units: %d\n",GET_PLAYER((PlayerTypes)iI).getNumUnits()).c_str() );

			for(int iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
				pLog->Msg( CvString::format("yield type %d: %d\n",iJ,GET_PLAYER((PlayerTypes)iI).calculateTotalYield((YieldTypes)iJ)).c_str() );

			for(int iJ = 0; iJ < GC.getNumImprovementInfos(); iJ++)
				pLog->Msg( CvString::format("improvement type %d: %d\n",iJ,GET_PLAYER((PlayerTypes)iI).getImprovementCount((ImprovementTypes)iJ)).c_str() );

			for(int iJ = 0; iJ < GC.getNumBuildingClassInfos(); iJ++)
			{
				CvBuildingClassInfo* pkBuildingClassInfo = GC.getBuildingClassInfo((BuildingClassTypes)iJ);
				if(!pkBuildingClassInfo)
					continue;

				int iCount = GET_PLAYER((PlayerTypes)iI).getBuildingClassCountPlusMaking((BuildingClassTypes)iJ);
				if (iCount>0)
					pLog->Msg( CvString::format("building type %d: %d\n",iJ,iCount).c_str() );
			}

			for(int iJ = 0; iJ < GC.getNumUnitClassInfos(); iJ++)
			{
				CvUnitClassInfo* pkUnitClassInfo = GC.getUnitClassInfo((UnitClassTypes)iJ);
				if(!pkUnitClassInfo)
					continue;

				int iCount = GET_PLAYER((PlayerTypes)iI).getUnitClassCountPlusMaking((UnitClassTypes)iJ);
				if (iCount>0)
					pLog->Msg( CvString::format("unit type %d: %d\n",iJ,iCount).c_str() );
			}

			int iLoop=0;
			for(CvUnit* pLoopUnit = GET_PLAYER((PlayerTypes)iI).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER((PlayerTypes)iI).nextUnit(&iLoop))
				pLog->Msg( CvString::format("unit %d: x %02d, y %02d, damage %02d, exp %02d, level %02d\n",
					iLoop, pLoopUnit->getX(), pLoopUnit->getY(), pLoopUnit->getDamage(), (pLoopUnit->getExperienceTimes100() / 100), pLoopUnit->getLevel() ).c_str() );
		}
	}
}

#endif

//	--------------------------------------------------------------------------------
int CvGame::calculateOptionsChecksum()
{
	int iValue = 0;
	int iI = 0;
	int iJ = 0;

	iValue = 0;

	for(iI = 0; iI < MAX_PLAYERS; iI++)
	{
		CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)iI);

		for(iJ = 0; iJ < GC.getNumPlayerOptionInfos(); iJ++)
		{
			CvPlayerOptionInfo* pkInfo = GC.getPlayerOptionInfo((PlayerOptionTypes)iJ);
			if (pkInfo)
			{
				uint uiID = FStringHash( pkInfo->GetType() );
				if(kPlayer.isOption((PlayerOptionTypes)uiID))
				{
					iValue += (iI * 943097);
					iValue += (iJ * 281541);
				}
			}
		}
	}

	return iValue;
}


//	--------------------------------------------------------------------------------
void CvGame::addReplayMessage(ReplayMessageTypes eType, PlayerTypes ePlayer, const CvString& pszText, int iPlotX, int iPlotY)
{
	int iGameTurn = getGameTurn();

	//If this is a plot-related message, search for any previously created messages that match this one and just add the plot.
	if(iPlotX != -1 || iPlotY != -1)
	{
		for(ReplayMessageList::iterator it = m_listReplayMessages.begin(); it != m_listReplayMessages.end(); ++it)
		{
			CvReplayMessage& msg = (*it);
			if(msg.getType() == eType && msg.getTurn() == iGameTurn && msg.getPlayer() == ePlayer && msg.getText() == pszText)
			{
				msg.addPlot(iPlotX, iPlotY);
				return;
			}
		}
	}

	CvReplayMessage message(iGameTurn, eType, ePlayer);
	message.addPlot(iPlotX, iPlotY);
	message.setText(pszText);
	m_listReplayMessages.push_back(message);
}

//	--------------------------------------------------------------------------------
void CvGame::clearReplayMessageMap()
{
	m_listReplayMessages.clear();
}

//	--------------------------------------------------------------------------------
uint CvGame::getNumReplayMessages() const
{
	return m_listReplayMessages.size();
}

//	--------------------------------------------------------------------------------
int CvGame::CalculateMedianNumCities()
{
	vector<int> v;
	for (int iPlayerLoop = 0; iPlayerLoop < MAX_MAJOR_CIVS; iPlayerLoop++)
	{
		PlayerTypes eLoopPlayer = (PlayerTypes) iPlayerLoop;
		const CvPlayer& kPlayer = GET_PLAYER(eLoopPlayer);
		
		if (kPlayer.isMajorCiv() && kPlayer.isAlive() && kPlayer.getNumCities() > 0)
			v.push_back(kPlayer.getNumCities());
	}

	if (v.empty())
		return 0;

	std::stable_sort(v.begin(), v.end());

	return v[v.size()/2];
}

//	--------------------------------------------------------------------------------
int CvGame::CalculateMedianNumPlots()
{
	vector<int> v;
	for (int iPlayerLoop = 0; iPlayerLoop < MAX_MAJOR_CIVS; iPlayerLoop++)
	{
		PlayerTypes eLoopPlayer = (PlayerTypes) iPlayerLoop;
		const CvPlayer& kPlayer = GET_PLAYER(eLoopPlayer);
		
		if (kPlayer.isMajorCiv() && kPlayer.isAlive() && kPlayer.getNumCities() > 0)
			v.push_back(kPlayer.getTotalLand());
	}

	if (v.empty())
		return 0;

	std::stable_sort(v.begin(), v.end());

	return v[v.size()/2];
}

//	--------------------------------------------------------------------------------
int CvGame::CalculateMedianNumWondersConstructed()
{
	vector<int> v;
	for (int iPlayerLoop = 0; iPlayerLoop < MAX_MAJOR_CIVS; iPlayerLoop++)
	{
		PlayerTypes eLoopPlayer = (PlayerTypes) iPlayerLoop;
		const CvPlayer& kPlayer = GET_PLAYER(eLoopPlayer);

		if (kPlayer.isMajorCiv() && kPlayer.GetWondersConstructed() > 0)
			v.push_back(kPlayer.GetWondersConstructed());
	}

	if (v.empty())
		return 0;

	std::stable_sort(v.begin(), v.end());

	return v[v.size()/2];
}


//	--------------------------------------------------------------------------------

void CvGame::updateEconomicTotal()
{
	CvCity* pLoopCity = NULL;
	int iCityLoop = 0;
	PlayerTypes eLoopPlayer;

	std::vector<int> viEconValues;

	int iTotalEconomicValue = 0;
	int iHighestVal = 0;
	for (int iPlayerLoop = 0; iPlayerLoop < MAX_CIV_PLAYERS; iPlayerLoop++)
	{
		eLoopPlayer = (PlayerTypes)iPlayerLoop;
		if (eLoopPlayer != NO_PLAYER && GET_PLAYER(eLoopPlayer).isAlive() && !GET_PLAYER(eLoopPlayer).isBarbarian())
		{
			for (pLoopCity = GET_PLAYER(eLoopPlayer).firstCity(&iCityLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(eLoopPlayer).nextCity(&iCityLoop))
			{
				if (pLoopCity != NULL)
				{
					int iVal = pLoopCity->getEconomicValue(pLoopCity->getOwner());
					if (iVal > iHighestVal)
						iHighestVal = iVal;

					iTotalEconomicValue += iVal;
					viEconValues.push_back(iVal);
				}
			}
		}
	}
	setTotalEconomicValue(iTotalEconomicValue);
	setHighestEconomicValue(iHighestVal);

	if (viEconValues.empty())
		return;

	size_t n = viEconValues.size() / 2;
	std::nth_element(viEconValues.begin(), viEconValues.begin() + n, viEconValues.end());
	setMedianEconomicValue(viEconValues[n]);
}
//	--------------------------------------------------------------------------------
/// Updates global medians for yields. Also updates global population (counting only citizens in non-puppet/razing/resistance cities).
void CvGame::updateGlobalMedians()
{
	if (!MOD_BALANCE_VP)
		return;

	std::vector<float> vfBasicNeedsYield;
	std::vector<float> vfGoldYield;
	std::vector<float> vfScienceYield;
	std::vector<float> vfCultureYield;
	std::vector<int> viTechsResearched;
	int iCityLoop = 0;

	for (int iPlayerLoop = 0; iPlayerLoop < MAX_MAJOR_CIVS; iPlayerLoop++)
	{
		PlayerTypes eLoopPlayer = (PlayerTypes) iPlayerLoop;

		if (!GET_PLAYER(eLoopPlayer).isAlive() || !GET_PLAYER(eLoopPlayer).isMajorCiv() || GET_PLAYER(eLoopPlayer).getNumCities() <= 0)
			continue;

		viTechsResearched.push_back(GET_TEAM(GET_PLAYER(eLoopPlayer).getTeam()).GetTeamTechs()->GetNumTechsKnown());

		for (CvCity* pLoopCity = GET_PLAYER(eLoopPlayer).firstCity(&iCityLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(eLoopPlayer).nextCity(&iCityLoop))
		{
			if (pLoopCity->IsPuppet() || pLoopCity->IsRazing() || pLoopCity->IsResistance())
				continue;

			int iPopulation = pLoopCity->getPopulation();
			if (iPopulation < /*3*/ GD_INT_GET(YIELD_MEDIAN_MIN_POP_REQUIREMENT))
				continue;

			float fPopulation = (float)iPopulation;

			// Distress
			int iBasicNeedsYield = pLoopCity->getYieldRateTimes100(YIELD_FOOD, true) + pLoopCity->getYieldRateTimes100(YIELD_PRODUCTION, true);
			float fBasicNeedsAvg = iBasicNeedsYield / fPopulation;
			vfBasicNeedsYield.push_back(fBasicNeedsAvg);

			// Poverty
			int iGoldYield = pLoopCity->getYieldRateTimes100(YIELD_GOLD, true);
			float fGoldAvg = iGoldYield / fPopulation;
			vfGoldYield.push_back(fGoldAvg);

			// Illiteracy
			int iScienceYield = pLoopCity->getYieldRateTimes100(YIELD_SCIENCE, true);
			float fScienceAvg = iScienceYield / fPopulation;
			vfScienceYield.push_back(fScienceAvg);

			// Boredom
			int iCultureYield = pLoopCity->getYieldRateTimes100(YIELD_CULTURE, true);
			float fCultureAvg = iCultureYield / fPopulation;
			vfCultureYield.push_back(fCultureAvg);
		}
	}

	// Cannot define median if calculations are at zero.
	if (vfBasicNeedsYield.empty() || vfGoldYield.empty() || vfCultureYield.empty() || vfScienceYield.empty() || viTechsResearched.empty())
		return;
	
	// Select n-th percentile of each category
	size_t n = (vfCultureYield.size() * /*50*/ range(GD_INT_GET(YIELD_MEDIAN_PERCENTILE), 1, 100)) / 100;
	size_t nt = (viTechsResearched.size() * /*50*/ range(GD_INT_GET(TECH_COUNT_MEDIAN_PERCENTILE), 1, 100)) / 100;

	// Find it ...
	std::nth_element(vfBasicNeedsYield.begin(), vfBasicNeedsYield.begin()+n, vfBasicNeedsYield.end());
	std::nth_element(vfGoldYield.begin(), vfGoldYield.begin()+n, vfGoldYield.end());
	std::nth_element(vfScienceYield.begin(), vfScienceYield.begin()+n, vfScienceYield.end());
	std::nth_element(vfCultureYield.begin(), vfCultureYield.begin()+n, vfCultureYield.end());
	std::nth_element(viTechsResearched.begin(), viTechsResearched.begin() + nt, viTechsResearched.end());

	// And set it.
	SetMedianTechsResearched((int)viTechsResearched[nt]);

	// Exponential smoothing so the yield medians change gradually
	float fAlpha = /*0.65f*/ GD_FLOAT_GET(DISTRESS_MEDIAN_RATE_CHANGE);
	int iNewMedian = int(0.5f + ((int)vfBasicNeedsYield[n] * fAlpha) + (GetBasicNeedsMedian() * (1 - fAlpha)));
	SetBasicNeedsMedian(iNewMedian);

	fAlpha = /*0.65f*/ GD_FLOAT_GET(POVERTY_MEDIAN_RATE_CHANGE);
	iNewMedian = int(0.5f + ((int)vfGoldYield[n] * fAlpha) + (GetGoldMedian() * (1 - fAlpha)));
	SetGoldMedian(iNewMedian);

	fAlpha = /*0.65f*/ GD_FLOAT_GET(ILLITERACY_MEDIAN_RATE_CHANGE);
	iNewMedian = int(0.5f + ((int)vfScienceYield[n] * fAlpha) + (GetScienceMedian() * (1 - fAlpha)));
	SetScienceMedian(iNewMedian);

	fAlpha = /*0.65f*/ GD_FLOAT_GET(BOREDOM_MEDIAN_RATE_CHANGE);
	iNewMedian = int(0.5f + ((int)vfCultureYield[n] * fAlpha) + (GetCultureMedian() * (1 - fAlpha)));
	SetCultureMedian(iNewMedian);

	DoGlobalMedianLogging();
}
//	--------------------------------------------------------------------------------
void CvGame::SetMedianTechsResearched(int iValue)
{
	m_iMedianTechsResearched = iValue;
}
void CvGame::SetBasicNeedsMedian(int iValue)
{
	m_iBasicNeedsMedian = iValue;
}
void CvGame::SetGoldMedian(int iValue)
{
	m_iGoldMedian = iValue;
}
void CvGame::SetScienceMedian(int iValue)
{
	m_iScienceMedian = iValue;
}
void CvGame::SetCultureMedian(int iValue)
{
	m_iCultureMedian = iValue;
}
//	--------------------------------------------------------------------------------
void CvGame::DoGlobalMedianLogging() const
{
	if (GC.getLogging() && GC.getAILogging())
	{
		CvString strOutput;
		CvString strTemp;

		CvString strLogName = "GlobalMedian_Log.csv";
		FILogFile* pLog = LOGFILEMGR.GetLog(strLogName, FILogFile::kDontTimeStamp);

		strOutput.Format("Turn: %03d", GC.getGame().getElapsedGameTurns());
		strTemp.Format("Food/Production: %d", GetBasicNeedsMedian());
		strOutput += ", " + strTemp;

		strTemp.Format("Gold: %d", GetGoldMedian());
		strOutput += ", " + strTemp;

		strTemp.Format("Science: %d", GetScienceMedian());
		strOutput += ", " + strTemp;

		strTemp.Format("Culture: %d", GetCultureMedian());
		strOutput += ", " + strTemp;
		
		pLog->Msg(strOutput);
	}
}
//	--------------------------------------------------------------------------------
int CvGame::GetMedianTechsResearched() const
{
	return m_iMedianTechsResearched;
}
int CvGame::GetBasicNeedsMedian() const
{
	return m_iBasicNeedsMedian;
}
int CvGame::GetGoldMedian() const
{
	return m_iGoldMedian;
}
int CvGame::GetScienceMedian() const
{
	return m_iScienceMedian;
}
int CvGame::GetCultureMedian() const
{
	return m_iCultureMedian;
}

void CvGame::initSpyThreshold()
{
	if (!MOD_BALANCE_CORE_SPIES)
		return;

	int iSpyRatio = /*20*/ GD_INT_GET(BALANCE_SPY_TO_PLAYER_RATIO);
	int iMajorMultiplier = /*2*/ GD_INT_GET(BALANCE_SPY_POINT_MAJOR_PLAYER_MULTIPLIER);
	int iMinThreshold = /*33*/ GD_INT_GET(BALANCE_SPY_POINT_THRESHOLD_MIN);
	int iMaxThreshold = /*100*/ GD_INT_GET(BALANCE_SPY_POINT_THRESHOLD_MAX);

	// Avoid division by zero
	int iNumMinors = max(1, GetNumMinorCivsEver(true));
	int iNumMajors = max(1, GetNumMajorCivsEver(true));

#if defined(VPDEBUG)
	CUSTOMLOG("Spy Threshold Config: Ratio=%d, MajorMult=%d, Min=%d, Max=%d",
		iSpyRatio, iMajorMultiplier, iMinThreshold, iMaxThreshold);
	CUSTOMLOG("Spy Threshold Civs: Minors=%d, Majors=%d", iNumMinors, iNumMajors);
#endif

	// iThreshold = (100 * 20) / (numMinors + 2 * numMajors)
	int iThreshold = 100 * iSpyRatio / (iNumMinors + iMajorMultiplier * iNumMajors);

	// Clamp the result between 33 and 100
	m_iSpyThreshold = range(iThreshold, iMinThreshold, iMaxThreshold);

#if defined(VPDEBUG)
	CUSTOMLOG("Spy Threshold Result: %d", m_iSpyThreshold);
#endif
}

int CvGame::GetSpyThreshold() const
{
	// failsafe: if initSpyThreshold has not been called yet for some reason, calculate the threshold based on the number of players ever alive
	if (m_iSpyThreshold == 0)
	{
		int iTempThreshold = 100 * /*20*/ GD_INT_GET(BALANCE_SPY_TO_PLAYER_RATIO) / (GetNumMinorCivsEver(false) + /*2*/ GD_INT_GET(BALANCE_SPY_POINT_MAJOR_PLAYER_MULTIPLIER) * GetNumMajorCivsEver());
		iTempThreshold = range(iTempThreshold, /*33*/ GD_INT_GET(BALANCE_SPY_POINT_THRESHOLD_MIN), /*100*/ GD_INT_GET(BALANCE_SPY_POINT_THRESHOLD_MAX));
		return iTempThreshold;
	}

	return m_iSpyThreshold;
}
//	--------------------------------------------------------------------------------
void CvGame::DoBarbCountdown()
{
	// Update the number of threatening Barbarians for each City-State
	// this is done here to make sure it's done after the Barbarians have moved
	for (int iMinorLoop = MAX_MAJOR_CIVS; iMinorLoop < MAX_CIV_PLAYERS; iMinorLoop++)
	{
		PlayerTypes eMinor = (PlayerTypes)iMinorLoop;

		if (GET_PLAYER(eMinor).isMinorCiv() && GET_PLAYER(eMinor).isAlive())
			GET_PLAYER(eMinor).GetMinorCivAI()->DoUpdateNumThreateningBarbarians();
	}

	if (!MOD_BALANCE_VP)
		return;

	for (int iMinorLoop = MAX_MAJOR_CIVS; iMinorLoop < MAX_CIV_PLAYERS; iMinorLoop++)
	{
		PlayerTypes eMinor = (PlayerTypes) iMinorLoop;

		if (!GET_PLAYER(eMinor).isMinorCiv())
			continue;

		int iTurnsSinceRebellion = GET_PLAYER(eMinor).GetMinorCivAI()->GetTurnsSinceRebellion();
		if (iTurnsSinceRebellion > 0)
		{
			GET_PLAYER(eMinor).GetMinorCivAI()->ChangeTurnsSinceRebellion(-1);
			if (iTurnsSinceRebellion > 1 && GET_PLAYER(eMinor).isAlive() && GET_PLAYER(eMinor).getCapitalCity())
			{
				// Spawn rebels once every X turns, and never on the final turn
				bool bSpawnRebels = iTurnsSinceRebellion == /*20*/ GD_INT_GET(MINOR_QUEST_REBELLION_TIMER);
				bSpawnRebels |= iTurnsSinceRebellion % /*4*/ std::max(GD_INT_GET(MINOR_QUEST_REBELLION_BARB_SPAWN_INTERVAL), 1) == 0;

				if (bSpawnRebels)
					GET_PLAYER(eMinor).GetMinorCivAI()->DoRebellion();
			}
		}
	}
}


void CvGame::SetLastTurnCSAnnexed(int iValue)
{
	m_iLastTurnCSSurrendered = iValue;
}
int CvGame::GetLastTurnCSAnnexed() const
{
	return m_iLastTurnCSSurrendered;
}

//	--------------------------------------------------------------------------------
const CvReplayMessage* CvGame::getReplayMessage(uint i) const
{
	if(i < m_listReplayMessages.size())
	{
		return &(m_listReplayMessages[i]);
	}

	return NULL;
}

// Private Functions...

//	--------------------------------------------------------------------------------
template<typename Game, typename Visitor>
void CvGame::Serialize(Game& game, Visitor& visitor)
{
	const bool bLoading = visitor.isLoading();
	//const bool bSaving = visitor.isSaving();

	visitor(game.m_iEndTurnMessagesSent);
	visitor(game.m_iElapsedGameTurns);
	visitor(game.m_iStartTurn);
	visitor(game.m_iWinningTurn);
	visitor(game.m_iStartYear);
	visitor(game.m_iEstimateEndTurn);
	visitor(game.m_iDefaultEstimateEndTurn);
	visitor(game.m_iTurnSlice);
	visitor(game.m_iCutoffSlice);
	visitor(game.m_iNumCities);
	visitor(game.m_iTotalPopulation);
	visitor(game.m_iTotalEconomicValue);
	visitor(game.m_iHighestEconomicValue);
	visitor(game.m_iMedianEconomicValue);
	visitor(game.m_iNoNukesCount);
	visitor(game.m_iNukesExploded);
	visitor(game.m_iMaxPopulation);
	visitor(game.m_iGlobalAssetCounterAllPreviousTurns);
	visitor(game.m_iGlobalAssetCounterCurrentTurn);
	visitor(game.m_iInitPopulation);
	visitor(game.m_iInitLand);
	visitor(game.m_iInitTech);
	visitor(game.m_iInitWonders);
	visitor(game.m_iAIAutoPlay);

	visitor(game.m_iUnitedNationsCountdown);
	visitor(game.m_iNumVictoryVotesTallied);
	visitor(game.m_iNumVictoryVotesExpected);
	visitor(game.m_iVotesNeededForDiploVictory);
	visitor(game.m_iMapScoreMod);
	visitor(game.m_iCityFoundValueReference);
	visitor(game.m_iNumReferenceCities);
	visitor(game.m_iNumMajorCivsAliveAtGameStart);
	visitor(game.m_iNumMinorCivsAliveAtGameStart);

	// m_uiInitialTime not saved

	visitor(game.m_bScoreDirty);
	visitor(game.m_bCircumnavigated);
	// m_bDebugMode not saved
	visitor(game.m_bFinalInitialized);
	// m_bPbemTurnSent not saved
	visitor(game.m_bHotPbemBetweenTurns);
	// m_bPlayerOptionsSent not saved
	visitor(game.m_bNukesValid);
	visitor(game.m_bEndGameTechResearched);
	visitor(game.m_eObserverUIOverridePlayer);
	visitor(game.m_bTunerEverConnected);
	visitor(game.m_bSavedOnce);
	visitor(game.m_bTutorialEverAttacked);
	visitor(game.m_bStaticTutorialActive);
	visitor(game.m_bEverRightClickMoved);
	visitor(game.m_AdvisorMessagesViewed);
	visitor(game.m_eHandicap);
	visitor(game.m_ePausePlayer);
	visitor(game.m_eAIAutoPlayReturnPlayer);
	visitor(game.m_eBestLandUnit);
	visitor(game.m_eWinner);
	visitor(game.m_eVictory);
	visitor(game.m_eGameState);
	if (bLoading && game.m_eGameState == GAMESTATE_OVER)
	{
		visitor.loadAssign(game.m_eGameState, GAMESTATE_EXTENDED);
	}
	visitor(game.m_eBestWondersPlayer);
	visitor(game.m_eBestPoliciesPlayer);
	visitor(game.m_eBestGreatPeoplePlayer);
	visitor(game.m_eIndustrialRoute);
	visitor(game.m_eGameEra);

	visitor(game.m_eTeamThatCircumnavigated);
	visitor(game.m_bVictoryRandomization);

	visitor(game.m_iMedianTechsResearched);
	visitor(game.m_iBasicNeedsMedian);
	visitor(game.m_iGoldMedian);
	visitor(game.m_iScienceMedian);
	visitor(game.m_iCultureMedian);

	visitor(game.m_iSpyThreshold);
	visitor(game.m_iLastTurnCSSurrendered);

	visitor(game.m_aiGreatestMonopolyPlayer);

	visitor(game.m_strScriptData);

	visitor(game.m_aiEndTurnMessagesReceived);
	visitor(game.m_aiRankPlayer);
	visitor(game.m_aiPlayerRank);
	visitor(game.m_aiPlayerScore);
	visitor(game.m_aiRankTeam);
	visitor(game.m_aiTeamRank);
	visitor(game.m_aiTeamScore);

	visitor(game.m_paiUnitCreatedCount);
	visitor(game.m_paiUnitClassCreatedCount);
	visitor(game.m_paiBuildingClassCreatedCount);

	visitor(game.m_paiProjectCreatedCount);

	visitor(game.m_aiVotesCast);
	visitor(game.m_aiPreviousVotesCast);
	visitor(game.m_aiNumVotesForTeam);

	visitor(game.m_pabSpecialUnitValid);
	for (std::size_t i = 0; i < game.m_ppaaiTeamVictoryRank.size(); ++i)
	{
		for (int j = 0; j < /*5*/ GD_INT_GET(NUM_VICTORY_POINT_AWARDS); ++j)
		{
			visitor(game.m_ppaaiTeamVictoryRank[i][j]);
		}
	}
	visitor(game.m_ppaiContractUnits);

	visitor(game.m_aszDestroyedCities);
	visitor(game.m_aszGreatPeopleBorn);

	visitor(game.m_mapRand);
	visitor(game.m_jonRand);

	visitor(game.m_listReplayMessages);

	visitor(game.m_iNumSessions);

	visitor(game.m_aPlotExtraYields);
	visitor(game.m_aPlotExtraCosts);

	visitor(game.m_bArchaeologyTriggered);

	visitor(game.m_iEarliestBarbarianReleaseTurn);

	visitor(game.m_kGameDeals);
	visitor(*game.m_pGameReligions);
	visitor(*game.m_pGameCulture);
	visitor(*game.m_pGameLeagues);
	visitor(*game.m_pGameTrade);

	visitor(*game.m_pGameCorporations);
	visitor(*game.m_pGameContracts);
}

//	--------------------------------------------------------------------------------
void CvGame::Read(FDataStream& kStream)
{
	int iI = 0;

	reset(NO_HANDICAP);

	// Save header information
	{
		uint32 saveVersion = 0;
		kStream >> saveVersion;
		GC.setSaveVersion(saveVersion);
		
		const CvGlobals::GameDataHash& gameDataHash = GC.getGameDataHash();
		CvGlobals::GameDataHash saveDataHash;
		kStream >> saveDataHash;

		if (saveDataHash != gameDataHash)
		{
			CUSTOMLOG(
				"WARNING - Save data hash mismatch!\n"
				"\tLoad will be attempted but corruption or crash is likely.\n"
				"\tSave Hash = [%#010x-%#010x-%#010x-%#010x]\n"
				"\tGame Hash = [%#010x-%#010x-%#010x-%#010x]",
				saveDataHash[0], saveDataHash[1], saveDataHash[2], saveDataHash[3],
				gameDataHash[0], gameDataHash[1], gameDataHash[2], gameDataHash[3]
			);
		}

		//check the commit id that was used when generating this file
		CvString save_gamecore_version;
		kStream >> save_gamecore_version;

		CUSTOMLOG("Savefile was generated from gamecore version %s", save_gamecore_version.c_str());
		if (strcmp(save_gamecore_version.c_str(), CURRENT_GAMECORE_VERSION)!=0)
			CUSTOMLOG("----> Potential savefile format mismatch, gamecore is version %s!", CURRENT_GAMECORE_VERSION);
	}

	CvStreamLoadVisitor serialVisitor(kStream);
	Serialize(*this, serialVisitor);

	// Save game database comes last
	readSaveGameDB(kStream);

	if(!isNetworkMultiPlayer())
	{
		++m_iNumSessions;
	}

	// Get the active player information from the initialization structure
	if(!isGameMultiPlayer())
	{
		for(iI = 0; iI < MAX_CIV_PLAYERS; iI++)
		{
			if(GET_PLAYER((PlayerTypes)iI).isHuman())
			{
				setActivePlayer((PlayerTypes)iI);
				break;
			}
		}
		Localization::String localizedText = Localization::Lookup("TXT_KEY_MISC_RELOAD");
		localizedText << m_iNumSessions;
		addReplayMessage(REPLAY_MESSAGE_MAJOR_EVENT, getActivePlayer(), localizedText.toUTF8());
	}

	if(isOption(GAMEOPTION_NEW_RANDOM_SEED))
	{
		if(!isNetworkMultiPlayer())
		{
			m_jonRand.reseed(timeGetTime());
		}
	}

#if defined(MOD_BALANCE_CORE_RESOURCE_MONOPOLIES)
	if (MOD_BALANCE_CORE_RESOURCE_MONOPOLIES)
	{
		for (int iResourceLoop = 0; iResourceLoop < GC.getNumResourceInfos(); iResourceLoop++)
		{
			for (int iPlayerLoop = 0; iPlayerLoop < MAX_MAJOR_CIVS; iPlayerLoop++)
			{
				PlayerTypes eLoopPlayer = (PlayerTypes)iPlayerLoop;
				if (eLoopPlayer == NO_PLAYER || eLoopPlayer > MAX_MAJOR_CIVS)
					continue;

				if (GET_PLAYER(eLoopPlayer).isAlive())
					GET_PLAYER(eLoopPlayer).CheckForMonopoly((ResourceTypes)iResourceLoop);
			}
		}
	}
#endif

	//when loading from file, we need to reset m_lastTurnAICivsProcessed 
	//so that updateMoves() can turn active players after loading an autosave in simultaneous turns multiplayer.
	m_lastTurnAICivsProcessed = -1;

	// used to be a static in updateMoves but made a member due to it not being re-inited and maybe causing issues when loading after an exit-to-menu. Not serialized - I wasn't willing to think about the implications.
	m_processPlayerAutoMoves = false;
}

//	--------------------------------------------------------------------------------
void CvGame::Write(FDataStream& kStream) const
{
#ifdef EA_EVENT_GAME_SAVE // Paz - This will fire before Civ5SavedGameDatabase.db serialization into the gamesave file
	if (m_bSavedOnce && EA_EVENT_GAME_SAVE) // But... running gDLL->GetScriptSystem on initial save causes a game hang, so skip first save
	{
		GAMEEVENTINVOKE_HOOK(GAMEEVENT_GameSave);
	}
#endif
	// Save header information
	{
		GC.setSaveVersion(CvGlobals::SAVE_VERSION_LATEST);
		kStream << GC.getSaveVersion();
		kStream << GC.getGameDataHash();
		kStream << CvString(CURRENT_GAMECORE_VERSION); //cannot store naked char*
	}

	CvStreamSaveVisitor serialVisitor(kStream);
	Serialize(*this, serialVisitor);

	// Save game database comes last
	writeSaveGameDB(kStream);
}

//	--------------------------------------------------------------------------------
void CvGame::readSaveGameDB(FDataStream& kStream)
{
	unsigned int lSize = 0;
	kStream >> lSize;
	if (lSize > 0)
	{
		//Deserialize the embedded SQLite database file.
		CvString strUTF8DatabasePath = gDLL->GetCacheFolderPath();
		strUTF8DatabasePath += "Civ5SavedGameDatabase.db";

		// Need to Convert the UTF-8 string into a wide character string.
		std::wstring wstrDatabasePath = CvStringUtils::FromUTF8ToUTF16(strUTF8DatabasePath);

		FIFile* pkFile = FFILESYSTEM.Create(wstrDatabasePath.c_str(), FIFile::modeWrite);
		if (pkFile != NULL)
		{
			byte* szBuffer = new byte[sizeof(char) * lSize];
			ZeroMemory((void*)szBuffer, lSize);

			kStream.ReadIt(lSize, szBuffer);

			pkFile->Write(szBuffer, lSize);

			pkFile->Close();

			delete[] szBuffer;
		}
		else
		{
			ASSERT_DEBUG(false, "Cannot open Civ5SavedGameDatabase.db for write! Does something have this opened?");
		}
	}
}

//	--------------------------------------------------------------------------------
void CvGame::writeSaveGameDB(FDataStream& kStream) const
{
	CvString strPath = gDLL->GetCacheFolderPath();
	strPath += "Civ5SavedGameDatabase.db";

	//Need to Convert the UTF-8 string into a wide character string so windows can open this file.
	wchar_t savePath[MAX_PATH] = { 0 };
	MultiByteToWideChar(CP_UTF8, 0, strPath.c_str(), -1, savePath, MAX_PATH);

	HANDLE hFile = CreateFileW(savePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwSize = GetFileSize(hFile, NULL);
		if (dwSize != INVALID_FILE_SIZE)
		{
			byte* szBuffer = new byte[sizeof(char) * dwSize];
			ZeroMemory((void*)szBuffer, dwSize);

			DWORD dwBytesRead = 0;
			if (ReadFile(hFile, szBuffer, dwSize, &dwBytesRead, NULL) == TRUE)
			{
				//Serialize out the file size first.
				kStream << dwBytesRead;
				kStream.WriteIt(dwBytesRead, szBuffer);
			}

			delete[] szBuffer;
		}
		else
		{
			ASSERT_DEBUG(false, "Saved game database exists, but could not get file size???");
		}

		if (CloseHandle(hFile) == FALSE)
		{
			ASSERT_DEBUG(false, "Could not close file handle to saved game database!");
		}
	}
	else
	{
		WIN32_FILE_ATTRIBUTE_DATA fileAttributes;
		if (GetFileAttributesExW(savePath, GetFileExInfoStandard, &fileAttributes) != static_cast<BOOL>(INVALID_FILE_ATTRIBUTES))
		{
			ASSERT_DEBUG(false, "Saved game database exists, but could not open it!");
		}

		DWORD nilSize = 0;
		kStream << nilSize;
	}
}

//	---------------------------------------------------------------------------
void CvGame::ReadSupportingClassData(FDataStream& kStream)
{
	CvBarbarians::Read(kStream);
	CvGoodyHuts::Read(kStream);
}

//	---------------------------------------------------------------------------
void CvGame::WriteSupportingClassData(FDataStream& kStream) const
{
	CvBarbarians::Write(kStream);
	CvGoodyHuts::Write(kStream);
}

//	--------------------------------------------------------------------------------
void CvGame::writeReplay(FDataStream& kStream)
{
	CvReplayInfo replayInfo;
	replayInfo.createInfo();
	replayInfo.write(kStream);
}

//	-----------------------------------------------------------------------------------------------
void CvGame::saveReplay()
{
	gDLL->saveReplay();

	CvPlayerAI& activePlayer = GET_PLAYER(getActivePlayer());

	bool playerTeamWon = (getActiveTeam() == getWinner());
	int score = activePlayer.GetScore(true, playerTeamWon);

	if(!isHotSeat())
	{
		gDLL->RecordVictoryInformation(score);
		gDLL->RecordLeaderboardScore(score);
	}
}
//	-----------------------------------------------------------------------------------------------

void CvGame::showEndGameSequence()
{

	GC.GetEngineUserInterface()->OpenEndGameMenu();
}

//	--------------------------------------------------------------------------------
void CvGame::addPlayer(PlayerTypes eNewPlayer, LeaderHeadTypes eLeader, CivilizationTypes eCiv)
{
	CvCivilizationInfo* pkCivilizationInfo = GC.getCivilizationInfo(eCiv);
	CvCivilizationInfo* pkBarbarianCivInfo = GC.getCivilizationInfo(static_cast<CivilizationTypes>(GD_INT_GET(BARBARIAN_CIVILIZATION)));

	if(pkCivilizationInfo == NULL || pkBarbarianCivInfo == NULL)
	{
		//Should never happen.
		ASSERT_DEBUG(false);
		return;
	}

	PlayerColorTypes eColor = (PlayerColorTypes)pkCivilizationInfo->getDefaultPlayerColor();

	for(int iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if(eColor == NO_PLAYERCOLOR || GET_PLAYER((PlayerTypes)iI).getPlayerColor() == eColor)
		{
			for(int iK = 0; iK < GC.GetNumPlayerColorInfos(); iK++)
			{
				const PlayerColorTypes ePlayerColor = static_cast<PlayerColorTypes>(iK);
				CvPlayerColorInfo* pkPlayerColorInfo = GC.GetPlayerColorInfo(ePlayerColor);
				if(pkPlayerColorInfo)
				{
					if(iK != pkBarbarianCivInfo->getDefaultPlayerColor())
					{
						bool bValid = true;

						for(int iL = 0; iL < MAX_CIV_PLAYERS; iL++)
						{
							if(GET_PLAYER((PlayerTypes)iL).getPlayerColor() == ePlayerColor)
							{
								bValid = false;
								break;
							}
						}

						if(bValid)
						{
							eColor = ePlayerColor;
							iI = MAX_CIV_PLAYERS;
							break;
						}
					}
				}
			}
		}
	}

	CvPreGame::setLeaderHead(eNewPlayer, eLeader);
	CvPreGame::setCivilization(eNewPlayer, eCiv);
	CvPreGame::setSlotStatus(eNewPlayer, SS_COMPUTER);
	CvPreGame::setPlayerColor(eNewPlayer, eColor);
	GET_PLAYER(eNewPlayer).init(eNewPlayer);
}

//	--------------------------------------------------------------------------------
int CvGame::getPlotExtraYield(int iX, int iY, YieldTypes eYield) const
{
	for(std::vector<PlotExtraYield>::const_iterator it = m_aPlotExtraYields.begin(); it != m_aPlotExtraYields.end(); ++it)
	{
		if((*it).m_iX == iX && (*it).m_iY == iY)
		{
			return (*it).m_aeExtraYield[eYield];
		}
	}

	return 0;
}

//	--------------------------------------------------------------------------------
void CvGame::setPlotExtraYield(int iX, int iY, YieldTypes eYield, int iExtraYield)
{
	bool bFound = false;

	for(std::vector<PlotExtraYield>::iterator it = m_aPlotExtraYields.begin(); it != m_aPlotExtraYields.end(); ++it)
	{
		if((*it).m_iX == iX && (*it).m_iY == iY)
		{
			(*it).m_aeExtraYield[eYield] += iExtraYield;
			bFound = true;
			break;
		}
	}

	if(!bFound)
	{
		PlotExtraYield kExtraYield;
		kExtraYield.m_iX = iX;
		kExtraYield.m_iY = iY;
		for(int i = 0; i < NUM_YIELD_TYPES; ++i)
		{
			if(eYield == i)
			{
				kExtraYield.m_aeExtraYield.push_back(iExtraYield);
			}
			else
			{
				kExtraYield.m_aeExtraYield.push_back(0);
			}
		}
		m_aPlotExtraYields.push_back(kExtraYield);
	}

	CvPlot* pPlot = GC.getMap().plot(iX, iY);
	if(NULL != pPlot)
	{
		pPlot->updateYield();
	}
}

//	--------------------------------------------------------------------------------
void CvGame::removePlotExtraYield(int iX, int iY)
{
	for(std::vector<PlotExtraYield>::iterator it = m_aPlotExtraYields.begin(); it != m_aPlotExtraYields.end(); ++it)
	{
		if((*it).m_iX == iX && (*it).m_iY == iY)
		{
			m_aPlotExtraYields.erase(it);
			break;
		}
	}

	CvPlot* pPlot = GC.getMap().plot(iX, iY);
	if(NULL != pPlot)
	{
		pPlot->updateYield();
	}
}

//	--------------------------------------------------------------------------------
int CvGame::getPlotExtraCost(int iX, int iY) const
{
	for(std::vector<PlotExtraCost>::const_iterator it = m_aPlotExtraCosts.begin(); it != m_aPlotExtraCosts.end(); ++it)
	{
		if((*it).m_iX == iX && (*it).m_iY == iY)
		{
			return (*it).m_iCost;
		}
	}

	return 0;
}

//	--------------------------------------------------------------------------------
void CvGame::changePlotExtraCost(int iX, int iY, int iCost)
{
	bool bFound = false;

	for(std::vector<PlotExtraCost>::iterator it = m_aPlotExtraCosts.begin(); it != m_aPlotExtraCosts.end(); ++it)
	{
		if((*it).m_iX == iX && (*it).m_iY == iY)
		{
			(*it).m_iCost += iCost;
			bFound = true;
			break;
		}
	}

	if(!bFound)
	{
		PlotExtraCost kExtraCost;
		kExtraCost.m_iX = iX;
		kExtraCost.m_iY = iY;
		kExtraCost.m_iCost = iCost;
		m_aPlotExtraCosts.push_back(kExtraCost);
	}
}

//	--------------------------------------------------------------------------------
void CvGame::removePlotExtraCost(int iX, int iY)
{
	for(std::vector<PlotExtraCost>::iterator it = m_aPlotExtraCosts.begin(); it != m_aPlotExtraCosts.end(); ++it)
	{
		if((*it).m_iX == iX && (*it).m_iY == iY)
		{
			m_aPlotExtraCosts.erase(it);
			break;
		}
	}
}


// CACHE: cache frequently used values
///////////////////////////////////////

//	--------------------------------------------------------------------------------
void CvGame::doUpdateCacheOnTurn()
{
	int iNumPlotsInEntireWorld = GC.getMap().numPlots();
	for(int iI = 0; iI < iNumPlotsInEntireWorld; iI++)
	{
		CvPlot* pLoopPlot = GC.getMap().plotByIndexUnchecked(iI);
		pLoopPlot->updateWaterFlags();
	}
}

//	--------------------------------------------------------------------------------
bool CvGame::isCivEverActive(CivilizationTypes eCivilization) const
{
	for(int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
	{
		CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);
		if(kLoopPlayer.isEverAlive())
		{
			if(kLoopPlayer.getCivilizationType() == eCivilization)
			{
				return true;
			}
		}
	}

	return false;
}

//	--------------------------------------------------------------------------------
bool CvGame::isLeaderEverActive(LeaderHeadTypes eLeader) const
{
	for(int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
	{
		CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);
		if(kLoopPlayer.isEverAlive())
		{
			if(kLoopPlayer.getLeaderType() == eLeader)
			{
				return true;
			}
		}
	}

	return false;
}

//	--------------------------------------------------------------------------------
bool CvGame::isUnitEverActive(UnitTypes eUnit) const
{
	CvUnitEntry* pkUnitInfo = GC.getUnitInfo(eUnit);
	if(pkUnitInfo == NULL)
		return false;

	for(int iCiv = 0; iCiv < GC.getNumCivilizationInfos(); ++iCiv)
	{
		const CivilizationTypes eCiv = static_cast<CivilizationTypes>(iCiv);
		CvCivilizationInfo* pkCivilizationInfo = GC.getCivilizationInfo(eCiv);
		if(pkCivilizationInfo)
		{
			if(isCivEverActive(eCiv))
			{
				if(eUnit == pkCivilizationInfo->getCivilizationUnits(pkUnitInfo->GetUnitClassType()))
				{
					return true;
				}
			}
		}
	}

	return false;
}

//	--------------------------------------------------------------------------------
bool CvGame::isBuildingEverActive(BuildingTypes eBuilding) const
{
	CvBuildingEntry* pkBuildingInfo = GC.getBuildingInfo(eBuilding);
	if(pkBuildingInfo)
	{
		for(int iCiv = 0; iCiv < GC.getNumCivilizationInfos(); ++iCiv)
		{
			const CivilizationTypes eCivilization = static_cast<CivilizationTypes>(iCiv);
			CvCivilizationInfo* pkCivilizationInfo = GC.getCivilizationInfo(eCivilization);
			if(pkCivilizationInfo)
			{
				if(isCivEverActive(eCivilization))
				{
					if(eBuilding == pkCivilizationInfo->getCivilizationBuildings(pkBuildingInfo->GetBuildingClassType()))
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

//	--------------------------------------------------------------------------------
/// What route type forms an industrial connection?
RouteTypes CvGame::GetIndustrialRoute() const
{
	return m_eIndustrialRoute;
}

//	--------------------------------------------------------------------------------
/// What route type forms an industrial connection?
void CvGame::DoUpdateIndustrialRoute()
{
	RouteTypes eIndustrialRoute = NO_ROUTE;

	for(int iRouteLoop = 0; iRouteLoop < GC.getNumRouteInfos(); iRouteLoop++)
	{
		const RouteTypes eRoute = static_cast<RouteTypes>(iRouteLoop);
		CvRouteInfo* pkRouteInfo = GC.getRouteInfo(eRoute);
		if(pkRouteInfo)
		{
			if(pkRouteInfo->IsIndustrial())
			{
				eIndustrialRoute = eRoute;
				break;
			}
		}
	}

	m_eIndustrialRoute = eIndustrialRoute;
}

//	--------------------------------------------------------------------------------
TeamTypes CvGame::GetTeamThatCircumnavigated() const
{
	return (TeamTypes) m_eTeamThatCircumnavigated;
}

//	--------------------------------------------------------------------------------
void CvGame::SetTeamThatCircumnavigated(TeamTypes eNewValue)
{
	m_eTeamThatCircumnavigated = eNewValue;
}

//	--------------------------------------------------------------------------------
CvSiteEvaluatorForSettler* CvGame::GetSettlerSiteEvaluator()
{
	return m_pSettlerSiteEvaluator;
}

//	--------------------------------------------------------------------------------
CvCitySiteEvaluator* CvGame::GetStartSiteEvaluator()
{
	return m_pStartSiteEvaluator;
}

//	--------------------------------------------------------------------------------
IStartPositioner* CvGame::GetStartPositioner()
{
	return m_pStartPositioner;
}

//	--------------------------------------------------------------------------------
CvGameDeals& CvGame::GetGameDeals()
{
	return m_kGameDeals;
}

//	--------------------------------------------------------------------------------
CvGameReligions* CvGame::GetGameReligions()
{
	return m_pGameReligions;
}

#if defined(MOD_BALANCE_CORE)
//	--------------------------------------------------------------------------------
CvGameCorporations* CvGame::GetGameCorporations()
{
	return m_pGameCorporations;
}
//	--------------------------------------------------------------------------------
CvGameContracts* CvGame::GetGameContracts()
{
	return m_pGameContracts;
}
#endif

//	--------------------------------------------------------------------------------
CvGameCulture* CvGame::GetGameCulture()
{
	return m_pGameCulture;
}

//	--------------------------------------------------------------------------------
CvGameLeagues* CvGame::GetGameLeagues()
{
	return m_pGameLeagues;
}

//	--------------------------------------------------------------------------------
CvGameTrade* CvGame::GetGameTrade()
{
	return m_pGameTrade;
}

//	--------------------------------------------------------------------------------
CvString CvGame::getDllGuid() const
{
	CvString szDllGuid = "";
	
	GUID guid = CvDllGameContext::GetSingleton()->GetDLLGUID();
	unsigned long d1 = guid.Data1;
	unsigned short d2 = guid.Data2;
	unsigned short d3 = guid.Data3;
	unsigned char* d4 = guid.Data4;

	CvString::format(szDllGuid, "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X", d1, d2, d3, d4[0], d4[1], d4[2], d4[3], d4[4], d4[5], d4[6], d4[7]);

	return szDllGuid;
}

//	--------------------------------------------------------------------------------
CvAdvisorCounsel* CvGame::GetAdvisorCounsel()
{
	return m_pAdvisorCounsel;
}

//	--------------------------------------------------------------------------------
CvAdvisorRecommender* CvGame::GetAdvisorRecommender()
{
	return m_pAdvisorRecommender;
}

//	--------------------------------------------------------------------------------
int CvGame::GetTurnsBetweenMinorCivElections()
{
	int iTurnsBetweenCityStateElections = /*15*/ GD_INT_GET(ESPIONAGE_TURNS_BETWEEN_CITY_STATE_ELECTIONS);
	iTurnsBetweenCityStateElections *= GC.getGame().getGameSpeedInfo().getMinorCivElectionFreqMod();
	iTurnsBetweenCityStateElections /= 100;
	return iTurnsBetweenCityStateElections;
}

//	--------------------------------------------------------------------------------
int CvGame::GetTurnsUntilMinorCivElection()
{
	int iMod = 0;
	int iTurnsBetweenCityStateElections = GetTurnsBetweenMinorCivElections();

	if(iTurnsBetweenCityStateElections != 0)
	{
		iMod = getGameTurn() % iTurnsBetweenCityStateElections;
	}
	if(iMod == 0)
	{
		return 0;
	}
	else
	{
		return iTurnsBetweenCityStateElections - iMod;
	}
}

//------------------------------------------------------------------------------
/// Get the action associated with the supplied key.
/// This will also test to see if the action can actually be done.
/// Returns: the action info index or -1.
int CvGame::GetAction(int iKeyStroke, bool bAlt, bool bShift, bool bCtrl)
{
	int i = 0;
	int iActionIndex = -1;
	int iPriority = -1;


	for(i=0; i<GC.getNumActionInfos(); i++)
	{
		CvActionInfo& thisActionInfo = *GC.getActionInfo(i);
		if((((thisActionInfo.getHotKeyVal() == iKeyStroke) &&
		        (thisActionInfo.getHotKeyPriority() > iPriority) &&
		        (bAlt == thisActionInfo.isAltDown()) &&
		        (bShift == thisActionInfo.isShiftDown()) &&
		        (bCtrl == thisActionInfo.isCtrlDown())
		    )
		        ||
		        ((thisActionInfo.getHotKeyValAlt() == iKeyStroke) &&
		         (thisActionInfo.getHotKeyPriorityAlt() > iPriority) &&
		         (bAlt == thisActionInfo.isAltDownAlt()) &&
		         (bShift == thisActionInfo.isShiftDownAlt()) &&
		         (bCtrl == thisActionInfo.isCtrlDownAlt())
		        )
		   )
		        &&
		        (canHandleAction(i))
		  )
		{
			iPriority = thisActionInfo.getHotKeyPriority();
			iActionIndex = i;
		}
	}

	return iActionIndex;
}

//------------------------------------------------------------------------------
/// Get the action associated with the supplied key.
/// This will NOT test to see if the action can actually be done.
/// Returns: the action info index or -1.
int CvGame::IsAction(int iKeyStroke, bool bAlt, bool bShift, bool bCtrl)
{
	int i = 0;
	int iActionIndex = -1;
	int iPriority = -1;


	for(i=0; i<GC.getNumActionInfos(); i++)
	{
		CvActionInfo& thisActionInfo = *GC.getActionInfo(i);
		if((((thisActionInfo.getHotKeyVal() == iKeyStroke) &&
			(thisActionInfo.getHotKeyPriority() > iPriority) &&
			(bAlt == thisActionInfo.isAltDown()) &&
			(bShift == thisActionInfo.isShiftDown()) &&
			(bCtrl == thisActionInfo.isCtrlDown())
			)
			||
			((thisActionInfo.getHotKeyValAlt() == iKeyStroke) &&
			(thisActionInfo.getHotKeyPriorityAlt() > iPriority) &&
			(bAlt == thisActionInfo.isAltDownAlt()) &&
			(bShift == thisActionInfo.isShiftDownAlt()) &&
			(bCtrl == thisActionInfo.isCtrlDownAlt())
			)
			)			
			)
		{
			iPriority = thisActionInfo.getHotKeyPriority();
			iActionIndex = i;
		}
	}

	return iActionIndex;
}

//------------------------------------------------------------------------------
void CvGame::endTurnTimerSemaphoreIncrement()
{
	++m_endTurnTimerSemaphore;
}

//	--------------------------------------------------------------------------------
void CvGame::endTurnTimerSemaphoreDecrement()
{
	--m_endTurnTimerSemaphore;
	if(m_endTurnTimerSemaphore <= 0)
	{
		m_endTurnTimerSemaphore = 0;
		m_endTurnTimer.Start();
	}
}

//	--------------------------------------------------------------------------------
void CvGame::endTurnTimerReset()
{
	m_endTurnTimerSemaphore = 0;
	m_endTurnTimer.Start();
}

//	--------------------------------------------------------------------------------
/// Called when a major changes its protection status towards a minor
void CvGame::DoMinorPledgeProtection(PlayerTypes eMajor, PlayerTypes eMinor, bool bProtect, bool bPledgeNowBroken)
{
	ASSERT_DEBUG(eMajor >= 0, "eMajor is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(eMajor < MAX_MAJOR_CIVS, "eMajor is expected to be within maximum bounds (invalid Index)");
	ASSERT_DEBUG(eMinor >= MAX_MAJOR_CIVS, "eMinor is not in expected range (invalid Index)");
	ASSERT_DEBUG(eMinor < MAX_CIV_PLAYERS, "eMinor is not in expected range (invalid Index)");

	if (bProtect)
	{
		ASSERT_DEBUG(GET_PLAYER(eMinor).GetMinorCivAI()->CanMajorProtect(eMajor, false), "eMajor is not allowed to protect this minor!");
	}

	gDLL->sendMinorPledgeProtection(eMajor, eMinor, bProtect, bPledgeNowBroken);
}

//	--------------------------------------------------------------------------------
/// Amount of Gold being gifted to the Minor by the active player
void CvGame::DoMinorGiftGold(PlayerTypes eMinor, int iNumGold)
{
	ASSERT_DEBUG(eMinor >= MAX_MAJOR_CIVS, "eMinor is not in expected range (invalid Index)");
	ASSERT_DEBUG(eMinor < MAX_CIV_PLAYERS, "eMinor is not in expected range (invalid Index)");

	gDLL->sendMinorGiftGold(eMinor, iNumGold);
}

//	--------------------------------------------------------------------------------
/// Do the action of a major gifting a tile improvement to a minor's plot, to improve its resource
void CvGame::DoMinorGiftTileImprovement(PlayerTypes eMajor, PlayerTypes eMinor, int iPlotX, int iPlotY)
{
	ASSERT_DEBUG(eMajor >= 0, "eMajor is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(eMajor < MAX_MAJOR_CIVS, "eMajor is expected to be within maximum bounds (invalid Index)");
	ASSERT_DEBUG(eMinor >= MAX_MAJOR_CIVS, "eMinor is not in expected range (invalid Index)");
	ASSERT_DEBUG(eMinor < MAX_CIV_PLAYERS, "eMinor is not in expected range (invalid Index)");

	gDLL->sendMinorGiftTileImprovement(eMajor, eMinor, iPlotX, iPlotY);
}

//	--------------------------------------------------------------------------------
/// Do the action of a major bullying gold from a minor
/// Demanded gold and a calculated bully metric are not provided (ex. from Lua, Player UI), so calculate them here
void CvGame::DoMinorBullyGold(PlayerTypes eBully, PlayerTypes eMinor)
{
	ASSERT_DEBUG(eBully >= 0, "eBully is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(eBully < MAX_MAJOR_CIVS, "eBully is expected to be within maximum bounds (invalid Index)");
	ASSERT_DEBUG(eMinor >= MAX_MAJOR_CIVS, "eMinor is not in expected range (invalid Index)");
	ASSERT_DEBUG(eMinor < MAX_CIV_PLAYERS, "eMinor is not in expected range (invalid Index)");

	int iGold = GET_PLAYER(eMinor).GetMinorCivAI()->GetBullyGoldAmount(eBully);
	if (iGold <= 0)
		return;

	gDLL->sendMinorBullyGold(eBully, eMinor, iGold);
}

//	--------------------------------------------------------------------------------
/// Do the action of a major bullying a unit from a minor
void CvGame::DoMinorBullyUnit(PlayerTypes eBully, PlayerTypes eMinor)
{
	ASSERT_DEBUG(eBully >= 0, "eBully is expected to be non-negative (invalid Index)");
	ASSERT_DEBUG(eBully < MAX_MAJOR_CIVS, "eBully is expected to be within maximum bounds (invalid Index)");
	ASSERT_DEBUG(eMinor >= MAX_MAJOR_CIVS, "eMinor is not in expected range (invalid Index)");
	ASSERT_DEBUG(eMinor < MAX_CIV_PLAYERS, "eMinor is not in expected range (invalid Index)");

	UnitClassTypes eUnitClassType = GET_PLAYER(eMinor).GetMinorCivAI()->GetBullyUnit();
	UnitTypes eUnitType = GET_PLAYER(eBully).GetSpecificUnitType(eUnitClassType);
	if (eUnitType != NO_UNIT)
	{
		gDLL->sendMinorBullyUnit(eBully, eMinor, eUnitType);
	}
}

//	--------------------------------------------------------------------------------
/// Do the action of a major buying out a minor and acquiring it
void CvGame::DoMinorBuyout(PlayerTypes eMajor, PlayerTypes eMinor)
{
	if (eMajor < 0 || eMajor >= MAX_MAJOR_CIVS) return;
	if (eMinor < MAX_MAJOR_CIVS || eMinor >= MAX_CIV_PLAYERS) return;

	gDLL->sendMinorBuyout(eMajor, eMinor);
}
//	--------------------------------------------------------------------------------
/// Do the action of a major annexing a minor using tribute (Rome UA)
void CvGame::DoMinorBullyAnnex(PlayerTypes eMajor, PlayerTypes eMinor)
{
	if (eMajor < 0 || eMajor >= MAX_MAJOR_CIVS) return;
	if (eMinor < MAX_MAJOR_CIVS || eMinor >= MAX_CIV_PLAYERS) return;

	NetMessageExt::Send::DoMinorBullyAnnex(eMajor, eMinor);
}
//	--------------------------------------------------------------------------------
/// Do the action of a major buying out a minor and marrying it
void CvGame::DoMinorMarriage(PlayerTypes eMajor, PlayerTypes eMinor)
{
	if (eMajor < 0 || eMajor >= MAX_MAJOR_CIVS) return;
	if (eMinor < MAX_MAJOR_CIVS || eMinor >= MAX_CIV_PLAYERS) return;

	gDLL->sendMinorBuyout(eMajor, eMinor);
}

//	--------------------------------------------------------------------------------
/// Notification letting all non-party players know that two players made a Defensive Pact.
void CvGame::DoDefensivePactNotification(PlayerTypes eFirstPlayer, PlayerTypes eSecondPlayer)
{
	for (int iPlayerLoop = 0; iPlayerLoop < MAX_PLAYERS; iPlayerLoop++)
	{
		PlayerTypes eLoopPlayer = (PlayerTypes) iPlayerLoop;
		if (eLoopPlayer == eFirstPlayer || eLoopPlayer == eSecondPlayer)
			continue;

		if (GET_PLAYER(eLoopPlayer).isObserver() || (GET_PLAYER(eLoopPlayer).isHuman() && GET_PLAYER(eLoopPlayer).isAlive()))
		{
			if (!GET_PLAYER(eLoopPlayer).isObserver())
			{
				if (!GET_TEAM(GET_PLAYER(eLoopPlayer).getTeam()).isHasMet(GET_PLAYER(eFirstPlayer).getTeam()))
					continue;

				if (!GET_TEAM(GET_PLAYER(eLoopPlayer).getTeam()).isHasMet(GET_PLAYER(eSecondPlayer).getTeam()))
					continue;
			}

			CvNotifications* pNotify = GET_PLAYER(eLoopPlayer).GetNotifications();
			if (pNotify)
			{
				Localization::String strText = Localization::Lookup("TXT_KEY_NOTIFICATION_DEFENSIVE_PACT");
				Localization::String strSummary = Localization::Lookup("TXT_KEY_NOTIFICATION_DEFENSIVE_PACT_S");
				strText << GET_PLAYER(eFirstPlayer).getCivilizationShortDescriptionKey();
				strText << GET_PLAYER(eSecondPlayer).getCivilizationShortDescriptionKey();
				pNotify->Add(NOTIFICATION_DIPLOMACY_DECLARATION, strText.toUTF8(), strSummary.toUTF8(), -1, -1, -1);
			}
		}
	}
}

//	--------------------------------------------------------------------------------
/// Notification letting all non-party players know that two players made a Research Agreement.
void CvGame::DoResearchAgreementNotification(PlayerTypes eFirstPlayer, PlayerTypes eSecondPlayer)
{
	for (int iPlayerLoop = 0; iPlayerLoop < MAX_PLAYERS; iPlayerLoop++)
	{
		PlayerTypes eLoopPlayer = (PlayerTypes) iPlayerLoop;
		if (eLoopPlayer == eFirstPlayer || eLoopPlayer == eSecondPlayer)
			continue;

		if (GET_PLAYER(eLoopPlayer).isObserver() || (GET_PLAYER(eLoopPlayer).isHuman() && GET_PLAYER(eLoopPlayer).isAlive()))
		{
			if (!GET_PLAYER(eLoopPlayer).isObserver())
			{
				if (!GET_TEAM(GET_PLAYER(eLoopPlayer).getTeam()).isHasMet(GET_PLAYER(eFirstPlayer).getTeam()))
					continue;

				if (!GET_TEAM(GET_PLAYER(eLoopPlayer).getTeam()).isHasMet(GET_PLAYER(eSecondPlayer).getTeam()))
					continue;
			}

			CvNotifications* pNotify = GET_PLAYER(eLoopPlayer).GetNotifications();
			if (pNotify)
			{
				Localization::String strText = Localization::Lookup("TXT_KEY_NOTIFICATION_RESEARCH_AGREEMENT");
				Localization::String strSummary = Localization::Lookup("TXT_KEY_NOTIFICATION_SUMMARY_RESEARCH_AGREEMENT");
				strText << GET_PLAYER(eFirstPlayer).getCivilizationShortDescriptionKey();
				strText << GET_PLAYER(eSecondPlayer).getCivilizationShortDescriptionKey();
				pNotify->Add(NOTIFICATION_DIPLOMACY_DECLARATION, strText.toUTF8(), strSummary.toUTF8(), -1, -1, -1);
			}
		}
	}
}

//	--------------------------------------------------------------------------------
int CvGame::GetResearchAgreementCost(PlayerTypes ePlayer1, PlayerTypes ePlayer2) const
{
	ASSERT_DEBUG(ePlayer1 > NO_PLAYER, "Invalid player.");
	ASSERT_DEBUG(ePlayer1 <= MAX_MAJOR_CIVS, "Invalid player.");
	ASSERT_DEBUG(ePlayer2 > NO_PLAYER, "Invalid player.");
	ASSERT_DEBUG(ePlayer2 <= MAX_MAJOR_CIVS, "Invalid player.");

	EraTypes ePlayer1Era = GET_TEAM(GET_PLAYER(ePlayer1).getTeam()).GetCurrentEra();
	EraTypes ePlayer2Era = GET_TEAM(GET_PLAYER(ePlayer2).getTeam()).GetCurrentEra();
	EraTypes eHighestEra = max(ePlayer1Era, ePlayer2Era);

	int iCost = GC.getEraInfo(eHighestEra)->getResearchAgreementCost();

	iCost *= getGameSpeedInfo().getGoldPercent();
	iCost /= 100;

	return iCost;
}


//	--------------------------------------------------------------------------------
/// See if someone has won a conquest Victory
/// slewis, 10.1.12 - changing conquest so that a player has to hold all capitals
void CvGame::DoTestConquestVictory()
{
	VictoryTypes eConquestVictory = NO_VICTORY;
	for (int iVictoryLoop = 0; iVictoryLoop < GC.getNumVictoryInfos(); iVictoryLoop++)
	{
		VictoryTypes eLoopVictory = static_cast<VictoryTypes>(iVictoryLoop);
		CvVictoryInfo* pkVictoryInfo = GC.getVictoryInfo(eLoopVictory);
		if (pkVictoryInfo && pkVictoryInfo->isConquest())
		{
			if (isVictoryValid(eLoopVictory))
			{
				eConquestVictory = eLoopVictory;
				break;
			}

			return; // Domination Victory is disabled
		}
	}

	// Find out how many original capitals there are
	std::vector<CvCity*> originalCapitals;
	CvMap& kMap = GC.getMap();

	for (int iPlayerLoop = 0; iPlayerLoop < MAX_MAJOR_CIVS; iPlayerLoop++)
	{
		CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayerLoop);
		if (!kLoopPlayer.isEverAlive())
			continue;

		const int iOriginalCapitalX = kLoopPlayer.GetOriginalCapitalX();
		const int iOriginalCapitalY = kLoopPlayer.GetOriginalCapitalY();
		if (iOriginalCapitalX == -1 || iOriginalCapitalY == -1)
			continue;

		CvPlot* pCapitalPlot = kMap.plot(iOriginalCapitalX, iOriginalCapitalY);
		if (pCapitalPlot)
		{
			CvCity* pCapital = pCapitalPlot->getPlotCity();
			if (pCapital)
				originalCapitals.push_back(pCapital);
		}
	}

	if (originalCapitals.empty())
		return;

	// go through all capitals and see which team controls how many
	std::map<TeamTypes, int> countPerTeam;
	for (size_t i = 0; i < originalCapitals.size(); i++)
	{
		//indirect ownership through vassalage or allied city states is also acceptable
		PlayerTypes eCapitalOwner = originalCapitals[i]->GetOwnerForDominationVictory();
		countPerTeam[GET_PLAYER(eCapitalOwner).getTeam()]++;
	}

	int iMinPercent = range(GD_INT_GET(VICTORY_DOMINATION_CONTROL_PERCENT), 51, 100);
	int iThreshold = max(2, int(originalCapitals.size() * iMinPercent) / 100);
	for (std::map<TeamTypes, int>::iterator it = countPerTeam.begin(); it != countPerTeam.end(); ++it)
	{
		//must have at least two to win because you start out with one ...
		if (it->second >= iThreshold)
		{
			// The winner wins!
			CUSTOMLOG("Calling setWinner from DoTestConquestVictory: %i, %i", it->first, eConquestVictory);
			setWinner(it->first, eConquestVictory);
			return;
		}
	}
}

//	--------------------------------------------------------------------------------
/// Player leading with Wonders
PlayerTypes CvGame::GetBestWondersPlayer()
{
	return m_eBestWondersPlayer;
}

//	--------------------------------------------------------------------------------
/// Set Player leading with Wonders
void CvGame::SetBestWondersPlayer(PlayerTypes ePlayer, int iWonderCount)
{
	int iVictoryPointChange = /*5*/ GD_INT_GET(ZERO_SUM_COMPETITION_WONDERS_VICTORY_POINTS);

	// Remove VPs from old player's team
	if(GetBestWondersPlayer() != NO_PLAYER)
	{
		GET_TEAM(GET_PLAYER(GetBestWondersPlayer()).getTeam()).changeVictoryPoints(-iVictoryPointChange);
	}

	m_eBestWondersPlayer = ePlayer;

	if(ePlayer != NO_PLAYER)
	{
		GET_TEAM(GET_PLAYER(ePlayer).getTeam()).changeVictoryPoints(iVictoryPointChange);

		// Notify everyone of this change.
		for(int iNotifyLoop = 0; iNotifyLoop < MAX_MAJOR_CIVS; ++iNotifyLoop){
			PlayerTypes eNotifyPlayer = (PlayerTypes) iNotifyLoop;
			CvPlayerAI& kCurNotifyPlayer = GET_PLAYER(eNotifyPlayer);
			TeamTypes eCurNotifyTeam = kCurNotifyPlayer.getTeam();

			CvNotifications* pNotifications = kCurNotifyPlayer.GetNotifications();
			if(pNotifications)
			{
				CvString strBuffer;
				CvString strSummary;

				// current player now has the most Wonders
				if(kCurNotifyPlayer.GetID() == ePlayer)
				{
					strBuffer = GetLocalizedText("TXT_KEY_NOTIFICATION_BEST_IN_WONDERS_YOU", iVictoryPointChange, iWonderCount+1);
					strSummary = GetLocalizedText("TXT_KEY_NOTIFICATION_SUMMARY_BEST_IN_WONDERS_YOU", iVictoryPointChange, iWonderCount+1);
				}
				// Unmet player
				else if(!GET_TEAM(eCurNotifyTeam).isHasMet(GET_PLAYER(ePlayer).getTeam()))
				{
					strBuffer = GetLocalizedText("TXT_KEY_NOTIFICATION_BEST_IN_WONDERS_UNMET", iVictoryPointChange, iWonderCount+1);
					strSummary = GetLocalizedText("TXT_KEY_NOTIFICATION_SUMMARY_BEST_IN_WONDERS_UNMET", iVictoryPointChange, iWonderCount+1);
				}
				// Player we've met
				else
				{
					strBuffer = GetLocalizedText("TXT_KEY_NOTIFICATION_BEST_IN_WONDERS_ANOTHER", GET_PLAYER(ePlayer).getCivilizationShortDescriptionKey(), iVictoryPointChange, iWonderCount+1);
					strSummary = GetLocalizedText("TXT_KEY_NOTIFICATION_SUMMARY_BEST_IN_WONDERS_ANOTHER", GET_PLAYER(ePlayer).getCivilizationShortDescriptionKey());
				}

				pNotifications->Add(NOTIFICATION_VICTORY, strBuffer, strSummary, -1, -1, -1);
			}
		}
	}
}

//	--------------------------------------------------------------------------------
/// Player leading with Policies
PlayerTypes CvGame::GetBestPoliciesPlayer()
{
	return m_eBestPoliciesPlayer;
}

//	--------------------------------------------------------------------------------
/// Set Player leading with Policies
void CvGame::SetBestPoliciesPlayer(PlayerTypes ePlayer, int iPolicyCount)
{
	int iVictoryPointChange = /*5*/ GD_INT_GET(ZERO_SUM_COMPETITION_POLICIES_VICTORY_POINTS);

	// Remove VPs from old player's team
	if(GetBestPoliciesPlayer() != NO_PLAYER)
	{
		GET_TEAM(GET_PLAYER(GetBestPoliciesPlayer()).getTeam()).changeVictoryPoints(-iVictoryPointChange);
	}

	m_eBestPoliciesPlayer = ePlayer;

	if(ePlayer != NO_PLAYER)
	{
		CvString strBuffer;
		CvString strSummary;

		GET_TEAM(GET_PLAYER(ePlayer).getTeam()).changeVictoryPoints(iVictoryPointChange);

		//Notify everyone
		for(int iNotifyLoop = 0; iNotifyLoop < MAX_MAJOR_CIVS; ++iNotifyLoop){
			PlayerTypes eNotifyPlayer = (PlayerTypes) iNotifyLoop;
			CvPlayerAI& kCurNotifyPlayer = GET_PLAYER(eNotifyPlayer);
			TeamTypes eCurNotifyTeam = kCurNotifyPlayer.getTeam();

			// This player has the most Policies
			if(eNotifyPlayer == ePlayer)
			{
				strBuffer = GetLocalizedText("TXT_KEY_NOTIFICATION_BEST_IN_POLICIES_YOU", iVictoryPointChange, iPolicyCount+1);
				strSummary = GetLocalizedText("TXT_KEY_NOTIFICATION_SUMMARY_BEST_IN_POLICIES_YOU", iVictoryPointChange, iPolicyCount+1);
			}
			// Unmet player
			else if(!GET_TEAM(eCurNotifyTeam).isHasMet(GET_PLAYER(ePlayer).getTeam()))
			{
				strBuffer = GetLocalizedText("TXT_KEY_NOTIFICATION_BEST_IN_POLICIES_UNMET", iVictoryPointChange, iPolicyCount+1);
				strSummary = GetLocalizedText("TXT_KEY_NOTIFICATION_SUMMARY_BEST_IN_POLICIES_UNMET", iVictoryPointChange, iPolicyCount+1);
			}
			// player met
			else
			{
				strBuffer = GetLocalizedText("TXT_KEY_NOTIFICATION_BEST_IN_POLICIES_ANOTHER", GET_PLAYER(ePlayer).getCivilizationShortDescriptionKey(), iVictoryPointChange, iPolicyCount+1);
				strSummary = GetLocalizedText("TXT_KEY_NOTIFICATION_SUMMARY_BEST_IN_POLICIES_ANOTHER", GET_PLAYER(ePlayer).getCivilizationShortDescriptionKey(), iVictoryPointChange, iPolicyCount+1);
			}

			CvNotifications* pNotifications = kCurNotifyPlayer.GetNotifications();
			if(pNotifications)
			{
				pNotifications->Add(NOTIFICATION_VICTORY, strBuffer, strSummary, -1, -1, -1);
			}
		}
	}
}

//	--------------------------------------------------------------------------------
/// Player leading with GreatPeople
PlayerTypes CvGame::GetBestGreatPeoplePlayer()
{
	return m_eBestGreatPeoplePlayer;
}

//	--------------------------------------------------------------------------------
/// Set Player leading with GreatPeople
void CvGame::SetBestGreatPeoplePlayer(PlayerTypes ePlayer, int iGreatPeopleCount)
{
	int iVictoryPointChange = /*5*/ GD_INT_GET(ZERO_SUM_COMPETITION_GREAT_PEOPLE_VICTORY_POINTS);

	// Remove VPs from old player's team
	if(GetBestGreatPeoplePlayer() != NO_PLAYER)
	{
		GET_TEAM(GET_PLAYER(GetBestGreatPeoplePlayer()).getTeam()).changeVictoryPoints(-iVictoryPointChange);
	}

	m_eBestGreatPeoplePlayer = ePlayer;

	if(ePlayer != NO_PLAYER)
	{
		GET_TEAM(GET_PLAYER(ePlayer).getTeam()).changeVictoryPoints(iVictoryPointChange);

		for(int iNotifyLoop = 0; iNotifyLoop < MAX_MAJOR_CIVS; ++iNotifyLoop){
			PlayerTypes eNotifyPlayer = (PlayerTypes) iNotifyLoop;
			CvPlayerAI& kCurNotifyPlayer = GET_PLAYER(eNotifyPlayer);
			TeamTypes eCurNotifyTeam = kCurNotifyPlayer.getTeam();

			CvNotifications* pNotifications =kCurNotifyPlayer.GetNotifications();
			if(pNotifications)
			{
				CvString strBuffer;
				CvString strSummary;

				// Active Player now has the most GreatPeople
				if(eNotifyPlayer == ePlayer)
				{
					strBuffer = GetLocalizedText("TXT_KEY_NOTIFICATION_BEST_IN_GREAT_PEOPLE_YOU", iVictoryPointChange, iGreatPeopleCount+1);
					strSummary = GetLocalizedText("TXT_KEY_NOTIFICATION_SUMMARY_BEST_IN_GREAT_PEOPLE_YOU", iVictoryPointChange, iGreatPeopleCount+1);
				}
				// Unmet player
				else if(!GET_TEAM(eCurNotifyTeam).isHasMet(GET_PLAYER(ePlayer).getTeam()))
				{
					strBuffer = GetLocalizedText("TXT_KEY_NOTIFICATION_BEST_IN_GREAT_PEOPLE_UNMET", iVictoryPointChange, iGreatPeopleCount+1);
					strSummary = GetLocalizedText("TXT_KEY_NOTIFICATION_SUMMARY_BEST_IN_GREAT_PEOPLE_UNMET", iVictoryPointChange, iGreatPeopleCount+1);
				}
				// Player we've met
				else
				{
					strBuffer = GetLocalizedText("TXT_KEY_NOTIFICATION_BEST_IN_GREAT_PEOPLE_ANOTHER", GET_PLAYER(ePlayer).getCivilizationShortDescriptionKey(), iVictoryPointChange, iGreatPeopleCount+1);
					strSummary = GetLocalizedText("TXT_KEY_NOTIFICATION_SUMMARY_BEST_IN_GREAT_PEOPLE_ANOTHER", GET_PLAYER(ePlayer).getCivilizationShortDescriptionKey(), iVictoryPointChange, iGreatPeopleCount+1);
				}

				pNotifications->Add(NOTIFICATION_VICTORY, strBuffer, strSummary, -1, -1, -1);
			}
		}
	}
}

//	--------------------------------------------------------------------------------
/// Has a player researched a Tech which ends the game?
bool CvGame::IsEndGameTechResearched() const
{
	return m_bEndGameTechResearched;
}

//	--------------------------------------------------------------------------------
/// Sets whether or not a player has researched a Tech which ends the game
void CvGame::SetEndGameTechResearched(bool bValue)
{
	m_bEndGameTechResearched = bValue;
}

//	--------------------------------------------------------------------------------
bool CvGame::TunerEverConnected() const
{
	return m_bTunerEverConnected || gDLL->TunerEverConnected();
}

//	--------------------------------------------------------------------------------
bool CvGame::IsEverAttackedTutorial() const
{
	return m_bTutorialEverAttacked;
}

//	--------------------------------------------------------------------------------
void CvGame::SetEverAttackedTutorial(bool bValue)
{
	m_bTutorialEverAttacked = bValue;
}

//	--------------------------------------------------------------------------------
bool CvGame::IsEverRightClickMoved() const
{
	return m_bEverRightClickMoved;
}

//	--------------------------------------------------------------------------------
void CvGame::SetEverRightClickMoved(bool bValue)
{
	m_bEverRightClickMoved = bValue;
}

//	--------------------------------------------------------------------------------
bool CvGame::IsCombatWarned() const
{
	return m_bCombatWarned;
}

//	--------------------------------------------------------------------------------
void CvGame::SetCombatWarned(bool bValue)
{
	m_bCombatWarned = bValue;
}

//	--------------------------------------------------------------------------------
/// Shortcut for generating yield tool tip help
void CvGame::BuildYieldTimes100HelpText(CvString* toolTipSink, const char* strTextKey, int iYieldTimes100, const char* strYieldIcon, bool bIgnoreZero) const
{
	if (toolTipSink && (iYieldTimes100 != 0 || !bIgnoreZero))
	{
		// add a newline for every item that's not the first one
		if (!toolTipSink->IsEmpty())
		{
			(*toolTipSink) += CvString("[NEWLINE]");
		}
		(*toolTipSink) += GetLocalizedText(strTextKey, (float)iYieldTimes100 / 100, strYieldIcon);
	}
}


//	--------------------------------------------------------------------------------
/// Shortcut for generating production mod tool tip help
void CvGame::BuildProdModHelpText(CvString* toolTipSink, const char* strTextKey, int iMod, const char* strExtraKey, bool bShowIfZero) const
{
	if((iMod != 0 || bShowIfZero) && toolTipSink != NULL)
	{
		Localization::String localizedText = Localization::Lookup(strTextKey);
		localizedText << iMod;

		if (strExtraKey)
		{
			std::string extraKey(strExtraKey);
			if (!extraKey.empty())
				localizedText << strExtraKey;
		}

		const char* const localized = localizedText.toUTF8();
		if(localized)
			(*toolTipSink) += localized;
	}
}

//	--------------------------------------------------------------------------------
/// Shortcut for generating build action tool tip help
void CvGame::BuildCannotPerformActionHelpText(CvString* toolTipSink, const char* strTextKey, const char* strExtraKey1, const char* strExtraKey2, int iValue) const
{
	if(toolTipSink != NULL)
	{
		Localization::String localizedText = Localization::Lookup(strTextKey);

		if(iValue != -666)
			localizedText << iValue;

		if(strExtraKey1)
		{
			std::string extraKey1(strExtraKey1);
			if(!extraKey1.empty())
				localizedText << strExtraKey1;
		}

		if(strExtraKey2)
		{
			std::string extraKey2(strExtraKey2);
			if(!extraKey2.empty())
				localizedText << strExtraKey2;
		}

		const char* const localized = localizedText.toUTF8();
		if(localized)
			(*toolTipSink) += localized;
	}
}

void CvGame::LogMapState() const
{
	// Only need to save the terrain once
	if (GC.getGame().getElapsedGameTurns() < 1)
	{
		CvString strMapName;
		strMapName = strMapName.format("Maps\\Civ5MapmapState_Turn%03d.Civ5Map", GC.getGame().getElapsedGameTurns());

		const char* sz_strMapName = strMapName.c_str();
		std::vector<wchar_t> vec;
		size_t len = strlen(sz_strMapName);
		vec.resize(len + 1);
		mbstowcs(&vec[0], sz_strMapName, len);
		const wchar_t* wsz = &vec[0];

		CvWorldBuilderMapLoader::Save(wsz, NULL);
	}
	/*
	 * The end goal of this data dump is for use with a map image generating tool (https://github.com/samuelyuan/Civ5MapImage)
	 * to create visual representations of each turn of a game. The problem is, this tool expects a scenario file and not a
	 * savefile, which requires two workarounds: the first is in CvWorldBuilderMapLoader::Save, which restores the scenario
	 * metadata, and the second is the rest of this function, which fills in the remaining scenario data required:
	 * civs, improvements, units, etc.
	 * 
	 * This is a bit ugly but what I deemed to be the approach that requires the least amount of effort is to simply manually
	 * construct a json string with this info that will be merged with the map json representation in a script later
	 */
	CvString strLogName;
	strLogName = strLogName.format("mapStateLog_Turn%03d.json", GC.getGame().getElapsedGameTurns());
	FILogFile* pLog = LOGFILEMGR.GetLog(strLogName, FILogFile::kDontTimeStamp);
	CvString outputJson = "{\"MapData\": {";
	CvString strTemp;

	// MapTileImprovements Section - cities and owners
	outputJson += "\"MapTileImprovements\":[\n";
	for (int i = 0; i < GC.getMap().numPlots(); i++)
	{
		CvPlot* pPlot = GC.getMap().plotByIndexUnchecked(i);

		int cityId = -1;
		CvString cityName = "";
		int owner = 255;
		int routeType = 255;

		if (pPlot->isCity())
		{
			cityId = pPlot->getPlotCity()->GetID();
			cityName = pPlot->getPlotCity()->getName();
			owner = pPlot->getPlotCity()->getOwner();
		}
		else if (pPlot->getOwner() != NO_PLAYER)
		{
			owner = pPlot->getOwner();
		}

		switch (pPlot->getRouteType())
		{
		case ROUTE_ROAD:
			routeType = 0;
			break;
		case ROUTE_RAILROAD:
			routeType = 1;
			break;
		default:
			routeType = 255;
		}

		// Get the combat unit on the tile to display - for now civilian units and aircraft are not considered
		// due to only having space to display one flag, hence the filtering for combat strength only
		CvString combatUnit = "";
		int combatUnitID = 0;
		int combatUnitOwner = 255;
		int combatUnitMaxHP = 0;
		int combatUnitCurrHP = 0;
		for (int iZ = 0; iZ < pPlot->getNumUnits(); iZ++)
		{
			CvUnit* pLoopUnit = pPlot->getUnitByIndex(iZ);
			if (pLoopUnit && pLoopUnit->GetBaseCombatStrength() > 0)
			{
				combatUnitID = pLoopUnit->GetID();
				combatUnit = GC.getUnitInfo(pLoopUnit->getUnitType())->GetType();
				combatUnitOwner = pLoopUnit->getOwner();
				combatUnitMaxHP = pLoopUnit->GetMaxHitPoints();
				combatUnitCurrHP = pLoopUnit->GetMaxHitPoints() - pLoopUnit->getDamage();

				break;
			}
		}

		strTemp.Format("{\"X\":%d,\"Y\":%d,\"CityId\":%d,\"CityName\":\"%s\",\"Owner\":%d,\"RouteType\":%d,\"UnitID\":%d,\"Unit\":\"%s\",\"UnitOwner\":%d,\"UnitMaxHp\":%d,\"UnitCurrHp\":%d}",
			pPlot->getX(),
			pPlot->getY(),
			cityId,
			cityName.GetCString(),
			owner,
			routeType,
			combatUnitID,
			combatUnit.c_str(),
			combatUnitOwner,
			combatUnitMaxHP,
			combatUnitCurrHP
		);
		outputJson += strTemp;

		if (i != GC.getMap().numPlots() - 1)
		{
			outputJson += ",\n";
		}
	}
	// Close MapTileImprovements Block
	outputJson += "],";


	// Civ5PlayerData Section
	outputJson += "\"Civ5PlayerData\":[\n";

	for (int iL = 0; iL < MAX_CIV_PLAYERS; iL++)
	{
		PlayerColorTypes pc = GET_PLAYER((PlayerTypes)iL).getPlayerColor();

		if (pc <= 0)
		{
			continue;
		}

		const CvCivilizationInfo& thisCivilization = GET_PLAYER((PlayerTypes)iL).getCivilizationInfo();
		strTemp.Format("{\"Index\":%d,\"CivType\":\"%s\",\"TeamColor\":\"%d\"}",
			iL,
			thisCivilization.GetType(),
			pc
		);
		outputJson += strTemp;

		if (iL != MAX_CIV_PLAYERS - 1)
		{
			outputJson += ",\n";
		}
	}

	// Close Civ5PlayerData Block
	outputJson += "],";

	std::map<int, int> CityOwnerIndexMap;
	// CivColorOverrides Section
	outputJson += "\"CivColorOverrides\":[\n";


	for (int iL = 0; iL < MAX_CIV_PLAYERS; iL++)
	{
		PlayerColorTypes pc = GET_PLAYER((PlayerTypes)iL).getPlayerColor();

		if (pc <= 0)
		{
			continue;
		}
		CityOwnerIndexMap[iL] = pc;

		// This still requires post-processing but getting this text is a pain
		strTemp.Format("{\"CivKey\":\"%d\",\"OuterColor\":{\"Model\":\"constant\",\"ColorConstant\":\"\"},\"InnerColor\":{\"Model\":\"constant\",\"ColorConstant\":\"\"}}",
			pc
		);
		outputJson += strTemp;


		if (iL != MAX_CIV_PLAYERS - 1)
		{
			outputJson += ",\n";
		}
	}

	// Close CivColorOverrides Block
	outputJson += "],";

	// File format seems to want this weird CityOwnerIndexMap thing so ok
	outputJson += "\"CityOwnerIndexMap\":{}}}";
	pLog->Msg(outputJson);
}

//	--------------------------------------------------------------------------------
void CvGame::LogGameState(bool bLogHeaders) const
{
	if(GC.getLogging() && GC.getAILogging())
	{
		if (MOD_LOG_MAP_STATE)
		{
			LogMapState();
		}

		CvString strOutput;

		CvString playerName;
		CvString otherPlayerName;
		CvString strMinorString;
		CvString strDesc;
		CvString strTemp;

		CvString strLogName = "WorldState_Log.csv";
		FILogFile* pLog = LOGFILEMGR.GetLog(strLogName, FILogFile::kDontTimeStamp);

		AIGrandStrategyTypes eGrandStrategy;
		int iGSConquest = 0;
		int iGSSpaceship = 0;
		int iGSUN = 0;
		int iGSCulture = 0;

		int iAlly = 0;
		int iFriend = 0;
		int iFavorable = 0;
		int iNeutral = 0;
		int iCompetitor = 0;
		int iEnemy = 0;
		int iUnforgivable = 0;

		int iMajorWar = 0;
		int iMajorHostile = 0;
		int iMajorDeceptive = 0;
		int iMajorGuarded = 0;
		int iMajorAfraid = 0;
		int iMajorFriendly = 0;
		int iMajorNeutral = 0;

		int iMinorIgnore = 0;
		int iMinorProtective = 0;
		int iMinorConquest = 0;
		int iMinorBully = 0;

		// Loop through all Players
		for (int iPlayerLoop = 0; iPlayerLoop < MAX_MAJOR_CIVS; iPlayerLoop++)
		{
			PlayerTypes eLoopPlayer = (PlayerTypes) iPlayerLoop;
			CvPlayer* pPlayer = &GET_PLAYER(eLoopPlayer);

			if (pPlayer->isAlive())
			{
				eGrandStrategy = pPlayer->GetGrandStrategyAI()->GetActiveGrandStrategy();

				if (eGrandStrategy == GC.getInfoTypeForString("AIGRANDSTRATEGY_CONQUEST"))
				{
					iGSConquest++;
				}
				else if (eGrandStrategy == GC.getInfoTypeForString("AIGRANDSTRATEGY_SPACESHIP"))
				{
					iGSSpaceship++;
				}
				else if (eGrandStrategy == GC.getInfoTypeForString("AIGRANDSTRATEGY_UNITED_NATIONS"))
				{
					iGSUN++;
				}
				else if (eGrandStrategy == GC.getInfoTypeForString("AIGRANDSTRATEGY_CULTURE"))
				{
					iGSCulture++;
				}

				// Loop through all players
				for (int iPlayerLoop2 = 0; iPlayerLoop2 < MAX_CIV_PLAYERS; iPlayerLoop2++)
				{
					PlayerTypes eLoopPlayer2 = (PlayerTypes) iPlayerLoop2;

					if (GET_PLAYER(eLoopPlayer2).isAlive())
					{
						// Major
						if (GET_PLAYER(eLoopPlayer2).isMajorCiv())
						{
							switch (pPlayer->GetDiplomacyAI()->GetCivOpinion(eLoopPlayer2))
							{
							case CIV_OPINION_ALLY:
								iAlly++;
								break;
							case CIV_OPINION_FRIEND:
								iFriend++;
								break;
							case CIV_OPINION_FAVORABLE:
								iFavorable++;
								break;
							case CIV_OPINION_NEUTRAL:
								iNeutral++;
								break;
							case CIV_OPINION_COMPETITOR:
								iCompetitor++;
								break;
							case CIV_OPINION_ENEMY:
								iEnemy++;
								break;
							case CIV_OPINION_UNFORGIVABLE:
								iUnforgivable++;
								break;
							}

							switch (pPlayer->GetDiplomacyAI()->GetCivApproach(eLoopPlayer2))
							{
							case CIV_APPROACH_WAR:
								iMajorWar++;
								break;
							case CIV_APPROACH_HOSTILE:
								iMajorHostile++;
								break;
							case CIV_APPROACH_DECEPTIVE:
								iMajorDeceptive++;
								break;
							case CIV_APPROACH_GUARDED:
								iMajorGuarded++;
								break;
							case CIV_APPROACH_AFRAID:
								iMajorAfraid++;
								break;
							case CIV_APPROACH_NEUTRAL:
								iMajorNeutral++;
								break;
							case CIV_APPROACH_FRIENDLY:
								iMajorFriendly++;
								break;
							}
						}
						// Minor
						else
						{
							switch (pPlayer->GetDiplomacyAI()->GetCivApproach(eLoopPlayer2))
							{
							case CIV_APPROACH_WAR:
								iMinorConquest++;
								break;
							case CIV_APPROACH_HOSTILE:
								iMinorBully++;
								break;
							case CIV_APPROACH_NEUTRAL:
								iMinorIgnore++;
								break;
							case CIV_APPROACH_FRIENDLY:
								iMinorProtective++;
								break;
							case CIV_APPROACH_DECEPTIVE:
							case CIV_APPROACH_GUARDED:
							case CIV_APPROACH_AFRAID:
								UNREACHABLE();
							}
						}
					}
				}
			}
		}

		bool bFirstTurn = bLogHeaders || getElapsedGameTurns() == 0;

		// Grand Strategies
		if (bFirstTurn)
		{
			strOutput = "Turn";
			strOutput += ", Conquest";
			strOutput += ", Spaceship";
			strOutput += ", Diplo";
			strOutput += ", Culture";
		}
		else
		{
			strOutput.Format("%03d", GC.getGame().getElapsedGameTurns());
			strTemp.Format("%d", iGSConquest);
			strOutput += ", " + strTemp;
			strTemp.Format("%d", iGSSpaceship);
			strOutput += ", " + strTemp;
			strTemp.Format("%d", iGSUN);
			strOutput += ", " + strTemp;
			strTemp.Format("%d", iGSCulture);
			strOutput += ", " + strTemp;
		}

		// Major Approaches
		if (bFirstTurn)
		{
			strOutput += ", Ally";
			strOutput += ", Friend";
			strOutput += ", Favorable";
			strOutput += ", Neutral";
			strOutput += ", Competitor";
			strOutput += ", Enemy";
			strOutput += ", Unforgivable";
		}
		else
		{
			strTemp.Format("%d", iAlly);
			strOutput += ", " + strTemp;
			strTemp.Format("%d", iFriend);
			strOutput += ", " + strTemp;
			strTemp.Format("%d", iFavorable);
			strOutput += ", " + strTemp;
			strTemp.Format("%d", iNeutral);
			strOutput += ", " + strTemp;
			strTemp.Format("%d", iCompetitor);
			strOutput += ", " + strTemp;
			strTemp.Format("%d", iEnemy);
			strOutput += ", " + strTemp;
			strTemp.Format("%d", iUnforgivable);
			strOutput += ", " + strTemp;
		}

		// Major Approaches
		if (bFirstTurn)
		{
			strOutput += ", War";
			strOutput += ", Hostile";
			strOutput += ", Deceptive";
			strOutput += ", Guarded";
			strOutput += ", Afraid";
			strOutput += ", Friendly";
			strOutput += ", Neutral";
		}
		else
		{
			strTemp.Format("%d", iMajorWar);
			strOutput += ", " + strTemp;
			strTemp.Format("%d", iMajorHostile);
			strOutput += ", " + strTemp;
			strTemp.Format("%d", iMajorDeceptive);
			strOutput += ", " + strTemp;
			strTemp.Format("%d", iMajorGuarded);
			strOutput += ", " + strTemp;
			strTemp.Format("%d", iMajorAfraid);
			strOutput += ", " + strTemp;
			strTemp.Format("%d", iMajorFriendly);
			strOutput += ", " + strTemp;
			strTemp.Format("%d", iMajorNeutral);
			strOutput += ", " + strTemp;
		}

		// Minor Approaches
		if (bFirstTurn)
		{
			strOutput += ", Ignore";
			strOutput += ", Protective";
			strOutput += ", Conquest";
			strOutput += ", Bully";
		}
		else
		{
			strTemp.Format("%d", iMinorIgnore);
			strOutput += ", " + strTemp;
			strTemp.Format("%d", iMinorProtective);
			strOutput += ", " + strTemp;
			strTemp.Format("%d", iMinorConquest);
			strOutput += ", " + strTemp;
			strTemp.Format("%d", iMinorBully);
			strOutput += ", " + strTemp;
		}

		// Global Yield Averages
		if (bFirstTurn)
		{
			strOutput += ", BasicNeedsMedian";
			strOutput += ", GoldMedian";
			strOutput += ", ScienceMedian";
			strOutput += ", CultureMedian";
		}
		else
		{
			strTemp.Format("%d", GC.getGame().GetBasicNeedsMedian());
			strOutput += ", " + strTemp;
			strTemp.Format("%d", GC.getGame().GetGoldMedian());
			strOutput += ", " + strTemp;
			strTemp.Format("%d", GC.getGame().GetScienceMedian());
			strOutput += ", " + strTemp;
			strTemp.Format("%d", GC.getGame().GetCultureMedian());
			strOutput += ", " + strTemp;
		}

		pLog->Msg(strOutput);
	}
}

//	------------------------------------------------------------------------------------------------
void CvGame::unitIsMoving() const
{
	s_unitMoveTurnSlice = getTurnSlice();
}

//	------------------------------------------------------------------------------------------------
bool CvGame::allUnitAIProcessed() const
{
	int i = 0;
	for(i = 0; i < MAX_PLAYERS; i++)
	{
		const CvPlayer& player = GET_PLAYER(static_cast<PlayerTypes>(i));
		if(player.isTurnActive() && player.hasUnitsThatNeedAIUpdate())
			return false;
	}
	return true;
}

//	--------------------------------------------------------------------------------
/// How long are deals in this game (based on game speed)
int CvGame::GetDealDuration() const
{
	return getGameSpeedInfo().GetDealDuration();
}

int CvGame::GetRelationshipDuration() const
{
	return getGameSpeedInfo().getRelationshipDuration();
}

//	--------------------------------------------------------------------------------
int CvGame::GetPeaceDuration() const
{
	return getGameSpeedInfo().getPeaceDealDuration();
}

//	--------------------------------------------------------------------------------
bool CvGame::IsArchaeologyTriggered() const
{
	return m_bArchaeologyTriggered;
}

//	--------------------------------------------------------------------------------
void CvGame::TriggerArchaeologySiteCreation(bool bCheckInitialized)
{
	if (!m_bArchaeologyTriggered)
	{
		if (!bCheckInitialized || isFinalInitialized())
		{
			SpawnArchaeologySitesHistorically();
			m_bArchaeologyTriggered = true;
		}
	}
}


//	--------------------------------------------------------------------------------
int CalculateDigSiteWeight(int iIndex, vector<CvArchaeologyData>& inputData, vector<CvArchaeologyData>& chosenDigSites)
{
	CvMap& theMap = GC.getMap();
	int iGridWidth = theMap.getGridWidth();
	int iBaseWeight = 0;
	if (chosenDigSites[iIndex].m_eArtifactType == NO_GREAT_WORK_ARTIFACT_CLASS) // if we have not already chosen this spot for a dig site
	{
		iBaseWeight = inputData[iIndex].m_eArtifactType + 1;
		iBaseWeight *= (10 - inputData[iIndex].m_eEra);

		int iPlotX = iIndex % iGridWidth;
		int iPlotY = iIndex / iGridWidth;

		CvPlot* pPlot = theMap.plotByIndexUnchecked(iIndex);

		// zero this value if this plot has a resource, water, ice, mountain, or natural wonder
		if (pPlot->getResourceType() != NO_RESOURCE || pPlot->isWater() || !pPlot->isValidMovePlot(NO_PLAYER) || pPlot->IsNaturalWonder())
			iBaseWeight = 0;

		// if this tile cannot be improved, zero it out
		if (iBaseWeight && pPlot->getFeatureType() != NO_FEATURE)
		{
			if (GC.getFeatureInfo(pPlot->getFeatureType())->isNoImprovement())
			{
				iBaseWeight = 0;
			}
		}

		// if this tile has a GP improvement, zero it out
		if (iBaseWeight && pPlot->getImprovementType() != NO_IMPROVEMENT)
		{
			if (GC.getImprovementInfo(pPlot->getImprovementType())->IsCreatedByGreatPerson())
			{
				iBaseWeight = 0;
			}
		}

		if (iBaseWeight > 0)
		{
			// add a small random factor
			iBaseWeight += 10 + GC.getGame().randRangeExclusive(0, 10, CvSeeder(iBaseWeight));

			// increase the value if unowned
			iBaseWeight *= (pPlot->getOwner() == NO_PLAYER) ? 9 : 8;
			iBaseWeight /= 8;

			// lower the value if owned by a major
			iBaseWeight *= (pPlot->getOwner() > NO_PLAYER && pPlot->getOwner() < MAX_MAJOR_CIVS) ? 11 : 12;
			iBaseWeight /= 12;

			// lower the value if tile has been improved
			iBaseWeight *= (pPlot->getImprovementType() != NO_IMPROVEMENT || pPlot->getRouteType() != NO_ROUTE) ? 7 : 8;
			iBaseWeight /= 8;

			// lower the value if tile has a city
			iBaseWeight *= (pPlot->isCity()) ? 1 : 5;
			iBaseWeight /= 5;

			// increase the value if in thematic terrain (desert, jungle, or small island)
			iBaseWeight *= (pPlot->getTerrainType() == TERRAIN_DESERT) ? 3 : 2;
			iBaseWeight *= (pPlot->getFeatureType() == FEATURE_JUNGLE) ? 3 : 2;
			CvArea* pArea = theMap.getAreaById(pPlot->getArea());
			iBaseWeight *= (pArea->getNumTiles() <= 4) ? 3 : 2;

			// lower the value by number of neighbors
			int iDivisor = 1;
			// lower the value if there is at least one nearby site (say, 3 tiles distance)
			int iRange = 3;
			for (int iDX = -iRange; iDX <= iRange; iDX++)
			{
				for (int iDY = -iRange; iDY <= iRange; iDY++)
				{
					CvPlot* pLoopPlot = plotXYWithRangeCheck(iPlotX, iPlotY, iDX, iDY, iRange);
					if (pLoopPlot)
					{
						if (chosenDigSites[pLoopPlot->GetPlotIndex()].m_eArtifactType != NO_GREAT_WORK_ARTIFACT_CLASS)
						{
							iDivisor++;
						}
					}
				}
			}
			iRange = 2;
			for (int iDX = -iRange; iDX <= iRange; iDX++)
			{
				for (int iDY = -iRange; iDY <= iRange; iDY++)
				{
					CvPlot* pLoopPlot = plotXYWithRangeCheck(iPlotX, iPlotY, iDX, iDY, iRange);
					if (pLoopPlot)
					{
						if (chosenDigSites[pLoopPlot->GetPlotIndex()].m_eArtifactType != NO_GREAT_WORK_ARTIFACT_CLASS)
						{
							iDivisor++;
						}
					}
				}
			}
			iRange = 1;
			for (int iDX = -iRange; iDX <= iRange; iDX++)
			{
				for (int iDY = -iRange; iDY <= iRange; iDY++)
				{
					CvPlot* pLoopPlot = plotXYWithRangeCheck(iPlotX, iPlotY, iDX, iDY, iRange);
					if (pLoopPlot)
					{
						if (chosenDigSites[pLoopPlot->GetPlotIndex()].m_eArtifactType != NO_GREAT_WORK_ARTIFACT_CLASS)
						{
							iDivisor++;
						}
					}
				}
			}
			iBaseWeight /= iDivisor;
		}
	}
	return iBaseWeight;
}


//	--------------------------------------------------------------------------------
void CalculateDigSiteWeights(int iGridSize, vector<CvArchaeologyData>& inputData, vector<CvArchaeologyData>& chosenDigSites, vector<int>& currentWeights)
{
	for (int i = 0; i < iGridSize; i++)
	{
		currentWeights[i] = CalculateDigSiteWeight(i, inputData, chosenDigSites);
	}
}

//	--------------------------------------------------------------------------------
int CvGame::GetNumArchaeologySites() const
{
	if (!IsArchaeologyTriggered())
	{
		return -1;
	}

	int iRtnValue = 0;
	int iPlotLoop = 0;
	CvPlot *pPlot = NULL;
	for (iPlotLoop = 0; iPlotLoop < GC.getMap().numPlots(); iPlotLoop++)
	{
		pPlot = GC.getMap().plotByIndexUnchecked(iPlotLoop);
		if (pPlot->getResourceType() == GD_INT_GET(ARTIFACT_RESOURCE))
		{
			iRtnValue++;
		}
	}
	return iRtnValue;
}
//	--------------------------------------------------------------------------------
int CvGame::GetNumHiddenArchaeologySites() const
{
	if (!IsArchaeologyTriggered())
	{
		return -1;
	}

	int iRtnValue = 0;
	int iPlotLoop = 0;
	CvPlot *pPlot = NULL;
	for (iPlotLoop = 0; iPlotLoop < GC.getMap().numPlots(); iPlotLoop++)
	{
		pPlot = GC.getMap().plotByIndexUnchecked(iPlotLoop);
		if (pPlot->getResourceType() == GD_INT_GET(HIDDEN_ARTIFACT_RESOURCE))
		{
			iRtnValue++;
		}
	}
	return iRtnValue;
}

//	--------------------------------------------------------------------------------
PlayerTypes GetRandomMajorPlayer(CvPlot* pPlot)
{
	std::vector<PlayerTypes>tempPlayers;
	for (int i = 0; i < MAX_MAJOR_CIVS; i++)
	{
		PlayerTypes ePlayer = (PlayerTypes)i;

		if (ePlayer == NO_PLAYER)
			continue;

		if (!GET_PLAYER(ePlayer).isEverAlive())
			continue;

		tempPlayers.push_back(ePlayer);
	}


	uint uValue = GC.getGame().urandLimitExclusive(tempPlayers.size(), pPlot->GetPseudoRandomSeed());

	PlayerTypes ePlayer = static_cast<PlayerTypes>(tempPlayers[uValue]);

	if (ePlayer == NO_PLAYER)
		return BARBARIAN_PLAYER;

	return ePlayer;
}


//	--------------------------------------------------------------------------------
PlayerTypes GetRandomPlayer(CvPlot* pPlot)
{
	std::vector<PlayerTypes>tempPlayers;
	for (int i = 0; i < MAX_CIV_PLAYERS; i++)
	{
		PlayerTypes ePlayer = (PlayerTypes)i;

		if (ePlayer == NO_PLAYER)
			continue;

		if (!GET_PLAYER(ePlayer).isEverAlive())
			continue;

		tempPlayers.push_back(ePlayer);
	}


	uint uValue = GC.getGame().urandLimitExclusive(tempPlayers.size(), pPlot->GetPseudoRandomSeed());

	PlayerTypes ePlayer = static_cast<PlayerTypes>(tempPlayers[uValue]);

	if (ePlayer == NO_PLAYER)
		return BARBARIAN_PLAYER;

	return ePlayer;
}


void CvGame::PopulateDigSite(CvPlot& kPlot, EraTypes eEra, GreatWorkArtifactClass eArtifact)
{
	CvMap& theMap = GC.getMap();
	CvArchaeologyData digSite;

	const int iPlotX = kPlot.getX();
	const int iPlotY = kPlot.getY();

	eEra = eEra > static_cast<EraTypes>(0) ? eEra : static_cast<EraTypes>(0);
	digSite.m_eArtifactType = eArtifact;
	digSite.m_eEra = eEra;

	// find nearest city (preferably on same area)
	CvCity* pNearestCity = theMap.findCity(iPlotX, iPlotY, NO_PLAYER, NO_TEAM, true /* bSameArea */);
	pNearestCity = pNearestCity ? pNearestCity : theMap.findCity(iPlotX, iPlotY, NO_PLAYER, NO_TEAM, false /* bSameArea */); // expand search if we need to
	if (pNearestCity)
	{
		digSite.m_ePlayer1 = pNearestCity->getOriginalOwner();
	}
	else //  we can't find a nearby city (likely a late era start)
	{
		// look for nearby units
		CvUnit* pUnit = theMap.findUnit(iPlotX, iPlotY);
		if (pUnit)
		{
			digSite.m_ePlayer1 = pUnit->GetOriginalOwner();
		}
		else
		{
			// look for the start location if it exists
			PlayerTypes thisPlayer;
			if (theMap.findNearestStartPlot(iPlotX, iPlotY, thisPlayer))
			{
				digSite.m_ePlayer1 = thisPlayer;
			}
			else // just make something up
			{
				PlayerTypes ePlayer2 = GetRandomMajorPlayer(&kPlot);
				digSite.m_ePlayer1 = ePlayer2 == NO_PLAYER ? BARBARIAN_PLAYER : ePlayer2;
			}
		}
	}

	if (eArtifact == CvTypes::getARTIFACT_BATTLE_MELEE() || eArtifact == CvTypes::getARTIFACT_BATTLE_RANGED() || eArtifact == CvTypes::getARTIFACT_RAZED_CITY())
	{
		PlayerTypes ePlayer2 = GetRandomPlayer(&kPlot);
		digSite.m_ePlayer2 = ePlayer2 == NO_PLAYER ? BARBARIAN_PLAYER : ePlayer2;
	}

	kPlot.AddArchaeologicalRecord(digSite.m_eArtifactType, digSite.m_eEra, digSite.m_ePlayer1, digSite.m_ePlayer2);
}
//	--------------------------------------------------------------------------------
void CvGame::SpawnArchaeologySitesHistorically()
{
	CvMap& theMap = GC.getMap();
	const int iGridWidth = theMap.getGridWidth();

	// we should now have a map of the dig sites
	// turn this map into set of RESOURCE_ARTIFACTS
	const ResourceTypes eArtifactResourceType = static_cast<ResourceTypes>(GD_INT_GET(ARTIFACT_RESOURCE));
	const ResourceTypes eHiddenArtifactResourceType = static_cast<ResourceTypes>(GD_INT_GET(HIDDEN_ARTIFACT_RESOURCE));

	const size_t aRandomArtifactsCount = 7;
	GreatWorkArtifactClass aRandomArtifacts[aRandomArtifactsCount] = { 
		CvTypes::getARTIFACT_ANCIENT_RUIN(), 
		CvTypes::getARTIFACT_ANCIENT_RUIN(), 
		CvTypes::getARTIFACT_RAZED_CITY(), 
		CvTypes::getARTIFACT_BARBARIAN_CAMP(), 
		CvTypes::getARTIFACT_BARBARIAN_CAMP(), 
		CvTypes::getARTIFACT_BATTLE_MELEE(), 
		CvTypes::getARTIFACT_BATTLE_RANGED() 
	};

	// find how many dig sites we need to create
	const int iNumMajorCivs = GetNumMajorCivsEver();
	const int iMinDigSites = iNumMajorCivs * /*5*/ GD_INT_GET(MIN_DIG_SITES_PER_MAJOR_CIV); //todo: parameterize this
	const int iMaxDigSites = iNumMajorCivs * /*8*/ GD_INT_GET(MAX_DIG_SITES_PER_MAJOR_CIV); //todo: parameterize this
	const int iIdealNumDigSites = iMinDigSites + getJonRandNum(iMaxDigSites - iMinDigSites, "dig sites");

	// find the highest era any player has gotten to
	EraTypes eHighestEra = NO_ERA;
	PlayerTypes eLoopPlayer;
	for(int iPlayerLoop = 0; iPlayerLoop < MAX_MAJOR_CIVS; iPlayerLoop++)
	{
		eLoopPlayer = (PlayerTypes) iPlayerLoop;

		// Player not ever alive
		if(!GET_PLAYER(eLoopPlayer).isEverAlive())
			continue;

		if (GET_PLAYER(eLoopPlayer).GetCurrentEra() > eHighestEra)
		{
			eHighestEra = GET_PLAYER(eLoopPlayer).GetCurrentEra();
		}
	}


	CvWeightedVector<int> eEraWeights;
	eEraWeights.clear();
	if (eHighestEra > 0)
	{
		for (int i = 0; i < static_cast<int>(eHighestEra); i++)
		{
			int iWeight = static_cast<int>(eHighestEra) - i;
			eEraWeights.push_back(i, iWeight);
		}
		eEraWeights.StableSortItems();
	}
	else
	{
		// make sure this isn't empty
		eEraWeights.push_back(0, 1);
	}

	// find out how many dig sites we have now
	int iHowManyChosenDigSites = 0;

	// fill the historical buffer with the archaeological data
	vector<CvArchaeologyData> historicalDigSites;
	vector<CvArchaeologyData> scratchDigSites;
	int iGridSize = theMap.numPlots();
	ASSERT_DEBUG(iGridSize > 0, "iGridSize is zero");
	historicalDigSites.resize(iGridSize);
	scratchDigSites.resize(iGridSize);
	for (int i = 0; i < iGridSize; i++)
	{
		scratchDigSites[i].m_eArtifactType = NO_GREAT_WORK_ARTIFACT_CLASS;
		scratchDigSites[i].m_eEra = NO_ERA;
		scratchDigSites[i].m_ePlayer1 = NO_PLAYER;
		scratchDigSites[i].m_ePlayer2 = NO_PLAYER;

		CvPlot* pPlot = theMap.plotByIndexUnchecked(i);
		const ResourceTypes eResource = pPlot->getResourceType();
		if (pPlot->isWater() || !pPlot->isValidMovePlot(BARBARIAN_PLAYER))
		{
			historicalDigSites[i].m_eArtifactType = NO_GREAT_WORK_ARTIFACT_CLASS;
			historicalDigSites[i].m_eEra = NO_ERA;
			historicalDigSites[i].m_ePlayer1 = NO_PLAYER;
			historicalDigSites[i].m_ePlayer2 = NO_PLAYER;

			//Cannot be an antiquity site if we cannot generate an artifact.
			if(eResource == eArtifactResourceType || eResource == eHiddenArtifactResourceType)
			{
				pPlot->setResourceType(NO_RESOURCE, 0, true);
			}
		}
		else
		{
			//If this plot is already marked as an antiquity site, ensure it's populated.
			if(eResource == eArtifactResourceType || eResource == eHiddenArtifactResourceType)
			{
				if(pPlot->GetArchaeologicalRecord().m_eArtifactType == NO_GREAT_WORK_ARTIFACT_CLASS)
				{
					// pick an era before this one
					EraTypes eEra = static_cast<EraTypes>(eEraWeights.ChooseByWeight(CvSeeder::fromRaw(0xab7cdc61).mix(pPlot->GetPseudoRandomSeed())));
					eEra = eEra > static_cast<EraTypes>(0) ? eEra : static_cast<EraTypes>(0);

					// pick a type of artifact
					GreatWorkArtifactClass eArtifact = aRandomArtifacts[urandLimitExclusive(aRandomArtifactsCount, pPlot->GetPseudoRandomSeed())];

					PopulateDigSite(*pPlot, eEra, eArtifact);

					//Record in scratch space for weights.
					scratchDigSites[i] = pPlot->GetArchaeologicalRecord();
				}

				iHowManyChosenDigSites++;
			}

			historicalDigSites[i] = pPlot->GetArchaeologicalRecord();
		}
	}

	// calculate initial weights
	vector<int> digSiteWeights(iGridSize,0);
	CalculateDigSiteWeights(iGridSize, historicalDigSites, scratchDigSites, digSiteWeights);

	// build a weight vector
	static CvWeightedVector<int> aDigSiteWeights; // size of a HUGE world
	aDigSiteWeights.resize(iGridSize);

	vector<GreatWorkType> aWorksWriting;
	Database::Connection* db = GC.GetGameDatabase();
	if(db != NULL)
	{
		Database::Results kQuery;
		if(db->Execute(kQuery, "SELECT ID from GreatWorks WHERE ArchaeologyOnly = '1'"))
		{
			while(kQuery.Step())
			{
				const GreatWorkType eWork = static_cast<GreatWorkType>(kQuery.GetInt(0));
				aWorksWriting.push_back(eWork);
			}
		}
	}

	int iApproxNumHiddenSites = iIdealNumDigSites * /*30*/ GD_INT_GET(PERCENT_SITES_HIDDEN) / 100;
	int iNumDesiredWritingSites = iApproxNumHiddenSites * /*30*/ GD_INT_GET(PERCENT_HIDDEN_SITES_WRITING) / 100;
	int iNumWritingSites = min((int)aWorksWriting.size(), iNumDesiredWritingSites);

	// while we are not in the proper range of number of dig sites
	while (iHowManyChosenDigSites < iIdealNumDigSites)
	{
		// populate a weight vector
		aDigSiteWeights.clear();
		for (int i = 0; i < iGridSize; i++)
		{
			if (digSiteWeights[i] > 0)
			{
				aDigSiteWeights.push_back(i, digSiteWeights[i]);
			}
		}

		// Nowhere left to place a dig site!
		if (aDigSiteWeights.empty())
			return;

		// sort the weight vector
		aDigSiteWeights.StableSortItems();

		// add the best dig site
		int iBestSite = aDigSiteWeights.GetElement(0);
		CvPlot* pPlot = theMap.plotByIndexUnchecked(iBestSite);

		// Hidden site?
		bool bHiddenSite = GC.getGame().randRangeInclusive(1, 100, CvSeeder::fromRaw(0x5b7e949d).mix(pPlot->GetPseudoRandomSeed())) <= /*30*/ GD_INT_GET(PERCENT_SITES_HIDDEN);
		if (bHiddenSite)
		{
			pPlot->setResourceType(eHiddenArtifactResourceType, 1);
		}
		else
		{
			pPlot->setResourceType(eArtifactResourceType, 1);
		}

		// if this is not a historical dig site
		if (scratchDigSites[iBestSite].m_eArtifactType == NO_GREAT_WORK_ARTIFACT_CLASS)
		{
			// fake the historical data
			// pick an era before this one			
			EraTypes eEra = static_cast<EraTypes>(eEraWeights.ChooseByWeight(CvSeeder::fromRaw(0x5b75b72a).mix(pPlot->GetPseudoRandomSeed())));
			eEra = eEra > static_cast<EraTypes>(0) ? eEra : static_cast<EraTypes>(0);

			// pick a type of artifact
			GreatWorkArtifactClass eArtifact;
			eArtifact = aRandomArtifacts[urandLimitExclusive(aRandomArtifactsCount, pPlot->GetPseudoRandomSeed())];

			PopulateDigSite(*pPlot, eEra, eArtifact);
		}

		// If this is a hidden slot getting a writing, override a few things
		if (bHiddenSite && iNumWritingSites > 0)
		{
			// First change the type
			pPlot->SetArtifactType(CvTypes::getARTIFACT_WRITING());

			// Then get a writing and set it
			int iIndex = urandLimitExclusive(aWorksWriting.size(), pPlot->GetPseudoRandomSeed().mix(aWorksWriting.size()));
			GreatWorkType eWrittenGreatWork = aWorksWriting[iIndex];
			pPlot->SetArtifactGreatWork(eWrittenGreatWork);

			// Erase that writing from future consideration
			vector<GreatWorkType>::const_iterator it = std::find (aWorksWriting.begin(), aWorksWriting.end(), eWrittenGreatWork);
			aWorksWriting.erase(it);

			// One less writing to give out
			iNumWritingSites--;
		}

		scratchDigSites[iBestSite] = pPlot->GetArchaeologicalRecord();

		iHowManyChosenDigSites++;

		// recalculate weights near the chosen dig site (the rest of the world should still be fine)
		const int iRange = 3;
		int iPlotX = iBestSite % iGridWidth;
		int iPlotY = iBestSite / iGridWidth;
		for (int iDX = -iRange; iDX <= iRange; iDX++)
		{
			for (int iDY = -iRange; iDY <= iRange; iDY++)
			{
				CvPlot* pLoopPlot = plotXYWithRangeCheck(iPlotX, iPlotY, iDX, iDY, iRange);
				if (pLoopPlot)
				{
					int iIndex = pLoopPlot->GetPlotIndex();
					digSiteWeights[iIndex] = CalculateDigSiteWeight(iIndex, historicalDigSites, scratchDigSites);				
				}
			}
		}
	}
}


//	--------------------------------------------------------------------------------
CombatPredictionTypes CvGame::GetCombatPrediction(const CvUnit* pAttackingUnit, const CvUnit* pDefendingUnit)
{
	if(!pAttackingUnit || !pDefendingUnit)
	{
		return NO_COMBAT_PREDICTION;
	}

	CombatPredictionTypes ePrediction = NO_COMBAT_PREDICTION;

	if(pAttackingUnit->IsCanAttackRanged())
	{
		return COMBAT_PREDICTION_RANGED;
	}

	CvPlot* pFromPlot = pAttackingUnit->plot();
	CvPlot* pToPlot = pDefendingUnit->plot();

	CvUnit* pTest = GET_PLAYER(pAttackingUnit->getOwner()).getUnit(pAttackingUnit->GetID());

	if (pTest->GeneratePath(pToPlot, CvUnit::MOVEFLAG_APPROX_TARGET_RING1))
	{
		//this  must be the same moveflags as above so we can reuse the path next turn
		CvPlot* pEnd = pTest->GetPathLastPlot();
		if (pEnd)
			pFromPlot = pEnd;
	}

	int iRangedSupportDamageInflicted = 0;
	if (pAttackingUnit->isRangedSupportFire()) 
	{
		int iUnusedReferenceVariable = 0;
		iRangedSupportDamageInflicted = pAttackingUnit->GetRangeCombatDamage(pDefendingUnit, NULL, 0, iUnusedReferenceVariable, false);
	}

	int iAttackingStrength = pAttackingUnit->GetMaxAttackStrength(pFromPlot, pToPlot, pDefendingUnit, false, false);
	if(iAttackingStrength == 0)
	{
		return NO_COMBAT_PREDICTION;
	}

	int iDefenderStrength = pDefendingUnit->GetMaxDefenseStrength(pToPlot, pAttackingUnit, pFromPlot, false, false, iRangedSupportDamageInflicted);

	int iDefenderDamageInflicted = 0; // passed by reference
	int iAttackingDamageInflicted = pAttackingUnit->getMeleeCombatDamage(iAttackingStrength, iDefenderStrength, iDefenderDamageInflicted, false, pDefendingUnit, iRangedSupportDamageInflicted);
	//iTheirDamageInflicted = iTheirDamageInflicted + iTheirFireSupportCombatDamage;

	int iAttackerMaxHitPoints = pAttackingUnit->GetMaxHitPoints();
	int iDefenderMaxHitPoints = pDefendingUnit->GetMaxHitPoints();

	if (iAttackingDamageInflicted > iDefenderMaxHitPoints)
	{
		iAttackingDamageInflicted = iDefenderMaxHitPoints;
	}
	if (iDefenderDamageInflicted > iAttackerMaxHitPoints)
	{
		iDefenderDamageInflicted = iAttackerMaxHitPoints;
	}

	bool bAttackerDies = (pAttackingUnit->getDamage() + iDefenderDamageInflicted >= iAttackerMaxHitPoints);
	bool bDefenderDies = (pDefendingUnit->getDamage() + iAttackingDamageInflicted >= iDefenderMaxHitPoints);

	if(bAttackerDies && bDefenderDies)
	{
		ePrediction = COMBAT_PREDICTION_STALEMATE;
	}
	else if(bAttackerDies)
	{
		ePrediction = COMBAT_PREDICTION_TOTAL_DEFEAT;
	}
	else if(bDefenderDies)
	{
		ePrediction = COMBAT_PREDICTION_TOTAL_VICTORY;
	}
	else if(iAttackingDamageInflicted - iDefenderDamageInflicted > 30)
	{
		ePrediction = COMBAT_PREDICTION_MAJOR_VICTORY;
	}
	else if(iAttackingDamageInflicted > iDefenderDamageInflicted)
	{
		ePrediction = COMBAT_PREDICTION_SMALL_VICTORY;
	}
	else if(iDefenderDamageInflicted - iAttackingDamageInflicted > 30)
	{
		ePrediction = COMBAT_PREDICTION_MAJOR_DEFEAT;
	}
	else if(iAttackingDamageInflicted < iDefenderDamageInflicted)
	{
		ePrediction = COMBAT_PREDICTION_SMALL_DEFEAT;
	}
	else
	{
		ePrediction = COMBAT_PREDICTION_STALEMATE;
	}

	return ePrediction;
}

void CvGame::SetClosestCityMapDirty()
{
	m_cityDistancePathLength.SetDirty();
	m_cityDistancePlots.SetDirty();

	//debugging
	if (false)
	{
		CvString fname = CvString::format("CityDistance%03d.txt",getGameTurn());
		FILogFile* pLog = LOGFILEMGR.GetLog(fname.c_str(), FILogFile::kDontTimeStamp);
		if (pLog)
		{
			pLog->Msg("#x,y,water,plot dist,plot city,plot owner,turn dist,turn city,turn owner\n");
			for (int i = 0; i < GC.getMap().numPlots(); i++)
			{
				CvPlot* pPlot = GC.getMap().plotByIndex(i);
				int iDP = m_cityDistancePlots.GetDistance(*pPlot,false,NO_PLAYER);
				int iCP = m_cityDistancePlots.GetFeatureId(*pPlot,false,NO_PLAYER);
				PlayerTypes iOP = m_cityDistancePlots.GetFeatureOwner(*pPlot,false,NO_PLAYER);
				int iDT = m_cityDistancePathLength.GetDistance(*pPlot,false,NO_PLAYER);
				int iCT = m_cityDistancePathLength.GetFeatureId(*pPlot,false,NO_PLAYER);
				PlayerTypes iOT = m_cityDistancePathLength.GetFeatureOwner(*pPlot,false,NO_PLAYER);

				CvString dump = CvString::format("%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
					pPlot->getX(), pPlot->getY(), pPlot->isWater() ? 1 : 0, iDP, iCP, iOP, iDT, iCT, iOT);

				pLog->Msg(dump.c_str());
			}
		}
	}
}

int CvGame::GetClosestCityDistancePathLength( const CvPlot* pPlot, PlayerTypes ePlayer )
{
	if (!pPlot)
		return INT_MAX;

	return m_cityDistancePathLength.GetDistance(*pPlot, false, ePlayer);
}

CvCity* CvGame::GetClosestCityByPathLength( const CvPlot* pPlot, PlayerTypes ePlayer )
{
	if (!pPlot)
		return NULL;

	int owner = m_cityDistancePathLength.GetFeatureOwner(*pPlot, false, ePlayer);
	int id = m_cityDistancePathLength.GetFeatureId(*pPlot, false, ePlayer);
	if (owner!=NO_PLAYER)
		return GET_PLAYER((PlayerTypes)owner).getCity(id);
	else
		return NULL;
}

int CvGame::GetClosestCityDistancePathLength( const CvPlot* pPlot, bool bMajorOnly )
{
	if (!pPlot)
		return INT_MAX;

	return m_cityDistancePathLength.GetDistance(*pPlot, bMajorOnly, NO_PLAYER);
}

CvCity* CvGame::GetClosestCityByPathLength( const CvPlot* pPlot, bool bMajorOnly )
{
	if (!pPlot)
		return NULL;

	PlayerTypes owner = m_cityDistancePathLength.GetFeatureOwner(*pPlot, bMajorOnly, NO_PLAYER);
	int id = m_cityDistancePathLength.GetFeatureId(*pPlot, bMajorOnly, NO_PLAYER);
	if (owner!=NO_PLAYER)
		return GET_PLAYER(owner).getCity(id);
	else
		return NULL;
}

int CvGame::GetClosestCityDistanceInPlots( const CvPlot* pPlot, PlayerTypes ePlayer )
{
	if (!pPlot)
		return INT_MAX;

	return m_cityDistancePlots.GetDistance(*pPlot, false, ePlayer);
}

CvCity* CvGame::GetClosestCityByPlots( const CvPlot* pPlot, PlayerTypes ePlayer )
{
	if (!pPlot)
		return NULL;

	PlayerTypes owner = m_cityDistancePlots.GetFeatureOwner(*pPlot, false, ePlayer);
	int id = m_cityDistancePlots.GetFeatureId(*pPlot, false, ePlayer);

	if (owner != NO_PLAYER)
	{
		CvCity* pCity = GET_PLAYER(owner).getCity(id);
#ifdef VPDEBUG
		if (pCity == NULL && m_cityDistancePlots.GetDistance(*pPlot, false, ePlayer) < 5)
			CUSTOMLOG( "closest city (%d,%d) for player %d at plot (%d,%d) is invalid!", owner, id, ePlayer, pPlot->getX(), pPlot->getY() );
#endif

		return pCity;
	}
	else
	{
#ifdef VPDEBUG
		if (m_cityDistancePlots.GetDistance(*pPlot, false, ePlayer) < 5)
			CUSTOMLOG( "closest city for player %d at plot (%d,%d) has no valid owner!", ePlayer, pPlot->getX(), pPlot->getY() );
#endif

		return NULL;
	}
}

PlayerTypes CvGame::GetClosestCityOwnerByPlots(const CvPlot * pPlot, bool bMajorsOnly)
{
	if (!pPlot)
		return NO_PLAYER;

	return m_cityDistancePlots.GetFeatureOwner(*pPlot, bMajorsOnly, NO_PLAYER);
}

int CvGame::GetClosestCityDistanceInPlots( const CvPlot* pPlot, bool bMajorOnly )
{
	if (!pPlot)
		return INT_MAX;

	return m_cityDistancePlots.GetDistance(*pPlot, bMajorOnly, NO_PLAYER);
}

CvCity* CvGame::GetClosestCityByPlots( const CvPlot* pPlot, bool bMajorOnly )
{
	if (!pPlot)
		return NULL;

	PlayerTypes owner = m_cityDistancePlots.GetFeatureOwner(*pPlot, bMajorOnly, NO_PLAYER);
	int id = m_cityDistancePlots.GetFeatureId(*pPlot, bMajorOnly, NO_PLAYER);
	if (owner!=NO_PLAYER)
	{
		CvCity* pCity = GET_PLAYER(owner).getCity(id);
#ifdef VPDEBUG
		if (pCity == NULL && m_cityDistancePlots.GetDistance(*pPlot, bMajorOnly, NO_PLAYER) < 5)
			CUSTOMLOG( "closest city (%d,%d) for all players at plot (%d,%d) is invalid!", owner, id, pPlot->getX(), pPlot->getY() );
#endif

		return pCity;
	}
	else
	{
#ifdef VPDEBUG
		if (m_cityDistancePlots.GetDistance(*pPlot, bMajorOnly, NO_PLAYER) < 5)
			CUSTOMLOG( "closest city for all players at plot (%d,%d) has no valid owner!", pPlot->getX(), pPlot->getY() );
#endif

		return NULL;
	}
}

//------------------------------------------------------------
//------------------------------------------------------------
//------------------------------------------------------------
// Convert from city population to discrete size
//------------------------------------------------------------
//------------------------------------------------------------
//------------------------------------------------------------

//	--------------------------------------------------------------------------------
//Function to determine city size from city population
unsigned int CvGame::GetVariableCitySizeFromPopulation(unsigned int nPopulation)
{
	//fibonacci
	const unsigned int aiSizes[10] = { 1, 2, 3, 5, 8, 13, 21, 34, 55, UINT_MAX };
	for(unsigned int i = 0; i < 10; ++i)
	{
		if(nPopulation < aiSizes[i])
		{
			return i;
		}
	}
	return 4;
};

//------------------------------------------------------------
//------------------------------------------------------------
//------------------------------------------------------------

//	--------------------------------------------------------------------------------
void CvGame::NetMessageStaticsReset()
{//The net message system reset its static variables.  
}

//	--------------------------------------------------------------------------------
void CvGame::SetLastTurnAICivsProcessed()
{
	if(m_lastTurnAICivsProcessed != getGameTurn()){
		gDLL->SendAICivsProcessed();
		m_lastTurnAICivsProcessed = getGameTurn();
	}
}

bool CvGame::AnyoneHasBelief(BeliefTypes iBeliefType) const
{
	for (int i = 0; i < MAX_PLAYERS; ++i) {
		CvPlayer& player = GET_PLAYER(static_cast<PlayerTypes>(i));

		if(player.isAlive() && player.HasBelief(iBeliefType)) {
			return true;
		}
	}

	return false;
}

bool CvGame::AnyoneHasBuilding(BuildingTypes iBuildingType) const
{
	for (int i = 0; i < MAX_PLAYERS; ++i) {
		CvPlayer& player = GET_PLAYER(static_cast<PlayerTypes>(i));

		if(player.isAlive() && player.HasBuilding(iBuildingType)) {
			return true;
		}
	}

	return false;
}

bool CvGame::AnyoneHasBuildingClass(BuildingClassTypes iBuildingClassType) const
{
	for (int i = 0; i < MAX_PLAYERS; ++i) {
		CvPlayer& player = GET_PLAYER(static_cast<PlayerTypes>(i));

		if(player.isAlive() && player.HasBuildingClass(iBuildingClassType)) {
			return true;
		}
	}

	return false;
}

bool CvGame::AnyoneHasAnyWonder() const
{
	for (int i = 0; i < MAX_PLAYERS; ++i) {
		CvPlayer& player = GET_PLAYER(static_cast<PlayerTypes>(i));

		if(player.isAlive() && player.HasAnyWonder()) {
			return true;
		}
	}

	return false;
}

bool CvGame::AnyoneHasWonder(BuildingTypes iBuildingType) const
{
	for (int i = 0; i < MAX_PLAYERS; ++i) {
		CvPlayer& player = GET_PLAYER(static_cast<PlayerTypes>(i));

		if(player.isAlive() && player.HasWonder(iBuildingType)) {
			return true;
		}
	}

	return false;
}

int CvGame::GetCivilizationPlayer(CivilizationTypes iCivilizationType) const
{
	for (int i = 0; i < MAX_PLAYERS; ++i) {
		CvPlayer& player = GET_PLAYER(static_cast<PlayerTypes>(i));

		if(player.isAlive() && player.IsCivilization(iCivilizationType)) {
			return i;
		}
	}

	return NO_PLAYER;
}

bool CvGame::AnyoneIsInEra(EraTypes iEraType) const
{
	for (int i = 0; i < MAX_PLAYERS; ++i) {
		CvPlayer& player = GET_PLAYER(static_cast<PlayerTypes>(i));

		if(player.isAlive() && player.IsInEra(iEraType)) {
			return true;
		}
	}

	return false;
}

bool CvGame::AnyoneHasReachedEra(EraTypes iEraType) const
{
	for (int i = 0; i < MAX_PLAYERS; ++i) {
		CvPlayer& player = GET_PLAYER(static_cast<PlayerTypes>(i));

		if(player.isAlive() && player.HasReachedEra(iEraType)) {
			return true;
		}
	}

	return false;
}

bool CvGame::AnyoneHasAnyNaturalWonder() const
{
	for (int i = 0; i < MAX_PLAYERS; ++i) {
		CvPlayer& player = GET_PLAYER(static_cast<PlayerTypes>(i));

		if(player.isAlive() && player.HasAnyNaturalWonder()) {
			return true;
		}
	}

	return false;
}

bool CvGame::AnyoneHasNaturalWonder(FeatureTypes iFeatureType) const
{
	for (int i = 0; i < MAX_PLAYERS; ++i) {
		CvPlayer& player = GET_PLAYER(static_cast<PlayerTypes>(i));

		if(player.isAlive() && player.HasNaturalWonder(iFeatureType)) {
			return true;
		}
	}

	return false;
}

bool CvGame::AnyoneHasPolicy(PolicyTypes iPolicyType) const
{
	for (int i = 0; i < MAX_PLAYERS; ++i) {
		CvPlayer& player = GET_PLAYER(static_cast<PlayerTypes>(i));

		if(player.isAlive() && player.HasPolicy(iPolicyType)) {
			return true;
		}
	}

	return false;
}

bool CvGame::AnyoneHasTenet(PolicyTypes iPolicyType) const
{
	return AnyoneHasPolicy(iPolicyType);
}

bool CvGame::AnyoneHasPolicyBranch(PolicyBranchTypes iPolicyBranchType) const
{
	for (int i = 0; i < MAX_PLAYERS; ++i) {
		CvPlayer& player = GET_PLAYER(static_cast<PlayerTypes>(i));

		if(player.isAlive() && player.HasPolicyBranch(iPolicyBranchType)) {
			return true;
		}
	}

	return false;
}

bool CvGame::AnyoneHasIdeology(PolicyBranchTypes iPolicyBranchType) const
{
	return AnyoneHasPolicyBranch(iPolicyBranchType);
}

bool CvGame::AnyoneHasProject(ProjectTypes iProjectType) const
{
	for (int i = 0; i < MAX_PLAYERS; ++i) {
		CvPlayer& player = GET_PLAYER(static_cast<PlayerTypes>(i));

		if(player.isAlive() && player.HasProject(iProjectType)) {
			return true;
		}
	}

	return false;
}

bool CvGame::AnyoneHasPantheon() const
{
	for (int i = 0; i < MAX_PLAYERS; ++i) {
		CvPlayer& player = GET_PLAYER(static_cast<PlayerTypes>(i));

		if(player.isAlive() && player.HasPantheon()) {
			return true;
		}
	}

	return false;
}

bool CvGame::AnyoneHasAnyReligion() const
{
	for (int i = 0; i < MAX_PLAYERS; ++i) {
		CvPlayer& player = GET_PLAYER(static_cast<PlayerTypes>(i));

		if(player.isAlive() && player.HasAnyReligion()) {
			return true;
		}
	}

	return false;
}

bool CvGame::AnyoneHasReligion(ReligionTypes iReligionType) const
{
	for (int i = 0; i < MAX_PLAYERS; ++i) {
		CvPlayer& player = GET_PLAYER(static_cast<PlayerTypes>(i));

		if(player.isAlive() && player.HasReligion(iReligionType)) {
			return true;
		}
	}

	return false;
}

bool CvGame::IsResolutionPassed(ResolutionTypes iResolutionType, int iChoice) const
{
	CvGameLeagues* pGameLeagues = GC.getGame().GetGameLeagues();

	if (pGameLeagues->GetNumActiveLeagues() > 0) {
		CvLeague* pLeague = pGameLeagues->GetActiveLeague();
		if (pLeague) {
			return pLeague->IsActiveResolution(iResolutionType, iChoice);
		}
	}

	return false;
}

bool CvGame::AnyoneHasTech(TechTypes iTechType) const
{
	for (int i = 0; i < MAX_PLAYERS; ++i) {
		CvPlayer& player = GET_PLAYER(static_cast<PlayerTypes>(i));

		if(player.isAlive() && player.HasTech(iTechType)) {
			return true;
		}
	}

	return false;
}

bool CvGame::AnyoneHasUnit(UnitTypes iUnitType) const
{
	for (int i = 0; i < MAX_PLAYERS; ++i) {
		CvPlayer& player = GET_PLAYER(static_cast<PlayerTypes>(i));

		if(player.isAlive() && player.HasUnit(iUnitType)) {
			return true;
		}
	}

	return false;
}

bool CvGame::AnyoneHasUnitClass(UnitClassTypes iUnitClassType) const
{
	for (int i = 0; i < MAX_PLAYERS; ++i) {
		CvPlayer& player = GET_PLAYER(static_cast<PlayerTypes>(i));

		if(player.isAlive() && player.HasUnitClass(iUnitClassType)) {
			return true;
		}
	}

	return false;
}

#if defined(MOD_BALANCE_CORE_JFD)	
void CvGame::SetContractUnits(ContractTypes eContract, UnitTypes eUnit, int iValue)
{
	VALIDATE_OBJECT();
	ASSERT_DEBUG(eContract > -1 && eContract < GC.getNumContractInfos(), "Invalid eContract index.");
	ASSERT_DEBUG(eUnit > -1 && eUnit < GC.getNumUnitInfos(), "Invalid eUnit index.");

	if(m_ppaiContractUnits[eContract][eUnit] != iValue)
	{
		m_ppaiContractUnits[eContract][eUnit] = iValue;
	}
}
int CvGame::GetContractUnits(ContractTypes eContract, UnitTypes eUnit) const
{
	VALIDATE_OBJECT();
	ASSERT_DEBUG(eContract > -1 && eContract < GC.getNumContractInfos(), "Invalid eContract index.");
	ASSERT_DEBUG(eUnit > -1 && eUnit < GC.getNumUnitInfos(), "Invalid eUnit index.");

	return m_ppaiContractUnits[eContract][eUnit];
}
#endif
#if defined(MOD_BALANCE_CORE)

PlayerTypes CvGame::GetCorporationFounder(CorporationTypes eCorporation) const
{
	CvCorporation* pCorporation = m_pGameCorporations->GetCorporation(eCorporation);
	if (pCorporation == NULL)
		return NO_PLAYER;

	return pCorporation->m_eFounder;
}

int CvGame::GetNumCorporationsFounded() const
{
	return m_pGameCorporations->GetNumActiveCorporations();
}

#if defined(MOD_BALANCE_CORE_RESOURCE_MONOPOLIES)
void CvGame::UpdateGreatestPlayerResourceMonopoly(ResourceTypes eTestResource)
{
	for (int iResourceLoop = 0; iResourceLoop < GC.getNumResourceInfos(); iResourceLoop++)
	{
		ResourceTypes eResource = (ResourceTypes)iResourceLoop;

		//got a specific one in mind?
		if (eTestResource != NO_RESOURCE && eResource != eTestResource)
			continue;

		const CvResourceInfo* pkResource = GC.getResourceInfo(eResource);
		if (pkResource == NULL)
			continue;
		if (!pkResource->isMonopoly())
			continue;

		m_aiGreatestMonopolyPlayer[eResource] = NO_PLAYER;

		PlayerTypes eGreatestMonopolyPlayer = NO_PLAYER;
		PlayerTypes eLoopPlayer;
		int iMax = 0;
		for (int iLoopPlayer = 0; iLoopPlayer < MAX_MAJOR_CIVS; iLoopPlayer++)
		{
			eLoopPlayer = (PlayerTypes)iLoopPlayer;

			if (!GET_PLAYER(eLoopPlayer).isAlive()) continue;

			int iMonopolyPercent = GET_PLAYER(eLoopPlayer).GetMonopolyPercent(eResource);
			if (iMonopolyPercent > iMax)
			{
				iMax = iMonopolyPercent;
				eGreatestMonopolyPlayer = eLoopPlayer;
			}
		}
		m_aiGreatestMonopolyPlayer[eResource] = eGreatestMonopolyPlayer;
	}
}

int CvGame::GetGreatestPlayerResourceMonopoly(ResourceTypes eResource) const
{
	return m_aiGreatestMonopolyPlayer[eResource];
}

int CvGame::GetGreatestPlayerResourceMonopolyValue(ResourceTypes eResource) const
{
	PlayerTypes eGreatestPlayer = (PlayerTypes)m_aiGreatestMonopolyPlayer[eResource];
	if (eGreatestPlayer == NO_PLAYER)
		return 0;

	return GET_PLAYER(eGreatestPlayer).GetMonopolyPercent(eResource);
}
#endif

PlayerTypes CvGame::GetPotentialFreeCityPlayer(CvCity* pCity)
{
	if (pCity != NULL)
	{
		for (int i = MAX_MAJOR_CIVS; i < MAX_CIV_PLAYERS; i++)
		{
			PlayerTypes ePlayer = (PlayerTypes)i;

			if (GET_PLAYER(ePlayer).isAlive())
				continue;

			if (pCity->GetNumTimesOwned(ePlayer) <= 0)
				continue;

			return ePlayer;
		}
	}

	for(int i = MAX_MAJOR_CIVS; i < MAX_CIV_PLAYERS; i++)
	{
		PlayerTypes ePlayer = (PlayerTypes)i;

		if (GET_PLAYER(ePlayer).isEverAlive() || GET_PLAYER(ePlayer).isObserver())
			continue;

		return ePlayer;
	}

	return NO_PLAYER;
}

TeamTypes CvGame::GetPotentialFreeCityTeam(CvCity* pCity)
{
	if (pCity != NULL)
	{
		for (int i = 0; i < MAX_TEAMS; i++)
		{
			TeamTypes eTeam = (TeamTypes)i;

			if (GET_TEAM(eTeam).isAlive())
				continue;

			const std::vector<PlayerTypes>& teammates = GET_TEAM(eTeam).getPlayers();
			for (size_t i = 0; i < teammates.size(); ++i)
			{
				CvPlayer& player = GET_PLAYER(teammates[i]);
				if (pCity->GetNumTimesOwned(player.GetID()) <= 0)
					continue;

				return eTeam;
			}
		}
	}
	for (int i = 0; i < MAX_TEAMS; i++)
	{
		TeamTypes eTeam = (TeamTypes)i;

		if (GET_TEAM(eTeam).isEverAlive() || GET_TEAM(eTeam).isObserver())
			continue;

		return eTeam;
	}
	return NO_TEAM;
}

bool CvGame::CreateFreeCityPlayer(CvCity* pStartingCity, bool bJustChecking, bool bMajorFoundingCityState)
{
	if (pStartingCity == NULL)
		return false;

	PlayerTypes eNewPlayer = GetPotentialFreeCityPlayer(pStartingCity);
	TeamTypes eNewTeam = GetPotentialFreeCityTeam(pStartingCity);
	if (eNewPlayer == NO_PLAYER || eNewTeam == NO_TEAM)
		return false;

	const TeamTypes eTeam(eNewTeam);
	CvTeam& kTeam = GET_TEAM(eTeam);

	MinorCivTypes eNewType = CvPreGame::minorCivType(eNewPlayer);

	if (eNewType == NO_MINORCIV)
		return false;

	CvMinorCivInfo* pMinorCivInfo = GC.getMinorCivInfo(eNewType);

	if (!pMinorCivInfo)
		return false;

	if (bJustChecking)
		return true;

	CvString strCityName = pStartingCity->getNameKey();

	CvPreGame::setSlotStatus(eNewPlayer, SS_COMPUTER);

	CvPlayerAI& kPlayer = GET_PLAYER(eNewPlayer);

	kTeam.init(eTeam);
	kPlayer.init(eNewPlayer);
	kPlayer.gameStartInit();
	kPlayer.setAlive(true, false);
	kTeam.updateTeamStatus();
	initDiplomacy();

	// get the plot before transferring ownership
	CvPlot *pPlot = pStartingCity->plot();

	// setStartingPlot needs to be before acquireCity, otherwise if this is a maritime city
	// state, it will crash when the game tries to use the starting plot to look for the
	// closest city to grant the bonus food for the civ first meeting this city state, before
	// this function has finished executing.
	kPlayer.setStartingPlot(pPlot);
	kPlayer.GetMinorCivAI()->DoPickInitialItems();

	// The Phoenicia modmod converts founded cities into City-States
	// Also, if a City-State was replaced with a City-State, this is actually a LUA workaround at game start to replace one City-State with another (since City-States can't have revolts).
	// In either of these cases, set the city up as if it had never been conquered
	PlayerTypes eOldOwner = pStartingCity->getOwner();
	bool bWorkaround = eOldOwner != NO_PLAYER && GET_PLAYER(eOldOwner).isMinorCiv();
	bool bBlockLiberation = bMajorFoundingCityState || bWorkaround;

	CvCity* pNewCity = kPlayer.acquireCity(pStartingCity, false, false, bBlockLiberation);
	pStartingCity = NULL; //no longer valid
	//we have to set this here!

	//if (strCityName != "")
	//{
	//	CvString localizedText = "TXT_KEY_FREE_CITY";
	//	pNewCity->setName(strCityName, false, true);
	//}

	kPlayer.ChangeNumCitiesFounded(1);
	pNewCity->SetOccupied(false);

	if (!pNewCity->IsNoOccupiedUnhappiness())
		pNewCity->ChangeNoOccupiedUnhappinessCount(1);

	if (bBlockLiberation)
	{
		GET_PLAYER(eNewPlayer).setOriginalCapitalXY(pNewCity);

		if (!bMajorFoundingCityState)
		{
			GET_PLAYER(eOldOwner).resetOriginalCapitalXY();
			GET_PLAYER(eOldOwner).setAlive(false, false);
		}
	}

	kPlayer.GetMinorCivAI()->SetTurnLiberated(getGameTurn());

	//update our techs!
	GET_TEAM(kPlayer.getTeam()).DoMinorCivTech();

	if (bWorkaround)
		GET_PLAYER(eNewPlayer).initFreeUnits();
	else
		pNewCity->SpawnPlayerUnitsNearby(eNewPlayer, GC.getGame().getCurrentEra() + 2, false, true);

	// Move Units from player that don't belong here
	if (pPlot->getNumUnits() > 0)
	{
		// Get the current list of units because we will possibly be moving them out of the plot's list
		IDInfoVector currentUnits;
		if (pPlot->getUnits(&currentUnits) > 0)
		{
			for (IDInfoVector::const_iterator itr = currentUnits.begin(); itr != currentUnits.end(); ++itr)
			{
				CvUnit* pLoopUnit = (CvUnit*)GetPlayerUnit(*itr);

				if (pLoopUnit && pLoopUnit->getOwner() != kPlayer.GetID())
				{
					pLoopUnit->finishMoves();
					if (!pLoopUnit->jumpToNearestValidPlot())
						pLoopUnit->kill(false);
				}
			}
		}
	}
	return true;
}
#endif
bool CvGame::isFirstActivationOfPlayersAfterLoad() const
{
	return m_firstActivationOfPlayersAfterLoad;
}

void CvGame::SetCurrentVisibilityPlayer(PlayerTypes ePlayer)
{
	m_eCurrentVisibilityPlayer = ePlayer;
}

PlayerTypes CvGame::GetCurrentVisibilityPlayer() const
{
	return m_eCurrentVisibilityPlayer;
}

//	--------------------------------------------------------------------------------
// exe things

void CvGame::SetExeBinType(CvBinType eBinType)
{
	m_eExeBinType = eBinType;
}
CvBinType CvGame::GetExeBinType() const
{
	return m_eExeBinType;
}
bool CvGame::IsExeWantForceResyncAvailable() 
{
	return MOD_EXE_HACKING && m_eExeBinType == BIN_DX11 && isNetworkMultiPlayer() && gDLL->IsHost();
}
void CvGame::SetExeWantForceResyncValue(int value) 
{
	if (IsExeWantForceResyncAvailable())
	{
		*s_iExeWantForceResync = value;
		if (value == 1)
		{
			CvString strWarningText = GetLocalizedText("TXT_KEY_VP_MP_WARNING_RESYNC_SCHEDULED");
			GC.getDLLIFace()->sendChat(strWarningText, CHATTARGET_ALL, NO_PLAYER);
		}
	}
}
void CvGame::SetExeWantForceResyncPointer(int* pointer)
{
	s_iExeWantForceResync = pointer;
}