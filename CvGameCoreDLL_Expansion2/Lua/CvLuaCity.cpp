/*	-------------------------------------------------------------------------------------------------------
	© 1991-2012 Take-Two Interactive Software and its subsidiaries.  Developed by Firaxis Games.  
	Sid Meier's Civilization V, Civ, Civilization, 2K Games, Firaxis Games, Take-Two Interactive Software 
	and their respective logos are all trademarks of Take-Two interactive Software, Inc.  
	All other marks and trademarks are the property of their respective owners.  
	All rights reserved. 
	------------------------------------------------------------------------------------------------------- */

#include "CvGameCoreDLLPCH.h"
#include "../CvGameCoreDLLPCH.h"
#include "../CustomMods.h"
#include "CvLuaSupport.h"
#include "CvLuaArea.h"
#include "CvLuaCity.h"
#include "CvLuaPlot.h"
#include "CvLuaUnit.h"
#if defined(MOD_BALANCE_CORE_BUILDING_INVESTMENTS) || defined (MOD_BALANCE_CORE_UNIT_INVESTMENTS)
#include "../CvInternalGameCoreUtils.h"
#endif

#pragma warning(disable:4800 ) //forcing value to bool 'true' or 'false'

//Utility macro for registering methods
#define Method(Name)			\
	lua_pushcclosure(L, l##Name, 0);	\
	lua_setfield(L, t, #Name);


using namespace CvLuaArgs;

//------------------------------------------------------------------------------
void CvLuaCity::PushMethods(lua_State* L, int t)
{
	Method(IsNone);
	Method(Kill);

	Method(CreateGreatGeneral);
	Method(CreateGreatAdmiral);

	Method(DoTask);
	Method(ChooseProduction);
	Method(GetCityPlotIndex);
	Method(GetCityIndexPlot);
	Method(CanWork);
	Method(IsPlotBlockaded);
	Method(IsBlockadedTest);
	Method(ClearWorkingOverride);
	Method(CountNumImprovedPlots);
	Method(CountNumWaterPlots);
	Method(CountNumRiverPlots);

	Method(FindPopulationRank);
	Method(FindBaseYieldRateRank);
	Method(FindYieldRateRank);

	Method(AllUpgradesAvailable);
	Method(IsWorldWondersMaxed);
	Method(IsTeamWondersMaxed);
	Method(IsNationalWondersMaxed);
	Method(IsBuildingsMaxed);

	Method(CanTrainTooltip);
	Method(CanTrain);
	Method(CanConstructTooltip);
	Method(CanConstruct);
	Method(CanCreateTooltip);
	Method(CanCreate);
	Method(CanMaintain);

	Method(GetPurchaseUnitTooltip);
	Method(GetFaithPurchaseUnitTooltip);
	Method(GetPurchaseBuildingTooltip);
	Method(GetFaithPurchaseBuildingTooltip);

	Method(CanJoin);
	Method(IsBuildingLocalResourceValid);

	Method(GetBuildingYieldRateTimes100)
	Method(GetBuildingYieldModifier)

	Method(GetPlotsBoostedByBuilding);

	Method(SetBuildingHidden);
	Method(ClearHiddenBuildings);
	Method(IsBuildingHidden);
	Method(GetNumHiddenBuildings);

	Method(GetResourceDemanded);
	Method(SetResourceDemanded);
	Method(DoPickResourceDemanded);

	Method(GetFoodTurnsLeft);
	Method(IsProduction);
	Method(IsProductionLimited);
	Method(IsProductionUnit);
	Method(IsProductionBuilding);
	Method(IsProductionProject);
	Method(IsProductionProcess);

	Method(CanContinueProduction);
	Method(GetProductionExperience);
	Method(AddProductionExperience);

	Method(GetProductionUnit);
	Method(GetProductionUnitAI);
	Method(GetProductionBuilding);
	Method(GetProductionProject);
	Method(GetProductionProcess);
	//Method(GetProductionName);
	Method(GetProductionNameKey);
	Method(GetGeneralProductionTurnsLeft);
	Method(IsFoodProduction);
	Method(GetFirstUnitOrder);
	Method(GetFirstProjectOrder);

	Method(GetOrderFromQueue);

	Method(GetNumTrainUnitAI);
	Method(GetFirstBuildingOrder);
	Method(IsUnitFoodProduction);
	Method(GetProduction);
	Method(GetProductionTimes100);
	Method(GetProductionNeeded);
	Method(GetUnitProductionNeeded);
	Method(GetBuildingProductionNeeded);
#if defined(MOD_BALANCE_CORE_BUILDING_INVESTMENTS)
	Method(GetBuildingInvestment);
	Method(IsWorldWonder);
	Method(GetWorldWonderCost);
	Method(GetNumPoliciesNeeded);
#endif
#if defined(MOD_BALANCE_CORE_BUILDING_INVESTMENTS)
	Method(GetUnitInvestment);
#endif
	Method(GetProjectProductionNeeded);
	Method(GetProductionTurnsLeft);
	Method(GetUnitProductionTurnsLeft);
	Method(GetBuildingProductionTurnsLeft);
	Method(GetProjectProductionTurnsLeft);
#if defined(MOD_PROCESS_STOCKPILE)
	Method(GetProcessProductionTurnsLeft);
#endif

	Method(CreateApolloProgram);

	Method(IsCanPurchase);
	Method(Purchase);
	Method(GetUnitPurchaseCost);
	Method(GetUnitFaithPurchaseCost);
	Method(GetBuildingPurchaseCost);
	Method(GetBuildingFaithPurchaseCost);
	Method(GetProjectPurchaseCost);

	Method(SetProduction);
	Method(ChangeProduction);

	Method(GetYieldModifierTooltip);
	Method(GetProductionModifier);
	Method(GetYieldRateTooltip);
	Method(GetTotalOverflowProductionTimes100);
	Method(GetCurrentProductionDifference);
	Method(GetRawProductionDifference);
	Method(GetCurrentProductionDifferenceTimes100);
	Method(GetRawProductionDifferenceTimes100);
	Method(GetUnitProductionModifier);
	Method(GetBuildingProductionModifier);
	Method(GetProjectProductionModifier);
	Method(GetResourceYieldRateModifier);
	Method(GetHandicapType);
	Method(GetCivilizationType);
	Method(GetPersonalityType);
	Method(GetArtStyleType);
	Method(GetCitySizeType);

	Method(IsBarbarian);
	Method(IsHuman);
	Method(IsVisible);

	Method(IsCapital);
	Method(IsOriginalCapital);
	Method(IsOriginalMajorCapital);
	Method(GetOwnerForDominationVictory);
	Method(IsCoastal);
	Method(IsAddsFreshWater);
	Method(FoodConsumptionSpecialistTimes100);

	Method(GetFoodConsumptionTimes100);
	Method(FoodConsumption);
	Method(FoodDifference);
	Method(FoodDifferenceTimes100);
	Method(GrowthThreshold);
	Method(ProductionLeft);

	Method(GetNumBuilding);
	Method(IsHasBuilding);
	Method(GetNumBuildingClass);
	Method(IsHasBuildingClass);
	Method(GetLocalBuildingClassYield);
	Method(GetEventBuildingClassYield);
	Method(GetEventBuildingClassModifier);
	Method(GetEventCityYield);
	Method(GetNumActiveBuilding);
	Method(GetID);
	Method(GetX);
	Method(GetY);
	Method(At);
	Method(AtPlot);
	Method(Plot);
	Method(Area);
	Method(WaterArea);
	Method(GetRallyPlot);

	Method(CanBuyPlot);
	Method(CanBuyPlotAt);
	Method(GetNextBuyablePlot);
	Method(GetBuyablePlotList);
	Method(GetBuyPlotCost);
	Method(CanBuyAnyPlot);

	Method(GetGarrisonedUnit);

	Method(GetGameTurnFounded);
	Method(GetGameTurnAcquired);
	Method(GetGameTurnLastExpanded);
	Method(GetPopulation);
	Method(SetPopulation);
	Method(ChangePopulation);
	Method(GetRealPopulation);
#if defined(MOD_GLOBAL_CITY_AUTOMATON_WORKERS)
	Method(GetAutomatons);
	Method(SetAutomatons);
#endif
	Method(GetHighestPopulation);
	Method(SetHighestPopulation);
	//Method(GetWorkingPopulation);
	//Method(GetSpecialistPopulation);
	Method(GetBaseGreatPeopleRate);
	Method(GetGreatPeopleRate);
	Method(GetEventGPPFromSpecialists);
	Method(GetSpecialistRate);
	Method(GetTotalGreatPeopleRateModifier);
	Method(ChangeBaseGreatPeopleRate);
	Method(GetGreatPeopleRateModifier);
	Method(GetImprovementGreatPersonRateModifier);
	Method(GetReligionGreatPersonRateModifier);

	Method(GetBorderGrowthRateIncreaseTotal);
	Method(GetJONSCultureStored);
	Method(GetJONSCultureStoredTimes100);
	Method(SetJONSCultureStored);
	Method(SetJONSCultureStoredTimes100);
	Method(ChangeJONSCultureStored);
	Method(ChangeJONSCultureStoredTimes100);
	Method(GetJONSCultureLevel);
	Method(SetJONSCultureLevel);
	Method(ChangeJONSCultureLevel);
	Method(DoJONSCultureLevelIncrease);
	Method(GetJONSCultureThreshold);

	Method(GetJONSCulturePerTurn);

	Method(GetBaseJONSCulturePerTurn);
	Method(GetJONSCulturePerTurnFromBuildings);
	Method(ChangeJONSCulturePerTurnFromBuildings);
	Method(GetJONSCulturePerTurnFromPolicies);
	Method(ChangeJONSCulturePerTurnFromPolicies);
	Method(GetJONSCulturePerTurnFromSpecialists);
	Method(ChangeJONSCulturePerTurnFromSpecialists);
	Method(GetJONSCulturePerTurnFromGreatWorks);
	Method(GetJONSCulturePerTurnFromTraits);
	Method(ChangeYieldFromTraits);
	Method(GetYieldPerTurnFromTraits);
	Method(GetYieldFromUnitsInCity);
	Method(ChangeJONSCulturePerTurnFromReligion);
	Method(GetJONSCulturePerTurnFromReligion);
	Method(GetJONSCulturePerTurnFromLeagues);

	Method(GetCultureRateModifier);
	Method(ChangeCultureRateModifier);
#if defined(MOD_BALANCE_CORE_RESOURCE_MONOPOLIES)
	Method(GetCityYieldModFromMonopoly);
#endif
	Method(GetTourismRateModifier);
	Method(ChangeTourismRateModifier);

	Method(GetNumGreatWorks);
	Method(GetNumGreatWorkSlots);
	Method(GetBaseTourism);
	Method(RefreshTourism);
	Method(GetNumGreatWorksFilled);
	Method(GetNumAvailableGreatWorkSlots);
	Method(GetTourismMultiplier);
	Method(GetTourismTooltip);
	Method(GetFilledSlotsTooltip);
	Method(GetTotalSlotsTooltip);
	Method(ClearGreatWorks);
	Method(GetFaithBuildingTourism);
	Method(GetBuildingClassTourism);

	Method(IsThemingBonusPossible);
	Method(GetThemingBonus);
	Method(GetThemingTooltip);

	Method(GetFaithPerTurn);
	Method(GetFaithPerTurnFromBuildings);
	Method(GetFaithPerTurnFromPolicies);
	Method(GetFaithPerTurnFromTraits);
	Method(GetFaithPerTurnFromReligion);
	Method(ChangeFaithPerTurnFromReligion);

	Method(HasConvertedToReligionEver);
	Method(IsReligionInCity);
	Method(IsHolyCityForReligion);
	Method(IsHolyCityAnyReligion);

	Method(GetNumFollowers);
	Method(GetReligiousMajority);
	Method(GetSecondaryReligion);
	Method(GetSecondaryReligionPantheonBelief);
	Method(GetPressurePerTurn);
	Method(ConvertPercentFollowers);
	Method(AdoptReligionFully);
	Method(GetReligionBuildingClassHappiness);
	Method(GetReligionBuildingClassYieldChange);
	Method(GetLeagueBuildingClassYieldChange);
	Method(GetNumTradeRoutesAddingPressure);

	Method(GetNumWorldWonders);
	Method(GetNumTeamWonders);
	Method(GetNumNationalWonders);
	Method(GetNumBuildings);
	Method(GetNumTotalBuildings);

	Method(GetWonderProductionModifier);
	Method(ChangeWonderProductionModifier);

	Method(GetLocalResourceWonderProductionMod);

	Method(GetBuyPlotDistance);
	Method(GetWorkPlotDistance);

#if defined(MOD_BUILDINGS_CITY_WORKING)
	Method(GetCityWorkingChange);
	Method(ChangeCityWorkingChange);
#endif
#if defined(MOD_BUILDINGS_CITY_AUTOMATON_WORKERS)
	Method(GetCityAutomatonWorkersChange);
	Method(ChangeCityAutomatonWorkersChange);
#endif

	Method(GetRemainingFreeSpecialists);
	Method(GetBasicNeedsMedian);
	Method(GetGoldMedian);
	Method(GetScienceMedian);
	Method(GetCultureMedian);
	Method(GetReligiousUnrestPerMinorityFollower);
	Method(GetTheoreticalNewBasicNeedsMedian);
	Method(GetTheoreticalNewGoldMedian);
	Method(GetTheoreticalNewScienceMedian);
	Method(GetTheoreticalNewCultureMedian);
	Method(GetTheoreticalNewReligiousUnrestPerMinorityFollower);
	Method(GetCachedTechNeedModifier);
	Method(GetCitySizeModifier);
	Method(GetEmpireSizeModifier);
	Method(getHappinessDelta);
	Method(GetUnhappinessAggregated);
	Method(GetAllNeedsModifier);
	Method(GetUnhappinessFromYield);
	Method(GetUnhappinessFromIsolation);
	Method(GetUnhappinessFromPillagedTiles);
	Method(GetUnhappinessFromFamine);
	Method(GetUnhappinessFromReligiousUnrest);
	Method(getPotentialUnhappinessWithGrowth);
	Method(GetCityUnhappinessBreakdown);
	Method(GetCityHappinessBreakdown);
	Method(getUnhappinessFromSpecialists);

	Method(ChangeHealRate);

	Method(IsNoOccupiedUnhappiness);

	Method(GetFood);
	Method(GetFoodTimes100);
	Method(SetFood);
	Method(ChangeFood);
	Method(GetFoodKept);
	Method(GetMaxFoodKeptPercent);
	Method(GetOverflowProduction);
	Method(SetOverflowProduction);
	Method(ChangeOverflowProduction);
	Method(GetFeatureProduction);
	Method(SetFeatureProduction);
	Method(GetMilitaryProductionModifier);
	Method(GetSpaceProductionModifier);
	Method(GetBuildingDefense);
	Method(GetFreeExperience);
	Method(GetNukeModifier);
	//Method(GetFreeSpecialist);

	Method(IsResistance);
	Method(GetResistanceTurns);
	Method(ChangeResistanceTurns);

	Method(IsRazing);
	Method(GetRazingTurns);
	Method(ChangeRazingTurns);

	Method(IsOccupied);
	Method(SetOccupied);

	Method(IsPuppet);
	Method(SetPuppet);

	Method(GetHappinessFromBuildings);
	Method(GetHappiness);
	Method(GetLocalHappiness);

	Method(IsNeverLost);
	Method(SetNeverLost);
	Method(IsDrafted);
	Method(SetDrafted);

	Method(IsBlockaded);
	Method(IsMined);
	Method(IsBorderObstacleLand);
	Method(IsBorderObstacleWater);

	Method(GetWeLoveTheKingDayCounter);
	Method(SetWeLoveTheKingDayCounter);
	Method(ChangeWeLoveTheKingDayCounter);

	Method(GetNumThingsProduced);

	Method(IsProductionAutomated);
	Method(SetProductionAutomated);
	Method(SetCitySizeBoost);
	Method(GetOwner);
	Method(GetTeam);
	Method(GetPreviousOwner);
	Method(GetOriginalOwner);
	Method(GetSeaPlotYield);
	Method(GetRiverPlotYield);
	Method(GetLakePlotYield);

	Method(GetBaseYieldRate);
	Method(GetBaseYieldRateTimes100);

#if defined(MOD_GLOBAL_GREATWORK_YIELDTYPES)
	Method(GetBaseYieldRateFromGreatWorks);
#endif

	Method(GetBaseYieldRateFromTerrain);
	Method(ChangeBaseYieldRateFromTerrain);

	Method(GetBaseYieldRateFromBuildings);
	Method(ChangeBaseYieldRateFromBuildings);

	Method(GetBaseYieldRateFromSpecialists);
	Method(ChangeBaseYieldRateFromSpecialists);

	Method(GetBaseYieldRateFromMisc);
	Method(ChangeBaseYieldRateFromMisc);

	Method(GetBaseYieldRateFromProcess);
	Method(GetBaseYieldRateFromTradeRoutes);

	// Base yield rate from League
	Method(GetBaseYieldRateFromLeague);

	Method(GetYieldFromCityYieldTimes100);

	Method(GetBaseYieldRateFromReligion);
	Method(ChangeBaseYieldRateFromReligion);

	Method(GetYieldPerPopTimes100);
	Method(GetYieldPerPopInEmpireTimes100);

	Method(GetYieldFromYieldPerBuildingTimes100);

	Method(GetBaseYieldRateModifier);
	Method(GetYieldRate);
	Method(GetYieldRateTimes100);
	Method(GetYieldRateModifier);

	Method(GetExtraSpecialistYield);
	Method(GetExtraSpecialistYieldOfType);

	Method(GetDomainFreeExperience);
	Method(GetDomainProductionModifier);

	Method(IsEverOwned);
	Method(GetNumTimesOwned);

	Method(IsRevealed);
	Method(SetRevealed);
	Method(GetNameKey);
	Method(GetName);
	Method(SetName);
	Method(IsHasResourceLocal);
	Method(GetBuildingProduction);
	Method(SetBuildingProduction);
	Method(ChangeBuildingProduction);
	Method(GetBuildingProductionTime);
	Method(SetBuildingProductionTime);
	Method(ChangeBuildingProductionTime);
	Method(GetBuildingOriginalOwner);
	Method(GetBuildingOriginalTime);
	Method(GetUnitProduction);
	Method(SetUnitProduction);
	Method(ChangeUnitProduction);

	Method(IsCanAddSpecialistToBuilding);
	Method(GetSpecialistUpgradeThreshold);
	Method(GetNumSpecialistsAllowedByBuilding);
	Method(GetSpecialistCount);
	Method(GetTotalSpecialistCount);
	Method(GetSpecialistCityModifier);
	Method(GetSpecialistGreatPersonProgress);
	Method(GetSpecialistGreatPersonProgressTimes100);
	Method(ChangeSpecialistGreatPersonProgressTimes100);
	Method(GetExtraSpecialistPoints);
	Method(GetNumSpecialistsInBuilding);
	Method(GetNumForcedSpecialistsInBuilding);
	Method(DoReallocateCitizens);
	Method(DoVerifyWorkingPlots);
	Method(IsNoAutoAssignSpecialists);

	Method(GetFocusType);
	Method(SetFocusType);

	Method(IsForcedAvoidGrowth);

	Method(GetUnitCombatFreeExperience);
	Method(GetFreePromotionCount);
	Method(IsFreePromotion);
	Method(GetSpecialistFreeExperience);

	Method(UpdateStrengthValue);
	Method(GetStrengthValue);

	Method(GetDamage);
	Method(SetDamage);
	Method(ChangeDamage);
	Method(GetMaxHitPoints);
#if defined(MOD_EVENTS_CITY_BOMBARD)
	Method(GetBombardRange);
	Method(GetCityBuildingRangeStrikeModifier);
#endif
	Method(CanRangeStrike);
	Method(CanRangeStrikeNow);
	Method(CanRangeStrikeAt);
	Method(HasPerformedRangedStrikeThisTurn);
	Method(RangeCombatUnitDefense);
	Method(RangeCombatDamage);
	Method(GetAirStrikeDefenseDamage);
	Method(GetMultiAttackBonusCity);
	Method(GetRangeStrikeModifierFromEspionage);

	Method(IsWorkingPlot);
	Method(AlterWorkingPlot);
	Method(IsForcedWorkingPlot);
	Method(GetNumRealBuilding);
	Method(SetNumRealBuilding);
	Method(GetNumFreeBuilding);
	Method(SetNumFreeBuilding);
	Method(IsBuildingSellable);
	Method(GetSellBuildingRefund);
	Method(GetTotalBaseBuildingMaintenance);
	Method(GetBuildingGreatWork);
	Method(SetBuildingGreatWork);
	Method(IsHoldingGreatWork);
	Method(GetNumGreatWorksInBuilding);

	Method(ClearOrderQueue);
	Method(PushOrder);
	Method(PopOrder);
	Method(GetOrderQueueLength);

	Method(GetBuildingYieldChange);
	Method(SetBuildingYieldChange);

#if defined(MOD_BALANCE_CORE_POLICIES)
	Method(GetBuildingClassCultureChange);
	Method(GetReligionYieldRateModifier);
	Method(GetReligionBuildingYieldRateModifier);
	Method(GetYieldPerTurnFromMinors);
	Method(SetYieldPerTurnFromMinors);
#endif
	Method(GetBaseYieldRateFromNonSpecialists);
	Method(GetBuildingYieldChangeFromCorporationFranchises);
	Method(GetYieldChangeFromCorporationFranchises);
	Method(GetTradeRouteCityMod);
	Method(GetGreatWorkYieldMod);
	Method(GetActiveSpyYieldMod);
	Method(GetResourceQuantityPerXFranchises);
	Method(GetGPRateModifierPerXFranchises);
	Method(IsFranchised);
	Method(DoFranchiseAtCity);
	Method(HasOffice);
	Method(GetYieldChangeTradeRoute);
	Method(GetSpecialistYieldChange);
	Method(GetModFromWLTKD);
	Method(GetCultureModFromCarnaval);
	Method(GetModFromGoldenAge);
	Method(GetBuildingEspionageModifier);
	Method(GetBuildingGlobalEspionageModifier);

	Method(HasDiplomat);
	Method(HasSpy);
	Method(HasCounterSpy);
	Method(GetCounterSpy);

#if defined(MOD_RELIGION_CONVERSION_MODIFIERS)
	Method(GetBuildingConversionModifier);
	Method(GetBuildingGlobalConversionModifier);
#endif

	Method(GetNumCityPlots);
	Method(CanPlaceUnitHere);

	Method(GetSpecialistYield);
	Method(GetCultureFromSpecialist);

	Method(GetNumForcedWorkingPlots);

	Method(GetReligionCityRangeStrikeModifier);

	Method(AddMessage);

	Method(HasBelief);
	Method(HasBuilding);
	Method(HasBuildingClass);
	Method(HasAnyWonder);
	Method(HasWonder);
	Method(IsCivilization);
	Method(HasFeature);
	Method(HasWorkedFeature);
	Method(HasAnyNaturalWonder);
	Method(HasNaturalWonder);
	Method(HasImprovement);
	Method(HasWorkedImprovement);
	Method(HasPlotType);
	Method(HasWorkedPlotType);
	Method(HasAnyReligion);
	Method(HasReligion);
	Method(HasResource);
	Method(HasWorkedResource);
	Method(IsConnectedToCapital);
	Method(IsIndustrialConnectedToCapital);
	Method(IsConnectedTo);
	Method(GetConnectionGoldTimes100);
	Method(HasSpecialistSlot);
	Method(HasSpecialist);
	Method(HasTerrain);
	Method(HasWorkedTerrain);
	Method(HasAnyDomesticTradeRoute);
	Method(HasAnyInternationalTradeRoute);
	Method(HasTradeRouteToAnyCity);
	Method(HasTradeRouteTo);
	Method(HasTradeRouteFromAnyCity);
	Method(HasTradeRouteFrom);
	Method(IsOnFeature);
	Method(IsAdjacentToFeature);
	Method(IsWithinDistanceOfFeature);
	Method(IsWithinDistanceOfUnit);
	Method(IsWithinDistanceOfUnitClass);
	Method(IsWithinDistanceOfUnitCombatType);
	Method(IsWithinDistanceOfUnitPromotion);
	Method(IsOnImprovement);
	Method(IsAdjacentToImprovement);
	Method(IsWithinDistanceOfImprovement);
	Method(IsOnPlotType);
	Method(IsAdjacentToPlotType);
	Method(IsWithinDistanceOfPlotType);
	Method(IsOnResource);
	Method(IsAdjacentToResource);
	Method(IsWithinDistanceOfResource);
	Method(IsOnTerrain);
	Method(IsAdjacentToTerrain);
	Method(IsWithinDistanceOfTerrain);
	Method(CountNumWorkedFeature);
	Method(CountNumWorkedImprovement);
	Method(CountNumWorkedResource);
	Method(CountNumImprovement);
	Method(CountNumWorkedRiverTiles);
	Method(CountFeature);
	Method(CountWorkedFeature);
	Method(CountImprovement);
	Method(CountWorkedImprovement);
	Method(CountPlotType);
	Method(CountWorkedPlotType);
	Method(CountResource);
	Method(CountWorkedResource);
	Method(CountTerrain);
	Method(CountWorkedTerrain);
	Method(GetAdditionalFood);
	Method(SetAdditionalFood);

	Method(IsProductionRoutes);
	Method(IsFoodRoutes);

	Method(GetSappedTurns);
	Method(SetSappedTurns);
	Method(ChangeSappedTurns);

#if defined(MOD_BALANCE_CORE_EVENTS)
	Method(GetDisabledTooltip);
	Method(GetScaledEventChoiceValue);
	Method(IsCityEventChoiceActive);
	Method(DoCityEventChoice);
	Method(DoCityStartEvent);
	Method(DoCancelCityEventChoice);
	Method(GetCityEventCooldown);
	Method(SetCityEventCooldown);
	Method(GetCityEventChoiceCooldown);
	Method(SetCityEventChoiceCooldown);
	Method(IsCityEventChoiceValid);
	Method(IsCityEventChoiceValidEspionage);
#endif


#if defined(MOD_BALANCE_CORE_JFD)
	Method(IsColony);
	Method(SetColony);

	Method(GetProvinceLevel);
	Method(SetProvinceLevel);
	Method(HasProvinceLevel);

	Method(GetOrganizedCrime);
	Method(SetOrganizedCrime);
	Method(HasOrganizedCrime);

	Method(ChangeResistanceCounter);
	Method(SetResistanceCounter);
	Method(GetResistanceCounter);

	Method(ChangePlagueCounter);
	Method(SetPlagueCounter);
	Method(GetPlagueCounter);

	Method(GetPlagueTurns);
	Method(ChangePlagueTurns);
	Method(SetPlagueTurns);

	Method(GetPlagueType);
	Method(SetPlagueType);
	Method(HasPlague);

	Method(ChangeLoyaltyCounter);
	Method(SetLoyaltyCounter);
	Method(GetLoyaltyCounter);

	Method(ChangeDisloyaltyCounter);
	Method(SetDisloyaltyCounter);
	Method(GetDisloyaltyCounter);

	Method(GetLoyaltyState);
	Method(SetLoyaltyState);
	Method(HasLoyaltyState);

	Method(GetYieldModifierFromHappiness);
	Method(SetYieldModifierFromHappiness);

	Method(GetYieldModifierFromHealth);
	Method(SetYieldModifierFromHealth);

	Method(GetYieldModifierFromCrime);
	Method(SetYieldModifierFromCrime);

	Method(GetYieldModifierFromDevelopment);
	Method(SetYieldModifierFromDevelopment);

	Method(GetYieldFromHappiness);
	Method(SetYieldFromHappiness);

	Method(GetYieldFromHealth);
	Method(SetYieldFromHealth);

	Method(GetYieldFromCrime);
	Method(SetYieldFromCrime);

	Method(GetYieldFromDevelopment);
	Method(SetYieldFromDevelopment);

	Method(GetCompetitiveSpawnUnitType);
#endif
}
//------------------------------------------------------------------------------
void CvLuaCity::HandleMissingInstance(lua_State* L)
{
	DefaultHandleMissingInstance(L);
}
//------------------------------------------------------------------------------
const char* CvLuaCity::GetTypeName()
{
	return "City";
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Lua Methods
//------------------------------------------------------------------------------
//bool isNone();
int CvLuaCity::lIsNone(lua_State* L)
{
	const bool bDoesNotExist = (GetInstance(L, false) == NULL);
	lua_pushboolean(L, bDoesNotExist);

	return 1;
}
//------------------------------------------------------------------------------
//void kill();
int CvLuaCity::lKill(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	pkCity->kill();

	return 1;
}
//------------------------------------------------------------------------------
//void CreateGreatGeneral(UnitTypes eGreatPersonUnit);
int CvLuaCity::lCreateGreatGeneral(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::createGreatGeneral);
}
//------------------------------------------------------------------------------
//void CreateGreatAdmiral(UnitTypes eGreatPersonUnit);
int CvLuaCity::lCreateGreatAdmiral(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::createGreatAdmiral);
}
//------------------------------------------------------------------------------
//void doTask(TaskTypes eTask, int iData1, int iData2, bool bOption);
int CvLuaCity::lDoTask(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const TaskTypes eTask = (TaskTypes)lua_tointeger(L, 2);
	const int iData1 = luaL_optint(L, 3, -1);
	const int iData2 = luaL_optint(L, 4, -1);
	const bool bOption = luaL_optint(L, 5, 0);
	pkCity->doTask(eTask, iData1, iData2, bOption);

	return 1;
}
//------------------------------------------------------------------------------
//void chooseProduction(UnitTypes eTrainUnit, BuildingTypes eConstructBuilding, ProjectTypes eCreateProject, bool bFinish, bool bFront);
int CvLuaCity::lChooseProduction(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	pkCity->chooseProduction();

	return 1;
}
//------------------------------------------------------------------------------
//int getCityPlotIndex(CvPlot* pPlot);
int CvLuaCity::lGetCityPlotIndex(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	CvPlot* pkPlot = CvLuaPlot::GetInstance(L, 2);
	const int iResult = pkCity->GetCityCitizens()->GetCityIndexFromPlot(pkPlot);

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//CyPlot* getCityIndexPlot(int iIndex);
int CvLuaCity::lGetCityIndexPlot(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iIndex = lua_tointeger(L, 2);

	CvPlot* pkPlot = pkCity->GetCityCitizens()->GetCityPlotFromIndex(iIndex);
	CvLuaPlot::Push(L, pkPlot);
	return 1;
}
//------------------------------------------------------------------------------
//bool canWork(CvPlot* pPlot);
int CvLuaCity::lCanWork(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	CvPlot* pkPlot = CvLuaPlot::GetInstance(L, 2);
	const bool bResult = pkCity->GetCityCitizens()->IsCanWork(pkPlot);

	lua_pushboolean(L, bResult);
	return 1;
}
//------------------------------------------------------------------------------
//bool IsBlockaded(CvPlot* pPlot);
int CvLuaCity::lIsPlotBlockaded(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	CvPlot* pkPlot = CvLuaPlot::GetInstance(L, 2);
	const bool bResult = pkPlot->isBlockaded(pkCity->getOwner());

	lua_pushboolean(L, bResult);
	return 1;
}
//------------------------------------------------------------------------------
//bool IsBlockadedTest(CvPlot* pPlot);
int CvLuaCity::lIsBlockadedTest(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const bool bResult = pkCity->IsBlockadedWaterAndLand();

	lua_pushboolean(L, bResult);
	return 1;
}
//------------------------------------------------------------------------------
//void clearWorkingOverride(int iIndex);
int CvLuaCity::lClearWorkingOverride(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::clearWorkingOverride);
}
//------------------------------------------------------------------------------
//int countNumImprovedPlots();
int CvLuaCity::lCountNumImprovedPlots(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = pkCity->countNumImprovedPlots();

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int countNumWaterPlots();
int CvLuaCity::lCountNumWaterPlots(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = pkCity->countNumWaterPlots();

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int countNumRiverPlots();
int CvLuaCity::lCountNumRiverPlots(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = pkCity->countNumRiverPlots();

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int findPopulationRank();
int CvLuaCity::lFindPopulationRank(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = pkCity->findPopulationRank();

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int findBaseYieldRateRank(YieldTypes eYield);
int CvLuaCity::lFindBaseYieldRateRank(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const YieldTypes eYield = (YieldTypes)lua_tointeger(L, 2);
	const int iResult = pkCity->findBaseYieldRateRank(eYield);

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int findYieldRateRank(YieldTypes eYield);
int CvLuaCity::lFindYieldRateRank(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const YieldTypes eYield = (YieldTypes)lua_tointeger(L, 2);
	const int iResult = pkCity->findYieldRateRank(eYield);

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//UnitTypes allUpgradesAvailable(UnitTypes eUnit, int iUpgradeCount = 0);
int CvLuaCity::lAllUpgradesAvailable(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const UnitTypes eUnit = (UnitTypes)lua_tointeger(L, 2);
	const int iUpgradeCount = luaL_optint(L, 3, 0);

	const UnitTypes eResult = pkCity->allUpgradesAvailable(eUnit, iUpgradeCount);
	lua_pushinteger(L, eResult);
	return 1;
}
//------------------------------------------------------------------------------
//bool isWorldWondersMaxed();
int CvLuaCity::lIsWorldWondersMaxed(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const bool bResult = pkCity->isWorldWondersMaxed();

	lua_pushboolean(L, bResult);
	return 1;
}
//------------------------------------------------------------------------------
//bool isTeamWondersMaxed();
int CvLuaCity::lIsTeamWondersMaxed(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const bool bResult = pkCity->isTeamWondersMaxed();

	lua_pushboolean(L, bResult);
	return 1;
}
//------------------------------------------------------------------------------
//bool isNationalWondersMaxed();
int CvLuaCity::lIsNationalWondersMaxed(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const bool bResult = pkCity->isNationalWondersMaxed();

	lua_pushboolean(L, bResult);
	return 1;
}
//------------------------------------------------------------------------------
//bool isBuildingsMaxed();
int CvLuaCity::lIsBuildingsMaxed(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const bool bResult = pkCity->isBuildingsMaxed();

	lua_pushboolean(L, bResult);
	return 1;
}
//------------------------------------------------------------------------------
int CvLuaCity::lCanTrainTooltip(lua_State* L)
{
	CvString toolTip;
	CvCity* pkCity = GetInstance(L);
	const UnitTypes eUnit = (UnitTypes) lua_tointeger(L, 2);

	// City Production Modifier
	pkCity->canTrain(eUnit, false, false, false, false, &toolTip);

	lua_pushstring(L, toolTip.c_str());
	return 1;
}
//------------------------------------------------------------------------------
//bool canTrain( int iUnit, bool bContinue, bool bTestVisible, bool bIgnoreCost, bool bWillPurchase);
int CvLuaCity::lCanTrain(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iUnit = lua_tointeger(L, 2);
	const bool bContinue = luaL_optint(L, 3, 0);
	const bool bTestVisible = luaL_optint(L, 4, 0);
	const bool bIgnoreCost = luaL_optint(L, 5, 0);
	const bool bWillPurchase = luaL_optint(L, 6, 0);
	const bool bResult = pkCity->canTrain((UnitTypes)iUnit, bContinue, bTestVisible, bIgnoreCost, bWillPurchase);

	lua_pushboolean(L, bResult);
	return 1;
}
//------------------------------------------------------------------------------
int CvLuaCity::lCanConstructTooltip(lua_State* L)
{
	CvString toolTip;
	CvCity* pkCity = GetInstance(L);
	const BuildingTypes eBuilding = (BuildingTypes) lua_tointeger(L, 2);

	// City Production Modifier
	pkCity->canConstruct(eBuilding, false, false, false, false, &toolTip);

	lua_pushstring(L, toolTip.c_str());
	return 1;
}
//------------------------------------------------------------------------------
int CvLuaCity::lCanCreateTooltip(lua_State* L)
{
	CvString toolTip;
	CvCity* pkCity = GetInstance(L);
	const ProjectTypes eProject = (ProjectTypes)lua_tointeger(L, 2);

	// City Production Modifier
	pkCity->canCreate(eProject, false, false, &toolTip);

	lua_pushstring(L, toolTip.c_str());
	return 1;
}
//------------------------------------------------------------------------------
//bool canConstruct( int iBuilding, bool bContinue, bool bTestVisible, bool bIgnoreCost);
int CvLuaCity::lCanConstruct(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iBuilding = lua_tointeger(L, 2);
	const bool bContinue = luaL_optint(L, 3, 0);
	const bool bTestVisible = luaL_optint(L, 4, 0);
	const bool bIgnoreCost = luaL_optint(L, 5, 0);
	const bool bWillPurchase = luaL_optint(L, 6, 0);
	const bool bResult = pkCity->canConstruct((BuildingTypes)iBuilding, bContinue, bTestVisible, bIgnoreCost, bWillPurchase);

	lua_pushboolean(L, bResult);
	return 1;
}
//------------------------------------------------------------------------------
//bool canCreate( int iProject, bool bContinue, bool bTestVisible );
int CvLuaCity::lCanCreate(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iProject = lua_tointeger(L, 2);
	const bool bContinue = luaL_optint(L, 3, 0);
	const bool bTestVisible = luaL_optint(L, 4, 0);
	const bool bResult = pkCity->canCreate((ProjectTypes)iProject, bContinue, bTestVisible);

	lua_pushboolean(L, bResult);
	return 1;
}
//------------------------------------------------------------------------------
//bool canMaintain( int iProcess, bool bContinue );
int CvLuaCity::lCanMaintain(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iProcess = lua_tointeger(L, 2);
	const bool bContinue = luaL_optint(L, 3, 0);
	const bool bResult = pkCity->canMaintain((ProcessTypes)iProcess, bContinue);

	lua_pushboolean(L, bResult);
	return 1;
}
//------------------------------------------------------------------------------
int CvLuaCity::lGetPurchaseUnitTooltip(lua_State* L)
{
	CvString toolTip;
	CvCity* pkCity = GetInstance(L);
	const UnitTypes eUnit = (UnitTypes) lua_tointeger(L, 2);

	// City Production Modifier
	pkCity->canTrain(eUnit, false, false, false, true, &toolTip);

	int iMaxSupplyPenalty = /*70*/ GD_INT_GET(MAX_UNIT_SUPPLY_PRODMOD);
	int iSupplyPenaltyPerUnit = /*10 in CP, 5 in VP*/ GD_INT_GET(PRODUCTION_PENALTY_PER_UNIT_OVER_SUPPLY);
	int iMaxUnitsOverSupply = (iMaxSupplyPenalty > 0 && iSupplyPenaltyPerUnit > 0) ? iMaxSupplyPenalty / iSupplyPenaltyPerUnit : INT_MAX;

	if (MOD_BALANCE_VP && GET_PLAYER(pkCity->getOwner()).GetNumUnitsOutOfSupply() >= iMaxUnitsOverSupply)
	{
		Localization::String localizedText = Localization::Lookup("TXT_KEY_NO_ACTION_NO_SUPPLY_PURCHASE");

		const char* const localized = localizedText.toUTF8();
		if(localized)
		{
			if(!toolTip.IsEmpty())
				toolTip += "[NEWLINE]";

			toolTip += localized;
		}
	}

	if (GC.getUnitInfo(eUnit)->GetCombat() > 0 || GC.getUnitInfo(eUnit)->GetRangedCombat() > 0)
	{
		if (pkCity->GetUnitPurchaseCooldown() > 0)
		{
			Localization::String localizedText = Localization::Lookup("TXT_KEY_COOLDOWN_X_TURNS_REMAINING");
			localizedText << pkCity->GetUnitPurchaseCooldown();

			const char* const localized = localizedText.toUTF8();
			if (localized)
			{
				if (!toolTip.IsEmpty())
					toolTip += "[NEWLINE]";

				toolTip += localized;
			}
		}
	}
	else if(pkCity->GetUnitPurchaseCooldown(true) > 0)
	{
		Localization::String localizedText = Localization::Lookup("TXT_KEY_COOLDOWN_X_TURNS_REMAINING");
		localizedText << pkCity->GetUnitPurchaseCooldown(true);

		const char* const localized = localizedText.toUTF8();
		if (localized)
		{
			if (!toolTip.IsEmpty())
				toolTip += "[NEWLINE]";

			toolTip += localized;
		}
	}

	if(eUnit != NO_UNIT)
	{
		CvUnitEntry* thisUnitInfo = GC.getUnitInfo(eUnit);
		// See if there are any BuildingClass requirements
		const int iNumBuildingClassInfos = GC.getNumBuildingClassInfos();
		for(int iBuildingClassLoop = 0; iBuildingClassLoop < iNumBuildingClassInfos; iBuildingClassLoop++)
		{
			const BuildingClassTypes eBuildingClass = (BuildingClassTypes) iBuildingClassLoop;

			// Requires Building
			if(thisUnitInfo->GetBuildingClassPurchaseRequireds(eBuildingClass))
			{
				BuildingTypes ePrereqBuilding = pkCity->GetBuildingTypeFromClass(eBuildingClass, true);
				if (ePrereqBuilding != NO_BUILDING && !pkCity->HasBuilding(ePrereqBuilding))
				{
					CvBuildingEntry* pkBuildingInfo = GC.getBuildingInfo(ePrereqBuilding);
					if(pkBuildingInfo)
					{
						Localization::String localizedText = Localization::Lookup("TXT_KEY_NO_ACTION_UNIT_REQUIRES_BUILDING_PURCHASE");
						localizedText << pkBuildingInfo->GetDescriptionKey();

						const char* const localized = localizedText.toUTF8();
						if(localized)
						{
							if(!toolTip.IsEmpty())
								toolTip += "[NEWLINE]";

							toolTip += localized;
						}
					}
				}
			}
		}

		//Have we already invested here?
		const UnitClassTypes eUnitClass = (UnitClassTypes)thisUnitInfo->GetUnitClassType();
		if (pkCity->IsUnitInvestment(eUnitClass))
		{
			int iValue = 100 * pkCity->GetUnitCostInvestmentReduction(eUnitClass) / pkCity->getProductionNeeded(eUnit, true);

			Localization::String localizedText = Localization::Lookup("TXT_KEY_ALREADY_INVESTED_UNIT");
			localizedText << iValue;

			const char* const localized = localizedText.toUTF8();
			if (localized)
			{
				if (!toolTip.IsEmpty())
					toolTip += "[NEWLINE]";

				toolTip += localized;
			}
		}
	}

	// Already a unit here
	if(!pkCity->CanPlaceUnitHere(eUnit))
	{
		Localization::String localizedText = Localization::Lookup("TXT_KEY_CANNOT_PURCHASE_UNIT_HERE");

		const char* const localized = localizedText.toUTF8();
		if(localized)
		{
			if(!toolTip.IsEmpty())
				toolTip += "[NEWLINE]";

			toolTip += localized;
		}
	}

	// Not enough cash money
	if(pkCity->GetPurchaseCost(eUnit) > GET_PLAYER(pkCity->getOwner()).GetTreasury()->GetGold())
	{
		Localization::String localizedText = Localization::Lookup("TXT_KEY_CANNOT_PURCHASE_NO_GOLD");

		const char* const localized = localizedText.toUTF8();
		if(localized)
		{
			if(!toolTip.IsEmpty())
				toolTip += "[NEWLINE]";

			toolTip += localized;
		}
	}

	lua_pushstring(L, toolTip.c_str());
	return 1;
}
//------------------------------------------------------------------------------
int CvLuaCity::lGetFaithPurchaseUnitTooltip(lua_State* L)
{
	CvString toolTip;
	CvCity* pkCity = GetInstance(L);
	const UnitTypes eUnit = (UnitTypes) lua_tointeger(L, 2);

	// Already a unit here
	if(!pkCity->CanPlaceUnitHere(eUnit))
	{
		Localization::String localizedText = Localization::Lookup("TXT_KEY_CANNOT_PURCHASE_UNIT_HERE");

		const char* const localized = localizedText.toUTF8();
		if(localized)
		{
			if(!toolTip.IsEmpty())
				toolTip += "[NEWLINE]";

			toolTip += localized;
		}
	}
	CvUnitEntry* pGameUnit = GC.getUnitInfo(eUnit);
	if(MOD_BALANCE_CORE && pGameUnit && GET_PLAYER(pkCity->getOwner()).GetFaithPurchaseCooldown() > 0 && pGameUnit->GetGlobalFaithCooldown() > 0)
	{
		Localization::String localizedText = Localization::Lookup("TXT_KEY_COOLDOWN_X_TURNS_REMAINING_FAITH");
		localizedText << GET_PLAYER(pkCity->getOwner()).GetFaithPurchaseCooldown();

		const char* const localized = localizedText.toUTF8();
		if(localized)
		{
			if(!toolTip.IsEmpty())
				toolTip += "[NEWLINE]";

			toolTip += localized;
		}
	}
#if defined(MOD_BALANCE_CORE)
	// Local faith purchase cooldown for combat units
	if (GC.getUnitInfo(eUnit)->GetCombat() > 0 || GC.getUnitInfo(eUnit)->GetRangedCombat() > 0)
	{
		if (pkCity->GetUnitFaithPurchaseCooldown() > 0)
		{
			Localization::String localizedText = Localization::Lookup("TXT_KEY_COOLDOWN_X_TURNS_REMAINING_FAITH_LOCAL");
			localizedText << pkCity->GetUnitFaithPurchaseCooldown();

			const char* const localized = localizedText.toUTF8();
			if (localized)
			{
				if (!toolTip.IsEmpty())
					toolTip += "[NEWLINE]";

				toolTip += localized;
			}
		}
	}
	// Local faith purchase cooldown for civilian units
	else if (pkCity->GetUnitFaithPurchaseCooldown(true) > 0)
	{
		Localization::String localizedText = Localization::Lookup("TXT_KEY_COOLDOWN_X_TURNS_REMAINING_FAITH_LOCAL");
		localizedText << pkCity->GetUnitFaithPurchaseCooldown(true);

		const char* const localized = localizedText.toUTF8();
		if (localized)
		{
			if (!toolTip.IsEmpty())
				toolTip += "[NEWLINE]";

			toolTip += localized;
		}
	}
#endif

	// Not enough faith
	if((pkCity->GetFaithPurchaseCost(eUnit, true) * 100) > GET_PLAYER(pkCity->getOwner()).GetFaithTimes100())
	{
		Localization::String localizedText = Localization::Lookup("TXT_KEY_CANNOT_PURCHASE_NO_FAITH");

		const char* const localized = localizedText.toUTF8();
		if(localized)
		{
			if(!toolTip.IsEmpty())
				toolTip += "[NEWLINE]";

			toolTip += localized;
		}
	}

	lua_pushstring(L, toolTip.c_str());
	return 1;
}
//------------------------------------------------------------------------------
int CvLuaCity::lGetPurchaseBuildingTooltip(lua_State* L)
{
	CvString toolTip;
	CvCity* pkCity = GetInstance(L);
	const BuildingTypes eBuilding = (BuildingTypes) lua_tointeger(L, 2);

	// City Production Modifier
	pkCity->canConstruct(eBuilding, false, false, false, false, &toolTip);

#if defined(MOD_BALANCE_CORE)
	if(MOD_BALANCE_CORE && pkCity->GetBuildingPurchaseCooldown() > 0)
	{
		Localization::String localizedText = Localization::Lookup("TXT_KEY_COOLDOWN_X_TURNS_REMAINING");
		localizedText << pkCity->GetBuildingPurchaseCooldown();

		const char* const localized = localizedText.toUTF8();
		if(localized)
		{
			if(!toolTip.IsEmpty())
				toolTip += "[NEWLINE]";

			toolTip += localized;
		}
	}
#endif

	if (MOD_BALANCE_CORE_BUILDING_INVESTMENTS && eBuilding != NO_BUILDING)
	{
		//Have we already invested here?
		CvBuildingEntry* pGameBuilding = GC.getBuildingInfo(eBuilding);
		if (pGameBuilding)
		{
			const BuildingClassTypes eBuildingClass = (BuildingClassTypes)(pGameBuilding->GetBuildingClassType());
			if (pkCity->IsBuildingInvestment(eBuildingClass))
			{
				int iValue = 100 * pkCity->GetBuildingCostInvestmentReduction(eBuildingClass) / pkCity->getProductionNeeded(eBuilding, true);
				Localization::String localizedText = Localization::Lookup("TXT_KEY_ALREADY_INVESTED");
				localizedText << iValue;

				const char* const localized = localizedText.toUTF8();
				if (localized)
				{
					if (!toolTip.IsEmpty())
						toolTip += "[NEWLINE]";

					toolTip += localized;
				}
			}
		}
	}

	// Not enough cash money
	if(pkCity->GetPurchaseCost(eBuilding) > GET_PLAYER(pkCity->getOwner()).GetTreasury()->GetGold())
	{
		Localization::String localizedText = Localization::Lookup("TXT_KEY_CANNOT_PURCHASE_NO_GOLD");

		const char* const localized = localizedText.toUTF8();
		if(localized)
		{
			if(!toolTip.IsEmpty())
				toolTip += "[NEWLINE]";

			toolTip += localized;
		}
	}
	lua_pushstring(L, toolTip.c_str());
	return 1;
}
//------------------------------------------------------------------------------
int CvLuaCity::lGetFaithPurchaseBuildingTooltip(lua_State* L)
{
	CvString toolTip;
	CvCity* pkCity = GetInstance(L);
	const BuildingTypes eBuilding = (BuildingTypes) lua_tointeger(L, 2);

	// City Production Modifier
	pkCity->canConstruct(eBuilding, false, false, false, false, &toolTip);

	// Not enough faith
	if((pkCity->GetFaithPurchaseCost(eBuilding) * 100) > GET_PLAYER(pkCity->getOwner()).GetFaithTimes100())
	{
		Localization::String localizedText = Localization::Lookup("TXT_KEY_CANNOT_PURCHASE_NO_FAITH");

		const char* const localized = localizedText.toUTF8();
		if(localized)
		{
			if(!toolTip.IsEmpty())
				toolTip += "[NEWLINE]";

			toolTip += localized;
		}
	}

	lua_pushstring(L, toolTip.c_str());
	return 1;
}
//------------------------------------------------------------------------------
//bool canJoin();
int CvLuaCity::lCanJoin(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const bool bResult = pkCity->canJoinCity();

	lua_pushboolean(L, bResult);
	return 1;
}
//------------------------------------------------------------------------------
//bool IsBuildingLocalResourceValid(BuildingTypes eBuilding, bool bCheckForImprovement);
int CvLuaCity::lIsBuildingLocalResourceValid(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);

	lua_pushboolean(L, pkCity->IsBuildingLocalResourceValid(static_cast<BuildingTypes>(lua_tointeger(L, 2)), lua_toboolean(L, 3)));
	return 1;
}

//------------------------------------------------------------------------------
//bool GetBuildingYieldRateTimes100(BuildingTypes eBuilding, YieldTypes eYield);
int CvLuaCity::lGetBuildingYieldRateTimes100(lua_State* L)
{
	CvCity* pCity = GetInstance(L);
	const BuildingTypes eBuilding = (BuildingTypes)lua_tointeger(L, 2);
	const YieldTypes eYield = (YieldTypes)lua_tointeger(L, 3);

	int iYieldTimes100 = 0;
	if (pCity && eBuilding != NO_BUILDING && eBuilding < GC.getNumBuildingInfos() && eYield != NO_YIELD && eYield < NUM_YIELD_TYPES)
	{
		PlayerTypes ePlayer = pCity->getOwner();
		CvPlayer* pPlayer = &GET_PLAYER(ePlayer);
		CvBuildingEntry* pBuildingInfo = GC.getBuildingInfo(eBuilding);
		BuildingClassTypes eBuildingClass = pBuildingInfo->GetBuildingClassType();
		CvBuildingClassInfo* pBuildingClassInfo = GC.getBuildingClassInfo(eBuildingClass);

		iYieldTimes100 += pBuildingInfo->GetYieldChange(eYield) * 100;

		if (!pBuildingInfo->GetTechEnhancedYields().empty())
		{
			map<int, std::map<int, int>> mTechEnhancedYields = pBuildingInfo->GetTechEnhancedYields();
			map<int, std::map<int, int>>::iterator it;
			for (it = mTechEnhancedYields.begin(); it != mTechEnhancedYields.end(); it++)
			{
				if (GET_TEAM(pPlayer->getTeam()).GetTeamTechs()->HasTech((TechTypes)it->first))
				{
					std::map<int, int>::const_iterator it2 = (it->second).find(eYield);
					if (it2 != (it->second).end())
					{
						iYieldTimes100 += it2->second * 100;
					}
				}
			}
		}

		if (!pBuildingInfo->GetYieldChangesFromAccomplishments().empty())
		{
			map<int, std::map<int, int>> mYieldsFromAccomplishments = pBuildingInfo->GetYieldChangesFromAccomplishments();
			map<int, std::map<int, int>>::iterator it;
			for (it = mYieldsFromAccomplishments.begin(); it != mYieldsFromAccomplishments.end(); it++)
			{
				if (pPlayer->GetNumTimesAccomplishmentCompleted((AccomplishmentTypes)it->first) > 0)
				{
					std::map<int, int>::const_iterator it2 = (it->second).find(eYield);
					if (it2 != (it->second).end())
					{
						iYieldTimes100 += it2->second * pPlayer->GetNumTimesAccomplishmentCompleted((AccomplishmentTypes)it->first) * 100;
					}
				}
			}
		}

		iYieldTimes100 += pPlayer->GetBuildingClassYieldChange(eBuildingClass, eYield) * 100;
		iYieldTimes100 += pPlayer->GetPlayerTraits()->GetBuildingClassYieldChange(eBuildingClass, eYield) * 100;

		ReligionTypes eMajority = pCity->GetCityReligions()->GetReligiousMajority();
		BeliefTypes eSecondaryPantheon = NO_BELIEF;
		ReligionTypes ePlayerReligion = GET_PLAYER(ePlayer).GetReligions()->GetStateReligion();
		if (ePlayerReligion != NO_RELIGION && eMajority == ePlayerReligion)
		{
			iYieldTimes100 += pCity->getReligionBuildingYieldRateModifier((BuildingClassTypes)eBuildingClass, (YieldTypes)eYield) * 100;
		}
		if (eMajority != NO_RELIGION)
		{
			const CvReligion* pReligion = GC.getGame().GetGameReligions()->GetReligion(eMajority, ePlayer);
			if (pReligion)
			{
				if (eYield == YIELD_TOURISM)
				{
					int iFaithBuildingTourism = pReligion->m_Beliefs.GetFaithBuildingTourism(ePlayer, pCity);
					if (iFaithBuildingTourism != 0 && pBuildingInfo->IsFaithPurchaseOnly())
					{
						iYieldTimes100 += iFaithBuildingTourism * 100;
					}
				}
				int iFollowers = pCity->GetCityReligions()->GetNumFollowers(eMajority);
				iYieldTimes100 += pReligion->m_Beliefs.GetBuildingClassYieldChange(eBuildingClass, eYield, iFollowers, ePlayer, pCity) * 100;
				if (::isWorldWonderClass(*pBuildingClassInfo))
				{
					iYieldTimes100 += pReligion->m_Beliefs.GetYieldChangeWorldWonder(eYield, ePlayer, pCity) * 100;
				}
				eSecondaryPantheon = pCity->GetCityReligions()->GetSecondaryReligionPantheonBelief();
				if (eSecondaryPantheon != NO_BELIEF)
				{
					iFollowers = pCity->GetCityReligions()->GetNumFollowers(pCity->GetCityReligions()->GetReligionByAccumulatedPressure(1));
					if (iFollowers >= GC.GetGameBeliefs()->GetEntry(eSecondaryPantheon)->GetMinFollowers())
					{
						iYieldTimes100 += GC.GetGameBeliefs()->GetEntry(eSecondaryPantheon)->GetBuildingClassYieldChange(eBuildingClass, eYield) * 100;
					}
				}
			}
		}
		// Mod for civs keeping their pantheon belief forever
		if (MOD_RELIGION_PERMANENT_PANTHEON)
		{
			if (GC.getGame().GetGameReligions()->HasCreatedPantheon(ePlayer))
			{
				const CvReligion* pPantheon = GC.getGame().GetGameReligions()->GetReligion(RELIGION_PANTHEON, ePlayer);
				BeliefTypes ePantheonBelief = GC.getGame().GetGameReligions()->GetBeliefInPantheon(ePlayer);
				if (pPantheon != NULL && ePantheonBelief != NO_BELIEF && ePantheonBelief != eSecondaryPantheon)
				{
					const CvReligion* pReligion = GC.getGame().GetGameReligions()->GetReligion(eMajority, ePlayer);
					if (pReligion == NULL || (pReligion != NULL && !pReligion->m_Beliefs.IsPantheonBeliefInReligion(ePantheonBelief, eMajority, ePlayer))) // check that the our religion does not have our belief, to prevent double counting
					{
						iYieldTimes100 += GC.GetGameBeliefs()->GetEntry(ePantheonBelief)->GetBuildingClassYieldChange(eBuildingClass, eYield) * 100;
					}
				}
			}
		}
		if (::isWorldWonderClass(*pBuildingClassInfo))
		{
			iYieldTimes100 += GC.getGame().GetGameLeagues()->GetWorldWonderYieldChange(ePlayer, eYield) * 100;
			iYieldTimes100 += pPlayer->GetPlayerTraits()->GetYieldChangeWorldWonder(eYield) * 100;
			for (int iPolicyLoop = 0; iPolicyLoop < GC.getNumPolicyInfos(); iPolicyLoop++)
			{
				const PolicyTypes ePolicy = static_cast<PolicyTypes>(iPolicyLoop);
				CvPolicyEntry* pkPolicyInfo = GC.getPolicyInfo(ePolicy);
				if (pkPolicyInfo)
				{
					if (pPlayer->GetPlayerPolicies()->HasPolicy(ePolicy) && !pPlayer->GetPlayerPolicies()->IsPolicyBlocked(ePolicy))
					{
						iYieldTimes100 += pkPolicyInfo->GetYieldChangeWorldWonder(eYield) * 100;
					}
				}
			}
		}

		iYieldTimes100 += pCity->getLocalBuildingClassYield(eBuildingClass, eYield) * 100;
		iYieldTimes100 += pCity->GetBuildingYieldChangeFromCorporationFranchises(eBuildingClass, eYield) * 100;
		iYieldTimes100 += pPlayer->GetPlayerPolicies()->GetBuildingClassYieldChange(eBuildingClass, eYield) * 100;

		iYieldTimes100 += pBuildingInfo->GetYieldChangePerPop(eYield) * pCity->getPopulation();
		iYieldTimes100 += pBuildingInfo->GetYieldChangePerPopInEmpire(eYield) * pPlayer->getTotalPopulation();
		iYieldTimes100 += (pBuildingInfo->GetYieldChangePerBuilding(eYield) * pCity->GetCityBuildings()->GetNumBuildings() * 100).Truncate();
		iYieldTimes100 += (pBuildingInfo->GetYieldChangePerTile(eYield) * pCity->GetPlotList().size() * 100).Truncate();
		iYieldTimes100 += pBuildingInfo->GetYieldChangeFromPassingTR(eYield) * pCity->plot()->GetNumTradeUnitRoute() * 100;
		iYieldTimes100 += (pBuildingInfo->GetYieldChangePerCityStateStrategicResource(eYield) * pPlayer->GetNumStrategicResourcesFromMinors() * 100).Truncate();
		iYieldTimes100 += pCity->GetEventBuildingClassCityYield(eBuildingClass, eYield) * 100;
		iYieldTimes100 += pBuildingInfo->GetYieldChangeEraScalingTimes100(eYield) * max(1, (int)pPlayer->GetCurrentEra());

		iYieldTimes100 += pBuildingInfo->GetYieldPerFriend(eYield) * pPlayer->GetNumCSFriends() * 100;
		iYieldTimes100 += pBuildingInfo->GetYieldPerAlly(eYield) * pPlayer->GetNumCSAllies() * 100;
		iYieldTimes100 += pBuildingInfo->GetYieldChangePerMonopoly(eYield) * pPlayer->GetNumGlobalMonopolies() * 100;
	}

	lua_pushinteger(L, iYieldTimes100);
	return 1;
}

//------------------------------------------------------------------------------
//bool GetBuildingYieldModifier(BuildingTypes eBuilding, YieldTypes eYield);
int CvLuaCity::lGetBuildingYieldModifier(lua_State* L)
{
	CvCity* pCity = GetInstance(L);
	const BuildingTypes eBuilding = (BuildingTypes)lua_tointeger(L, 2);
	const YieldTypes eYield = (YieldTypes)lua_tointeger(L, 3);

	int iModifier = 0;
	if (pCity && eBuilding != NO_BUILDING && eBuilding < GC.getNumBuildingInfos() && eYield != NO_YIELD && eYield < NUM_YIELD_TYPES)
	{
		PlayerTypes ePlayer = pCity->getOwner();
		CvPlayer* pPlayer = &GET_PLAYER(ePlayer);
		CvBuildingEntry* pBuildingInfo = GC.getBuildingInfo(eBuilding);
		BuildingClassTypes eBuildingClass = pBuildingInfo->GetBuildingClassType();

		iModifier += pBuildingInfo->GetYieldModifier(eYield);
		iModifier += pPlayer->GetPlayerPolicies()->GetBuildingClassYieldModifier(eBuildingClass, eYield);
		iModifier += pCity->GetEventBuildingClassCityYieldModifier(eBuildingClass, eYield);
	}

	lua_pushinteger(L, iModifier);
	return 1;
}

int CvLuaCity::lGetPlotsBoostedByBuilding(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const BuildingTypes eBuildingType = (BuildingTypes)lua_tointeger(L, 2);

	std::vector<int> aiPlotList;
	pkCity->GetPlotsBoostedByBuilding(aiPlotList, eBuildingType);

	int iReturnValues = 0;

	for (uint ui = 0; ui < aiPlotList.size(); ui++)
	{
		if (aiPlotList[ui] >= 0)
		{
			CvPlot* pkPlot = GC.getMap().plotByIndex(aiPlotList[ui]);
			CvLuaPlot::Push(L, pkPlot);
			iReturnValues++;
		}
		else
		{
			break;
		}
	}

	return iReturnValues;
}
int CvLuaCity::lSetBuildingHidden(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::SetBuildingHidden);
}
int CvLuaCity::lClearHiddenBuildings(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::ClearHiddenBuildings);
}
int CvLuaCity::lIsBuildingHidden(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::IsBuildingHidden);
}
int CvLuaCity::lGetNumHiddenBuildings(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::GetNumHiddenBuildings);
}
//------------------------------------------------------------------------------
//bool GetResourceDemanded();
int CvLuaCity::lGetResourceDemanded(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::GetResourceDemanded);
}
//------------------------------------------------------------------------------
//bool SetResourceDemanded(ResourceTypes eResource);
int CvLuaCity::lSetResourceDemanded(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::SetResourceDemanded);
}
//------------------------------------------------------------------------------
//bool DoPickResourceDemanded();
int CvLuaCity::lDoPickResourceDemanded(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::DoPickResourceDemanded);
}

