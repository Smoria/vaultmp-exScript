#ifndef EXRECORDS_H
#define EXRECORDS_H

#include "vaultscript.h"
using namespace vaultmp;

static std::vector<WTHR> RandomWeatherList
{
	WTHR::DefaultWeather,
	WTHR::MegatonFalloutDecay,
	//WTHR::InvertedDaylightWeatherTenPenny, dark nebula
	WTHR::UrbanDeepInnerDCMONtop,
	WTHR::UrbanDeepInnerMamaDC,
	WTHR::UrbanDeepInnerCitadel,
	WTHR::UrbanDeepInnerDCMall,
	WTHR::UrbanDeepInner,
	WTHR::UrbanDeep,
	WTHR::WastelandDecay,
	WTHR::WastelandEastOasis,
	WTHR::WastelandClearMegaton,
	WTHR::WastelandEast,
	WTHR::WastelandNorth,
	//WTHR::InvertedDaylightWeatherWarm, dark nebula
	WTHR::WastelandClearNoLighting,
	//WTHR::InvertedDaylightWeather, dark nebula
	WTHR::WastelandClear,
	WTHR::gWastelandClear01,
	WTHR::gWastelandCloudy,
	//WTHR::366697, dark nebula
	WTHR::TranquilityClear, // sepia
	WTHR::GNRRoofOvercast01,
	WTHR::UrbanOvercast01Intro01,
	WTHR::MegatonCloudy01,
	WTHR::SuburbanCloudy,
	WTHR::UrbanOvercast,
};

static std::vector<WTHR> DynamicWeatherList
{
	WTHR::DefaultWeather,
	WTHR::WastelandClearNoLighting,
	WTHR::WastelandClear,
	WTHR::WastelandClearNoLighting,
	WTHR::GNRRoofOvercast01,
	WTHR::DefaultWeather,
	WTHR::MegatonCloudy01,
	WTHR::UrbanDeepInnerDCMONtop,
	WTHR::UrbanDeepInnerMamaDC,
	WTHR::UrbanDeepInnerCitadel,
	WTHR::UrbanDeepInnerDCMall,
	WTHR::UrbanDeepInner,
	WTHR::MegatonFalloutDecay,
	WTHR::WastelandDecay,
	WTHR::WastelandClearMegaton,
	WTHR::WastelandEast,
	WTHR::WastelandEastOasis,
	WTHR::WastelandEast,
	WTHR::WastelandClearMegaton,
	WTHR::SuburbanCloudy,
	WTHR::TranquilityClear,
	WTHR::UrbanOvercast,
	WTHR::GNRRoofOvercast01,
	WTHR::gWastelandClear01,
	WTHR::WastelandNorth,
	WTHR::UrbanDeep,
	WTHR::UrbanOvercast01Intro01,
	WTHR::GNRRoofOvercast01,
	WTHR::WastelandClearNoLighting,
	WTHR::GNRRoofOvercast01,
}; 

