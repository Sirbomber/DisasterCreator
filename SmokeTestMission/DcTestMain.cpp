#include <DisasterCreator/DisasterCreator.h>
#include <Tethys/API/API.h>

using namespace Tethys;
using namespace Tethys::TethysAPI;

HINSTANCE hInstDLL = NULL;

MISSION_API char MapName[] = "cm01.map";
MISSION_API char LevelDesc[] = "DC Smoke Test";
MISSION_API char TechtreeName[] = "multitek.txt";
MISSION_API ModDesc DescBlock = { MissionType::Colony, 6, 12, 0 };

BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH) 
	{
		hInstDLL = hinstDLL;
	}
	else if (fdwReason == DLL_PROCESS_DETACH)
	{
	}

	return TRUE;
}

DisasterCreator DC;

void SetupPlayer_Debug();

MISSION_API int InitProc()
{
	// Setup test players
	SetupPlayer_Debug();
	
	// Map settings
	TethysGame::SetDaylightEverywhere(0);
	TethysGame::SetDaylightMoves(1);
	GameMap::SetInitialLightLevel(90);

	// DisasterCreator configuration.
	DC.SetMinimumWait(10);
	//DC.SetIgnoreChance(2);
	//DC.EnableDisaster(disQuake);
	//DC.EnableDisaster(disStorm);
	//DC.EnableDisaster(disVortex);
	DC.EnableDisaster(disMeteor);
	//DC.SetDisasterTypeWeight(disQuake, 24);
	//DC.SetDisasterTypeWeight(disStorm, 24);
	//DC.SetDisasterTypeWeight(disVortex, 24);
	//DC.SetDisasterTypeWeight(disMeteor, 24);
	DC.SetDisasterTypeWeight(disNone, 4);
	DC.SetDisasterPowerWeight(pwrLow, 100);
	//DC.SetDisasterPowerWeight(pwrMedium, 15);
	//DC.SetDisasterPowerWeight(pwrHigh, 4);
	//DC.SetDisasterPowerWeight(pwrApocalyptic, 1);

	// Test zone and player targetting
	DC.SetDisasterTargetWeight(trgZone, 100);
	//DC.SetDisasterTargetWeight(trgPlayer, 50);


	// Use meteors to test disaster zones
	//DC.AddDisasterZone(disMeteor, MapRect(0, 64, 128, 128));

	// Start testing script
	//CreateTimeTrigger(200, "TestBlight_Step1");
	CreateTimeTrigger(200, "TestTargettedDisasters_Step1");

	// Set callback trigger
	DC.SetCallbackTrigger("DisasterCreator_Callback", 200);

	// Change callback trigger later
	CreateTimeTrigger(5000, "NewCallback");

	return 1; // return 1 if OK; 0 on failure
}

// ------------------------------------------------------------------
// Disaster Creator test callbacks
MISSION_API void DisasterCreator_Callback()
{
	DC.RollRandom();
	DC.CheckVolcanoes();
}

MISSION_API void DisasterCreator_Callback2()
{
	DC.RollRandom();
	DC.CheckVolcanoes();
}

MISSION_API void NewCallback()
{
	DC.SetCallbackTrigger("DisasterCreator_Callback2", 100);
}

// Blight tests
MISSION_API void TestBlight_Step1()
{
	TethysGame::AddMessage("Test: spawn explicity immobile blight square at 52, 97", SoundID::Beep2, -1, GameMap::At(52, 97));
	CreateTimeTrigger(50, "TestBlight_Step2");
}

MISSION_API void TestBlight_Step2()
{
	DC.SetBlight(GameMap::At(52, 97), bshpSquare, 4, spdStopped);
	CreateTimeTrigger(50, "TestBlight_Step3");
}

MISSION_API void TestBlight_Step3()
{
	TethysGame::AddMessage("Test: spawn blight diamond (explicit no speed change, no warning) at 75, 104", SoundID::Beep2, -1, GameMap::At(75, 104));
	CreateTimeTrigger(50, "TestBlight_Step4");
}

MISSION_API void TestBlight_Step4()
{
	DC.SetBlight(GameMap::At(75, 104), bshpDiamond, 4, spdNoChange, true, true);
	CreateTimeTrigger(50, "TestBlight_Step5");
}

MISSION_API void TestBlight_Step5()
{
	TethysGame::AddMessage("Test: spawn blight up triangle (implicit no speed change) at 104, 80", SoundID::Beep2, -1, GameMap::At(104, 80));
	CreateTimeTrigger(50, "TestBlight_Step6");
}

MISSION_API void TestBlight_Step6()
{
	DC.SetBlight(GameMap::At(104, 80), bshpTriUp, 4);
	CreateTimeTrigger(50, "TestBlight_Step7");
}

MISSION_API void TestBlight_Step7()
{
	TethysGame::AddMessage("Test: spawn blight down triangle (implicit no speed change) at 109, 112", SoundID::Beep2, -1, GameMap::At(109, 112));
	CreateTimeTrigger(50, "TestBlight_Step8");
}