//------------------------------------------------------------------------------
//int getFoodTurnsLeft();
int CvLuaCity::lGetFoodTurnsLeft(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getFoodTurnsLeft);

}
//------------------------------------------------------------------------------
//bool isProduction();
int CvLuaCity::lIsProduction(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::isProduction);
}
//------------------------------------------------------------------------------
//bool isProductionLimited();
int CvLuaCity::lIsProductionLimited(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::isProductionLimited);
}
//------------------------------------------------------------------------------
//bool isProductionUnit();
int CvLuaCity::lIsProductionUnit(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::isProductionUnit);
}
//------------------------------------------------------------------------------
//bool isProductionBuilding();
int CvLuaCity::lIsProductionBuilding(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::isProductionBuilding);
}
//------------------------------------------------------------------------------
//bool isProductionProject();
int CvLuaCity::lIsProductionProject(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::isProductionProject);
}
//------------------------------------------------------------------------------
//bool isProductionProcess();
int CvLuaCity::lIsProductionProcess(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::isProductionProcess);
}
//------------------------------------------------------------------------------
//bool canContinueProduction(OrderTypes eOrderType, int iData1, int iData2, bool bSave)
int CvLuaCity::lCanContinueProduction(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	OrderData order;
	order.eOrderType = (OrderTypes)lua_tointeger(L, 2);
	order.iData1 = lua_tointeger(L, 3);
	order.iData2 = lua_tointeger(L, 4);
	order.bSave = lua_toboolean(L, 5);

	const bool bResult = pkCity->canContinueProduction(order);

	lua_pushboolean(L, bResult);
	return 1;
}
//------------------------------------------------------------------------------
//int getProductionExperience(UnitTypes eUnit);
int CvLuaCity::lGetProductionExperience(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getProductionExperience);
}
//------------------------------------------------------------------------------
//void addProductionExperience(CvUnit* pUnit, bool bHalveXP = false);
int CvLuaCity::lAddProductionExperience(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	CvUnit* pkUnit = CvLuaUnit::GetInstance(L, 2);
	const bool bHalveXP = luaL_optint(L, 3, 0);
	pkCity->addProductionExperience(pkUnit, bHalveXP);

	return 1;
}
//------------------------------------------------------------------------------
//UnitTypes getProductionUnit()
int CvLuaCity::lGetProductionUnit(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getProductionUnit);
}
//------------------------------------------------------------------------------
//UnitAITypes getProductionUnitAI()
int CvLuaCity::lGetProductionUnitAI(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const UnitAITypes eValue = pkCity->getProductionUnitAI();

	lua_pushinteger(L, eValue);
	return 1;
}
//------------------------------------------------------------------------------
//BuildingTypes getProductionBuilding()
int CvLuaCity::lGetProductionBuilding(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const BuildingTypes eValue = pkCity->getProductionBuilding();

	lua_pushinteger(L, eValue);
	return 1;
}
//------------------------------------------------------------------------------
//ProjectTypes getProductionProject()
int CvLuaCity::lGetProductionProject(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const ProjectTypes eValue = pkCity->getProductionProject();

	lua_pushinteger(L, eValue);
	return 1;
}
//------------------------------------------------------------------------------
//ProcessTypes getProductionProcess()
int CvLuaCity::lGetProductionProcess(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getProductionProcess);
}
//------------------------------------------------------------------------------
//std::string getProductionName();
//------------------------------------------------------------------------------
//string getProductionNameKey();
int CvLuaCity::lGetProductionNameKey(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushstring(L, pkCity->getProductionNameKey());
	return 1;
}
//------------------------------------------------------------------------------
//int getGeneralProductionTurnsLeft();
int CvLuaCity::lGetGeneralProductionTurnsLeft(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getGeneralProductionTurnsLeft);
}
//------------------------------------------------------------------------------
//bool isFoodProduction();
int CvLuaCity::lIsFoodProduction(lua_State* L)
{
	return BasicLuaMethod<bool>(L, &CvCity::isFoodProduction);
}
//------------------------------------------------------------------------------
//int getFirstUnitOrder(UnitTypes eUnit);
int CvLuaCity::lGetFirstUnitOrder(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getFirstUnitOrder);
}
//------------------------------------------------------------------------------
//int getFirstProjectOrder(ProjectTypes eProject);
int CvLuaCity::lGetFirstProjectOrder(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getFirstProjectOrder);
}
//------------------------------------------------------------------------------
//int getNumTrainUnitAI(UnitAITypes eUnitAI);
int CvLuaCity::lGetNumTrainUnitAI(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getNumTrainUnitAI);
}
//------------------------------------------------------------------------------
//int getFirstBuildingOrder(BuildingTypes eBuilding);
int CvLuaCity::lGetFirstBuildingOrder(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getFirstBuildingOrder);
}
//------------------------------------------------------------------------------
//bool isUnitFoodProduction(UnitTypes iUnit);
int CvLuaCity::lIsUnitFoodProduction(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const UnitTypes eUnitType = (UnitTypes) lua_tointeger(L, 2);
	PlayerTypes eOwner = pkCity->getOwner();

	const int iResult = isUnitTypeFoodProduction(eOwner,eUnitType);
	lua_pushboolean(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int getProduction();
int CvLuaCity::lGetProduction(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getProduction);
}
//------------------------------------------------------------------------------
//int getProductionTimes100();
int CvLuaCity::lGetProductionTimes100(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getProductionTimes100);
}
//------------------------------------------------------------------------------
//int getProductionNeeded();
int CvLuaCity::lGetProductionNeeded(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = pkCity->getProductionNeeded();

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int GetUnitProductionNeeded();
int CvLuaCity::lGetUnitProductionNeeded(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const UnitTypes eUnitType = (UnitTypes) lua_tointeger(L, 2);

	const int iResult = pkCity->getProductionNeeded(eUnitType);

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int GetBuildingProductionNeeded();
int CvLuaCity::lGetBuildingProductionNeeded(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const BuildingTypes eBuildingType = (BuildingTypes) lua_tointeger(L, 2);

	const int iResult = pkCity->getProductionNeeded(eBuildingType);

	lua_pushinteger(L, iResult);
	return 1;
}

//------------------------------------------------------------------------------
//bool IsBuildingInvestment();
int CvLuaCity::lGetBuildingInvestment(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	int iResult = 0;
	const BuildingTypes eBuildingType = (BuildingTypes) lua_tointeger(L, 2);
	CvBuildingEntry* pGameBuilding = GC.getBuildingInfo(eBuildingType);
	if (pGameBuilding)
	{
		const BuildingClassTypes eBuildingClass = (BuildingClassTypes)(pGameBuilding->GetBuildingClassType());
		if (pkCity->IsBuildingInvestment(eBuildingClass))
		{
			iResult = pkCity->getProductionNeeded(eBuildingType, true) - pkCity->GetBuildingCostInvestmentReduction(eBuildingClass);
		}
	}

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//bool IsWorldWonder();
int CvLuaCity::lIsWorldWonder(lua_State* L)
{
	bool bResult = false;
	const BuildingTypes eBuildingType = (BuildingTypes)lua_tointeger(L, 2);
	CvBuildingEntry* pGameBuilding = GC.getBuildingInfo(eBuildingType);
	if (pGameBuilding)
	{
		const CvBuildingClassInfo& kBuildingClassInfo = pGameBuilding->GetBuildingClassInfo();
		if (::isWorldWonderClass(kBuildingClassInfo))
		{
			bResult = true;
		}
	}

	lua_pushboolean(L, bResult);
	return 1;
}
int CvLuaCity::lGetWorldWonderCost(lua_State* L)
{
	int iNumWorldWonderPercent = 0;
	CvCity* pkCity = GetInstance(L);

	const BuildingTypes eBuildingType = (BuildingTypes)lua_tointeger(L, 2);
	CvBuildingEntry* pGameBuilding = GC.getBuildingInfo(eBuildingType);
	if (MOD_BALANCE_CORE_WONDER_COST_INCREASE && pGameBuilding)
	{
		const CvBuildingClassInfo& kBuildingClassInfo = pGameBuilding->GetBuildingClassInfo();
		if (::isWorldWonderClass(kBuildingClassInfo)) {
			const CvCity* pLoopCity;
			int iLoop;
			for (pLoopCity = GET_PLAYER(pkCity->getOwner()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(pkCity->getOwner()).nextCity(&iLoop))
			{
				if (pLoopCity->getNumWorldWonders() > 0)
				{
					for (int iBuildingLoop = 0; iBuildingLoop < GC.getNumBuildingInfos(); iBuildingLoop++)
					{
						const BuildingTypes eBuilding = static_cast<BuildingTypes>(iBuildingLoop);
						CvBuildingEntry* pkeBuildingInfo = GC.getBuildingInfo(eBuilding);

						// Has this Building
						if (pkeBuildingInfo && pLoopCity->GetCityBuildings()->GetNumBuilding(eBuilding) > 0)
						{
							if (isWorldWonderClass(pkeBuildingInfo->GetBuildingClassInfo()))
							{
								if (pkeBuildingInfo->GetPrereqAndTech() == NO_TECH)
									continue;

								CvTechEntry* pkTechInfo = GC.getTechInfo((TechTypes)pkeBuildingInfo->GetPrereqAndTech());
								if (pkTechInfo)
								{
									// Loop through all eras and apply Building production mod based on how much time has passed
									EraTypes eBuildingUnlockedEra = (EraTypes)pkTechInfo->GetEra();

									if (eBuildingUnlockedEra == NO_ERA)
										continue;

									int iEraDifference = GET_PLAYER(pkCity->getOwner()).GetCurrentEra() - eBuildingUnlockedEra;
									switch (iEraDifference)
									{
									case 0:
										iNumWorldWonderPercent += /*25*/ GD_INT_GET(BALANCE_CORE_WORLD_WONDER_SAME_ERA_COST_MODIFIER);
										break;
									case 1:
										iNumWorldWonderPercent += /*15*/ GD_INT_GET(BALANCE_CORE_WORLD_WONDER_PREVIOUS_ERA_COST_MODIFIER);
										break;
									case 2:
										iNumWorldWonderPercent += /*10*/ GD_INT_GET(BALANCE_CORE_WORLD_WONDER_SECOND_PREVIOUS_ERA_COST_MODIFIER);
										break;
									default:
										iNumWorldWonderPercent += /*5*/ GD_INT_GET(BALANCE_CORE_WORLD_WONDER_EARLIER_ERA_COST_MODIFIER);
									}
								}
							}
						}
					}
				}
			}
		}
	}

	lua_pushinteger(L, iNumWorldWonderPercent);
	return 1;
}
int CvLuaCity::lGetNumPoliciesNeeded(lua_State* L)
{
	int iNumPoliciesReduced = 0;
	int iTotalPoliciesNeeded = 0;
	CvCity* pkCity = GetInstance(L);

	const BuildingTypes eBuilding = (BuildingTypes)lua_tointeger(L, 2);
	if (eBuilding != NO_BUILDING)
	{
		CvBuildingEntry* pBuildingInfo = GC.getBuildingInfo(eBuilding);
		if (pBuildingInfo)
		{
			//If # of policies will do it, then we need to see the either/or here.
			iTotalPoliciesNeeded = pBuildingInfo->GetNumPoliciesNeeded();
			if (iTotalPoliciesNeeded > 0)
			{
				ReligionTypes eOwnedReligion = GET_PLAYER(pkCity->getOwner()).GetReligions()->GetOwnedReligion();
				if (eOwnedReligion != NO_RELIGION)
				{
					CvGameReligions* pReligions = GC.getGame().GetGameReligions();
					const CvReligion* pReligion = pReligions->GetReligion(eOwnedReligion, pkCity->getOwner());
					if (pReligion)
					{
						CvCity* pHolyCity = pReligion->GetHolyCity();
						if (pHolyCity == NULL)
						{
							pHolyCity = GET_PLAYER(pkCity->getOwner()).getCapitalCity();
						}
						// Depends on era of wonder
						EraTypes eEra;
						TechTypes eTech = (TechTypes)pBuildingInfo->GetPrereqAndTech();
						if (eTech != NO_TECH)
						{
							CvTechEntry* pEntry = GC.GetGameTechs()->GetEntry(eTech);
							if (pEntry)
							{
								eEra = (EraTypes)pEntry->GetEra();
								if (eEra != NO_ERA)
								{
									iNumPoliciesReduced += pReligion->m_Beliefs.GetIgnorePolicyRequirementsAmount(eEra, pkCity->getOwner(), pHolyCity);
								}
							}
						}
						int iReligionPolicyReduction = pReligion->m_Beliefs.GetPolicyReductionWonderXFollowerCities(pkCity->getOwner(), pHolyCity);
						if (iReligionPolicyReduction > 0)
						{
							int iNumFollowerCities = pReligions->GetNumCitiesFollowing(eOwnedReligion);
							if (iNumFollowerCities > 0)
							{
								iNumPoliciesReduced += (iNumFollowerCities / iReligionPolicyReduction);
							}
						}
					}
				}

				int iCSPolicyReduction = GET_PLAYER(pkCity->getOwner()).GetCSAlliesLowersPolicyNeedWonders();
				if (iCSPolicyReduction > 0)
				{
					int iNumAllies = GET_PLAYER(pkCity->getOwner()).GetNumCSAllies();
					iNumPoliciesReduced += (iNumAllies / iCSPolicyReduction);
				}
			}
		}
	}
	iTotalPoliciesNeeded -= iNumPoliciesReduced;

	lua_pushinteger(L, iTotalPoliciesNeeded);
	return 1;
}

//------------------------------------------------------------------------------
//bool IsUnitInvestment();
int CvLuaCity::lGetUnitInvestment(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	int iResult = 0;
	const UnitTypes eUnitType = (UnitTypes) lua_tointeger(L, 2);
	CvUnitEntry* pGameUnit = GC.getUnitInfo(eUnitType);
	const UnitClassTypes eUnitClass = (UnitClassTypes)(pGameUnit->GetUnitClassType());
	if(pkCity->IsUnitInvestment(eUnitClass))
	{
		iResult = pkCity->getProductionNeeded(eUnitType, true) - pkCity->GetUnitCostInvestmentReduction(eUnitClass);
	}

	lua_pushinteger(L, iResult);
	return 1;
}

//------------------------------------------------------------------------------
//int GetProjectProductionNeeded();
int CvLuaCity::lGetProjectProductionNeeded(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const ProjectTypes eProjectType = (ProjectTypes) lua_tointeger(L, 2);

	const int iResult = pkCity->getProductionNeeded(eProjectType);

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int getProductionTurnsLeft();
int CvLuaCity::lGetProductionTurnsLeft(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = pkCity->getProductionTurnsLeft();

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int getUnitProductionTurnsLeft(UnitTypes iUnit, int iNum);
int CvLuaCity::lGetUnitProductionTurnsLeft(lua_State* L)
{
	return BasicLuaMethod<int, UnitTypes>(L, &CvCity::getProductionTurnsLeft);
}
//------------------------------------------------------------------------------
//int getBuildingProductionTurnsLeft(BuildingTypes iBuilding, int iNum);
int CvLuaCity::lGetBuildingProductionTurnsLeft(lua_State* L)
{
	return BasicLuaMethod<int, BuildingTypes>(L, &CvCity::getProductionTurnsLeft);
}
//------------------------------------------------------------------------------
//int getProjectProductionTurnsLeft(ProjectTypes eProject, int iNum);
int CvLuaCity::lGetProjectProductionTurnsLeft(lua_State* L)
{
	return BasicLuaMethod<int, ProjectTypes>(L, &CvCity::getProductionTurnsLeft);
}
#if defined(MOD_PROCESS_STOCKPILE)
//------------------------------------------------------------------------------
//int getProcessProductionTurnsLeft(ProcessTypes eProcess, int iNum);
int CvLuaCity::lGetProcessProductionTurnsLeft(lua_State* L)
{
	return BasicLuaMethod<int, ProcessTypes>(L, &CvCity::getProductionTurnsLeft);
}
#endif
//------------------------------------------------------------------------------
// int IsCanPurchase(UnitTypes eUnitType, BuildingTypes eBuildingType, ProjectTypes eProjectType, YieldTypes ePurchaseYield);
int CvLuaCity::lIsCanPurchase(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const bool bTestPurchaseCost = lua_toboolean(L, 2);
	const bool bTestTrainable = lua_toboolean(L, 3);
	const UnitTypes eUnitType = (UnitTypes) lua_tointeger(L, 4);
	const BuildingTypes eBuildingType = (BuildingTypes) lua_tointeger(L, 5);
	const ProjectTypes eProjectType = (ProjectTypes) lua_tointeger(L, 6);
	const YieldTypes ePurchaseYield = (YieldTypes) lua_tointeger(L, 7);

	// TODO: throw error for non-gold/faith ePurchaseYield input?
	const bool bResult = pkCity->IsCanPurchase(bTestPurchaseCost, bTestTrainable, eUnitType, eBuildingType, eProjectType, ePurchaseYield);

	lua_pushboolean(L, bResult);
	return 1;
}
//------------------------------------------------------------------------------
// void Purchase(UnitTypes eUnitType, BuildingTypes eBuildingType, ProjectTypes eProjectType, YieldTypes ePurchaseYield);
int CvLuaCity::lPurchase(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const UnitTypes eUnitType = (UnitTypes) lua_tointeger(L, 2);
	const BuildingTypes eBuildingType = (BuildingTypes) lua_tointeger(L, 3);
	const ProjectTypes eProjectType = (ProjectTypes) lua_tointeger(L, 4);
	const YieldTypes ePurchaseYield = (YieldTypes) lua_tointeger(L, 5);

	// TODO: throw error for non-gold/faith ePurchaseYield input?
	pkCity->PurchaseUnit(eUnitType, ePurchaseYield);
	pkCity->PurchaseBuilding(eBuildingType, ePurchaseYield);
	pkCity->PurchaseProject(eProjectType, ePurchaseYield);

	return 0;
}
//------------------------------------------------------------------------------
// int GetPurchaseCost(UnitTypes eUnit);
int CvLuaCity::lGetUnitPurchaseCost(lua_State* L)
{
	return BasicLuaMethod<int, UnitTypes>(L, &CvCity::GetPurchaseCost);
}
//------------------------------------------------------------------------------
// int GetFaithPurchaseCost(UnitTypes eUnit, bool bIncludeBeliefDiscounts);
int CvLuaCity::lGetUnitFaithPurchaseCost(lua_State* L)
{
	return BasicLuaMethod<int, UnitTypes>(L, &CvCity::GetFaithPurchaseCost);
}
//------------------------------------------------------------------------------
//int GetPurchaseCost(BuildingTypes eBuilding);
int CvLuaCity::lGetBuildingPurchaseCost(lua_State* L)
{
	return BasicLuaMethod<int, BuildingTypes>(L, &CvCity::GetPurchaseCost);
}
//------------------------------------------------------------------------------
//int GetFaithPurchaseCost(BuildingTypes eBuilding);
int CvLuaCity::lGetBuildingFaithPurchaseCost(lua_State* L)
{
	return BasicLuaMethod<int, BuildingTypes>(L, &CvCity::GetFaithPurchaseCost);
}
//------------------------------------------------------------------------------
//int GetPurchaseCost(ProjectTypes eProject);
int CvLuaCity::lGetProjectPurchaseCost(lua_State* L)
{
	return BasicLuaMethod<int, ProjectTypes>(L, &CvCity::GetPurchaseCost);
}
//------------------------------------------------------------------------------
//void setProduction(int iNewValue);
int CvLuaCity::lSetProduction(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::setProduction);
}
//------------------------------------------------------------------------------
//void changeProduction(int iChange);
int CvLuaCity::lChangeProduction(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::changeProduction);
}
//------------------------------------------------------------------------------
// LEGACY FUNCTION, no longer functional
int CvLuaCity::lGetYieldModifierTooltip(lua_State* L)
{
	CvString toolTip = "";
	lua_pushstring(L, toolTip.c_str());
	return 1;
}
//------------------------------------------------------------------------------
int CvLuaCity::lGetYieldRateTooltip(lua_State* L)
{
	CvString toolTip;
	CvCity* pkCity = GetInstance(L);
	const YieldTypes eYield = (YieldTypes) lua_tointeger(L, 2);

	// City Yield Rate Modifier
	pkCity->getYieldRateTimes100(eYield, false, false, false, &toolTip);
	lua_pushstring(L, toolTip.c_str());
	return 1;
}
//------------------------------------------------------------------------------
//int getProductionModifier();
int CvLuaCity::lGetProductionModifier(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = pkCity->getCurrentProductionModifier();

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int GetCurrentProductionDifference(bool bIgnoreFood, bool bOverflow);
// LEGACY METHOD, use `getYieldRateTimes100(YIELD_PRODUCTION) [+pkCity->getTotalOverflowProductionTimes100()]` instead

int CvLuaCity::lGetCurrentProductionDifference(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const bool bOverflow = lua_toboolean(L, 3);
	int iResult = pkCity->getYieldRateTimes100(YIELD_PRODUCTION) / 100;
	if (bOverflow)
		iResult += pkCity->getTotalOverflowProductionTimes100() / 100;

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int GetRawProductionDifference(bool bIgnoreFood, bool bOverflow);
// LEGACY METHOD, use getRawProductionDifferenceTimes100 instead
int CvLuaCity::lGetRawProductionDifference(lua_State *L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->getRawProductionPerTurnTimes100() / 100);
	return 1;
}
//------------------------------------------------------------------------------
//int GetTotalOverflowProductionTimes100();
int CvLuaCity::lGetTotalOverflowProductionTimes100(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getTotalOverflowProductionTimes100);
}
//------------------------------------------------------------------------------
//int GetCurrentProductionDifferenceTimes100(bool bIgnoreFood, bool bOverflow);
// LEGACY METHOD, use `getYieldRateTimes100(YIELD_PRODUCTION) [+pkCity->getTotalOverflowProductionTimes100()]` instead
int CvLuaCity::lGetCurrentProductionDifferenceTimes100(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const bool bOverflow = lua_toboolean(L, 3);
	int iResult = pkCity->getYieldRateTimes100(YIELD_PRODUCTION);
	if (bOverflow)
		iResult += pkCity->getTotalOverflowProductionTimes100();

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int GetRawProductionDifferenceTimes100(bool bIgnoreFood, bool bOverflow);
int CvLuaCity::lGetRawProductionDifferenceTimes100(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getRawProductionPerTurnTimes100);
}
//------------------------------------------------------------------------------
//int getUnitProductionModifier(UnitTypes iUnit);
int CvLuaCity::lGetUnitProductionModifier(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const UnitTypes iUnit = (UnitTypes)lua_tointeger(L, 2);
	const int iResult = pkCity->getProductionModifier(iUnit);

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int getBuildingProductionModifier(BuildingTypes iBuilding);
int CvLuaCity::lGetBuildingProductionModifier(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const BuildingTypes iBuilding = (BuildingTypes)lua_tointeger(L, 2);
	if(iBuilding != NO_BUILDING)
	{
		const int iResult = pkCity->getProductionModifier(iBuilding);
		lua_pushinteger(L, iResult);
	}
	else
	{
		lua_pushinteger(L, 0);
	}
	return 1;
}
//------------------------------------------------------------------------------
//int getProjectProductionModifier(ProjectTypes eProject);
int CvLuaCity::lGetProjectProductionModifier(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const ProjectTypes eProject = (ProjectTypes)lua_tointeger(L, 2);
	const int iResult = pkCity->getProductionModifier(eProject);

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int getResourceYieldRateModifier(YieldTypes eIndex, ResourceTypes eResource);
int CvLuaCity::lGetResourceYieldRateModifier(lua_State* L)
{
	return BasicLuaMethod<int, YieldTypes, ResourceTypes>(L, &CvCity::getResourceYieldRateModifier);
}
//------------------------------------------------------------------------------
//HandicapTypes getHandicapType();
int CvLuaCity::lGetHandicapType(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getHandicapType);
}
//------------------------------------------------------------------------------
//CivilizationTypes getCivilizationType();
int CvLuaCity::lGetCivilizationType(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getCivilizationType);
}
//------------------------------------------------------------------------------
//LeaderHeadTypes getPersonalityType()
int CvLuaCity::lGetPersonalityType(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const LeaderHeadTypes eValue = pkCity->getPersonalityType();

	lua_pushinteger(L, eValue);
	return 1;
}
//------------------------------------------------------------------------------
//ArtStyleTypes getArtStyleType()
int CvLuaCity::lGetArtStyleType(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const ArtStyleTypes eValue = pkCity->getArtStyleType();

	lua_pushinteger(L, eValue);
	return 1;
}
//------------------------------------------------------------------------------
//CitySizeTypes getCitySizeType()
int CvLuaCity::lGetCitySizeType(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const CitySizeTypes eValue = pkCity->getCitySizeType();

	lua_pushinteger(L, eValue);
	return 1;
}
//------------------------------------------------------------------------------
//bool isBarbarian();
int CvLuaCity::lIsBarbarian(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::isBarbarian);
}
//------------------------------------------------------------------------------
//bool isHuman();
int CvLuaCity::lIsHuman(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::isHuman);
}
//------------------------------------------------------------------------------
//bool isVisible(TeamTypes eTeam, bool bDebug);
int CvLuaCity::lIsVisible(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::isVisible);
}
//------------------------------------------------------------------------------
//bool isCapital();
int CvLuaCity::lIsCapital(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::isCapital);
}
//------------------------------------------------------------------------------
//bool isOriginalCapital();
int CvLuaCity::lIsOriginalCapital(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::IsOriginalCapital);
}
//------------------------------------------------------------------------------
//bool isOriginalMajorCapital();
int CvLuaCity::lIsOriginalMajorCapital(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::IsOriginalMajorCapital);
}
//------------------------------------------------------------------------------
//PlayerTypes GetOwnerForDominationVictory();
int CvLuaCity::lGetOwnerForDominationVictory(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::GetOwnerForDominationVictory);
}
//------------------------------------------------------------------------------
//bool isCoastal(int iMinWaterSize);
int CvLuaCity::lIsCoastal(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::isCoastal);
}
//------------------------------------------------------------------------------
//bool isAddsFreshWater();
int CvLuaCity::lIsAddsFreshWater(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::isAddsFreshWater);
}
//int foodConsumptionSpecialistTimes100();
int CvLuaCity::lFoodConsumptionSpecialistTimes100(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getFoodConsumptionSpecialistTimes100);
}
//------------------------------------------------------------------------------
//int getFoodConsumptionTimes100();
int CvLuaCity::lGetFoodConsumptionTimes100(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getFoodConsumptionTimes100);
}
//------------------------------------------------------------------------------
// LEGACY method, use `GetFoodConsumptionTimes100() / 100` instead
int CvLuaCity::lFoodConsumption(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = pkCity->getFoodConsumptionTimes100() / 100;
	lua_pushinteger(L, iResult);
	return 1;
}
//int foodDifference(bool bBottom);
//LEGACY METHOD, use `getYieldRateTimes100(YIELD_FOOD)` instead
int CvLuaCity::lFoodDifference(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = pkCity->getYieldRateTimes100(YIELD_FOOD) / 100;
	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int foodDifferenceTimes100(bool bBottom);
//LEGACY METHOD, use `getYieldRateTimes100(YIELD_FOOD)` instead
int CvLuaCity::lFoodDifferenceTimes100(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = pkCity->getYieldRateTimes100(YIELD_FOOD);
	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int growthThreshold();
int CvLuaCity::lGrowthThreshold(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::growthThreshold);
}
//------------------------------------------------------------------------------
//int productionLeft();
int CvLuaCity::lProductionLeft(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::productionLeft);
}
//------------------------------------------------------------------------------
//int getNumBuilding(BuildingTypes eBuildingType);
int CvLuaCity::lGetNumBuilding(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const BuildingTypes eBuildingType = (BuildingTypes)lua_tointeger(L, 2);
	if(eBuildingType != NO_BUILDING)
	{
		const int iResult = pkCity->GetCityBuildings()->GetNumBuilding(eBuildingType);
		lua_pushinteger(L, iResult);
	}
	else
	{
		lua_pushinteger(L, 0);
	}
	return 1;
}
//------------------------------------------------------------------------------
//bool isHasBuilding(BuildingTypes eBuildingType);
// This is a function to help modders out, since it was replaced with getNumBuildings() in the C++
int CvLuaCity::lIsHasBuilding(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const BuildingTypes eBuildingType = (BuildingTypes)lua_tointeger(L, 2);
	if(eBuildingType != NO_BUILDING)
	{
		const bool bResult = pkCity->GetCityBuildings()->GetNumBuilding(eBuildingType);
		lua_pushboolean(L, bResult);
	}
	else
	{
		lua_pushboolean(L, false);
	}
	return 1;
}
//------------------------------------------------------------------------------
//int getNumBuildingClass(BuildingClassTypes eBuildingClassType);
int CvLuaCity::lGetNumBuildingClass(lua_State* L)
{
	CvCity* pCity = GetInstance(L);
	const BuildingClassTypes eBuildingClass = static_cast<BuildingClassTypes>(lua_tointeger(L, 2));
	int iResult = 0;
	if (eBuildingClass != NO_BUILDINGCLASS)
	{
		iResult = pCity->GetCityBuildings()->GetNumBuildingClass(eBuildingClass);
	}

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//bool isHasBuildingClass(BuildingClassTypes eBuildingClassType);
int CvLuaCity::lIsHasBuildingClass(lua_State* L)
{
	CvCity* pCity = GetInstance(L);
	const BuildingClassTypes eBuildingClass = static_cast<BuildingClassTypes>(lua_tointeger(L, 2));
	bool bResult = false;
	if(eBuildingClass != NO_BUILDINGCLASS)
	{
		bResult = pCity->HasBuildingClass(eBuildingClass);
	}

	lua_pushboolean(L, bResult);
	return 1;
}
//------------------------------------------------------------------------------
int CvLuaCity::lGetLocalBuildingClassYield(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const BuildingClassTypes eBuildingClassType = (BuildingClassTypes)lua_tointeger(L, 2);
	const YieldTypes eIndex = (YieldTypes)lua_tointeger(L, 3);
	int iResult = 0;
	if(eBuildingClassType != NO_BUILDINGCLASS && eIndex != NO_YIELD)
	{	
		iResult = pkCity->getLocalBuildingClassYield(eBuildingClassType, eIndex);
	}

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
int CvLuaCity::lGetEventBuildingClassYield(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const BuildingClassTypes eBuildingClassType = (BuildingClassTypes)lua_tointeger(L, 2);
	const YieldTypes eIndex = (YieldTypes)lua_tointeger(L, 3);
	int iResult = 0;
	if(eBuildingClassType != NO_BUILDINGCLASS && eIndex != NO_YIELD)
	{	
		iResult = pkCity->GetEventBuildingClassCityYield(eBuildingClassType, eIndex);
	}

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
int CvLuaCity::lGetEventBuildingClassModifier(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const BuildingClassTypes eBuildingClassType = (BuildingClassTypes)lua_tointeger(L, 2);
	const YieldTypes eIndex = (YieldTypes)lua_tointeger(L, 3);
	int iResult = 0;
	if(eBuildingClassType != NO_BUILDINGCLASS && eIndex != NO_YIELD)
	{	
		iResult = pkCity->GetEventBuildingClassCityYieldModifier(eBuildingClassType, eIndex);
	}

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
int CvLuaCity::lGetEventCityYield(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::GetEventCityYield);
}
//------------------------------------------------------------------------------
//int getNumActiveBuilding(BuildingTypes eBuildingType);
int CvLuaCity::lGetNumActiveBuilding(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const BuildingTypes eBuildingType = (BuildingTypes)lua_tointeger(L, 2);
	if(eBuildingType != NO_BUILDING)
	{
		const int iResult = pkCity->GetCityBuildings()->GetNumActiveBuilding(eBuildingType);
		lua_pushinteger(L, iResult);
	}
	else
	{
		lua_pushinteger(L, 0);
	}
	return 1;
}
//------------------------------------------------------------------------------
//int getID();
int CvLuaCity::lGetID(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = pkCity->GetID();

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int getX();
int CvLuaCity::lGetX(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = pkCity->getX();

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int getY();
int CvLuaCity::lGetY(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = pkCity->getY();

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//bool at(int iX, int iY);
int CvLuaCity::lAt(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int x = lua_tointeger(L, 2);
	const int y = lua_tointeger(L, 3);
	const bool bResult = pkCity->at(x, y);

	lua_pushboolean(L, bResult);
	return 1;
}
//------------------------------------------------------------------------------
//bool atPlot(CyPlot* pPlot);
int CvLuaCity::lAtPlot(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	CvPlot* pkPlot = CvLuaPlot::GetInstance(L, 2);
	const bool bResult = pkCity->at(pkPlot);

	lua_pushboolean(L, bResult);
	return 1;
}
//------------------------------------------------------------------------------
//CyPlot* plot();
int CvLuaCity::lPlot(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	CvPlot* pkPlot = pkCity->plot();
	CvLuaPlot::Push(L, pkPlot);
	return 1;
}
//------------------------------------------------------------------------------
//CyArea* area();
int CvLuaCity::lArea(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	//there might be other areas adjacent to the city ...
	CvArea* pkArea = pkCity->plot()->area();
	CvLuaArea::Push(L, pkArea);
	return 1;
}
//------------------------------------------------------------------------------
//CyArea* waterArea();
int CvLuaCity::lWaterArea(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);

	std::vector<int> areas = pkCity->plot()->getAllAdjacentAreas();
	for (std::vector<int>::iterator it=areas.begin(); it!=areas.end(); ++it)
	{
		//just return the first one ...
		CvArea* pkArea = GC.getMap().getAreaById(*it);
		if (pkArea->isWater())
		{
			CvLuaArea::Push(L, pkArea);
			return 1;
		}
	}

	CvLuaArea::Push(L, NULL);
	return 1;
}
//------------------------------------------------------------------------------
//CyPlot* getRallyPlot();
int CvLuaCity::lGetRallyPlot(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	CvPlot* pkPlot = pkCity->getRallyPlot();
	CvLuaPlot::Push(L, pkPlot);
	return 1;
}

//------------------------------------------------------------------------------
//bool getCanBuyPlot();
int CvLuaCity::lCanBuyPlot(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const bool bPurchase = lua_toboolean(L, 2);
	CvPlot* pPlot = pkCity->GetNextBuyablePlot(bPurchase);
	lua_pushboolean(L, pkCity->CanBuyPlot(pPlot->getX(), pPlot->getY()));
	return 1;
}

//------------------------------------------------------------------------------
//bool getCanBuyPlotAt();
int CvLuaCity::lCanBuyPlotAt(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iX = lua_tointeger(L, 2);
	const int iY = lua_tointeger(L, 3);
	const bool bIgnoreCost = lua_toboolean(L, 4);
	lua_pushboolean(L, pkCity->CanBuyPlot(iX, iY, bIgnoreCost));
	return 1;
}
//------------------------------------------------------------------------------
//bool CanBuyAnyPlot(void)
int CvLuaCity::lCanBuyAnyPlot(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushboolean(L, pkCity->CanBuyAnyPlot());
	return 1;
}
//------------------------------------------------------------------------------
//CvPlot* getNextBuyablePlot();
int CvLuaCity::lGetNextBuyablePlot(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const bool bPurchase = lua_toboolean(L, 2);
	CvPlot* pkPlot = pkCity->GetNextBuyablePlot(bPurchase);
	CvLuaPlot::Push(L, pkPlot);
	return 1;
}

//------------------------------------------------------------------------------
int CvLuaCity::lGetBuyablePlotList(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	std::vector<int> aiPlotList;
	aiPlotList.resize(20, -1);
	const bool bPurchase = lua_toboolean(L, 2);
	pkCity->GetBuyablePlotList(aiPlotList, bPurchase);

	int iReturnValues = 0;

	for(uint ui = 0; ui < aiPlotList.size(); ui++)
	{
		if(aiPlotList[ui] >= 0)
		{
			CvPlot* pkPlot = GC.getMap().plotByIndex(aiPlotList[ui]);
			CvLuaPlot::Push(L, pkPlot);
			iReturnValues++;
		}
		else
		{
			break;
		}
	}

	return iReturnValues;
}

//------------------------------------------------------------------------------
//int GetBuyPlotCost()
int CvLuaCity::lGetBuyPlotCost(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iX = lua_tointeger(L, 2);
	const int iY = lua_tointeger(L, 3);
	lua_pushinteger(L, pkCity->GetBuyPlotCost(iX, iY));
	return 1;
}

//------------------------------------------------------------------------------
// CvUnit* GetGarrisonedUnit()
int CvLuaCity::lGetGarrisonedUnit(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	CvUnit* pkUnit = pkCity->GetGarrisonedUnit();
	CvLuaUnit::Push(L, pkUnit);
	return 1;
}

//------------------------------------------------------------------------------
//int getGameTurnFounded();
int CvLuaCity::lGetGameTurnFounded(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getGameTurnFounded);
}
//------------------------------------------------------------------------------
//int getGameTurnAcquired();
int CvLuaCity::lGetGameTurnAcquired(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getGameTurnAcquired);
}
//------------------------------------------------------------------------------
//int getGameTurnLastExpanded();
int CvLuaCity::lGetGameTurnLastExpanded(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getGameTurnLastExpanded);
}
//------------------------------------------------------------------------------
//int getPopulation();
int CvLuaCity::lGetPopulation(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getPopulation);
}
//------------------------------------------------------------------------------
//void setPopulation(int iNewValue, bool bReassignPop = true);
int CvLuaCity::lSetPopulation(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	int iValue = lua_tointeger(L, 2);
	bool bReassignPop = lua_isboolean(L, 3) ? lua_toboolean(L, 3) : true;
	ASSERT_DEBUG(bReassignPop, "It is super dangerous to set this to false.  Ken would love to see why you are doing this.");
	pkCity->setPopulation(iValue, bReassignPop);

	return 1;
//	return BasicLuaMethod(L, &CvCity::setPopulation);
}
//------------------------------------------------------------------------------
//void changePopulation(int iChange, bool bReassignPop = true);
int CvLuaCity::lChangePopulation(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	int iChange = lua_tointeger(L, 2);
	bool bReassignPop = lua_isboolean(L, 3) ? lua_toboolean(L, 3) : true;
	ASSERT_DEBUG(bReassignPop, "It is super dangerous to set this to false.  Ken would love to see why you are doing this.");
	pkCity->changePopulation(iChange, bReassignPop);

	return 1;
//	return BasicLuaMethod(L, &CvCity::changePopulation);
}
//------------------------------------------------------------------------------
//int getRealPopulation();
int CvLuaCity::lGetRealPopulation(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getRealPopulation);
}
//------------------------------------------------------------------------------
//int getHighestPopulation();
int CvLuaCity::lGetHighestPopulation(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getHighestPopulation);
}
//------------------------------------------------------------------------------
//void setHighestPopulation(int iNewValue);
int CvLuaCity::lSetHighestPopulation(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::setHighestPopulation);
}
//------------------------------------------------------------------------------
//int getWorkingPopulation();
//int CvLuaCity::lGetWorkingPopulation(lua_State* L)
//{
//	return BasicLuaMethod(L, &CvCity::getWorkingPopulation);
//}
////------------------------------------------------------------------------------
////int getSpecialistPopulation();
//int CvLuaCity::lGetSpecialistPopulation(lua_State* L)
//{
//	return BasicLuaMethod(L, &CvCity::getSpecialistPopulation);
//}
#if defined(MOD_GLOBAL_CITY_AUTOMATON_WORKERS)
//------------------------------------------------------------------------------
//int getAutomatons();
int CvLuaCity::lGetAutomatons(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getAutomatons);
}
//------------------------------------------------------------------------------
//void setAutomatons(int iNewValue, bool bReassignPop);
int CvLuaCity::lSetAutomatons(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::setAutomatons);
}
#endif
//------------------------------------------------------------------------------
//int getBaseGreatPeopleRate();
int CvLuaCity::lGetBaseGreatPeopleRate(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getBaseGreatPeopleRate);
}
//------------------------------------------------------------------------------
//int getGreatPeopleRate();
int CvLuaCity::lGetGreatPeopleRate(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getGreatPeopleRate);
}
//------------------------------------------------------------------------------
//int getGreatPeopleRate();
int CvLuaCity::lGetEventGPPFromSpecialists(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::GetEventGPPFromSpecialists);
}