static std::vector<IDLE> SelectedAnimations
{
	// single
	IDLE::LooseMQ11PlayersDeath,
	IDLE::LooseWaveHello,
	IDLE::LooseNPCGiving,
	IDLE::LooseNPCTaking,
	IDLE::NPCSmokingIdle,
	IDLE::LooseMiddleFinger,
	IDLE::LooseDoctorHeal,
	IDLE::LooseHandsOnHeart,
	IDLE::FloorSleepDynamicIdle,
	// chat
	IDLE::TalkLHCasualE100,
	IDLE::TalkLHCasualC110,
	IDLE::PipboyExamine,
	IDLE::TalkRHCasualA124,
	IDLE::PanicGrenadePlant,
	IDLE::ttnpcDGSTArmsCrossedListenA,
	IDLE::MS11HandsClaspedBehnid,
	IDLE::LooseLazySalute,
	IDLE::LooseFingerUp,
	IDLE::LooseFingerToLips,
	IDLE::LooseHandUpStop,
	IDLE::LooseWaveHello,
	IDLE::LooseOneHandUp,
	IDLE::LooseWaveHelloShort,
	IDLE::LooseCongratGesture,
	IDLE::LooseWildlyAngry,
	IDLE::LooseShoutA,
	IDLE::_3rdP1HPShrugs,
	IDLE::MS12GreatJobKid,
	IDLE::UseIntercomA,
	// combat
	IDLE::_2HMAttackPower,
	IDLE::_2HMAttackPowerBack,
	IDLE::VATSH2HAttackPowerLowKick,
	IDLE::_2HMAttackPowerRight,
	// warm
	IDLE::MS09WarmingUp01,
	IDLE::MS09WarmingUp02,
	IDLE::MS09WarmingUp03,
	// wall
	IDLE::WallLeanFrontEntry,
	IDLE::WallLeanFrontExit,
	IDLE::WallLeanLegUpListenA,
	IDLE::WallLeanDynamicIdle,
	IDLE::WallLeanLegUpA,
	IDLE::WallLeanFrontExitFast,
	// weapon
	IDLE::_2hrLoiter,
	IDLE::_2hrShiftWeight,
	IDLE::_2hrCrouch,
	IDLE::_2haAlertScan,
	// work
	IDLE::MTSpecialIdleCheckStove,
	IDLE::_3rdPRepairWall01,
	IDLE::LooseSweeping,
	IDLE::LooseRaking,
	IDLE::_3rdPBlacksmith,
	IDLE::LooseScientistInspectTerminalB,
	IDLE::RepairWall01,
	IDLE::RepairWall02,
	// tour
	IDLE::MS05TourGuide04,
	IDLE::MS05TourGuide03,
	IDLE::MS05TourGuide02,
	IDLE::MS05TourGuide01,
	// cook
	IDLE::MTSpecialIdlePotBooze,
	IDLE::MTSpecialIdlePotStrd,
	IDLE::_3rdPFixElevator,
	// ground
	IDLE::PushUpsTalk,
	IDLE::FloorSitTalk,
	IDLE::FloorExitFast,
	IDLE::FloorExit,
	IDLE::FloorBedChildLeftEnter,
	IDLE::SandManFlrLeft,
	IDLE::StartCannibal,
	IDLE::_3rdPRepairingFlr01,
	IDLE::MS13MezzDeath,
	IDLE::FloorSitDrinkAlchohol,
	// sit
	IDLE::SitChairRelaxIdleA,
	IDLE::SitChairRelaxIdleC,
	IDLE::SitChairTalkPlayerB,
	IDLE::SitUseComputer,
	IDLE::SitChairDrinkB,
	// bar
	IDLE::SitStoolTalkA,
	IDLE::SitStoolListenA,
	IDLE::BartenderDialogue,
	IDLE::SitStoolDrinkMugA,
	IDLE::SitStoolDrinkNukaBottleA,
	IDLE::SitStoolEatMeat,
	IDLE::LooseCryingStandingB,
	// main
	IDLE::GetUpFaceDown,
	IDLE::GetUpFaceUp,
	IDLE::EatIdles,
	IDLE::HeadRight,
	IDLE::HeadLeft,
	IDLE::PipboyManipulate,
	IDLE::_3rdPersonActWater,
	IDLE::_3rdP1HPShrugsA,
	IDLE::SodaDrinkingStanding,
	IDLE::PickingNose02,
	IDLE::ScratchingSelf,
	IDLE::ScratchingSelf02,
	IDLE::Swatting,
	IDLE::LooseSleepyRubEyes,
	IDLE::LooseHandsOnHip,
	IDLE::_3rdP2HASearch,
	IDLE::RestrainedSitDynamicIdle,
	// wtf
	IDLE::HitBrain,
	IDLE::QueenAntHead1,
	IDLE::Halt,
};

#endif