MISSION_API void TestBlight_Step8()
{
	DC.SetBlight(GameMap::At(109, 112), bshpTriDown, 4);
	CreateTimeTrigger(50, "TestBlight_Step9");
}

MISSION_API void TestBlight_Step9()
{
	TethysGame::AddMessage("Test: set blight spread speed to medium for 10 marks", SoundID::Beep2);
	DC.SetBlightSpeed(spdMedium);
	CreateTimeTrigger(1000, "TestBlight_Step10");
}

MISSION_API void TestBlight_Step10()
{
	TethysGame::AddMessage("Test: stop blight spread", SoundID::Beep2);
	DC.SetBlightSpeed(spdStopped);
	CreateTimeTrigger(100, "TestBlight_Done");
}

MISSION_API void TestBlight_Done()
{
	TethysGame::AddMessage("Blight tests done.  Moving to volcano tests.", SoundID::Beep2);
	CreateTimeTrigger(100, "TestVolcano_Step1");
}

// Volcano tests
MISSION_API void TestVolcano_Step1()
{
	TethysGame::AddMessage("Test: define volcano.  Eruption in 10 marks.  Next test in 20 marks.", SoundID::Beep2, -1, GameMap::At(113, 13));
	CreateTimeTrigger(2000, "TestVolcano_Step2");

	// Define volcano
	DC.DefineVolcano(GameMap::At(113, 13), 0, 0, volSouthWest, spdInstant);
	DC.SetLavaTiles({ CellType::SlowPassible1 });
}

MISSION_API void TestVolcano_Step2()
{
	TethysGame::AddMessage("Test: expand lava flow area", SoundID::Beep2, -1, GameMap::At(113, 13));
	DC.SetLavaTiles({ CellType::SlowPassible1, CellType::MediumPassible2 });

	CreateTimeTrigger(100, "TestVolcano_Done");
}

MISSION_API void TestVolcano_Done()
{
	TethysGame::AddMessage("Volcano tests done.  Moving to player-targetting disaster tests.", SoundID::Beep2);
	CreateTimeTrigger(100, "TestTargettedDisasters_Step1");
}

MISSION_API void TestTargettedDisasters_Step1()
{
	TethysGame::AddMessage("Spawning 10 meteors.", SoundID::Beep2);
	for (int i = 0; i < 10; i++)
	{
		DC.DoDisaster(disMeteor, pwrLow, trgPlayer, true, true);
	}
}

// ------------------------------------------------------------------
// Extra stuff for the level

void SetupPlayer_Debug()
{
	// Initialization
	Player[0].SetColor(PlayerColor::Red);
	Player[0].GoEden();

	Player[0].SetKids(10);
	Player[0].SetWorkers(40);
	Player[0].SetScientists(35);
	Player[0].SetCommonOre(6000);
	Player[0].SetFoodStored(8000);
	TethysGame::ForceMorale(MoraleLevel::Excellent);

	Unit Unit1;

	TethysGame::CreateUnit(MapID::CommandCenter, GameMap::At(18, 14), 0);
	TethysGame::CreateUnit(MapID::Scout, GameMap::At(17, 15), 0);
	TethysGame::CreateUnit(MapID::Scout, GameMap::At(18, 15), 0);
	TethysGame::CreateUnit(MapID::Scout, GameMap::At(19, 15), 0);
	TethysGame::CreateUnit(MapID::Earthworker, GameMap::At(20, 15), 0);
	TethysGame::CreateUnit(MapID::StructureFactory, GameMap::At(22, 13), 0);
		Unit1.SetFactoryCargo(0, MapID::CommonOreSmelter, MapID::None);
		Unit1.SetFactoryCargo(1, MapID::VehicleFactory, MapID::None);
		Unit1.SetFactoryCargo(2, MapID::RareOreSmelter, MapID::None);
		Unit1.SetFactoryCargo(3, MapID::AdvancedLab, MapID::None);
		Unit1.SetFactoryCargo(4, MapID::Spaceport, MapID::None);
	TethysGame::CreateUnit(MapID::CommonOreSmelter, Location(30 + 31, 22 - 1), 0, MapID::None);
	TethysGame::CreateUnit(MapID::CommonOreSmelter, Location(18 + 31, 33 - 1), 0, MapID::None);
	TethysGame::CreateUnit(MapID::CommonOreSmelter, Location(13 + 31, 33 - 1), 0, MapID::None);
	TethysGame::CreateUnit(MapID::GeothermalPlant, Location(95 + 31, 12 - 1), 0);

	// Vehicles
	TethysGame::CreateUnit(MapID::ConVec, Location(24 + 31, 14 - 1), 0);
	Unit1.DoSetLights(1);

}

// ------------------------------------------------------------------
// The usual OP2 stuff
MISSION_API void AIProc()
{
	//
}

// I use "None" instead of "NoResponseToTrigger" because it's shorter
MISSION_API void None()
{
}