int CvLuaCity::lGetSpecialistRate(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const SpecialistTypes eSpecialist = (SpecialistTypes)lua_tointeger(L, 2);
	int Rate = 0;
	if (eSpecialist != NO_SPECIALIST)
	{
		Rate = pkCity->GetCityCitizens()->GetSpecialistRate(eSpecialist);
	}
	lua_pushinteger(L, Rate);
	return 1;
}

//------------------------------------------------------------------------------
//int getTotalGreatPeopleRateModifier();
int CvLuaCity::lGetTotalGreatPeopleRateModifier(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getTotalGreatPeopleRateModifier);
}
//------------------------------------------------------------------------------
//void changeBaseGreatPeopleRate(int iChange);
int CvLuaCity::lChangeBaseGreatPeopleRate(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::changeBaseGreatPeopleRate);
}
//------------------------------------------------------------------------------
//int getGreatPeopleRateModifier();
int CvLuaCity::lGetGreatPeopleRateModifier(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getGreatPeopleRateModifier);
}
//int GetImprovementGreatPersonRateModifier();
int CvLuaCity::lGetImprovementGreatPersonRateModifier(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::GetImprovementGreatPersonRateModifier);
}
//int GetReligionGreatPersonRateModifier();
int CvLuaCity::lGetReligionGreatPersonRateModifier(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	int iResult = 0;

	GreatPersonTypes eGreatPerson = GetGreatPersonFromSpecialist((SpecialistTypes)toValue<SpecialistTypes>(L, 2));

	if (eGreatPerson != NO_GREATPERSON)
	{
		iResult += pkCity->GetReligionGreatPersonRateModifier(eGreatPerson);
	}

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int GetBorderGrowthRateIncreaseTotal() const;
int CvLuaCity::lGetBorderGrowthRateIncreaseTotal(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->GetBorderGrowthRateIncreaseTotal());
	return 1;
}
//------------------------------------------------------------------------------
//int GetJONSCultureStored() const;
// LEGACY METHOD, use GetJONSCultureStoredTimes100 instead
int CvLuaCity::lGetJONSCultureStored(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->GetJONSCultureStoredTimes100() / 100);
	return 1;
}
//------------------------------------------------------------------------------
//int GetJONSCultureStoredTimes100() const;
int CvLuaCity::lGetJONSCultureStoredTimes100(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::GetJONSCultureStoredTimes100);
}
//------------------------------------------------------------------------------
//void SetJONSCultureStored(int iValue);
int CvLuaCity::lSetJONSCultureStored(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	int iValue = lua_tointeger(L, 2);
	pkCity->SetJONSCultureStoredTimes100(iValue * 100);
	return 0;
}
//------------------------------------------------------------------------------
//void SetJONSCultureStoredTimes100(int iValue);
int CvLuaCity::lSetJONSCultureStoredTimes100(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::SetJONSCultureStoredTimes100);
}
//------------------------------------------------------------------------------
//void ChangeJONSCultureStored(int iChange);
int CvLuaCity::lChangeJONSCultureStored(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::ChangeJONSCultureStored);
}
//------------------------------------------------------------------------------
//void ChangeJONSCultureStoredTimes100(int iChange);
int CvLuaCity::lChangeJONSCultureStoredTimes100(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::ChangeJONSCultureStoredTimes100);
}
//------------------------------------------------------------------------------
//int GetJONSCultureLevel() const;
int CvLuaCity::lGetJONSCultureLevel(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::GetJONSCultureLevel);
}
//------------------------------------------------------------------------------
//void SetJONSCultureLevel(int iValue);
int CvLuaCity::lSetJONSCultureLevel(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::SetJONSCultureLevel);
}
//------------------------------------------------------------------------------
//void ChangeJONSCultureLevel(int iChange);
int CvLuaCity::lChangeJONSCultureLevel(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::ChangeJONSCultureLevel);
}
//------------------------------------------------------------------------------
//void DoJONSCultureLevelIncrease();
int CvLuaCity::lDoJONSCultureLevelIncrease(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::DoJONSCultureLevelIncrease);
}
//------------------------------------------------------------------------------
//int GetJONSCultureThreshold() const;
int CvLuaCity::lGetJONSCultureThreshold(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::GetJONSCultureThreshold);
}

//------------------------------------------------------------------------------
//int getJONSCulturePerTurn() const;
// LEGACY METHOD, use GetYieldRateTimes100(YIELD_CULTURE) instead
int CvLuaCity::lGetJONSCulturePerTurn(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->getYieldRateTimes100(YIELD_CULTURE) / 100);
	return 1;
}
//------------------------------------------------------------------------------
//int GetBaseJONSCulturePerTurn() const;
//LEGACY METHOD, use GetBaseYieldRateTimes100(YIELD_CULTURE) instead
int CvLuaCity::lGetBaseJONSCulturePerTurn(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->getBaseYieldRateTimes100(YIELD_CULTURE) / 100);
	return 1;
}
//------------------------------------------------------------------------------
//int GetJONSCulturePerTurnFromBuildings() const;
int CvLuaCity::lGetJONSCulturePerTurnFromBuildings(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->GetBaseYieldRateFromBuildings(YIELD_CULTURE));
	return 1;
}
//------------------------------------------------------------------------------
//void ChangeJONSCulturePerTurnFromBuildings(int iChange);
int CvLuaCity::lChangeJONSCulturePerTurnFromBuildings(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	int iChange = lua_tointeger(L, 2);
	pkCity->ChangeBaseYieldRateFromBuildings(YIELD_CULTURE, iChange);
	return 0;
}
//------------------------------------------------------------------------------
//int GetJONSCulturePerTurnFromPolicies() const;
int CvLuaCity::lGetJONSCulturePerTurnFromPolicies(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->GetBaseYieldRateFromPolicies(YIELD_CULTURE));
	return 1;
}
//------------------------------------------------------------------------------
//void ChangeJONSCulturePerTurnFromPolicies(int iChange);
int CvLuaCity::lChangeJONSCulturePerTurnFromPolicies(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	int iChange = lua_tointeger(L, 2);
	pkCity->ChangeBaseYieldRateFromPolicies(YIELD_CULTURE, iChange);
	return 0;
}
//------------------------------------------------------------------------------
//int GetJONSCulturePerTurnFromSpecialists() const;
int CvLuaCity::lGetJONSCulturePerTurnFromSpecialists(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->GetBaseYieldRateFromSpecialists(YIELD_CULTURE));
	return 1;
}
//------------------------------------------------------------------------------
//void ChangeJONSCulturePerTurnFromSpecialists(int iChange);
int CvLuaCity::lChangeJONSCulturePerTurnFromSpecialists(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	int iChange = lua_tointeger(L, 2);
	pkCity->ChangeBaseYieldRateFromSpecialists(YIELD_CULTURE, iChange);
	return 0;
}
//------------------------------------------------------------------------------
//int GetJONSCulturePerTurnFromGreatWorks() const;
// LEGACY METHOD
int CvLuaCity::lGetJONSCulturePerTurnFromGreatWorks(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->GetBaseYieldRateFromGreatWorksTimes100(YIELD_CULTURE) / 100);
	return 1;
}
//------------------------------------------------------------------------------
//int GetJONSCulturePerTurnFromTraits() const;
// LEGACY METHOD
int CvLuaCity::lGetJONSCulturePerTurnFromTraits(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, GET_PLAYER(pkCity->getOwner()).GetPlayerTraits()->GetCityCultureBonus());
	return 1;
}
//void ChangeYieldFromTraits(YieldTypes eIndex, int iChange);
int CvLuaCity::lChangeYieldFromTraits(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::ChangeYieldFromTraits);
}
//------------------------------------------------------------------------------
//int GetYieldPerTurnFromTraits() const;
int CvLuaCity::lGetYieldPerTurnFromTraits(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::GetYieldPerTurnFromTraits);
}

//int GetYieldPerTurnFromTraits() const;
int CvLuaCity::lGetYieldFromUnitsInCity(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	YieldTypes eYieldType = (YieldTypes)lua_tointeger(L, 2);

	int Total = 0;
	CvPlot* pCityPlot = pkCity->plot();
	for (int iUnitLoop = 0; iUnitLoop < pCityPlot->getNumUnits(); iUnitLoop++)
	{
		int iTempVal = pCityPlot->getUnitByIndex(iUnitLoop)->GetYieldChange(eYieldType);
		if (iTempVal != 0)
		{
			Total += iTempVal;
		}
	}

	lua_pushinteger(L, Total);
	return 1;
}
//------------------------------------------------------------------------------
//int GetJONSCulturePerTurnFromReligion() const;
// LEGACY METHOD
int CvLuaCity::lGetJONSCulturePerTurnFromReligion(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->GetBaseYieldRateFromReligionTimes100(YIELD_CULTURE) / 100);
	return 1;
}
//------------------------------------------------------------------------------
//void ChangeJONSCulturePerTurnFromReligion(int iChange);
int CvLuaCity::lChangeJONSCulturePerTurnFromReligion(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	int iChange = lua_tointeger(L, 2);
	pkCity->ChangeBaseYieldRateFromReligion(YIELD_CULTURE, iChange);
	return 0;
}
//------------------------------------------------------------------------------
//int GetJONSCulturePerTurnFromLeagues() const;
int CvLuaCity::lGetJONSCulturePerTurnFromLeagues(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::GetJONSCulturePerTurnFromLeagues);
}
//------------------------------------------------------------------------------
//int getCultureRateModifier() const;
int CvLuaCity::lGetCultureRateModifier(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getCultureRateModifier);
}
//------------------------------------------------------------------------------
//void changeCultureRateModifier(int iChange);
int CvLuaCity::lChangeCultureRateModifier(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::changeCultureRateModifier);
}
#if defined(MOD_BALANCE_CORE_RESOURCE_MONOPOLIES)
//int GetCityYieldModFromMonopoly() const;
int CvLuaCity::lGetCityYieldModFromMonopoly(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	YieldTypes eYieldType = (YieldTypes)lua_tointeger(L, 2);
	int iModifier = 0;
	// Do we get increased yields from a resource monopoly?
	for (int iResourceLoop = 0; iResourceLoop < GC.getNumResourceInfos(); iResourceLoop++)
	{
		ResourceTypes eResourceLoop = (ResourceTypes) iResourceLoop;
		CvResourceInfo* pInfo = GC.getResourceInfo(eResourceLoop);
		if (pInfo && pInfo->isMonopoly())
		{
			if(GET_PLAYER(pkCity->getOwner()).HasGlobalMonopoly(eResourceLoop) && pInfo->getCityYieldModFromMonopoly(eYieldType) > 0)
			{
				int iTemp = pInfo->getCityYieldModFromMonopoly(eYieldType);
				iTemp += GET_PLAYER(pkCity->getOwner()).GetMonopolyModPercent();
				iModifier += iTemp;
			}
		}
	}
	lua_pushinteger(L, iModifier);
	return 1;
}
#endif
//------------------------------------------------------------------------------
//int getTourismRateModifier() const;
int CvLuaCity::lGetTourismRateModifier(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getTourismRateModifier);
}
//------------------------------------------------------------------------------
//void changeTourismRateModifier(int iChange);
int CvLuaCity::lChangeTourismRateModifier(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::changeTourismRateModifier);
}
//------------------------------------------------------------------------------
//int GetNumGreatWorks();
int CvLuaCity::lGetNumGreatWorks(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
#if defined(MOD_GLOBAL_GREATWORK_YIELDTYPES)
	const bool bIgnoreYield = luaL_optbool(L, 2, true);
	lua_pushinteger(L, pkCity->GetCityCulture()->GetNumGreatWorks(bIgnoreYield));
#else
	lua_pushinteger(L, pkCity->GetCityCulture()->GetNumGreatWorks());
#endif
	return 1;
}
//------------------------------------------------------------------------------
//int GetNumGreatWorkSlots();
int CvLuaCity::lGetNumGreatWorkSlots(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->GetCityCulture()->GetNumGreatWorkSlots());
	return 1;
}
//------------------------------------------------------------------------------
//int GetBaseTourism();
int CvLuaCity::lGetBaseTourism(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	int iValue = pkCity->getYieldRateTimes100(YIELD_TOURISM);
	if (!MOD_BALANCE_CORE_TOURISM_HUNDREDS)
		iValue /= 100;

	lua_pushinteger(L, iValue);
	return 1;
}

int CvLuaCity::lRefreshTourism(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	pkCity->UpdateCityYields(YIELD_TOURISM);
	return 0;
}
//------------------------------------------------------------------------------
//int GetNumGreatWorksFilled();
int CvLuaCity::lGetNumGreatWorksFilled(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	GreatWorkSlotType eGreatWorkSlot = static_cast<GreatWorkSlotType>(lua_tointeger(L, 2));

	lua_pushinteger(L, pkCity->GetCityCulture()->GetNumFilledGreatWorkSlots(eGreatWorkSlot));
	return 1;
}

int CvLuaCity::lGetNumAvailableGreatWorkSlots(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	GreatWorkSlotType eGreatWorkSlot = static_cast<GreatWorkSlotType>(lua_tointeger(L, 2));

	lua_pushinteger(L, pkCity->GetCityCulture()->GetNumAvailableGreatWorkSlots(eGreatWorkSlot));
	return 1;
}

//------------------------------------------------------------------------------
//int GetTourismMultiplier(PlayerTypes ePlayer);
int CvLuaCity::lGetTourismMultiplier(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const PlayerTypes ePlayer = (PlayerTypes)lua_tointeger(L, 2);
	lua_pushinteger(L, GET_PLAYER(pkCity->getOwner()).GetCulture()->GetTourismModifierWith(ePlayer));
	return 1;
}
//------------------------------------------------------------------------------
//CvString GetTourismTooltip();
int CvLuaCity::lGetTourismTooltip(lua_State* L)
{
	CvString toolTip;
	CvCity* pkCity = GetInstance(L);
	toolTip = pkCity->GetCityCulture()->GetTourismTooltip();
	lua_pushstring(L, toolTip.c_str());
	return 1;
}
//------------------------------------------------------------------------------
//CvString GetFilledSlotsTooltip();
int CvLuaCity::lGetFilledSlotsTooltip(lua_State* L)
{
	CvString toolTip;
	CvCity* pkCity = GetInstance(L);
	toolTip = pkCity->GetCityCulture()->GetFilledSlotsTooltip();
	lua_pushstring(L, toolTip.c_str());
	return 1;
}
//------------------------------------------------------------------------------
//CvString GetTotalSlotsTooltip();
int CvLuaCity::lGetTotalSlotsTooltip(lua_State* L)
{
	CvString toolTip;
	CvCity* pkCity = GetInstance(L);
	toolTip = pkCity->GetCityCulture()->GetTotalSlotsTooltip();
	lua_pushstring(L, toolTip.c_str());
	return 1;
}
//------------------------------------------------------------------------------
//void ClearGreatWorks();
int CvLuaCity::lClearGreatWorks(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	pkCity->GetCityCulture()->ClearGreatWorks();
	return 1;
}
//------------------------------------------------------------------------------
// int GetFaithBuildingTourism()
int CvLuaCity::lGetFaithBuildingTourism(lua_State* L)
{
	int iRtnValue = 0;
	CvCity* pkCity = GetInstance(L);
	ReligionTypes eMajority = pkCity->GetCityReligions()->GetReligiousMajority();
	const CvReligion* pReligion = GC.getGame().GetGameReligions()->GetReligion(eMajority, pkCity->getOwner());
	if(pReligion)
	{
		iRtnValue = pReligion->m_Beliefs.GetFaithBuildingTourism(pkCity->getOwner(), pkCity);
	}
	lua_pushinteger(L, iRtnValue);
	return 1;
}
//------------------------------------------------------------------------------
// int GetBuildingClassTourism()
int CvLuaCity::lGetBuildingClassTourism(lua_State* L)
{
	int iRtnValue = 0;
	CvCity* pkCity = GetInstance(L);
	const BuildingClassTypes iIndex = toValue<BuildingClassTypes>(L, 2);
	ReligionTypes eMajority = pkCity->GetCityReligions()->GetReligiousMajority();
	const CvReligion* pReligion = GC.getGame().GetGameReligions()->GetReligion(eMajority, pkCity->getOwner());
	if (pReligion)
	{
		iRtnValue = pReligion->m_Beliefs.GetBuildingClassYieldChange(iIndex, YIELD_TOURISM, pkCity->GetCityReligions()->GetNumFollowers(eMajority), pkCity->getOwner(), pkCity);
	}
	lua_pushinteger(L, iRtnValue);
	return 1;
}

//------------------------------------------------------------------------------
//bool IsThemingBonusPossible(BuildingClassTypes eBuildingClass);
int CvLuaCity::lIsThemingBonusPossible(lua_State* L)
{
	bool bPossible;
	CvCity* pkCity = GetInstance(L);
	const BuildingClassTypes iIndex = toValue<BuildingClassTypes>(L, 2);
	bPossible = pkCity->GetCityCulture()->IsThemingBonusPossible(iIndex);
	lua_pushboolean(L, bPossible);
	return 1;
}
//------------------------------------------------------------------------------
//int GetThemingBonus(BuildingClassTypes eBuildingClass);
int CvLuaCity::lGetThemingBonus(lua_State* L)
{
	int iBonus;
	CvCity* pkCity = GetInstance(L);
	const BuildingClassTypes iIndex = toValue<BuildingClassTypes>(L, 2);
	iBonus = pkCity->GetCityCulture()->GetThemingBonus(iIndex);
	lua_pushinteger(L, iBonus);
	return 1;
}
//------------------------------------------------------------------------------
//CvString GetThemingTooltip(BuildingClassTypes eBuildingClass);
int CvLuaCity::lGetThemingTooltip(lua_State* L)
{
	CvString toolTip;
	CvCity* pkCity = GetInstance(L);
	const BuildingClassTypes iIndex = toValue<BuildingClassTypes>(L, 2);
	toolTip = pkCity->GetCityCulture()->GetThemingTooltip(iIndex);
	lua_pushstring(L, toolTip.c_str());
	return 1;
}
//------------------------------------------------------------------------------
//int GetFaithPerTurn() const;
//LEGACY METHOD, use getYieldRateTimes100(YIELD_FAITH) instead
int CvLuaCity::lGetFaithPerTurn(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->getYieldRateTimes100(YIELD_FAITH) / 100);
	return 1;
}
//------------------------------------------------------------------------------
//int GetFaithPerTurnFromBuildings() const;
int CvLuaCity::lGetFaithPerTurnFromBuildings(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->GetBaseYieldRateFromBuildings(YIELD_FAITH));
	return 1;
}
//------------------------------------------------------------------------------
//int GetFaithPerTurnFromPolicies() const;
int CvLuaCity::lGetFaithPerTurnFromPolicies(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->GetBaseYieldRateFromPolicies(YIELD_FAITH));
	return 1;
}
//------------------------------------------------------------------------------
//int GetFaithPerTurnFromTraits() const;
int CvLuaCity::lGetFaithPerTurnFromTraits(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->GetYieldFromUnimprovedFeatures(YIELD_FAITH));
	return 1;
}
//------------------------------------------------------------------------------
//int GetFaithPerTurnFromReligion() const;
int CvLuaCity::lGetFaithPerTurnFromReligion(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->GetBaseYieldRateFromReligionTimes100(YIELD_FAITH) / 100);
	return 1;
}
//------------------------------------------------------------------------------
//void ChangeFaithPerTurnFromReligion(int iChange);
int CvLuaCity::lChangeFaithPerTurnFromReligion(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	int iChange = lua_tointeger(L, 2);
	pkCity->ChangeBaseYieldRateFromReligion(YIELD_FAITH, iChange);
	return 0;
}
//------------------------------------------------------------------------------
//int IsReligionInCity() const;
int CvLuaCity::lIsReligionInCity(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const bool bResult = pkCity->GetCityReligions()->IsReligionInCity();

	lua_pushboolean(L, bResult);
	return 1;
}

//------------------------------------------------------------------------------
//int HasConvertedToReligionEver() const;
int CvLuaCity::lHasConvertedToReligionEver(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	ReligionTypes eReligion = (ReligionTypes)lua_tointeger(L, 2);
	const bool bResult = pkCity->HasPaidAdoptionBonus(eReligion);

	lua_pushboolean(L, bResult);
	return 1;
}
//------------------------------------------------------------------------------
//int IsHolyCityForReligion(ReligionTypes eReligion) const;
int CvLuaCity::lIsHolyCityForReligion(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	ReligionTypes eReligion = (ReligionTypes)lua_tointeger(L, 2);
	const bool bResult = pkCity->GetCityReligions()->IsHolyCityForReligion(eReligion);

	lua_pushboolean(L, bResult);
	return 1;
}
//------------------------------------------------------------------------------
//int IsHolyCityAnyReligion() const;
int CvLuaCity::lIsHolyCityAnyReligion(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const bool bResult = pkCity->GetCityReligions()->IsHolyCityAnyReligion();

	lua_pushboolean(L, bResult);
	return 1;
}
//------------------------------------------------------------------------------
//int GetNumFollowers(ReligionTypes eReligion) const;
int CvLuaCity::lGetNumFollowers(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	ReligionTypes eReligion = (ReligionTypes)lua_tointeger(L, 2);
	const int iResult = pkCity->GetCityReligions()->GetNumFollowers(eReligion);

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int GetReligiousMajority() const;
int CvLuaCity::lGetReligiousMajority(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = (int)pkCity->GetCityReligions()->GetReligiousMajority();
	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int GetSecondaryReligion() const;
int CvLuaCity::lGetSecondaryReligion(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = (int)pkCity->GetCityReligions()->GetReligionByAccumulatedPressure(1);
	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int GetSecondaryReligionPantheonBelief() const;
int CvLuaCity::lGetSecondaryReligionPantheonBelief(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = (int)pkCity->GetCityReligions()->GetSecondaryReligionPantheonBelief();
	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int GetPressurePerTurn() const;
int CvLuaCity::lGetPressurePerTurn(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	ReligionTypes eReligion = (ReligionTypes)lua_tointeger(L, 2);
	int iNumSourceCities = 0;
	const int iResult = (int)pkCity->GetCityReligions()->GetPressurePerTurn(eReligion, &iNumSourceCities);
	lua_pushinteger(L, iResult);
	lua_pushinteger(L, iNumSourceCities);
	lua_pushinteger(L, pkCity->GetCityReligions()->GetPressureAccumulated(eReligion));
	return 3;
}
//------------------------------------------------------------------------------
//int ConvertPercentFollowers(ReligionTypes eToReligion, ReligionTypes eFromReligion, int iPercent) const;
int CvLuaCity::lConvertPercentFollowers(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	ReligionTypes eToReligion = (ReligionTypes)lua_tointeger(L, 2);
	ReligionTypes eFromReligion = (ReligionTypes)lua_tointeger(L, 3);
	int iPercent = lua_tointeger(L, 4);
	pkCity->GetCityReligions()->ConvertPercentFollowers(eToReligion, eFromReligion, iPercent);
	return 1;
}
//------------------------------------------------------------------------------
//int AdoptReligionFully() const;
int CvLuaCity::lAdoptReligionFully(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	ReligionTypes eReligion = (ReligionTypes)lua_tointeger(L, 2);
	pkCity->GetCityReligions()->AdoptReligionFully(eReligion);
	return 1;
}
//------------------------------------------------------------------------------
//int lGetReligionBuildingClassHappiness(eBuildingClass) const;
int CvLuaCity::lGetReligionBuildingClassHappiness(lua_State* L)
{
	int iHappinessFromBuilding = 0;

	CvCity* pkCity = GetInstance(L);
	BuildingClassTypes eBuildingClass = (BuildingClassTypes)lua_tointeger(L, 2);

	ReligionTypes eMajority = pkCity->GetCityReligions()->GetReligiousMajority();
	if(eMajority != NO_RELIGION)
	{
		const CvReligion* pReligion = GC.getGame().GetGameReligions()->GetReligion(eMajority, pkCity->getOwner());
		if(pReligion)
		{	
			int iFollowers = pkCity->GetCityReligions()->GetNumFollowers(eMajority);
			iHappinessFromBuilding += pReligion->m_Beliefs.GetBuildingClassHappiness(eBuildingClass, iFollowers, pkCity->getOwner(), pkCity);
		}
	}
	lua_pushinteger(L, iHappinessFromBuilding);
	return 1;
}
//------------------------------------------------------------------------------
//int GetReligionBuildingClassYieldChange(eBuildingClass, eYieldType) const;
int CvLuaCity::lGetReligionBuildingClassYieldChange(lua_State* L)
{
	int iYieldFromBuilding = 0;

	CvCity* pkCity = GetInstance(L);
	BuildingClassTypes eBuildingClass = (BuildingClassTypes)lua_tointeger(L, 2);
	YieldTypes eYieldType = (YieldTypes)lua_tointeger(L, 3);

	ReligionTypes eMajority = pkCity->GetCityReligions()->GetReligiousMajority();
	BeliefTypes eSecondaryPantheon = NO_BELIEF;
	if(eMajority != NO_RELIGION && eBuildingClass != NO_BUILDINGCLASS)
	{
		const CvReligion* pReligion = GC.getGame().GetGameReligions()->GetReligion(eMajority, pkCity->getOwner());
		CvBuildingClassInfo* pInfo = GC.getBuildingClassInfo(eBuildingClass);
		if(pReligion && pInfo)
		{	
			int iFollowers = pkCity->GetCityReligions()->GetNumFollowers(eMajority);
			iYieldFromBuilding += pReligion->m_Beliefs.GetBuildingClassYieldChange(eBuildingClass, eYieldType, iFollowers, pkCity->getOwner(), pkCity);
			if (::isWorldWonderClass(*pInfo))
			{
				iYieldFromBuilding += pReligion->m_Beliefs.GetYieldChangeWorldWonder(eYieldType, pkCity->getOwner(), pkCity);
			}
			eSecondaryPantheon = pkCity->GetCityReligions()->GetSecondaryReligionPantheonBelief();
			if (eSecondaryPantheon != NO_BELIEF)
			{
				iFollowers =  pkCity->GetCityReligions()->GetNumFollowers(pkCity->GetCityReligions()->GetReligionByAccumulatedPressure(1));
				if (iFollowers >= GC.GetGameBeliefs()->GetEntry(eSecondaryPantheon)->GetMinFollowers())
				{
					iYieldFromBuilding += GC.GetGameBeliefs()->GetEntry(eSecondaryPantheon)->GetBuildingClassYieldChange(eBuildingClass, eYieldType);
				}
			}
		}
	}
#if defined(MOD_RELIGION_PERMANENT_PANTHEON)
	// Mod for civs keeping their pantheon belief forever
	if (MOD_RELIGION_PERMANENT_PANTHEON)
	{
		if (GC.getGame().GetGameReligions()->HasCreatedPantheon(pkCity->getOwner()))
		{
			const CvReligion* pPantheon = GC.getGame().GetGameReligions()->GetReligion(RELIGION_PANTHEON, pkCity->getOwner());
			BeliefTypes ePantheonBelief = GC.getGame().GetGameReligions()->GetBeliefInPantheon(pkCity->getOwner());
			if (pPantheon != NULL && ePantheonBelief != NO_BELIEF && ePantheonBelief != eSecondaryPantheon)
			{
				const CvReligion* pReligion = GC.getGame().GetGameReligions()->GetReligion(eMajority, pkCity->getOwner());
				if (pReligion == NULL || (pReligion != NULL && !pReligion->m_Beliefs.IsPantheonBeliefInReligion(ePantheonBelief, eMajority, pkCity->getOwner()))) // check that the our religion does not have our belief, to prevent double counting
				{
					iYieldFromBuilding += GC.GetGameBeliefs()->GetEntry(ePantheonBelief)->GetBuildingClassYieldChange(eBuildingClass, eYieldType);
				}
			}
		}
	}
#endif
	lua_pushinteger(L, iYieldFromBuilding);
	return 1;
}
//------------------------------------------------------------------------------
//int GetLeagueBuildingClassYieldChange(eBuildingClass, eYieldType) const;
int CvLuaCity::lGetLeagueBuildingClassYieldChange(lua_State* L)
{
	int iYieldFromBuilding = 0;

	CvCity* pkCity = GetInstance(L);
	BuildingClassTypes eBuildingClass = (BuildingClassTypes)lua_tointeger(L, 2);
	YieldTypes eYieldType = (YieldTypes)lua_tointeger(L, 3);

	if (eBuildingClass != NO_BUILDINGCLASS)
	{
		CvBuildingClassInfo* pInfo = GC.getBuildingClassInfo(eBuildingClass);
		if (pInfo && pInfo->getMaxGlobalInstances() != -1)
		{
			int iYieldChange = GC.getGame().GetGameLeagues()->GetWorldWonderYieldChange(pkCity->getOwner(), eYieldType);
			if (iYieldChange != 0)
			{
				iYieldFromBuilding += iYieldChange;
			}
		}
	}

	lua_pushinteger(L, iYieldFromBuilding);
	return 1;
}
//------------------------------------------------------------------------------
int CvLuaCity::lGetNumTradeRoutesAddingPressure(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	ReligionTypes eReligion = (ReligionTypes)lua_tointeger(L, 2);
	
	int iNumTradeRoutes = pkCity->GetCityReligions()->GetNumTradeRouteConnections(eReligion);
	lua_pushinteger(L, iNumTradeRoutes);
	return 1;

}
//------------------------------------------------------------------------------
//int getNumWorldWonders();
int CvLuaCity::lGetNumWorldWonders(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getNumWorldWonders);
}
//------------------------------------------------------------------------------
//int getNumTeamWonders();
int CvLuaCity::lGetNumTeamWonders(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getNumTeamWonders);
}
//------------------------------------------------------------------------------
//int getNumNationalWonders();
int CvLuaCity::lGetNumNationalWonders(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getNumNationalWonders);
}
//------------------------------------------------------------------------------
//int getNumBuildings();
int CvLuaCity::lGetNumBuildings(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = pkCity->GetCityBuildings()->GetNumBuildings();

	lua_pushinteger(L, iResult);
	return 1;
}

int CvLuaCity::lGetNumTotalBuildings(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	int iResult = 0;
	const bool bSkipDummy = luaL_optbool(L, 2, true);
	const bool bSkipWW = luaL_optbool(L, 3, true);
	const bool bSkipNW = luaL_optbool(L, 4, true);
	for(int iBuildingLoop = 0; iBuildingLoop < GC.getNumBuildingInfos(); iBuildingLoop++)
	{
		const BuildingTypes eBuilding = static_cast<BuildingTypes>(iBuildingLoop);
		CvBuildingEntry* pkBuildingInfo = GC.getBuildingInfo(eBuilding);

		if(pkBuildingInfo)
		{
			if(bSkipDummy && pkBuildingInfo->IsDummy())
				continue;

			if(bSkipWW && ::isWorldWonderClass(pkBuildingInfo->GetBuildingClassInfo()))
				continue;

			if(bSkipNW && ::isNationalWonderClass(pkBuildingInfo->GetBuildingClassInfo()))
				continue;

			iResult += pkCity->GetCityBuildings()->GetNumBuilding(eBuilding);
		}
	}

	lua_pushinteger(L, iResult);
	return 1;
}

//------------------------------------------------------------------------------
//int GetWonderProductionModifier();
int CvLuaCity::lGetWonderProductionModifier(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::GetWonderProductionModifier);
}
//------------------------------------------------------------------------------
//void ChangeWonderProductionModifier(int iChange);
int CvLuaCity::lChangeWonderProductionModifier(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::ChangeWonderProductionModifier);
}
//------------------------------------------------------------------------------
//int GetLocalResourceWonderProductionMod(BuildingTypes eBuilding);
int CvLuaCity::lGetLocalResourceWonderProductionMod(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);

	lua_pushinteger(L, pkCity->GetLocalResourceWonderProductionMod(static_cast<BuildingTypes>(lua_tointeger(L, 2))));
	return 1;
}

//------------------------------------------------------------------------------
//int GetBuyPlotDistance();
int CvLuaCity::lGetBuyPlotDistance(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getBuyPlotDistance);
}

//------------------------------------------------------------------------------
//void GetWorkPlotDistance();
int CvLuaCity::lGetWorkPlotDistance(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getWorkPlotDistance);
}

#if defined(MOD_BUILDINGS_CITY_WORKING)
//------------------------------------------------------------------------------
//int getCityWorkingChange();
int CvLuaCity::lGetCityWorkingChange(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::GetCityWorkingChange);
}

//------------------------------------------------------------------------------
//void changeCityWorkingChange(int iChange);
int CvLuaCity::lChangeCityWorkingChange(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::changeCityWorkingChange);
}
#endif

#if defined(MOD_BUILDINGS_CITY_AUTOMATON_WORKERS)
//------------------------------------------------------------------------------
//int getCityAutomatonWorkersChange();
int CvLuaCity::lGetCityAutomatonWorkersChange(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::GetCityAutomatonWorkersChange);
}

//------------------------------------------------------------------------------
//void changeCityAutomatonWorkersChange(int iChange);
int CvLuaCity::lChangeCityAutomatonWorkersChange(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::changeCityAutomatonWorkersChange);
}
#endif

int CvLuaCity::lGetRemainingFreeSpecialists(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	int iTotalSpecialists = pkCity->GetNumFreeSpecialists() - pkCity->GetCityCitizens()->GetTotalSpecialistCount();
	lua_pushinteger(L, iTotalSpecialists);
	return 1;
}
int CvLuaCity::lGetBasicNeedsMedian(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	int iMedian = (int)pkCity->GetBasicNeedsMedian(false, 0);
	lua_pushinteger(L, iMedian);
	return 1;
}
int CvLuaCity::lGetGoldMedian(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	int iMedian = (int)pkCity->GetGoldMedian(false, 0);
	lua_pushinteger(L, iMedian);
	return 1;
}
int CvLuaCity::lGetScienceMedian(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	int iMedian = (int)pkCity->GetScienceMedian(false, 0);
	lua_pushinteger(L, iMedian);
	return 1;
}
int CvLuaCity::lGetCultureMedian(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	int iMedian = (int)pkCity->GetCultureMedian(false, 0);
	lua_pushinteger(L, iMedian);
	return 1;
}
int CvLuaCity::lGetReligiousUnrestPerMinorityFollower(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	int iReligiousUnrestPerMinorityFollower = 0;
	if (!GC.getGame().isOption(GAMEOPTION_NO_RELIGION))
	{
		float fUnhappyPerMinorityPop = 0.00f;
		fUnhappyPerMinorityPop += /*50.0f*/ GD_FLOAT_GET(UNHAPPINESS_PER_RELIGIOUS_MINORITY_POP) * 100;
		fUnhappyPerMinorityPop *= (100 + pkCity->GetTotalNeedModifierForYield(YIELD_FAITH, false));
		fUnhappyPerMinorityPop /= 100;
		iReligiousUnrestPerMinorityFollower = (int)fUnhappyPerMinorityPop;
	}

	lua_pushinteger(L, iReligiousUnrestPerMinorityFollower);
	return 1;
}
int CvLuaCity::lGetTheoreticalNewBasicNeedsMedian(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const BuildingTypes eBuilding = (BuildingTypes) lua_tointeger(L, 2);
	int iNewMedian = 0;
	if (eBuilding != NO_BUILDING)
	{
		CvBuildingEntry* pkBuildingInfo = GC.getBuildingInfo(eBuilding);
		if (pkBuildingInfo && pkCity->canConstruct(eBuilding, false, false, false, false))
		{
			int iModifier = pkBuildingInfo->GetBasicNeedsMedianModifier() + pkBuildingInfo->GetBasicNeedsMedianModifierGlobal();
			iNewMedian = (int)pkCity->GetBasicNeedsMedian(false, iModifier);
		}
	}
	lua_pushinteger(L, iNewMedian);
	return 1;
}
int CvLuaCity::lGetTheoreticalNewGoldMedian(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const BuildingTypes eBuilding = (BuildingTypes) lua_tointeger(L, 2);
	int iNewMedian = 0;
	if (eBuilding != NO_BUILDING)
	{
		CvBuildingEntry* pkBuildingInfo = GC.getBuildingInfo(eBuilding);
		if (pkBuildingInfo && pkCity->canConstruct(eBuilding, false, false, false, false))
		{
			int iModifier = pkBuildingInfo->GetGoldMedianModifier() + pkBuildingInfo->GetGoldMedianModifierGlobal();
			iNewMedian = (int)pkCity->GetGoldMedian(false, iModifier);
		}
	}
	lua_pushinteger(L, iNewMedian);
	return 1;
}
int CvLuaCity::lGetTheoreticalNewScienceMedian(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const BuildingTypes eBuilding = (BuildingTypes) lua_tointeger(L, 2);
	int iNewMedian = 0;
	if (eBuilding != NO_BUILDING)
	{
		CvBuildingEntry* pkBuildingInfo = GC.getBuildingInfo(eBuilding);
		if (pkBuildingInfo && pkCity->canConstruct(eBuilding, false, false, false, false))
		{
			int iModifier = pkBuildingInfo->GetScienceMedianModifier() + pkBuildingInfo->GetScienceMedianModifierGlobal();
			iNewMedian = (int)pkCity->GetScienceMedian(false, iModifier);
		}
	}
	lua_pushinteger(L, iNewMedian);
	return 1;
}
int CvLuaCity::lGetTheoreticalNewCultureMedian(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const BuildingTypes eBuilding = (BuildingTypes) lua_tointeger(L, 2);
	int iNewMedian = 0;
	if (eBuilding != NO_BUILDING)
	{
		CvBuildingEntry* pkBuildingInfo = GC.getBuildingInfo(eBuilding);
		if (pkBuildingInfo && pkCity->canConstruct(eBuilding, false, false, false, false))
		{
			int iModifier = pkBuildingInfo->GetCultureMedianModifier() + pkBuildingInfo->GetCultureMedianModifierGlobal();
			iNewMedian = (int)pkCity->GetCultureMedian(false, iModifier);
		}
	}
	lua_pushinteger(L, iNewMedian);
	return 1;
}
int CvLuaCity::lGetTheoreticalNewReligiousUnrestPerMinorityFollower(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const BuildingTypes eBuilding = (BuildingTypes) lua_tointeger(L, 2);
	int iNewReligiousUnrestPerMinorityFollower = 0;
	if (eBuilding != NO_BUILDING && !GC.getGame().isOption(GAMEOPTION_NO_RELIGION))
	{
		CvBuildingEntry* pkBuildingInfo = GC.getBuildingInfo(eBuilding);
		if (pkBuildingInfo && pkCity->canConstruct(eBuilding, false, false, false, false))
		{
			int iModifier = pkBuildingInfo->GetReligiousUnrestModifier() + pkBuildingInfo->GetReligiousUnrestModifierGlobal();
			float fUnhappyPerMinorityPop = 0.00f;
			fUnhappyPerMinorityPop += /*50.0f*/ GD_FLOAT_GET(UNHAPPINESS_PER_RELIGIOUS_MINORITY_POP) * 100;
			fUnhappyPerMinorityPop *= (100 + pkCity->GetTotalNeedModifierForYield(YIELD_FAITH, false) + iModifier);
			fUnhappyPerMinorityPop /= 100;
			iNewReligiousUnrestPerMinorityFollower = (int)fUnhappyPerMinorityPop;
		}
	}

	lua_pushinteger(L, iNewReligiousUnrestPerMinorityFollower);
	return 1;
}
//int getHappinessDelta();
int CvLuaCity::lGetCitySizeModifier(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->GetCitySizeModifier());
	return 1;
}

int CvLuaCity::lGetEmpireSizeModifier(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->GetEmpireSizeModifier());
	return 1;
}

int CvLuaCity::lGetCachedTechNeedModifier(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->GetCachedTechNeedModifier());
	return 1;
}

int CvLuaCity::lgetHappinessDelta(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->getHappinessDelta());
	return 1;
}
int CvLuaCity::lGetUnhappinessAggregated(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->GetUnhappinessAggregated());
	return 1;
}

//int GetAllNeedsModifier();
int CvLuaCity::lGetAllNeedsModifier(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->GetAllNeedsModifier(false));
	return 1;
}
int CvLuaCity::lGetUnhappinessFromYield(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const YieldTypes eIndex = (YieldTypes)lua_tointeger(L, 2);

	int iValue = 0;
	if (eIndex == YIELD_FOOD || eIndex == YIELD_PRODUCTION)
		iValue = pkCity->GetDistress(false);
	else if (eIndex == YIELD_GOLD)
		iValue = pkCity->GetPoverty(false);
	else if (eIndex == YIELD_SCIENCE)
		iValue = pkCity->GetIlliteracy(false);
	else if (eIndex == YIELD_CULTURE)
		iValue = pkCity->GetBoredom(false);
	else if (eIndex == YIELD_FAITH)
		iValue = pkCity->GetUnhappinessFromReligiousUnrest();

	lua_pushinteger(L, iValue);
	return 1;
}
//int getUnhappinessFromIsolation();
int CvLuaCity::lGetUnhappinessFromIsolation(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->GetUnhappinessFromIsolation());
	return 1;
}
//int GetUnhappinessFromPillagedTiles();
int CvLuaCity::lGetUnhappinessFromPillagedTiles(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->GetUnhappinessFromPillagedTiles());
	return 1;
}
//int GetUnhappinessFromFamine();
int CvLuaCity::lGetUnhappinessFromFamine(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->GetUnhappinessFromFamine());
	return 1;
}
//int GetUnhappinessFromReligiousUnrest();
int CvLuaCity::lGetUnhappinessFromReligiousUnrest(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->GetUnhappinessFromReligiousUnrest());
	return 1;
}

int CvLuaCity::lgetPotentialUnhappinessWithGrowth(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	CvString string = pkCity->getPotentialUnhappinessWithGrowth() + pkCity->GetPotentialHappinessWithGrowth();
	lua_pushstring(L, string);
	return 1;
}

int CvLuaCity::lGetCityUnhappinessBreakdown(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const bool bIncludeMedian = luaL_optbool(L, 2, false);
	const bool bFlavor = luaL_optbool(L, 3, false);
	lua_pushstring(L, pkCity->GetCityUnhappinessBreakdown(bIncludeMedian, bFlavor));
	return 1;
}

int CvLuaCity::lGetCityHappinessBreakdown(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushstring(L, pkCity->GetCityHappinessBreakdown());
	return 1;
}

int CvLuaCity::lgetUnhappinessFromSpecialists(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->getUnhappinessFromSpecialists(pkCity->GetCityCitizens()->GetTotalSpecialistCount()));
	return 1;
}

//------------------------------------------------------------------------------
//void changeHealRate(int iChange);
int CvLuaCity::lChangeHealRate(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::changeHealRate);
}

//------------------------------------------------------------------------------
//bool IsNoOccupiedUnhappiness();
int CvLuaCity::lIsNoOccupiedUnhappiness(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::IsNoOccupiedUnhappiness);
}
//------------------------------------------------------------------------------
//int getFood();
int CvLuaCity::lGetFood(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getFood);
}
//------------------------------------------------------------------------------
//int getFoodTimes100();
int CvLuaCity::lGetFoodTimes100(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getFoodTimes100);
}
//------------------------------------------------------------------------------
//void setFood(int iNewValue);
int CvLuaCity::lSetFood(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::setFood);
}
//------------------------------------------------------------------------------
//void changeFood(int iChange);
int CvLuaCity::lChangeFood(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::changeFood);
}
//------------------------------------------------------------------------------
//int getFoodKept();
int CvLuaCity::lGetFoodKept(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	int iFoodKept = (min(pkCity->getFood(),pkCity->growthThreshold())*pkCity->getMaxFoodKeptPercent()) / 100;
	lua_pushinteger(L, iFoodKept);
	return 1;
}
//------------------------------------------------------------------------------
//int getMaxFoodKeptPercent();
int CvLuaCity::lGetMaxFoodKeptPercent(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getMaxFoodKeptPercent);
}
//------------------------------------------------------------------------------
//int getOverflowProduction();
int CvLuaCity::lGetOverflowProduction(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getOverflowProduction);
}
//------------------------------------------------------------------------------
//void setOverflowProduction(int iNewValue);
int CvLuaCity::lSetOverflowProduction(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::setOverflowProduction);
}
//------------------------------------------------------------------------------
//void changeOverflowProduction(int iChange);
int CvLuaCity::lChangeOverflowProduction(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::changeOverflowProduction);
}
//------------------------------------------------------------------------------
//int getFeatureProduction();
int CvLuaCity::lGetFeatureProduction(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getFeatureProduction);
}
//------------------------------------------------------------------------------
//void setFeatureProduction(int iNewValue);
int CvLuaCity::lSetFeatureProduction(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::setFeatureProduction);
}
//------------------------------------------------------------------------------
//int getMilitaryProductionModifier();
int CvLuaCity::lGetMilitaryProductionModifier(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = pkCity->getMilitaryProductionModifier();

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int getSpaceProductionModifier();
int CvLuaCity::lGetSpaceProductionModifier(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = pkCity->getSpaceProductionModifier();

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//void CreateApolloProgram();
int CvLuaCity::lCreateApolloProgram(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	if(pkCity != NULL)
	{
		ProjectTypes eApolloProgram = (ProjectTypes)GD_INT_GET(SPACE_RACE_TRIGGER_PROJECT);
		pkCity->CreateProject(eApolloProgram);
	}

	return 0;
}
//------------------------------------------------------------------------------
//int getBuildingDefense();
int CvLuaCity::lGetBuildingDefense(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = pkCity->GetCityBuildings()->GetBuildingDefense();

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int getFreeExperience();
int CvLuaCity::lGetFreeExperience(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = pkCity->getFreeExperience();

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int getNukeModifier();
int CvLuaCity::lGetNukeModifier(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = pkCity->getNukeModifier();

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int getFreeSpecialist();
//int CvLuaCity::lGetFreeSpecialist(lua_State* L)
//{
//	CvCity* pkCity = GetInstance(L);
//	const int iResult = pkCity->getFreeSpecialist();
//
//	lua_pushinteger(L, iResult);
//	return 1;
//}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//int IsResistance();
int CvLuaCity::lIsResistance(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::IsResistance);
}
//int GetResistanceTurns();
int CvLuaCity::lGetResistanceTurns(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::GetResistanceTurns);
}
//int ChangeResistanceTurns();
int CvLuaCity::lChangeResistanceTurns(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::ChangeResistanceTurns);
}

//int IsRazing();
int CvLuaCity::lIsRazing(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::IsRazing);
}
//int GetRazingTurns();
int CvLuaCity::lGetRazingTurns(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::GetRazingTurns);
}
//int ChangeRazingTurns();
int CvLuaCity::lChangeRazingTurns(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::ChangeRazingTurns);
}

//int IsOccupied();
int CvLuaCity::lIsOccupied(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::IsOccupied);
}
//------------------------------------------------------------------------------
//void SetOccupied(bool bValue);
int CvLuaCity::lSetOccupied(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::SetOccupied);
}
//------------------------------------------------------------------------------
//int IsPuppet();
int CvLuaCity::lIsPuppet(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::IsPuppet);
}
//------------------------------------------------------------------------------
//void SetPuppet(bool bValue);
int CvLuaCity::lSetPuppet(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::SetPuppet);
}
//------------------------------------------------------------------------------
int CvLuaCity::lGetHappinessFromBuildings(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::GetHappinessFromBuildings);
}
//------------------------------------------------------------------------------
int CvLuaCity::lGetLocalHappiness(lua_State* L)
{
	//careful, lua supplies its own default parameters for int (0) and bool (false)
	return BasicLuaMethod(L, &CvCity::GetLocalHappiness);
}
//------------------------------------------------------------------------------
int CvLuaCity::lGetHappiness(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	int iHappiness = pkCity->GetHappinessFromBuildings();
	lua_pushinteger(L, iHappiness);
	return 1;
}
//------------------------------------------------------------------------------
//bool isNeverLost();
int CvLuaCity::lIsNeverLost(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::isNeverLost);
}
//------------------------------------------------------------------------------
//void setNeverLost(int iNewValue);
int CvLuaCity::lSetNeverLost(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::setNeverLost);
}
//------------------------------------------------------------------------------
//bool isDrafted();
int CvLuaCity::lIsDrafted(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::isDrafted);
}
//------------------------------------------------------------------------------
//void setDrafted(int iNewValue);
int CvLuaCity::lSetDrafted(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::setDrafted);
}

//------------------------------------------------------------------------------
int CvLuaCity::lIsBlockaded(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::IsBlockadedWaterAndLand);
}

int CvLuaCity::lIsMined(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const bool bResult = pkCity->GetDeepWaterTileDamage() > 0;

	lua_pushboolean(L, bResult);
	return 1;
}

int CvLuaCity::lIsBorderObstacleLand(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::IsBorderObstacleLand);
}

int CvLuaCity::lIsBorderObstacleWater(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::IsBorderObstacleWater);
}

//------------------------------------------------------------------------------
//int GetWeLoveTheKingDayCounter();
int CvLuaCity::lGetWeLoveTheKingDayCounter(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::GetWeLoveTheKingDayCounter);
}
//------------------------------------------------------------------------------
//void SetWeLoveTheKingDayCounter(int iValue);
int CvLuaCity::lSetWeLoveTheKingDayCounter(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::SetWeLoveTheKingDayCounter);
}
//------------------------------------------------------------------------------
//void ChangeWeLoveTheKingDayCounter(int iChange);
int CvLuaCity::lChangeWeLoveTheKingDayCounter(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::ChangeWeLoveTheKingDayCounter);
}
//------------------------------------------------------------------------------
//void GetNumThingsProduced() const;
int CvLuaCity::lGetNumThingsProduced(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::GetNumThingsProduced);
}

//------------------------------------------------------------------------------
//bool isCitizensAutomated();
//int CvLuaCity::lIsCitizensAutomated(lua_State* L)
//{
//	return BasicLuaMethod(L, &CvCity::isCitizensAutomated);
//}
////------------------------------------------------------------------------------
////void setCitizensAutomated(bool bNewValue);
//int CvLuaCity::lSetCitizensAutomated(lua_State* L)
//{
//	return BasicLuaMethod(L, &CvCity::setCitizensAutomated);
//}
//------------------------------------------------------------------------------
//bool isProductionAutomated();
int CvLuaCity::lIsProductionAutomated(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::isProductionAutomated);
}
//------------------------------------------------------------------------------
//void setProductionAutomated(bool bNewValue);
int CvLuaCity::lSetProductionAutomated(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::setProductionAutomated);
}
//------------------------------------------------------------------------------
//void setCitySizeBoost(int iBoost);
int CvLuaCity::lSetCitySizeBoost(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::setCitySizeBoost);
}
//------------------------------------------------------------------------------
//PlayerTypes getOwner();
int CvLuaCity::lGetOwner(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getOwner);
}
//------------------------------------------------------------------------------
//TeamTypes getTeam();
int CvLuaCity::lGetTeam(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getTeam);
}
//------------------------------------------------------------------------------
//PlayerTypes getPreviousOwner();
int CvLuaCity::lGetPreviousOwner(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getPreviousOwner);
}
//------------------------------------------------------------------------------
//PlayerTypes getOriginalOwner();
int CvLuaCity::lGetOriginalOwner(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getOriginalOwner);
}
//------------------------------------------------------------------------------
//int getSeaPlotYield(YieldTypes eIndex);
int CvLuaCity::lGetSeaPlotYield(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const YieldTypes eIndex = (YieldTypes)lua_tointeger(L, 2);
	const int iResult = pkCity->getSeaPlotYield(eIndex);

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int getRiverPlotYield(YieldTypes eIndex);
int CvLuaCity::lGetRiverPlotYield(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const YieldTypes eIndex = (YieldTypes)lua_tointeger(L, 2);
	const int iResult = pkCity->getRiverPlotYield(eIndex);

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int getLakePlotYield(YieldTypes eIndex);
int CvLuaCity::lGetLakePlotYield(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const YieldTypes eIndex = (YieldTypes)lua_tointeger(L, 2);
	const int iResult = pkCity->getLakePlotYield(eIndex);

	lua_pushinteger(L, iResult);
	return 1;
}

//------------------------------------------------------------------------------
//int getBaseYieldRate(YieldTypes eIndex, bool bTooltip);
//LEGACY METHOD, use GetBaseYieldRateTimes100 instead
int CvLuaCity::lGetBaseYieldRate(lua_State* L)
{
	CvCity* pCity = GetInstance(L);
	const YieldTypes eYield = static_cast<YieldTypes>(lua_tointeger(L, 2));
	int iResult;
	iResult = pCity->getBaseYieldRateTimes100(eYield) / 100;
	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int getBaseYieldRateTimes100(YieldTypes eIndex, bool bTooltip);
int CvLuaCity::lGetBaseYieldRateTimes100(lua_State* L)
{
	CvCity* pCity = GetInstance(L);
	const YieldTypes eYield = static_cast<YieldTypes>(lua_tointeger(L, 2));
	int iResult;
	iResult = pCity->getBaseYieldRateTimes100(eYield);
	lua_pushinteger(L, iResult);
	return 1;
}
//-------------------------------------------------------------------------
int CvLuaCity::lGetYieldPerTurnFromMinors(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const YieldTypes eYield = (YieldTypes)lua_tointeger(L, 2);
	const int iResult = pkCity->GetYieldFromMinors(eYield);
	lua_pushinteger(L, iResult);
	return 1;
}
//-------------------------------------------------------------------------
int CvLuaCity::lSetYieldPerTurnFromMinors(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const YieldTypes eYield = (YieldTypes)lua_tointeger(L, 2);
	const int iValue = lua_tointeger(L, 3);
	pkCity->SetYieldFromMinors(eYield, iValue);
	return 1;
}
#if defined(MOD_GLOBAL_GREATWORK_YIELDTYPES)
//------------------------------------------------------------------------------
// LEGACY METHOD
int CvLuaCity::lGetBaseYieldRateFromGreatWorks(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const YieldTypes eYield = (YieldTypes)lua_tointeger(L, 2);
	const int iResult = pkCity->GetBaseYieldRateFromGreatWorksTimes100(eYield) / 100;
	lua_pushinteger(L, iResult);
	return 1;
}
#endif
//------------------------------------------------------------------------------
int CvLuaCity::lGetBaseYieldRateFromTerrain(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::GetBaseYieldRateFromTerrain);
}
//------------------------------------------------------------------------------
int CvLuaCity::lChangeBaseYieldRateFromTerrain(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::ChangeBaseYieldRateFromTerrain);
}
//------------------------------------------------------------------------------
int CvLuaCity::lGetBaseYieldRateFromBuildings(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::GetBaseYieldRateFromBuildings);
}
//------------------------------------------------------------------------------
int CvLuaCity::lChangeBaseYieldRateFromBuildings(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::ChangeBaseYieldRateFromBuildings);
}
//------------------------------------------------------------------------------
int CvLuaCity::lGetBaseYieldRateFromSpecialists(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::GetBaseYieldRateFromSpecialists);
}
//------------------------------------------------------------------------------
int CvLuaCity::lChangeBaseYieldRateFromSpecialists(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::ChangeBaseYieldRateFromSpecialists);
}
//------------------------------------------------------------------------------
int CvLuaCity::lGetBaseYieldRateFromMisc(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::GetBaseYieldRateFromMisc);
}
//------------------------------------------------------------------------------
int CvLuaCity::lChangeBaseYieldRateFromMisc(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::ChangeBaseYieldRateFromMisc);
}
// Base yield rate from active conversion process
// LEGACY METHOD, do not use
int CvLuaCity::lGetBaseYieldRateFromProcess(lua_State* L)
{
	lua_pushinteger(L, 0);
	return 1;
}
//	Base yield rate from trade routes established with this city
int CvLuaCity::lGetBaseYieldRateFromTradeRoutes(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	YieldTypes eIndex = (YieldTypes)lua_tointeger(L, 2);
	int iReturnValue = GET_PLAYER(pkCity->getOwner()).GetTrade()->GetTradeValuesAtCityTimes100(pkCity, eIndex);
	lua_pushinteger(L, iReturnValue);
	return 1;
}
// Base yield rate from League
int CvLuaCity::lGetBaseYieldRateFromLeague(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::GetBaseYieldRateFromLeague);
}
//------------------------------------------------------------------------------
int CvLuaCity::lGetYieldFromCityYieldTimes100(lua_State* L)
{
	int iResult = 0;
	CvCity* pkCity = GetInstance(L);
	YieldTypes eIndex1 = (YieldTypes)lua_tointeger(L, 2);
	for(int iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		YieldTypes eIndex2 = (YieldTypes)iI;
		if(eIndex2 == NO_YIELD)
		{
			continue;
		}
		if (eIndex2 == eIndex1)
		{
			continue;
		}
		//NOTE! We flip it here, because we want the OUT yield
		iResult += pkCity->GetRealYieldFromYieldTimes100(eIndex2, eIndex1);
	}
	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
int CvLuaCity::lGetBaseYieldRateFromReligion(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	YieldTypes eIndex = (YieldTypes)lua_tointeger(L, 2);
	lua_pushinteger(L, pkCity->GetBaseYieldRateFromReligionTimes100(eIndex) / 100);
	return 1;
}
//------------------------------------------------------------------------------
int CvLuaCity::lChangeBaseYieldRateFromReligion(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::ChangeBaseYieldRateFromReligion);
}
//------------------------------------------------------------------------------
int CvLuaCity::lGetYieldPerPopTimes100(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::GetYieldPerPopTimes100);
}

#if defined(MOD_BALANCE_CORE)
//------------------------------------------------------------------------------
int CvLuaCity::lGetYieldPerPopInEmpireTimes100(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::GetYieldPerPopInEmpireTimes100);
}
#endif

//------------------------------------------------------------------------------
int CvLuaCity::lGetYieldFromYieldPerBuildingTimes100(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const YieldTypes eIndex = (YieldTypes)lua_tointeger(L, 2);
	if (eIndex == YIELD_CULTURE || eIndex == YIELD_FAITH)
	{
		// these yields don't support decimal values
		const int iResult = (pkCity->GetYieldPerBuilding(eIndex) * pkCity->GetCityBuildings()->GetNumBuildings()).Truncate() * 100;
		lua_pushinteger(L, iResult);
	}
	else
	{
		const int iResult = (pkCity->GetYieldPerBuilding(eIndex) * pkCity->GetCityBuildings()->GetNumBuildings() * 100).Truncate();
		lua_pushinteger(L, iResult);
	}
	return 1;
}

//------------------------------------------------------------------------------
//int getBaseYieldRateModifier(YieldTypes eIndex);
int CvLuaCity::lGetBaseYieldRateModifier(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const YieldTypes eIndex = (YieldTypes)lua_tointeger(L, 2);
	const int iResult = pkCity->getBaseYieldRateModifier(eIndex);

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int getYieldRate(YieldTypes eIndex);
//LEGACY METHOD, use getYieldRateTimes100 instead
int CvLuaCity::lGetYieldRate(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const YieldTypes eIndex = (YieldTypes)lua_tointeger(L, 2);
	const bool bIgnoreTrade = luaL_optbool(L, 3, false);
	const int iResult = pkCity->getYieldRateTimes100(eIndex, bIgnoreTrade) / 100;

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int getYieldRateTimes100(YieldTypes eIndex);
int CvLuaCity::lGetYieldRateTimes100(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const YieldTypes eIndex = (YieldTypes)lua_tointeger(L, 2);
	const bool bIgnoreTrade = luaL_optbool(L, 3, false);
	const bool bIgnoreProcess = luaL_optbool(L, 4, false);
	const int iResult = pkCity->getYieldRateTimes100(eIndex, bIgnoreTrade, bIgnoreProcess);

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int getYieldRateModifier(YieldTypes eIndex);
int CvLuaCity::lGetYieldRateModifier(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const YieldTypes eIndex = (YieldTypes)lua_tointeger(L, 2);
	const int iResult = pkCity->getYieldRateModifier(eIndex);

	lua_pushinteger(L, iResult);
	return 1;
}

//------------------------------------------------------------------------------
//int getExtraSpecialistYield(YieldTypes eIndex);
int CvLuaCity::lGetExtraSpecialistYield(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const YieldTypes eIndex = (YieldTypes)lua_tointeger(L, 2);
	const int iResult = pkCity->getExtraSpecialistYield(eIndex);

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int getExtraSpecialistYieldOfType(YieldTypes eIndex, SpecialistTypes eSpecialist);
int CvLuaCity::lGetExtraSpecialistYieldOfType(lua_State* L)
{
	return BasicLuaMethod<int, YieldTypes, SpecialistTypes>(L, &CvCity::getExtraSpecialistYield);
}

//------------------------------------------------------------------------------
//int getDomainFreeExperience(DomainTypes eIndex);
int CvLuaCity::lGetDomainFreeExperience(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getDomainFreeExperience);
}
//------------------------------------------------------------------------------
//int getDomainProductionModifier(DomainTypes eIndex);
int CvLuaCity::lGetDomainProductionModifier(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getDomainProductionModifier);
}
//------------------------------------------------------------------------------
//bool isEverOwned(PlayerTypes eIndex);
int CvLuaCity::lIsEverOwned(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::isEverOwned);
}
//------------------------------------------------------------------------------
//bool isEverOwned(PlayerTypes eIndex);
int CvLuaCity::lGetNumTimesOwned(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::GetNumTimesOwned);
}
//------------------------------------------------------------------------------
//bool isRevealed(TeamTypes eIndex, bool bDebug);
int CvLuaCity::lIsRevealed(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::isRevealed);
}
//------------------------------------------------------------------------------
//void setRevealed(TeamTypes eIndex, bool bNewValue);
int CvLuaCity::lSetRevealed(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::setRevealed);
}
//------------------------------------------------------------------------------
//std::string getName();
int CvLuaCity::lGetName(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushstring(L, pkCity->getName());
	return 1;
}

//------------------------------------------------------------------------------
//std::string getNameForm(int iForm);
//------------------------------------------------------------------------------
//string GetNameKey();
int CvLuaCity::lGetNameKey(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushstring(L, pkCity->getNameKey());
	return 1;
}
//------------------------------------------------------------------------------
//void SetName(string szNewValue, bool bFound);
int CvLuaCity::lSetName(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);

	const char* cityName = lua_tostring(L, 2);
	const bool bFound = luaL_optbool(L, 3, false);

	pkCity->setName(cityName, bFound);
	return 0;
}
//------------------------------------------------------------------------------
//bool IsHasResourceLocal(ResourceTypes iResource);
int CvLuaCity::lIsHasResourceLocal(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResource = lua_tointeger(L, 2);
	const bool bTestVisible = lua_toboolean(L, 3);
	const bool bResult = pkCity->IsHasResourceLocal((ResourceTypes)iResource, bTestVisible);

	lua_pushboolean(L, bResult);
	return 1;
}
//------------------------------------------------------------------------------
//int getBuildingProduction(BuildingTypes iIndex);
int CvLuaCity::lGetBuildingProduction(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iIndex = lua_tointeger(L, 2);
	if(iIndex != NO_BUILDING)
	{
		const int iResult = pkCity->GetCityBuildings()->GetBuildingProduction((BuildingTypes)iIndex);
		lua_pushinteger(L, iResult);
	}
	else
	{
		lua_pushinteger(L, 0);
	}
	return 1;
}
//------------------------------------------------------------------------------
//void setBuildingProduction(BuildingTypes iIndex, int iNewValue);
int CvLuaCity::lSetBuildingProduction(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iIndex = lua_tointeger(L, 2);
	const int iNewValue = lua_tointeger(L, 3);
	if(iIndex != NO_BUILDING)
	{
		pkCity->GetCityBuildings()->SetBuildingProduction((BuildingTypes)iIndex, iNewValue);
	}

	return 1;
}
//------------------------------------------------------------------------------
//void changeBuildingProduction(BuildingTypes iIndex, int iChange);
int CvLuaCity::lChangeBuildingProduction(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iIndex = lua_tointeger(L, 2);
	const int iChange = lua_tointeger(L, 3);
	if(iIndex != NO_BUILDING)
	{
		pkCity->GetCityBuildings()->ChangeBuildingProduction((BuildingTypes)iIndex, iChange);
	}

	return 1;
}
//------------------------------------------------------------------------------
//int getBuildingProductionTime(BuildingTypes eIndex);
int CvLuaCity::lGetBuildingProductionTime(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iIndex = lua_tointeger(L, 2);
	if(iIndex != NO_BUILDING)
	{
		const int iResult = pkCity->GetCityBuildings()->GetBuildingProductionTime((BuildingTypes)iIndex);
		lua_pushinteger(L, iResult);
	}
	else
	{
		lua_pushinteger(L, 0);
	}
	return 1;
}
//------------------------------------------------------------------------------
//void setBuildingProductionTime(BuildingTypes eIndex, int iNewValue);
int CvLuaCity::lSetBuildingProductionTime(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iIndex = lua_tointeger(L, 2);
	const int iNewValue = lua_tointeger(L, 3);
	if(iIndex != NO_BUILDING)
	{
		pkCity->GetCityBuildings()->SetBuildingProductionTime((BuildingTypes)iIndex, iNewValue);
	}

	return 1;
}
//------------------------------------------------------------------------------
//void changeBuildingProductionTime(BuildingTypes eIndex, int iChange);
int CvLuaCity::lChangeBuildingProductionTime(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iIndex = toValue<BuildingTypes>(L, 2);
	if(iIndex != NO_BUILDING)
	{
		pkCity->GetCityBuildings()->ChangeBuildingProductionTime((BuildingTypes)iIndex, toValue<int>(L, 3));
	}

	return 1;
}
//------------------------------------------------------------------------------
//int getBuildingOriginalOwner(BuildingTypes iIndex);
int CvLuaCity::lGetBuildingOriginalOwner(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iIndex = toValue<BuildingTypes>(L, 2);
	if(iIndex != NO_BUILDING)
	{
		const int iResult = pkCity->GetCityBuildings()->GetBuildingOriginalOwner((BuildingTypes)iIndex);
		lua_pushinteger(L, iResult);
	}
	else
	{
		lua_pushinteger(L, -1);
	}
	return 1;
}
//------------------------------------------------------------------------------
//int getBuildingOriginalTime(BuildingTypes iIndex);
int CvLuaCity::lGetBuildingOriginalTime(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iIndex = toValue<BuildingTypes>(L, 2);
	if(iIndex != NO_BUILDING)
	{
		const int iResult = pkCity->GetCityBuildings()->GetBuildingOriginalTime((BuildingTypes)iIndex);
		lua_pushinteger(L, iResult);
	}
	else
	{
		lua_pushinteger(L, 0);
	}
	return 1;
}
//------------------------------------------------------------------------------
//int getUnitProduction(int iIndex);
int CvLuaCity::lGetUnitProduction(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = pkCity->getUnitProduction(toValue<UnitTypes>(L, 2));

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//void setUnitProduction(int iIndex, int iNewValue);
int CvLuaCity::lSetUnitProduction(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	pkCity->setUnitProduction(toValue<UnitTypes>(L, 2), toValue<int>(L, 3));

	return 1;
}
//------------------------------------------------------------------------------
//void changeUnitProduction(UnitTypes iIndex, int iChange);
int CvLuaCity::lChangeUnitProduction(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	pkCity->changeUnitProduction(toValue<UnitTypes>(L, 2), toValue<int>(L, 3));

	return 1;
}
//------------------------------------------------------------------------------
//int IsCanAddSpecialistToBuilding(BuildingTypes eBuilding);
int CvLuaCity::lIsCanAddSpecialistToBuilding(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	BuildingTypes bt = toValue<BuildingTypes>(L, 2);
	if(bt != NO_BUILDING)
	{
		const bool bResult = pkCity->GetCityCitizens()->IsCanAddSpecialistToBuilding(bt);
		lua_pushboolean(L, bResult);
	}
	else
	{
		lua_pushboolean(L, false);
	}
	return 1;
}
//------------------------------------------------------------------------------
//int GetSpecialistUpgradeThreshold();
int CvLuaCity::lGetSpecialistUpgradeThreshold(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const UnitClassTypes eUnitClassType = toValue<UnitClassTypes>(L, 2);
	const int iResult = pkCity->GetCityCitizens()->GetSpecialistUpgradeThreshold(eUnitClassType);

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int GetNumSpecialistsAllowedByBuilding(BuildingTypes eBuilding);
int CvLuaCity::lGetNumSpecialistsAllowedByBuilding(lua_State* L)
{
	int iResult = 0;
	CvCity* pkCity = GetInstance(L);
	BuildingTypes bt = toValue<BuildingTypes>(L, 2);
	if(bt != NO_BUILDING)
	{
		CvBuildingEntry* pkBuildingInfo = GC.getBuildingInfo(bt);
		if(pkBuildingInfo)
		{
			iResult = pkCity->GetCityCitizens()->GetNumSpecialistsAllowedByBuilding(*pkBuildingInfo);
		}
	}

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int getSpecialistCount(SpecialistTypes eIndex);
int CvLuaCity::lGetSpecialistCount(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = pkCity->GetCityCitizens()->GetSpecialistCount(toValue<SpecialistTypes>(L, 2));

	lua_pushinteger(L, iResult);
	return 1;
}
#if defined(MOD_BALANCE_CORE)
//------------------------------------------------------------------------------
//int GetTotalSpecialistCount();
int CvLuaCity::lGetTotalSpecialistCount(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = pkCity->GetCityCitizens()->GetTotalSpecialistCount();

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int lGetSpecialistCityModifier(SpecialistTypes eIndex);
int CvLuaCity::lGetSpecialistCityModifier(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	int iResult = pkCity->GetSpecialistRateModifierFromBuildings(toValue<SpecialistTypes>(L, 2));

	GreatPersonTypes eGreatPerson = GetGreatPersonFromSpecialist((SpecialistTypes)toValue<SpecialistTypes>(L, 2));

	if (eGreatPerson != NO_GREATPERSON)
	{

		int iNumPuppets = GET_PLAYER(pkCity->getOwner()).GetNumPuppetCities();
		if (iNumPuppets > 0)
		{

			iResult += (iNumPuppets * GET_PLAYER(pkCity->getOwner()).GetPlayerTraits()->GetPerPuppetGreatPersonRateModifier(eGreatPerson));
		}

		iResult += pkCity->GetReligionGreatPersonRateModifier(eGreatPerson);
	}

	lua_pushinteger(L, iResult);
	return 1;
}
#endif
//------------------------------------------------------------------------------
//int GetSpecialistGreatPersonProgress(SpecialistTypes eIndex);
int CvLuaCity::lGetSpecialistGreatPersonProgress(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = pkCity->GetCityCitizens()->GetSpecialistGreatPersonProgress(toValue<SpecialistTypes>(L, 2));
	lua_pushinteger(L, iResult);
	return 1;
}

//------------------------------------------------------------------------------
//int GetSpecialistGreatPersonProgressTimes100(SpecialistTypes eIndex);
int CvLuaCity::lGetSpecialistGreatPersonProgressTimes100(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = pkCity->GetCityCitizens()->GetSpecialistGreatPersonProgressTimes100(toValue<SpecialistTypes>(L, 2));
	lua_pushinteger(L, iResult);
	return 1;
}

//------------------------------------------------------------------------------
//int ChangeSpecialistGreatPersonProgressTimes100(SpecialistTypes eIndex, int iChange);
int CvLuaCity::lChangeSpecialistGreatPersonProgressTimes100(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iChange = lua_tointeger(L, 3);
	pkCity->GetCityCitizens()->ChangeSpecialistGreatPersonProgressTimes100(toValue<SpecialistTypes>(L, 2), iChange, true);
	return 1;
}

int CvLuaCity::lGetExtraSpecialistPoints(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const SpecialistTypes eSpecialist = (SpecialistTypes)lua_tointeger(L, 2);

	if (eSpecialist != NO_SPECIALIST)
	{
		ReligionTypes eMajority = pkCity->GetCityReligions()->GetReligiousMajority();
		if (eMajority != NO_RELIGION)
		{
			const CvReligion* pReligion = GC.getGame().GetGameReligions()->GetReligion(eMajority, pkCity->getOwner());
			if (pReligion)
			{
				lua_pushinteger(L, pReligion->m_Beliefs.GetGreatPersonPoints(GetGreatPersonFromSpecialist(eSpecialist), pkCity->getOwner(), pkCity, true));
				return 1;
			}
		}
	}
	lua_pushinteger(L, 0);
	return 1;
}

//------------------------------------------------------------------------------
//int GetNumSpecialistsInBuilding(BuildingTypes eIndex);
int CvLuaCity::lGetNumSpecialistsInBuilding(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = pkCity->GetCityCitizens()->GetNumSpecialistsInBuilding(toValue<BuildingTypes>(L, 2));
	lua_pushinteger(L, iResult);
	return 1;
}

//int GetNumFprcedSpecialistsInBuilding(BuildingTypes eIndex);
int CvLuaCity::lGetNumForcedSpecialistsInBuilding(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = pkCity->GetCityCitizens()->GetNumForcedSpecialistsInBuilding(toValue<BuildingTypes>(L, 2));
	lua_pushinteger(L, iResult);
	return 1;
}

//------------------------------------------------------------------------------
//int DoReallocateCitizens();
int CvLuaCity::lDoReallocateCitizens(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	pkCity->GetCityCitizens()->DoReallocateCitizens();

	return 1;
}

//------------------------------------------------------------------------------
//int DoVerifyWorkingPlots();
int CvLuaCity::lDoVerifyWorkingPlots(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	pkCity->GetCityCitizens()->DoVerifyWorkingPlots();

	return 1;
}

//------------------------------------------------------------------------------
//int IsNoAutoAssignSpecialists();
int CvLuaCity::lIsNoAutoAssignSpecialists(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const bool bResult = pkCity->GetCityCitizens()->IsNoAutoAssignSpecialists();

	lua_pushboolean(L, bResult);

	return 1;
}

//------------------------------------------------------------------------------
//int GetFocusType();
int CvLuaCity::lGetFocusType(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = pkCity->GetCityCitizens()->GetFocusType();

	lua_pushinteger(L, iResult);

	return 1;
}

//------------------------------------------------------------------------------
//int DoVerifyWorkingPlots();
int CvLuaCity::lSetFocusType(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iFocus = lua_tointeger(L, 2);

	pkCity->GetCityCitizens()->SetFocusType((CityAIFocusTypes) iFocus, true);

	return 1;
}

//------------------------------------------------------------------------------
//int GetForcedAvoidGrowth();
int CvLuaCity::lIsForcedAvoidGrowth(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = pkCity->GetCityCitizens()->IsForcedAvoidGrowth();

	lua_pushboolean(L, iResult);

	return 1;
}



//------------------------------------------------------------------------------
//int getUnitCombatFreeExperience(UnitCombatTypes eIndex);
int CvLuaCity::lGetUnitCombatFreeExperience(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getUnitCombatFreeExperience);
}
//------------------------------------------------------------------------------
//int getFreePromotionCount(PromotionTypes eIndex);
int CvLuaCity::lGetFreePromotionCount(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getFreePromotionCount);
}
//------------------------------------------------------------------------------
//bool isFreePromotion(PromotionTypes eIndex);
int CvLuaCity::lIsFreePromotion(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	PromotionTypes promo = CvLuaArgs::toValue<PromotionTypes>(L, 2);

	bool bFound = false;
	vector<PromotionTypes> freePromotions = pkCity->getFreePromotions();
	for (size_t iI = 0; iI < freePromotions.size(); iI++)
	{
		if (freePromotions[iI] == promo)
		{
			bFound = true;
			break;
		}
	}

	lua_pushboolean(L, bFound);
	return 1;
}
//------------------------------------------------------------------------------
//int getSpecialistFreeExperience();
int CvLuaCity::lGetSpecialistFreeExperience(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = pkCity->getSpecialistFreeExperience();

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//void updateStrengthValue();
int CvLuaCity::lUpdateStrengthValue(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	pkCity->updateStrengthValue();

	return 1;
}
//------------------------------------------------------------------------------
//int getStrengthValue(bForRangeStrike);
int CvLuaCity::lGetStrengthValue(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	bool bForRangeStrike = luaL_optbool(L, 2, false);
	bool bIgnoreBuildingDefense = luaL_optbool(L, 3, false);
	CvUnit* pkOther = CvLuaUnit::GetInstance(L, 4, false);
	const int iResult = pkCity->getStrengthValue(bForRangeStrike, bIgnoreBuildingDefense, pkOther);

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int getDamage();
int CvLuaCity::lGetDamage(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = pkCity->getDamage();

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//void setDamage(int iValue);
int CvLuaCity::lSetDamage(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::setDamage);
}
//------------------------------------------------------------------------------
//void changeDamage(int iChange);
int CvLuaCity::lChangeDamage(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::changeDamage);
}
//------------------------------------------------------------------------------
//int GetMaxHitPoints();
int CvLuaCity::lGetMaxHitPoints(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = pkCity->GetMaxHitPoints();

	lua_pushinteger(L, iResult);
	return 1;
}
#if defined(MOD_EVENTS_CITY_BOMBARD)
//int, bool GetBombardRange();
int CvLuaCity::lGetBombardRange(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	bool bIndirectFireAllowed;
	const int iResult = pkCity->getBombardRange(bIndirectFireAllowed);

	lua_pushinteger(L, iResult);
	lua_pushinteger(L, bIndirectFireAllowed);
	return 2;
}
int CvLuaCity::lGetCityBuildingRangeStrikeModifier(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = pkCity->getCityBuildingRangeStrikeModifier();
	lua_pushinteger(L, iResult);
	return 1;
}
#endif
//------------------------------------------------------------------------------
//bool CanRangeStrike()
int CvLuaCity::lCanRangeStrike(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::canRangeStrike);
}
//------------------------------------------------------------------------------
int CvLuaCity::lCanRangeStrikeNow(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::CanRangeStrikeNow);
}
//------------------------------------------------------------------------------
//bool CanRangeStrikeAt(int x, int y)
int CvLuaCity::lCanRangeStrikeAt(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::canRangeStrikeAt);
}
//------------------------------------------------------------------------------
int CvLuaCity::lHasPerformedRangedStrikeThisTurn(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::isMadeAttack);
}
//------------------------------------------------------------------------------
int CvLuaCity::lRangeCombatUnitDefense(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	CvUnit* pkDefender = CvLuaUnit::GetInstance(L, 2);

	const int iDefenseStr = pkCity->rangeCombatUnitDefense(pkDefender);
	lua_pushinteger(L, iDefenseStr);
	return 1;
}
//------------------------------------------------------------------------------
int CvLuaCity::lRangeCombatDamage(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	CvUnit* pkDefendingUnit = CvLuaUnit::GetInstance(L, 2, false);
	//ignored
	//CvCity* pkDefendingCity = GetInstance(L, 3, false);
	bool bIncludeRand = luaL_optbool(L, 4, false);

	const int iRangedDamage = pkCity->rangeCombatDamage(pkDefendingUnit, bIncludeRand);
	lua_pushinteger(L, iRangedDamage);
	return 1;
}
//------------------------------------------------------------------------------
int CvLuaCity::lGetAirStrikeDefenseDamage(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	CvUnit* pkAttackingUnit = CvLuaUnit::GetInstance(L, 2, false);
	bool bIncludeRand = luaL_optbool(L, 3, false);

	const int iRangedDamage = pkCity->GetAirStrikeDefenseDamage(pkAttackingUnit, bIncludeRand);
	lua_pushinteger(L, iRangedDamage);
	return 1;
}
//------------------------------------------------------------------------------
int CvLuaCity::lGetMultiAttackBonusCity(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	CvUnit* pkUnit = CvLuaUnit::GetInstance(L, 2);

	if (pkUnit == NULL || pkCity == NULL)
		return 0;

	int iModifier = 0;
	//bonus for attacking same unit over and over in a turn?
	if (GET_PLAYER(pkCity->getOwner()).GetPlayerTraits()->GetMultipleAttackBonus() != 0)
	{
		int iTempModifier = GET_PLAYER(pkCity->getOwner()).GetPlayerTraits()->GetMultipleAttackBonus() * pkUnit->GetNumTimesAttackedThisTurn(pkCity->getOwner());
		iModifier += iTempModifier;
	}

	lua_pushinteger(L, iModifier);
	return 1;
}
//------------------------------------------------------------------------------
int CvLuaCity::lGetRangeStrikeModifierFromEspionage(lua_State* L)
{
	CvCity* pCity = GetInstance(L);

	// Only major civs can have this modifier
	if (pCity->getOwner() >= static_cast<PlayerTypes>(MAX_MAJOR_CIVS))
		return 0;

	int iModifier = 0;
	CvCityEspionage* pCityEspionage = pCity->GetCityEspionage();
	if (pCityEspionage)
	{
		CityEventChoiceTypes eSpyFocus = pCityEspionage->GetCounterSpyFocus();
		if (eSpyFocus != NO_EVENT_CHOICE_CITY)
		{
			CvModEventCityChoiceInfo* pEventChoiceInfo = GC.getCityEventChoiceInfo(eSpyFocus);
			if (pEventChoiceInfo)
			{
				iModifier += pEventChoiceInfo->getCityDefenseModifierBase();
				if (pEventChoiceInfo->getCityDefenseModifier() != 0)
					iModifier += pEventChoiceInfo->getCityDefenseModifier() * (pCityEspionage->GetCounterSpyRank() + 1);
			}
		}
	}

	lua_pushinteger(L, iModifier);
	return 1;
}
//------------------------------------------------------------------------------
//bool isWorkingPlot(CyPlot* pPlot);
int CvLuaCity::lIsWorkingPlot(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	CvPlot* pkPlot = CvLuaPlot::GetInstance(L, 2);
	const bool bResult = pkCity->GetCityCitizens()->IsWorkingPlot(pkPlot);

	lua_pushboolean(L, bResult);
	return 1;
}
//------------------------------------------------------------------------------
//void alterWorkingPlot(int iIndex);
int CvLuaCity::lAlterWorkingPlot(lua_State* L)
{
//	return BasicLuaMethod(L, &CvCity::GetCityCitizens()->DoAlterWorkingPlot);
	CvCity* pkCity = GetInstance(L);
	const int iIndex = lua_tointeger(L, 2);
	pkCity->GetCityCitizens()->DoAlterWorkingPlot(iIndex);

	return 1;
}
//------------------------------------------------------------------------------
//bool IsForcedWorkingPlot(CyPlot* pPlot);
int CvLuaCity::lIsForcedWorkingPlot(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	CvPlot* pkPlot = CvLuaPlot::GetInstance(L, 2);
	const bool bResult = pkCity->GetCityCitizens()->IsForcedWorkingPlot(pkPlot);

	lua_pushboolean(L, bResult);
	return 1;
}
//------------------------------------------------------------------------------
//bool GetNumForcedWorkingPlots(CyPlot* pPlot);
int CvLuaCity::lGetNumForcedWorkingPlots(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = pkCity->GetCityCitizens()->GetNumForcedWorkingPlots();

	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int getNumRealBuilding(BuildingTypes iIndex);
int CvLuaCity::lGetNumRealBuilding(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const BuildingTypes iIndex = toValue<BuildingTypes>(L, 2);
	if(iIndex != NO_BUILDING)
	{
		const int iResult = pkCity->GetCityBuildings()->GetNumRealBuilding(iIndex);
		lua_pushinteger(L, iResult);
	}
	else
	{
		lua_pushinteger(L, 0);
	}
	return 1;
}
//------------------------------------------------------------------------------
//void setNumRealBuilding(BuildingTypes iIndex, int iNewValue);
int CvLuaCity::lSetNumRealBuilding(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const BuildingTypes iIndex = toValue<BuildingTypes>(L, 2);
	if(iIndex != NO_BUILDING)
	{
		const int iNewValue = lua_tointeger(L, 3);
		const bool bNoBonus = luaL_optbool(L, 4, true);
		pkCity->GetCityBuildings()->SetNumRealBuilding(iIndex, iNewValue, bNoBonus);
	}

	return 1;
}
//------------------------------------------------------------------------------
//int getNumFreeBuilding(BuildingTypes iIndex);
int CvLuaCity::lGetNumFreeBuilding(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const BuildingTypes iIndex = toValue<BuildingTypes>(L, 2);
	if(iIndex != NO_BUILDING)
	{
		const int iResult = pkCity->GetCityBuildings()->GetNumFreeBuilding(iIndex);
		lua_pushinteger(L, iResult);
	}
	else
	{
		lua_pushinteger(L, 0);
	}
	return 1;
}
//------------------------------------------------------------------------------
//bool SetNumFreeBuilding(BuildingTypes iIndex, int iNewValue, bool bRefund = false, bool bValidate = false);
int CvLuaCity::lSetNumFreeBuilding(lua_State* L)
{
	CvCity* pCity = GetInstance(L);
	const BuildingTypes eBuilding = toValue<BuildingTypes>(L, 2);
	bool bResult = false;

	if (eBuilding != NO_BUILDING)
	{
		const int iNewValue = lua_tointeger(L, 3);
		const bool bRefund = luaL_optbool(L, 4, false);
		const bool bValidate = luaL_optbool(L, 5, false);
		bResult = pCity->SetNumFreeBuilding(eBuilding, iNewValue, bRefund, bValidate);
	}

	lua_pushboolean(L, bResult);
	return 1;
}
//------------------------------------------------------------------------------
//bool IsBuildingSellable(BuildingTypes iIndex);
int CvLuaCity::lIsBuildingSellable(lua_State* L)
{
	bool bResult = false;
	CvCity* pkCity = GetInstance(L);
	const BuildingTypes iIndex = toValue<BuildingTypes>(L, 2);
	if(iIndex != NO_BUILDING)
	{
		CvBuildingEntry* pkBuildingInfo = GC.getBuildingInfo(iIndex);
		if(pkBuildingInfo)
		{
			bResult = pkCity->GetCityBuildings()->IsBuildingSellable(*pkBuildingInfo);
		}
	}

	lua_pushboolean(L, bResult);
	return 1;
}

//------------------------------------------------------------------------------
//int GetSellBuildingRefund(BuildingTypes iIndex);
int CvLuaCity::lGetSellBuildingRefund(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const BuildingTypes iIndex = toValue<BuildingTypes>(L, 2);
	if(iIndex != NO_BUILDING)
	{
		const int iResult = pkCity->GetCityBuildings()->GetSellBuildingRefund(iIndex);
		lua_pushinteger(L, iResult);
	}
	else
	{
		lua_pushinteger(L, 0);
	}
	return 1;
}

//------------------------------------------------------------------------------
//int GetTotalBaseBuildingMaintenance(BuildingTypes iIndex);
int CvLuaCity::lGetTotalBaseBuildingMaintenance(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = pkCity->GetCityBuildings()->GetTotalBaseBuildingMaintenance();
	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int GetBuildingGreatWork(BuildingClassTypes eBuildingClass, int iSlot) const;
int CvLuaCity::lGetBuildingGreatWork(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const BuildingClassTypes iIndex = toValue<BuildingClassTypes>(L, 2);
	const int iSlot = lua_tointeger(L, 3);
	const int iResult = pkCity->GetCityBuildings()->GetBuildingGreatWork(iIndex, iSlot);
	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//int SetBuildingGreatWork(BuildingClassTypes eBuildingClass, int iSlot, int iGreatWorkIndex);
int CvLuaCity::lSetBuildingGreatWork(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const BuildingClassTypes iIndex = toValue<BuildingClassTypes>(L, 2);
	const int iSlot = lua_tointeger(L, 3);
	const int iGreatWorkIndex = lua_tointeger(L, 4);
	if(iIndex != NO_BUILDINGCLASS)
	{
		pkCity->GetCityBuildings()->SetBuildingGreatWork(iIndex, iSlot, iGreatWorkIndex);
	}
	return 1;
}
//------------------------------------------------------------------------------
//int IsHoldingGreatWork(BuildingClassTypes eBuildingClass) const;
int CvLuaCity::lIsHoldingGreatWork(lua_State* L)
{
	bool bResult = false;
	CvCity* pkCity = GetInstance(L);
	const BuildingClassTypes iIndex = toValue<BuildingClassTypes>(L, 2);
	if(iIndex != NO_BUILDINGCLASS)
	{
		CvBuildingClassInfo* pkBuildingClassInfo = GC.getBuildingClassInfo(iIndex);
		if(pkBuildingClassInfo)
		{
			bResult = pkCity->GetCityBuildings()->IsHoldingGreatWork(iIndex);
		}
	}
	lua_pushboolean(L, bResult);
	return 1;
}
//------------------------------------------------------------------------------
//int GetNumGreatWorksInBuilding(BuildingClassTypes eBuildingClass) const;
int CvLuaCity::lGetNumGreatWorksInBuilding(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const BuildingClassTypes iIndex = toValue<BuildingClassTypes>(L, 2);
	const int iResult = pkCity->GetCityBuildings()->GetNumGreatWorksInBuilding(iIndex);
	lua_pushinteger(L, iResult);
	return 1;
}
//------------------------------------------------------------------------------
//void clearOrderQueue();
int CvLuaCity::lClearOrderQueue(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::clearOrderQueue);
}
//------------------------------------------------------------------------------
//void pushOrder(OrderTypes eOrder, int iData1, int iData2, bool bSave, bool bPop, bool bAppend, bool bForce);
int CvLuaCity::lPushOrder(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const OrderTypes eOrder	= (OrderTypes)lua_tointeger(L, 2);
	const int iData1		= lua_tointeger(L, 3);
	const int iData2		= lua_tointeger(L, 4);
	const bool bSave		= lua_tointeger(L, 5);
	const bool bPop			= lua_toboolean(L, 6);
	const bool bAppend		= lua_toboolean(L, 7);
	const bool bForce		= luaL_optint(L, 8, 0);
	pkCity->pushOrder(eOrder, iData1, iData2, bSave, bPop, bAppend, bForce);

	return 1;
}
//------------------------------------------------------------------------------
//void popOrder(int iNum, bool bFinish, bool bChoose);
int CvLuaCity::lPopOrder(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iNum = lua_tointeger(L, 2);
	const bool bFinish = luaL_optint(L, 3, 0);
	const bool bChoose = luaL_optint(L, 4, 0);
	pkCity->popOrder(iNum, bFinish, bChoose);

	return 1;
}
//------------------------------------------------------------------------------
//int getOrderQueueLength();
int CvLuaCity::lGetOrderQueueLength(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::getOrderQueueLength);
}
//------------------------------------------------------------------------------
//OrderData* getOrderFromQueue(int iIndex);
//------------------------------------------------------------------------------
int CvLuaCity::lGetOrderFromQueue(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	if(pkCity)
	{
		const int iNum = lua_tointeger(L, 2);
		OrderData* pkOrder = pkCity->getOrderFromQueue(iNum);
		if(pkOrder)
		{
			lua_pushinteger(L, pkOrder->eOrderType);
			lua_pushinteger(L, pkOrder->iData1);
			lua_pushinteger(L, pkOrder->iData2);
			lua_pushboolean(L, pkOrder->bSave);
			lua_pushboolean(L, pkOrder->bRush);
			return 5;
		}
	}
	lua_pushinteger(L, -1);
	lua_pushinteger(L, 0);
	lua_pushinteger(L, 0);
	lua_pushboolean(L, false);
	lua_pushboolean(L, false);
	return 5;
}

//int getBuildingYieldChange(BuildingClassTypes eBuildingClass, YieldTypes eYield);
int CvLuaCity::lGetBuildingYieldChange(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int eBuildingClass = lua_tointeger(L, 2);
	const int eYield = lua_tointeger(L, 3);
	const int iResult = pkCity->GetCityBuildings()->GetBuildingYieldChange((BuildingClassTypes)eBuildingClass, (YieldTypes)eYield);

	lua_pushinteger(L, iResult);
	return 1;
}

#if defined(MOD_BALANCE_CORE_POLICIES)
int CvLuaCity::lGetBuildingClassCultureChange(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const BuildingClassTypes eBuildingClassType = (BuildingClassTypes)lua_tointeger(L, 2);
	const int iResult = pkCity->getBuildingClassCultureChange((BuildingClassTypes)eBuildingClassType);

	lua_pushinteger(L, iResult);
	return 1;
}
//int getReligionYieldRateModifier(YieldTypes eYield);
int CvLuaCity::lGetReligionYieldRateModifier(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const YieldTypes eYield = (YieldTypes) lua_tointeger(L, 2);
	const PlayerTypes ePlayer = pkCity->getOwner();
	ReligionTypes eMajority = pkCity->GetCityReligions()->GetReligiousMajority();
	ReligionTypes ePlayerReligion = GET_PLAYER(ePlayer).GetReligions()->GetStateReligion();
	if (ePlayerReligion != NO_RELIGION && eMajority == ePlayerReligion)
	{
		const int iRtnValue = GET_PLAYER(ePlayer).getReligionYieldRateModifier(eYield);
		lua_pushinteger(L, iRtnValue);
	}
	else
	{
		lua_pushinteger(L, 0);
	}

	return 1;
}
//int getReligionBuildingYieldRateModifier(BuildingClassTypes eBuildingClass, YieldTypes eYield);
int CvLuaCity::lGetReligionBuildingYieldRateModifier(lua_State* L)
{
	int iRtnValue = 0;
	CvCity* pkCity = GetInstance(L);
	const int eBuildingClass = lua_tointeger(L, 2);
	const int eYield = lua_tointeger(L, 3);
	const PlayerTypes ePlayer = pkCity->getOwner();
	ReligionTypes eReligion = pkCity->GetCityReligions()->GetReligiousMajority();
	ReligionTypes ePlayerReligion = GET_PLAYER(ePlayer).GetReligions()->GetStateReligion();
	if (ePlayerReligion != NO_RELIGION && eReligion == ePlayerReligion)
	{
		iRtnValue = pkCity->getReligionBuildingYieldRateModifier((BuildingClassTypes)eBuildingClass, (YieldTypes)eYield);
	}

	lua_pushinteger(L, iRtnValue);

	return 1;
}
#endif

int CvLuaCity::lGetBaseYieldRateFromNonSpecialists(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const YieldTypes eIndex = (YieldTypes)lua_tointeger(L, 2);

	int iNonSpecialist = GET_PLAYER(pkCity->getOwner()).getYieldFromNonSpecialistCitizensTimes100(eIndex);
	int iBonusTimes100 = (iNonSpecialist * (pkCity->getPopulation() - pkCity->GetCityCitizens()->GetTotalSpecialistCount()));
	lua_pushinteger(L, iBonusTimes100 / 100);
	return 1;
}

int CvLuaCity::lGetBuildingYieldChangeFromCorporationFranchises(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const BuildingClassTypes eBuildingClass = (BuildingClassTypes)lua_tointeger(L, 2);
	const YieldTypes eIndex = (YieldTypes)lua_tointeger(L, 3);
	int iResult = pkCity->GetBuildingYieldChangeFromCorporationFranchises(eBuildingClass, eIndex);
	lua_pushinteger(L, iResult);
	return 1;
}
int CvLuaCity::lGetYieldChangeFromCorporationFranchises(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const YieldTypes eIndex = (YieldTypes)lua_tointeger(L, 2);
	int iResult = pkCity->GetYieldChangeFromCorporationFranchises(eIndex);
	lua_pushinteger(L, iResult);
	return 1;
}
int CvLuaCity::lGetTradeRouteCityMod(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const YieldTypes eIndex = (YieldTypes)lua_tointeger(L, 2);
	const int iResult = (pkCity->GetTradeRouteCityMod(eIndex));

	lua_pushinteger(L, iResult);
	return 1;
}

int CvLuaCity::lGetGreatWorkYieldMod(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const YieldTypes eIndex = (YieldTypes)lua_tointeger(L, 2);
	const int iResult = min(20, (GET_PLAYER(pkCity->getOwner()).getYieldModifierFromGreatWorks(eIndex) * pkCity->GetCityBuildings()->GetNumGreatWorks()));

	lua_pushinteger(L, iResult);
	return 1;
}
int CvLuaCity::lGetActiveSpyYieldMod(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const YieldTypes eIndex = (YieldTypes)lua_tointeger(L, 2);
	const int iResult = min(30, (GET_PLAYER(pkCity->getOwner()).getYieldModifierFromActiveSpies(eIndex) * GET_PLAYER(pkCity->getOwner()).GetSpyPoints(true) / 100));

	lua_pushinteger(L, iResult);
	return 1;
}
int CvLuaCity::lGetResourceQuantityPerXFranchises(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResource = lua_tointeger(L, 2);
	int iFranchises = GET_PLAYER(pkCity->getOwner()).GetCorporations()->GetNumFranchises();
	int iCorpResource = pkCity->GetResourceQuantityPerXFranchises((ResourceTypes)iResource);
	int iResult = 0;
	if(iCorpResource > 0)
	{
		iResult = (iFranchises / iCorpResource);
	}

	lua_pushinteger(L, iResult);
	return 1;
}
int CvLuaCity::lGetGPRateModifierPerXFranchises(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iResult = (pkCity->GetGPRateModifierPerXFranchises());

	lua_pushinteger(L, iResult);
	return 1;
}
int CvLuaCity::lIsFranchised(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const PlayerTypes ePlayer = (PlayerTypes)lua_tointeger(L, 2);
	bool bResult = pkCity->IsHasFranchise(GET_PLAYER(ePlayer).GetCorporations()->GetFoundedCorporation());
	lua_pushboolean(L, bResult);
	return 1;
}
int CvLuaCity::lDoFranchiseAtCity(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	CvCity* pkDestCity = GetInstance(L, 2);
	GET_PLAYER(pkCity->getOwner()).GetCorporations()->BuildFranchiseInCity(pkCity, pkDestCity);
	return 0;
}
int CvLuaCity::lHasOffice(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	bool bResult = pkCity->IsHasOffice();
	lua_pushboolean(L, bResult);
	return 1;
}
int CvLuaCity::lGetYieldChangeTradeRoute(lua_State* L)
{
	CvCity* pCity = GetInstance(L);
	int iResult = 0;
	const YieldTypes eYield = static_cast<YieldTypes>(lua_tointeger(L, 2));
	if (pCity->IsRouteToCapitalConnected())
	{
		CvPlayer& kPlayer = GET_PLAYER(pCity->getOwner());
		int iEra = max(1, static_cast<int>(kPlayer.GetCurrentEra()));
		iResult = kPlayer.GetYieldChangeTradeRoute(eYield) + kPlayer.GetPlayerTraits()->GetYieldChangeTradeRoute(eYield) * iEra;
	}

	lua_pushinteger(L, iResult);
	return 1;
}

//int BeliefClasses::GetSpecialistYieldChange(SpecialistTypes eSpecialist, YieldTypes eYield);
int CvLuaCity::lGetSpecialistYieldChange(lua_State* L)
{
	int iRtnValue = 0;
	CvCity* pkCity = GetInstance(L);
	const SpecialistTypes eSpecialist = (SpecialistTypes) lua_tointeger(L, 2);
	const YieldTypes eYield = (YieldTypes) lua_tointeger(L, 3);
	ReligionTypes eReligion = pkCity->GetCityReligions()->GetReligiousMajority();
	const CvReligion* pReligion = GC.getGame().GetGameReligions()->GetReligion(eReligion, pkCity->getOwner());
	if(eReligion != NO_RELIGION)
	{
		iRtnValue = pReligion->m_Beliefs.GetSpecialistYieldChange(eSpecialist, eYield, pkCity->getOwner(), pkCity);
	}
	BeliefTypes eSecondaryPantheon = pkCity->GetCityReligions()->GetSecondaryReligionPantheonBelief();
	if (eSecondaryPantheon != NO_BELIEF)
	{
		iRtnValue += GC.GetGameBeliefs()->GetEntry(eSecondaryPantheon)->GetSpecialistYieldChange(eSpecialist, eYield);
	}

	// Mod for civs keeping their pantheon belief forever
	if (MOD_RELIGION_PERMANENT_PANTHEON)
	{
		if (GC.getGame().GetGameReligions()->HasCreatedPantheon(pkCity->getOwner()))
		{
			const CvReligion* pPantheon = GC.getGame().GetGameReligions()->GetReligion(RELIGION_PANTHEON, pkCity->getOwner());
			BeliefTypes ePantheonBelief = GC.getGame().GetGameReligions()->GetBeliefInPantheon(pkCity->getOwner());
			if (pPantheon != NULL && ePantheonBelief != NO_BELIEF && ePantheonBelief != eSecondaryPantheon)
			{
				if (pReligion == NULL || (pReligion != NULL && !pReligion->m_Beliefs.IsPantheonBeliefInReligion(ePantheonBelief, eReligion, pkCity->getOwner()))) // check that the our religion does not have our belief, to prevent double counting
				{
					iRtnValue += GC.GetGameBeliefs()->GetEntry(ePantheonBelief)->GetSpecialistYieldChange(eSpecialist, eYield);
				}
			}
		}
	}

	iRtnValue += pkCity->GetEventSpecialistYield(eSpecialist, eYield);

	lua_pushinteger(L, iRtnValue);

	return 1;
}

//int GetModFromWLTKD(YieldTypes eYield);
int CvLuaCity::lGetModFromWLTKD(lua_State* L)
{
	int iRtnValue = 0;
	CvCity* pkCity = GetInstance(L);
	const int eYield = lua_tointeger(L, 2);
	if(pkCity->GetWeLoveTheKingDayCounter() > 0)
	{
		const PlayerTypes ePlayer = pkCity->getOwner();
		CvGameReligions* pReligions = GC.getGame().GetGameReligions();
		ReligionTypes eReligion = pkCity->GetCityReligions()->GetReligiousMajority();
		if(eReligion != NO_RELIGION)
		{
			const CvReligion* pReligion = pReligions->GetReligion(eReligion, ePlayer);
			iRtnValue = pReligion->m_Beliefs.GetYieldFromWLTKD((YieldTypes)eYield, ePlayer, pkCity);
		}
		iRtnValue += GET_PLAYER(ePlayer).GetYieldFromWLTKD((YieldTypes)eYield);
		iRtnValue += pkCity->GetYieldFromWLTKD((YieldTypes)eYield);
	}
	lua_pushinteger(L, iRtnValue);

	return 1;
}
int CvLuaCity::lGetCultureModFromCarnaval(lua_State* L)
{
	int iRtnValue = 0;
	CvCity* pkCity = GetInstance(L);
	if(pkCity->GetWeLoveTheKingDayCounter() > 0)
	{
		const PlayerTypes ePlayer = pkCity->getOwner();
		iRtnValue += GET_PLAYER(ePlayer).GetPlayerTraits()->GetWLTKDCulture();
	}
	lua_pushinteger(L, iRtnValue);

	return 1;
}
//int GetModFromGoldenAge(YieldTypes eYield);
int CvLuaCity::lGetModFromGoldenAge(lua_State* L)
{
	int iRtnValue = 0;
	CvCity* pkCity = GetInstance(L);
	const int eYield = lua_tointeger(L, 2);
	const PlayerTypes ePlayer = pkCity->getOwner();
	if(GET_PLAYER(ePlayer).getGoldenAgeTurns() > 0)
	{
		CvGameReligions* pReligions = GC.getGame().GetGameReligions();
		ReligionTypes eFoundedReligion = GET_PLAYER(ePlayer).GetReligions()->GetOwnedReligion();
		if(eFoundedReligion != NO_RELIGION)
		{
			const CvReligion* pReligion = pReligions->GetReligion(eFoundedReligion, ePlayer);
			if(pkCity->GetCityReligions()->IsHolyCityForReligion(pReligion->m_eReligion))
			{
				iRtnValue = pReligion->m_Beliefs.GetYieldBonusGoldenAge((YieldTypes)eYield, ePlayer, pkCity);
			}
		}
		iRtnValue += pkCity->GetGoldenAgeYieldMod((YieldTypes)eYield);
	}
	lua_pushinteger(L, iRtnValue);

	return 1;
}

//int GetBuildingEspionageModifier(BuildingClassTypes eBuildingClass)
int CvLuaCity::lGetBuildingEspionageModifier(lua_State* L)
{
	//CvCity* pkCity = GetInstance(L);
	const BuildingTypes eBuilding = (BuildingTypes) lua_tointeger(L, 2);
	CvBuildingEntry* pBuildingInfo = GC.getBuildingInfo(eBuilding);
	ASSERT_DEBUG(pBuildingInfo, "pBuildingInfo is null!");
	if (pBuildingInfo)
	{
		lua_pushinteger(L, pBuildingInfo->GetEspionageModifier());
	}
	else
	{
		lua_pushinteger(L, 0);
	}
	return 1;
}

// int GetBuildingGlobalEspionageModifier(BuildingClassTypes eBuildingClass)
int CvLuaCity::lGetBuildingGlobalEspionageModifier(lua_State* L)
{
	const BuildingTypes eBuilding = (BuildingTypes)lua_tointeger(L, 2);
	CvBuildingEntry* pBuildingInfo = GC.getBuildingInfo(eBuilding);
	ASSERT_DEBUG(pBuildingInfo, "pBuildingInfo is null!");
	if (pBuildingInfo)
	{
		lua_pushinteger(L, pBuildingInfo->GetGlobalEspionageModifier());
	}
	else
	{
		lua_pushinteger(L, 0);
	}
	return 1;
}

//------------------------------------------------------------------------------
int CvLuaCity::lHasDiplomat(lua_State* L)
{
	bool bResult = false;
	CvCity* pkCity = GetInstance(L);
	const PlayerTypes iPlayer = toValue<PlayerTypes>(L, 2);
	if(iPlayer != NO_PLAYER && pkCity->isCapital())
	{
		int iSpyIndex = pkCity->GetCityEspionage()->m_aiSpyAssignment[iPlayer];
		bResult = (iSpyIndex != -1 && GET_PLAYER(iPlayer).GetEspionage()->IsDiplomat(iSpyIndex));
	}
	lua_pushboolean(L, bResult);
	return 1;
}
//------------------------------------------------------------------------------
int CvLuaCity::lHasSpy(lua_State* L)
{
	bool bResult = false;
	CvCity* pkCity = GetInstance(L);
	const PlayerTypes iPlayer = toValue<PlayerTypes>(L, 2);
	if(iPlayer != NO_PLAYER)
	{
		int iSpyIndex = pkCity->GetCityEspionage()->m_aiSpyAssignment[iPlayer];
		bResult = (iSpyIndex != -1 && !GET_PLAYER(iPlayer).GetEspionage()->IsDiplomat(iSpyIndex));
	}
	lua_pushboolean(L, bResult);
	return 1;
}
//------------------------------------------------------------------------------
int CvLuaCity::lHasCounterSpy(lua_State* L)
{
	bool bResult = false;
	CvCity* pkCity = GetInstance(L);
	bResult = (pkCity->GetCityEspionage()->m_aiSpyAssignment[pkCity->getOwner()] != -1);
	lua_pushboolean(L, bResult);
	return 1;
}
//------------------------------------------------------------------------------
int CvLuaCity::lGetCounterSpy(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->GetCityEspionage()->m_aiSpyAssignment[pkCity->getOwner()]);
	return 1;
}

#if defined(MOD_RELIGION_CONVERSION_MODIFIERS)
//int GetBuildingConversionModifier(BuildingTypes eBuilding)
int CvLuaCity::lGetBuildingConversionModifier(lua_State* L)
{
	//CvCity* pkCity = GetInstance(L);
	const BuildingTypes eBuilding = (BuildingTypes) lua_tointeger(L, 2);
	CvBuildingEntry* pBuildingInfo = GC.getBuildingInfo(eBuilding);
	ASSERT_DEBUG(pBuildingInfo, "pBuildingInfo is null!");
	if (pBuildingInfo)
	{
		lua_pushinteger(L, pBuildingInfo->GetConversionModifier());
	}
	else
	{
		lua_pushinteger(L, 0);
	}
	return 1;
}

// int GetBuildingGlobalConversionModifier(BuildingTypes eBuilding)
int CvLuaCity::lGetBuildingGlobalConversionModifier(lua_State* L)
{
	const BuildingTypes eBuilding = (BuildingTypes)lua_tointeger(L, 2);
	CvBuildingEntry* pBuildingInfo = GC.getBuildingInfo(eBuilding);
	ASSERT_DEBUG(pBuildingInfo, "pBuildingInfo is null!");
	if (pBuildingInfo)
	{
		lua_pushinteger(L, pBuildingInfo->GetGlobalConversionModifier());
	}
	else
	{
		lua_pushinteger(L, 0);
	}
	return 1;
}
#endif

//------------------------------------------------------------------------------
//void setBuildingYieldChange(BuildingClassTypes eBuildingClass, YieldTypes eYield, int iChange);
int CvLuaCity::lSetBuildingYieldChange(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int eBuildingClass = lua_tointeger(L, 2);
	const int eYield = lua_tointeger(L, 3);
	const int iChange = lua_tointeger(L, 4);

	pkCity->GetCityBuildings()->SetBuildingYieldChange((BuildingClassTypes)eBuildingClass, (YieldTypes)eYield, iChange);

	return 1;
}

//------------------------------------------------------------------------------
int CvLuaCity::lGetNumCityPlots(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->GetNumWorkablePlots());
	return 1;
}

//------------------------------------------------------------------------------
int CvLuaCity::lCanPlaceUnitHere(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	int iUnitType = lua_tointeger(L, 2);
	lua_pushboolean(L, pkCity->CanPlaceUnitHere((UnitTypes)iUnitType));

	return 1;
}
//------------------------------------------------------------------------------
int CvLuaCity::lGetSpecialistYield(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const SpecialistTypes eSpecialist = (SpecialistTypes) lua_tointeger(L, 2);
	const YieldTypes eYield = (YieldTypes) lua_tointeger(L, 3);

	const PlayerTypes ePlayer = pkCity->getOwner();

	const int iValue = (GET_PLAYER(ePlayer).specialistYield(eSpecialist, eYield) + pkCity->getSpecialistExtraYield(eSpecialist, eYield));

	lua_pushinteger(L, iValue);

	return 1;
}
//------------------------------------------------------------------------------
int CvLuaCity::lGetCultureFromSpecialist(lua_State* L)
{
	return BasicLuaMethod(L, &CvCity::GetCultureFromSpecialist);
}
//------------------------------------------------------------------------------
int CvLuaCity::lGetReligionCityRangeStrikeModifier(lua_State* L)
{
	int iReligionRangeStrikeMod = 0;

	CvCity* pkCity = GetInstance(L);
	ReligionTypes eMajority = pkCity->GetCityReligions()->GetReligiousMajority();
	BeliefTypes eSecondaryPantheon = NO_BELIEF;
	if(eMajority != NO_RELIGION)
	{
		const CvReligion* pReligion = GC.getGame().GetGameReligions()->GetReligion(eMajority, pkCity->getOwner());
		if(pReligion)
		{
			iReligionRangeStrikeMod = pReligion->m_Beliefs.GetCityRangeStrikeModifier(pkCity->getOwner(), pkCity);
			eSecondaryPantheon = pkCity->GetCityReligions()->GetSecondaryReligionPantheonBelief();
			if (eSecondaryPantheon != NO_BELIEF)
			{
				iReligionRangeStrikeMod += GC.GetGameBeliefs()->GetEntry(eSecondaryPantheon)->GetCityRangeStrikeModifier();
			}
		}
	}

#if defined(MOD_RELIGION_PERMANENT_PANTHEON)
	// Mod for civs keeping their pantheon belief forever
	if (MOD_RELIGION_PERMANENT_PANTHEON)
	{
		if (GC.getGame().GetGameReligions()->HasCreatedPantheon(pkCity->getOwner()))
		{
			const CvReligion* pPantheon = GC.getGame().GetGameReligions()->GetReligion(RELIGION_PANTHEON, pkCity->getOwner());
			BeliefTypes ePantheonBelief = GC.getGame().GetGameReligions()->GetBeliefInPantheon(pkCity->getOwner());
			if (pPantheon != NULL && ePantheonBelief != NO_BELIEF && ePantheonBelief != eSecondaryPantheon)
			{
				const CvReligion* pReligion = GC.getGame().GetGameReligions()->GetReligion(eMajority, pkCity->getOwner());
				if (pReligion == NULL || (pReligion != NULL && !pReligion->m_Beliefs.IsPantheonBeliefInReligion(ePantheonBelief, eMajority, pkCity->getOwner()))) // check that the our religion does not have our belief, to prevent double counting
				{
					iReligionRangeStrikeMod += GC.GetGameBeliefs()->GetEntry(ePantheonBelief)->GetCityRangeStrikeModifier();
				}
			}
		}
	}
#endif

	lua_pushinteger(L, iReligionRangeStrikeMod);

	return 1;
}

//------------------------------------------------------------------------------
int CvLuaCity::lAddMessage(lua_State* L)
{
	CvCity* pCity = GetInstance(L);
	const char* szMessage = lua_tostring(L, 2);
	const PlayerTypes ePlayer = (PlayerTypes) luaL_optinteger(L, 3, pCity->getOwner());

	SHOW_CITY_MESSAGE(pCity, ePlayer, szMessage);
	return 0;
}
//------------------------------------------------------------------------------
int CvLuaCity::lCountNumWorkedFeature(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const FeatureTypes eFeature = (FeatureTypes) lua_tointeger(L, 2);
	const int iValue = pkCity->CountNumWorkedFeature(eFeature);

	lua_pushinteger(L, iValue);

	return 1;
}
//------------------------------------------------------------------------------
int CvLuaCity::lCountNumWorkedImprovement(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const ImprovementTypes eImprovement = (ImprovementTypes) lua_tointeger(L, 2);
	const bool bIgnorePillaged = luaL_optbool(L, 3, true);
	const int iValue = pkCity->CountNumWorkedImprovement(eImprovement, bIgnorePillaged);

	lua_pushinteger(L, iValue);

	return 1;
}
//------------------------------------------------------------------------------
int CvLuaCity::lCountNumWorkedResource(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const ResourceTypes eResource = (ResourceTypes) lua_tointeger(L, 2);
	const int iValue = pkCity->CountNumWorkedResource(eResource);

	lua_pushinteger(L, iValue);

	return 1;
}
//------------------------------------------------------------------------------
int CvLuaCity::lCountNumImprovement(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const ImprovementTypes eImprovement = (ImprovementTypes) lua_tointeger(L, 2);
	const int iValue = pkCity->CountNumImprovement(eImprovement);

	lua_pushinteger(L, iValue);

	return 1;
}
//------------------------------------------------------------------------------
int CvLuaCity::lCountNumWorkedRiverTiles(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const TerrainTypes eTerrain = (TerrainTypes)luaL_optint(L, 2, NO_TERRAIN);
	const int iValue = pkCity->CountNumWorkedRiverTiles(eTerrain);

	lua_pushinteger(L, iValue);

	return 1;
}

LUAAPIIMPL(City, HasBelief)
LUAAPIIMPL(City, HasBuilding)
LUAAPIIMPL(City, HasBuildingClass)
LUAAPIIMPL(City, HasAnyWonder)
LUAAPIIMPL(City, HasWonder)
LUAAPIIMPL(City, IsCivilization)
LUAAPIIMPL(City, HasFeature)
LUAAPIIMPL(City, HasWorkedFeature)
LUAAPIIMPL(City, HasAnyNaturalWonder)
LUAAPIIMPL(City, HasNaturalWonder)
LUAAPIIMPL(City, HasImprovement)
LUAAPIIMPL(City, HasWorkedImprovement)
LUAAPIIMPL(City, HasPlotType)
LUAAPIIMPL(City, HasWorkedPlotType)
LUAAPIIMPL(City, HasAnyReligion)
LUAAPIIMPL(City, HasReligion)
LUAAPIIMPL(City, HasResource)
LUAAPIIMPL(City, HasWorkedResource)
LUAAPIIMPL(City, IsConnectedToCapital)
LUAAPIIMPL(City, IsIndustrialConnectedToCapital)
LUAAPIIMPL(City, IsConnectedTo)
LUAAPIIMPL(City, GetConnectionGoldTimes100)
LUAAPIIMPL(City, HasSpecialistSlot)
LUAAPIIMPL(City, HasSpecialist)
LUAAPIIMPL(City, HasTerrain)
LUAAPIIMPL(City, HasWorkedTerrain)
LUAAPIIMPL(City, HasAnyDomesticTradeRoute)
LUAAPIIMPL(City, HasAnyInternationalTradeRoute)
LUAAPIIMPL(City, HasTradeRouteToAnyCity)
LUAAPIIMPL(City, HasTradeRouteTo)
LUAAPIIMPL(City, HasTradeRouteFromAnyCity)
LUAAPIIMPL(City, HasTradeRouteFrom)
LUAAPIIMPL(City, IsOnFeature)
LUAAPIIMPL(City, IsAdjacentToFeature)
LUAAPIIMPL(City, IsWithinDistanceOfFeature)
LUAAPIIMPL(City, IsWithinDistanceOfUnit)
LUAAPIIMPL(City, IsWithinDistanceOfUnitClass)
LUAAPIIMPL(City, IsWithinDistanceOfUnitCombatType)
LUAAPIIMPL(City, IsWithinDistanceOfUnitPromotion)
LUAAPIIMPL(City, IsOnImprovement)
LUAAPIIMPL(City, IsAdjacentToImprovement)
LUAAPIIMPL(City, IsWithinDistanceOfImprovement)
LUAAPIIMPL(City, IsOnPlotType)
LUAAPIIMPL(City, IsAdjacentToPlotType)
LUAAPIIMPL(City, IsWithinDistanceOfPlotType)
LUAAPIIMPL(City, IsOnResource)
LUAAPIIMPL(City, IsAdjacentToResource)
LUAAPIIMPL(City, IsWithinDistanceOfResource)
LUAAPIIMPL(City, IsOnTerrain)
LUAAPIIMPL(City, IsAdjacentToTerrain)
LUAAPIIMPL(City, IsWithinDistanceOfTerrain)
LUAAPIIMPL(City, CountFeature)
LUAAPIIMPL(City, CountWorkedFeature)
LUAAPIIMPL(City, CountImprovement)
LUAAPIIMPL(City, CountWorkedImprovement)
LUAAPIIMPL(City, CountPlotType)
LUAAPIIMPL(City, CountWorkedPlotType)
LUAAPIIMPL(City, CountResource)
LUAAPIIMPL(City, CountWorkedResource)
LUAAPIIMPL(City, CountTerrain)
LUAAPIIMPL(City, CountWorkedTerrain)

int CvLuaCity::lGetAdditionalFood(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iAdditionalFood = pkCity->GetAdditionalFood();
	lua_pushinteger(L, iAdditionalFood);

	return 1;
}
int CvLuaCity::lSetAdditionalFood(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iFood = lua_tointeger(L, 2);
	pkCity->SetAdditionalFood(iFood);
	return 1;
}

//------------------------------------------------------------------------------
int CvLuaCity::lIsProductionRoutes(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushboolean(L, pkCity->IsProductionRoutes() || GET_PLAYER(pkCity->getOwner()).IsProductionRoutesAllCities());

	return 1;
}
//------------------------------------------------------------------------------
int CvLuaCity::lIsFoodRoutes(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushboolean(L, pkCity->IsFoodRoutes() || GET_PLAYER(pkCity->getOwner()).IsFoodRoutesAllCities());

	return 1;
}

#if defined(MOD_BALANCE_CORE_EVENTS)
int CvLuaCity::lGetDisabledTooltip(lua_State* L)
{
	CvString DisabledTT = "";
	CvCity* pkCity = GetInstance(L);
	const CityEventChoiceTypes eEventChoice = (CityEventChoiceTypes)lua_tointeger(L, 2);
	const int iSpyID = luaL_optint(L, 3, -1);
	const PlayerTypes eSpyOwner = (PlayerTypes)luaL_optint(L, 4, NO_PLAYER);
	if(eEventChoice != NO_EVENT_CHOICE_CITY)
	{
		DisabledTT = pkCity->GetDisabledTooltip(eEventChoice, iSpyID, eSpyOwner);
	}

	lua_pushstring(L, DisabledTT.c_str());
	return 1;
}
int CvLuaCity::lGetScaledEventChoiceValue(lua_State* L)
{
	CvString CoreYieldTip = "";
	CvCity* pkCity = GetInstance(L);
	const CityEventChoiceTypes eEventChoice = (CityEventChoiceTypes)lua_tointeger(L, 2);
	const bool bYieldsOnly = lua_toboolean(L, 3);
	const int iSpyID = luaL_optint(L, 4, -1);
	const PlayerTypes eSpyOwner = (PlayerTypes)luaL_optint(L, 5, NO_PLAYER);
	if(eEventChoice != NO_EVENT_CHOICE_CITY)
	{
		CoreYieldTip = pkCity->GetScaledHelpText(eEventChoice, bYieldsOnly, iSpyID, eSpyOwner);
	}

	lua_pushstring(L, CoreYieldTip.c_str());
	return 1;
}
int CvLuaCity::lIsCityEventChoiceActive(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const CityEventChoiceTypes eEventChoice = (CityEventChoiceTypes)lua_tointeger(L, 2);
	const bool bInstantEvents = luaL_optbool(L, 3, false);
	bool bResult = false;
	if(eEventChoice != NO_EVENT_CHOICE_CITY)
	{
		CvModEventCityChoiceInfo* pkEventChoiceInfo = GC.getCityEventChoiceInfo(eEventChoice);
		if(pkEventChoiceInfo != NULL)
		{
			if(pkCity->IsEventChoiceActive(eEventChoice))
			{
				if(bInstantEvents)
				{
					if(!pkEventChoiceInfo->isOneShot() && !pkEventChoiceInfo->Expires())
					{
						for(int iLoop = 0; iLoop < GC.getNumCityEventInfos(); iLoop++)
						{
							CityEventTypes eEvent = (CityEventTypes)iLoop;
							if(eEvent != NO_EVENT_CITY)
							{
								CvModCityEventInfo* pkEventInfo = GC.getCityEventInfo(eEvent);
								if(pkEventInfo != NULL)
								{
									if(pkEventChoiceInfo->isParentEvent(eEvent))
									{
										CvModCityEventInfo* pkEventInfo = GC.getCityEventInfo(eEvent);
										if(pkEventInfo != NULL)
										{
											if(pkEventInfo->getNumChoices() == 1)
											{
												bResult = true;
												break;
											}
										}
									}
								}
							}
						}
					}
				}
				else
				{		
					if(pkEventChoiceInfo->isOneShot() || pkEventChoiceInfo->Expires())
					{
						bResult = true;
					}
				}
			}
		}
	}
	lua_pushboolean(L, bResult);
	return 1;
}

int CvLuaCity::lDoCityEventChoice(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const CityEventChoiceTypes eEventChoice = (CityEventChoiceTypes)lua_tointeger(L, 2);
	const int iSpyID = luaL_optint(L, 3, -1);
	const PlayerTypes eSpyOwner = (PlayerTypes)luaL_optint(L, 4, -1);
	pkCity->DoEventChoice(eEventChoice, NO_EVENT_CITY, true, iSpyID, eSpyOwner);
	return 1;
}
int CvLuaCity::lDoCityStartEvent(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const CityEventTypes eEvent = (CityEventTypes)lua_tointeger(L, 2);
	pkCity->DoStartEvent(eEvent, true);
	return 1;
}
int CvLuaCity::lDoCancelCityEventChoice(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const CityEventChoiceTypes eEvent = (CityEventChoiceTypes)lua_tointeger(L, 2);
	pkCity->DoCancelEventChoice(eEvent);
	return 1;
}
int CvLuaCity::lGetCityEventCooldown(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const CityEventTypes eEvent = (CityEventTypes)lua_tointeger(L, 2);
	const int iCooldown = pkCity->GetEventCooldown(eEvent);
	CvModCityEventInfo* pkEventInfo = GC.getCityEventInfo(eEvent);
	if(pkEventInfo != NULL && pkEventInfo->isOneShot())
	{
		lua_pushinteger(L, -1);
		return 1;
	}
	lua_pushinteger(L, iCooldown);
	return 1;
}
int CvLuaCity::lSetCityEventCooldown(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const CityEventTypes eEvent = (CityEventTypes)lua_tointeger(L, 2);
	const int iCooldown = lua_tointeger(L, 3);
	pkCity->SetEventCooldown(eEvent, iCooldown);
	return 1;
}
int CvLuaCity::lGetCityEventChoiceCooldown(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const CityEventChoiceTypes eEvent = (CityEventChoiceTypes)lua_tointeger(L, 2);
	CvModEventCityChoiceInfo* pkEventChoiceInfo = GC.getCityEventChoiceInfo(eEvent);
	if(pkEventChoiceInfo != NULL && pkEventChoiceInfo->isOneShot())
	{
		lua_pushinteger(L, -1);
		return 1;
	}
	const int iCooldown = pkCity->GetEventChoiceDuration(eEvent);
	lua_pushinteger(L, iCooldown);
	return 1;
}
int CvLuaCity::lSetCityEventChoiceCooldown(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const CityEventChoiceTypes eEvent = (CityEventChoiceTypes)lua_tointeger(L, 2);
	const int iCooldown = lua_tointeger(L, 3);
	pkCity->SetEventChoiceDuration(eEvent, iCooldown);
	return 1;
}
int CvLuaCity::lIsCityEventChoiceValid(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const CityEventChoiceTypes eEventChoice = (CityEventChoiceTypes)lua_tointeger(L, 2);
	const CityEventTypes eEvent = (CityEventTypes)lua_tointeger(L, 3);
	const bool bValue = pkCity->IsCityEventChoiceValid(eEventChoice, eEvent);

	lua_pushboolean(L, bValue);

	return 1;
}
int CvLuaCity::lIsCityEventChoiceValidEspionage(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const CityEventChoiceTypes eEventChoice = (CityEventChoiceTypes)lua_tointeger(L, 2);
	const CityEventTypes eEvent = (CityEventTypes)lua_tointeger(L, 3);
	const int uiSpyIndex = lua_tointeger(L, 4);
	const PlayerTypes eSpyOwner = (PlayerTypes)lua_tointeger(L, 5);
	const bool bValue = pkCity->IsCityEventChoiceValidEspionage(eEventChoice, eEvent, uiSpyIndex, eSpyOwner);

	lua_pushboolean(L, bValue);

	return 1;
}
#endif

int CvLuaCity::lGetSappedTurns(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->GetSappedTurns());
	return 1;
}
int CvLuaCity::lSetSappedTurns(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iValue = lua_tointeger(L, 2);
	pkCity->SetSappedTurns(iValue);
	return 1;
}
int CvLuaCity::lChangeSappedTurns(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iValue = lua_tointeger(L, 2);
	pkCity->ChangeSappedTurns(iValue);
	return 1;
}

#if defined(MOD_BALANCE_CORE_JFD)
int CvLuaCity::lIsColony(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);

	lua_pushboolean(L, pkCity->IsColony());

	return 1;
}
int CvLuaCity::lSetColony(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const bool bValue = lua_toboolean(L, 2);
	pkCity->SetColony(bValue);
	return 1;
}
int CvLuaCity::lGetProvinceLevel(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->GetProvinceLevel());
	return 1;
}
int CvLuaCity::lSetProvinceLevel(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iValue = lua_tointeger(L, 2);
	pkCity->SetProvinceLevel(iValue);
	return 1;
}

int CvLuaCity::lHasProvinceLevel(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iValue = lua_tointeger(L, 2);
	const bool bValue = (pkCity->GetProvinceLevel() == iValue);
	lua_pushboolean(L, bValue);

	return 1;
}

int CvLuaCity::lGetOrganizedCrime(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->GetOrganizedCrime());
	return 1;
}
int CvLuaCity::lSetOrganizedCrime(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iValue = lua_tointeger(L, 2);
	pkCity->SetOrganizedCrime(iValue);
	return 1;
}
int CvLuaCity::lHasOrganizedCrime(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);

	lua_pushboolean(L, pkCity->HasOrganizedCrime());

	return 1;
}

int CvLuaCity::lChangeResistanceCounter(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iValue = lua_tointeger(L, 2);
	pkCity->ChangeResistanceCounter(iValue);
	return 1;
}
int CvLuaCity::lSetResistanceCounter(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iValue = lua_tointeger(L, 2);
	pkCity->SetResistanceCounter(iValue);
	return 1;
}
int CvLuaCity::lGetResistanceCounter(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->GetResistanceCounter());
	return 1;
}
int CvLuaCity::lChangePlagueCounter(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iValue = lua_tointeger(L, 2);
	pkCity->ChangePlagueCounter(iValue);
	return 1;
}
int CvLuaCity::lSetPlagueCounter(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iValue = lua_tointeger(L, 2);
	pkCity->SetPlagueCounter(iValue);
	return 1;
}
int CvLuaCity::lGetPlagueCounter(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->GetPlagueCounter());
	return 1;
}
int CvLuaCity::lGetPlagueTurns(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->GetPlagueTurns());
	return 1;
}
int CvLuaCity::lChangePlagueTurns(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iValue = lua_tointeger(L, 2);
	pkCity->ChangePlagueTurns(iValue);
	return 1;
}
int CvLuaCity::lSetPlagueTurns(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iValue = lua_tointeger(L, 2);
	pkCity->SetPlagueTurns(iValue);
	return 1;
}

int CvLuaCity::lGetPlagueType(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->GetPlagueType());
	return 1;
}
int CvLuaCity::lSetPlagueType(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iValue = lua_tointeger(L, 2);
	pkCity->SetPlagueType(iValue);
	return 1;
}
int CvLuaCity::lHasPlague(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);

	lua_pushboolean(L, pkCity->HasPlague());

	return 1;
}

int CvLuaCity::lChangeLoyaltyCounter(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iValue = lua_tointeger(L, 2);
	pkCity->ChangeLoyaltyCounter(iValue);
	return 1;
}
int CvLuaCity::lSetLoyaltyCounter(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iValue = lua_tointeger(L, 2);
	pkCity->SetLoyaltyCounter(iValue);
	return 1;
}
int CvLuaCity::lGetLoyaltyCounter(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->GetLoyaltyCounter());
	return 1;
}
int CvLuaCity::lChangeDisloyaltyCounter(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iValue = lua_tointeger(L, 2);
	pkCity->ChangeDisloyaltyCounter(iValue);
	return 1;
}
int CvLuaCity::lSetDisloyaltyCounter(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iValue = lua_tointeger(L, 2);
	pkCity->SetDisloyaltyCounter(iValue);
	return 1;
}
int CvLuaCity::lGetDisloyaltyCounter(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	lua_pushinteger(L, pkCity->GetDisloyaltyCounter());
	return 1;
}
int CvLuaCity::lGetLoyaltyState(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int eLoyalty = pkCity->GetLoyaltyState();
	lua_pushinteger(L, (LoyaltyStateTypes)eLoyalty);
	return 1;
}
int CvLuaCity::lSetLoyaltyState(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const LoyaltyStateTypes eLoyalty = (LoyaltyStateTypes)lua_tointeger(L, 2);
	pkCity->SetLoyaltyState((int)eLoyalty);
	return 1;
}

int CvLuaCity::lHasLoyaltyState(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iValue = lua_tointeger(L, 2);
	const bool bValue = (pkCity->GetLoyaltyState() == iValue);
	lua_pushboolean(L, bValue);

	return 1;
}

int CvLuaCity::lGetYieldModifierFromHappiness(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const YieldTypes eYield = (YieldTypes)lua_tointeger(L, 2);
	const int iValue = pkCity->GetYieldModifierFromHappiness(eYield);
	lua_pushinteger(L, iValue);
	return 1;
}
int CvLuaCity::lSetYieldModifierFromHappiness(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iValue = lua_tointeger(L, 3);
	const YieldTypes eYield = (YieldTypes)lua_tointeger(L, 2);
	pkCity->SetYieldModifierFromHappiness(eYield, iValue);
	return 1;
}

int CvLuaCity::lGetYieldModifierFromHealth(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const YieldTypes eYield = (YieldTypes)lua_tointeger(L, 2);
	const int iValue = pkCity->GetYieldModifierFromHealth(eYield);
	lua_pushinteger(L, iValue);
	return 1;
}
int CvLuaCity::lSetYieldModifierFromHealth(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iValue = lua_tointeger(L, 3);
	const YieldTypes eYield = (YieldTypes)lua_tointeger(L, 2);
	pkCity->SetYieldModifierFromHealth(eYield, iValue);
	return 1;
}

int CvLuaCity::lGetYieldModifierFromCrime(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const YieldTypes eYield = (YieldTypes)lua_tointeger(L, 2);
	const int iValue = pkCity->GetYieldModifierFromCrime(eYield);
	lua_pushinteger(L, iValue);
	return 1;
}
int CvLuaCity::lSetYieldModifierFromCrime(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iValue = lua_tointeger(L, 3);
	const YieldTypes eYield = (YieldTypes)lua_tointeger(L, 2);
	pkCity->SetYieldModifierFromCrime(eYield, iValue);
	return 1;
}


int CvLuaCity::lGetYieldModifierFromDevelopment(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const YieldTypes eYield = (YieldTypes)lua_tointeger(L, 2);
	const int iValue = pkCity->GetYieldModifierFromDevelopment(eYield);
	lua_pushinteger(L, iValue);
	return 1;
}
int CvLuaCity::lSetYieldModifierFromDevelopment(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iValue = lua_tointeger(L, 3);
	const YieldTypes eYield = (YieldTypes)lua_tointeger(L, 2);
	pkCity->SetYieldModifierFromDevelopment(eYield, iValue);
	return 1;
}

int CvLuaCity::lGetYieldFromHappiness(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const YieldTypes eYield = (YieldTypes)lua_tointeger(L, 2);
	const int iValue = pkCity->GetYieldFromHappiness(eYield);
	lua_pushinteger(L, iValue);
	return 1;
}
int CvLuaCity::lSetYieldFromHappiness(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iValue = lua_tointeger(L, 3);
	const YieldTypes eYield = (YieldTypes)lua_tointeger(L, 2);
	pkCity->SetYieldFromHappiness(eYield, iValue);
	return 1;
}

int CvLuaCity::lGetYieldFromHealth(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const YieldTypes eYield = (YieldTypes)lua_tointeger(L, 2);
	const int iValue = pkCity->GetYieldFromHealth(eYield);
	lua_pushinteger(L, iValue);
	return 1;
}
int CvLuaCity::lSetYieldFromHealth(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iValue = lua_tointeger(L, 3);
	const YieldTypes eYield = (YieldTypes)lua_tointeger(L, 2);
	pkCity->SetYieldFromHealth(eYield, iValue);
	return 1;
}

int CvLuaCity::lGetYieldFromCrime(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const YieldTypes eYield = (YieldTypes)lua_tointeger(L, 2);
	const int iValue = pkCity->GetYieldFromCrime(eYield);
	lua_pushinteger(L, iValue);
	return 1;
}
int CvLuaCity::lSetYieldFromCrime(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iValue = lua_tointeger(L, 3);
	const YieldTypes eYield = (YieldTypes)lua_tointeger(L, 2);
	pkCity->SetYieldFromCrime(eYield, iValue);
	return 1;
}

int CvLuaCity::lGetYieldFromDevelopment(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const YieldTypes eYield = (YieldTypes)lua_tointeger(L, 2);
	const int iValue = pkCity->GetYieldFromDevelopment(eYield);
	lua_pushinteger(L, iValue);
	return 1;
}
int CvLuaCity::lSetYieldFromDevelopment(lua_State* L)
{
	CvCity* pkCity = GetInstance(L);
	const int iValue = lua_tointeger(L, 3);
	const YieldTypes eYield = (YieldTypes)lua_tointeger(L, 2);
	pkCity->SetYieldFromDevelopment(eYield, iValue);
	return 1;
}

int CvLuaCity::lGetCompetitiveSpawnUnitType(lua_State* L)
{
	CvCity* pCity = GetInstance(L);
	CvPlayer& kPlayer = GET_PLAYER(pCity->getOwner());
	const bool bIncludeRanged = luaL_optbool(L, 2, false);
	const bool bIncludeShips = luaL_optbool(L, 3, false);
	const bool bIncludeRecon = luaL_optbool(L, 4, false);
	const bool bIncludeUUs = luaL_optbool(L, 5, false);
	const bool bNoResource = luaL_optbool(L, 6, false);
	const bool bMinorCivGift = luaL_optbool(L, 7, false);
	const bool bRandom = luaL_optbool(L, 8, false);
	vector<int> viUnitCombat;
	if (!(lua_istable(L, 9) || lua_isnoneornil(L, 9)))
		return luaL_error(L, "Argument must be table or nil");

	const int iLength = lua_objlen(L, 9);
	for (int i = 1; i <= iLength; i++)
	{
		lua_rawgeti(L, 9, i);
		const int iUnitCombat = lua_tointeger(L, -1);
		if (!lua_isnumber(L, -1) || lua_tonumber(L, -1) != iUnitCombat)
			return luaL_error(L, "Table values must be integers");

		viUnitCombat.push_back(iUnitCombat);
		lua_pop(L, 1);
	}

	CvSeeder seed;
	if (bRandom)
	{
		seed = CvSeeder::fromRaw(0x77a1fc31).mix(kPlayer.getNumMilitaryUnits());
	}
	UnitTypes eUnit = kPlayer.GetCompetitiveSpawnUnitType(bIncludeRanged, bIncludeShips, bIncludeRecon, bIncludeUUs, pCity, bNoResource, bMinorCivGift, bRandom, &seed, viUnitCombat);
	lua_pushinteger(L, eUnit);
	return 1;
}

#endif
