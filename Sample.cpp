#include <windows.h>
#include <Outpost2DLL.h>

#include "DisasterCreator.h"

HINSTANCE hInstDLL = NULL;

ExportLevelDetails("DisasterCreator Sample", "cm01.map", "multitek.txt", Colony, 2)
BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH) 
	{
		DisableThreadLibraryCalls(hinstDLL);
		hInstDLL = hinstDLL;
	}
	else if (fdwReason == DLL_PROCESS_DETACH)
	{
		//
	}

	return TRUE;
}

struct SaveData
{
	// DisasterCreator and associated triggers
	DisasterCreator DC;
	Trigger DC_Callback,
		    DC_Timer1,
		    DC_Timer2;

	// Victory/Defeat conditions
	Trigger Skydock,
		IonDriv,
		FusDriv,
		FuelSys,
		CommMod,
		HabRing,
		SnsPack,
		StasSys,
		OrbPack,
		PhnxMod,
		ComCarg,
		RareCarg,
		FoodCarg,
		EvacModule,
		NoCC;

	// AI stuff
	BuildingGroup SF,
		Earthworker,
		VF[2];
	MiningGroup Common,
		Rare;
	FightGroup Defense,
		Offense;

	Trigger Trig_BuiltVf2,
		Trig_BuiltCommonSmelter2,
		Trig_TubeToAdvLav,
		Trig_BuiltRareSmelter,
		Trig_ArmyStart,
		Trig_Attacks[10],
		Trig_MoreTech[4];

};
SaveData SD;

// Etc
void SetupPlayer();
void SetupPlayer_Debug();
void SetupAI();
void SetupVictoryDefeat();

Export int InitProc()
{
	// Setup test players
	SetupPlayer();
	SetupAI();
	SetupVictoryDefeat();
	
	// Map settings
	TethysGame::SetDaylightEverywhere(0);
	TethysGame::SetDaylightMoves(1);
	GameMap::SetInitialLightLevel(32);

	// DisasterCreator configuration.
	SD.DC.SetMapSize(128, 128);
	SD.DC.SetMinimumWait(3700);						// Define the minimum time, after a disaster has occurred, to wait before spawning another disaster.
	SD.DC.SetIgnoreChance(2);						// Define a chance, between 0 and 100 (inclusive), for the engine to ignore the minimum wait time when trying to spawn a disaster.  In this case, we have a 2% chance.
	SD.DC.SetNumPlayers(TethysGame::NoPlayers());	// Since the DC object is stored in the savedata struct, it gets initialized before OP2 sets the number of players.
	
	// Tell the engine which disasters you want to allow...
	SD.DC.EnableDisaster(disQuake);					// Allow quakes to happen.
	SD.DC.EnableDisaster(disMeteor);				// Allow meteors to happen.

	// ...how likely they should be to happen...
	SD.DC.SetDisasterTypeWeight(disQuake, 15);		// 15% chance of a quake
	SD.DC.SetDisasterTypeWeight(disMeteor, 80);		// 80% chance of a meteor
	SD.DC.SetDisasterTypeWeight(disNone, 5);        // 5% chance nothing happens (note: these don't have to add up to 100%; they do here for convenience)

	// ...the power the disasters should have...
	SD.DC.SetDisasterPowerWeight(pwrLow, 80);		    // 80% chance of a light quake or minor meteor
	SD.DC.SetDisasterPowerWeight(pwrMedium, 15);	    // 15% chance of something a little more substantial
	SD.DC.SetDisasterPowerWeight(pwrHigh, 4);		    // 4% chance of something dangerous
	SD.DC.SetDisasterPowerWeight(pwrApocalyptic, 1);    // 1% chance of something really bad

	// ...and where you'd like them, roughly, to appear.
	SD.DC.SetDisasterTargetWeight(trgZone, 100);		// 100% chance of disasters only happening in designated zones.
													    // Note that if a disaster has no zone defined, it can spawn anywhere on the map.

	// Add a disaster zone for quakes.  Since we don't add one for meteors, they can spawn anywhere.
	SD.DC.AddDisasterZone(disQuake, MAP_RECT(17 + 31, 78 - 1, 86 + 31, 110 - 1));

	// Just for kicks: let's add a volcano.
	// The arguments are:
	//  1: Tile lava should start flowing from.
	//  2: Tick the "volcano is about to erupt" animation should start playing.
	//  3: Tick the first "volcano watch initiated" warning should play.  Volcano will erupt 100 ticks (10 marks) later.
	//  4: Which volcano animation to play.  Choices are volSouthWest, volSouthEast, volSouth, and volNone.
	//  5: Roughly how fast you'd like the lava to flow.
	SD.DC.DefineVolcano(LOCATION(113 + 31, 13 - 1), 66600, 72000, volSouthWest, spdFast);

	// By default, this function will set all "black rock" tile to lava-possible.  You can optionally pass in a list of
	// celltypes; if you do only black rock tiles of those celltypes will be lava-possible.
	SD.DC.SetLavaTiles({ cellSlowPassible1 });
	
	// Finally, define our callback trigger.  This should be a time trigger that will fire somewhat regularly,
	// to allow the engine to invoke disasters.
	SD.DC_Callback = SD.DC.SetCallbackTrigger("DisasterCreator_Callback", 200);

	// Also define some triggers to make the disasters a little tougher as the game goes on.
	SD.DC_Timer2 = CreateTimeTrigger(1, 1, 67000, "StrongerDisasters");
	SD.DC_Timer2 = CreateTimeTrigger(1, 1, 114000, "StrongestDisasters");

	return 1; // return 1 if OK; 0 on failure
}

// ------------------------------------------------------------------
// Disaster Creator stuff
Export void DisasterCreator_Callback()
{
	SD.DC.RollRandom();
	SD.DC.CheckVolcanoes();
}

Export void StrongerDisasters()
{
	// Enable the good stuff.
	SD.DC.EnableDisaster(disStorm);
	SD.DC.EnableDisaster(disVortex);

	// Adjust weights for each disaster
	SD.DC.SetDisasterTypeWeight(disQuake, 35);			// 35% chance of a quake
	SD.DC.SetDisasterTypeWeight(disStorm, 30);			// 30% chance of a storm
	SD.DC.SetDisasterTypeWeight(disMeteor, 20);		// 20% chance of a meteor
	SD.DC.SetDisasterTypeWeight(disVortex, 10);		// 10% chance of a vortex
	SD.DC.SetDisasterTypeWeight(disNone, 5);			// 5% chance of nothing

	// Also adjust power weights.
	SD.DC.SetDisasterPowerWeight(pwrLow, 30);		    // 30% chance
	SD.DC.SetDisasterPowerWeight(pwrMedium, 55);	    // 55% chance
	SD.DC.SetDisasterPowerWeight(pwrHigh, 10);		    // 10% chance
	SD.DC.SetDisasterPowerWeight(pwrApocalyptic, 5);   // 5% chance

	// We're going to take the quakes off the leash, and also add a new source of fun: player-seeking disasters
	SD.DC.SetDisasterTargetWeight(trgZone, 0);
	SD.DC.SetDisasterTargetWeight(trgRandom, 60);
	SD.DC.SetDisasterTargetWeight(trgPlayer, 40);
}

Export void StrongestDisasters()
{
	// Adjust weights for each disaster
	SD.DC.SetDisasterTypeWeight(disQuake, 30);			// 30% chance of a quake
	SD.DC.SetDisasterTypeWeight(disStorm, 30);			// 30% chance of a storm
	SD.DC.SetDisasterTypeWeight(disMeteor, 15);		// 15% chance of a meteor
	SD.DC.SetDisasterTypeWeight(disVortex, 20);		// 20% chance of a vortex
	SD.DC.SetDisasterTypeWeight(disNone, 5);			// 5% chance of nothing

	// Also adjust power weights.
	SD.DC.SetDisasterPowerWeight(pwrLow, 20);		    // 20% chance
	SD.DC.SetDisasterPowerWeight(pwrMedium, 50);	    // 50% chance
	SD.DC.SetDisasterPowerWeight(pwrHigh, 20);		    // 20% chance
	SD.DC.SetDisasterPowerWeight(pwrApocalyptic, 10);  // 10% chance

	// Increase the likelihood of targetting a player, and trick the engine into ignoring the AI player.
	SD.DC.SetDisasterTargetWeight(trgRandom, 30);
	SD.DC.SetDisasterTargetWeight(trgPlayer, 70);
	SD.DC.SetNumPlayers(1);
}

// ------------------------------------------------------------------
// Extra stuff for the level
void SetupPlayer()
{
	// Initialization
	Player[0].CenterViewOn(18 + 31, 14 - 1);
	Player[0].SetColorNumber(1);
	Player[0].GoEden();

	// Common beacons
	TethysGame::CreateBeacon(mapMiningBeacon, 17 + 31, 36 - 1, 0, 1, 1);
	TethysGame::CreateBeacon(mapMiningBeacon, 37 + 31, 30 - 1, 0, 2, 2);
	TethysGame::CreateBeacon(mapMiningBeacon, 67 + 31,  3 - 1, 0, 0, 0);
	TethysGame::CreateBeacon(mapMiningBeacon, 16 + 31, 60 - 1, 0, 1, 1);
	TethysGame::CreateBeacon(mapMiningBeacon, 89 + 31, 77 - 1, 0, 0, 0);
	TethysGame::CreateBeacon(mapMiningBeacon, 92 + 31, 24 - 1, 0, 0, 0);

	// Rare beacons
	TethysGame::CreateBeacon(mapMiningBeacon, 67 + 31, 17 - 1, 1, 1, 1);
	TethysGame::CreateBeacon(mapMiningBeacon, 11 + 31, 31 - 1, 1, 2, 1);
	TethysGame::CreateBeacon(mapMiningBeacon, 33 + 31, 118 - 1, 1, 0, 0);
	TethysGame::CreateBeacon(mapMiningBeacon, 110 + 31, 21 - 1, 1, 0, 0);

	// Fumaroles
	TethysGame::CreateBeacon(mapFumarole, 94 + 31, 12 - 1, -1, -1, -1);

	// Magma vents
	TethysGame::CreateBeacon(mapMagmaVent, 25 + 31, 98 - 1, -1, -1, -1);

	// Structures
	Unit Unit1;
	TethysGame::CreateUnit(Unit1, mapCommandCenter, LOCATION(18 + 31, 14 - 1), 0, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapStructureFactory, LOCATION(22 + 31, 13 - 1), 0, mapNone, 0);
	switch (Player[0].Difficulty())
	{
		case 0:
			Unit1.SetFactoryCargo(0, mapAgridome, mapNone);
			Unit1.SetFactoryCargo(1, mapResidence, mapNone);
			Unit1.SetFactoryCargo(3, mapTokamak, mapNone);
			break;
		case 1:
			Unit1.SetFactoryCargo(0, mapStandardLab, mapNone);
			break;
		default:
			Unit1.SetFactoryCargo(5, mapLightTower, mapNone);		// :D
			break;
	}
	TethysGame::CreateUnit(Unit1, mapCommonOreSmelter, LOCATION(30 + 31, 22 - 1), 0, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapTokamak, LOCATION(8 + 31, 6 - 1), 0, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapCommonOreMine, LOCATION(17 + 31, 36 - 1), 0, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapAgridome, LOCATION(18 + 31, 17 - 1), 0, mapNone, 0);
	if (Player[0].Difficulty() < 2)
	{
		TethysGame::CreateUnit(Unit1, mapResidence, LOCATION(14 + 31, 17 - 1), 0, mapNone, 0);
		TethysGame::CreateWallOrTube(16 + 31, 17 - 1, 0, mapTube);
	}
	if (Player[0].Difficulty() == 0)
	{
		TethysGame::CreateUnit(Unit1, mapStandardLab, LOCATION(23 + 31, 10 - 1), 0, mapNone, 0);
	}
	CreateTubeOrWallLine(26 + 31, 13 - 1, 27 + 31, 21 - 1, mapTube);

	// Vehicles
	TethysGame::CreateUnit(Unit1, mapConVec, LOCATION(24 + 31, 14 - 1), 0, mapNone, 4);
		Unit1.DoSetLights(1);
	TethysGame::CreateUnit(Unit1, mapConVec, LOCATION(24 + 31, 15 - 1), 0, mapNone, 4);
		Unit1.DoSetLights(1);

	TethysGame::CreateUnit(Unit1, mapCargoTruck, LOCATION(28 + 31, 25 - 1), 0, mapNone, 2);
		Unit1.DoSetLights(1);
	TethysGame::CreateUnit(Unit1, mapCargoTruck, LOCATION(22 + 31, 27 - 1), 0, mapNone, 3);
		Unit1.DoSetLights(1);
	TethysGame::CreateUnit(Unit1, mapCargoTruck, LOCATION(29 + 31, 30 - 1), 0, mapNone, 4);
		Unit1.DoSetLights(1);
	if (Player[0].Difficulty() == 0)
	{
		TethysGame::CreateUnit(Unit1, mapCargoTruck, LOCATION(20 + 31, 34 - 1), 0, mapNone, 4);
		Unit1.DoSetLights(1);
	}
	TethysGame::CreateUnit(Unit1, mapEarthworker, LOCATION(15 + 31, 15 - 1), 0, mapNone, 1);
		Unit1.DoSetLights(1);
	TethysGame::CreateUnit(Unit1, mapRoboSurveyor, LOCATION(29 + 31, 16 - 1), 0, mapNone, 7);
		Unit1.DoSetLights(1);

	// Starting resources
	switch (Player[0].Difficulty())
	{
	case 0:
		Player[0].SetKids(16);
		Player[0].SetWorkers(28);
		Player[0].SetScientists(14);
		Player[0].SetOre(4000);
		Player[0].SetFoodStored(3000);
		break;
	case 1:
		Player[0].SetKids(18);
		Player[0].SetWorkers(24);
		Player[0].SetScientists(12);
		Player[0].SetOre(3000);
		Player[0].SetFoodStored(2000);
		break;
	case 2:
		Player[0].SetKids(20);
		Player[0].SetWorkers(22);
		Player[0].SetScientists(10);
		Player[0].SetOre(2000);
		Player[0].SetFoodStored(600);
		break;
	}
}

void SetupPlayer_Debug()
{
	// Initialization
	Player[0].CenterViewOn(18 + 31, 14 - 1);
	Player[0].SetColorNumber(1);
	Player[0].GoEden();

	// Common beacons
	TethysGame::CreateBeacon(mapMiningBeacon, 17 + 31, 36 - 1, 0, 1, 1);
	TethysGame::CreateBeacon(mapMiningBeacon, 37 + 31, 30 - 1, 0, 2, 2);
	TethysGame::CreateBeacon(mapMiningBeacon, 67 + 31, 3 - 1, 0, 0, 0);
	TethysGame::CreateBeacon(mapMiningBeacon, 16 + 31, 60 - 1, 0, 1, 1);
	TethysGame::CreateBeacon(mapMiningBeacon, 89 + 31, 77 - 1, 0, 0, 0);
	TethysGame::CreateBeacon(mapMiningBeacon, 92 + 31, 24 - 1, 0, 0, 0);

	// Rare beacons
	TethysGame::CreateBeacon(mapMiningBeacon, 67 + 31, 17 - 1, 1, 1, 1);
	TethysGame::CreateBeacon(mapMiningBeacon, 11 + 31, 31 - 1, 1, 2, 1);
	TethysGame::CreateBeacon(mapMiningBeacon, 33 + 31, 118 - 1, 1, 0, 0);
	TethysGame::CreateBeacon(mapMiningBeacon, 110 + 31, 21 - 1, 1, 0, 0);

	// Fumaroles
	TethysGame::CreateBeacon(mapFumarole, 94 + 31, 12 - 1, -1, -1, -1);

	// Magma vents
	TethysGame::CreateBeacon(mapMagmaVent, 25 + 31, 98 - 1, -1, -1, -1);

	Player[0].SetKids(10);
	Player[0].SetWorkers(40);
	Player[0].SetScientists(35);
	Player[0].SetOre(6000);
	Player[0].SetFoodStored(8000);
	Player[0].SetTechLevel(7);

	Unit Unit1;

	TethysGame::CreateUnit(Unit1, mapCommandCenter, LOCATION(18 + 31, 14 - 1), 0, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapStructureFactory, LOCATION(22 + 31, 13 - 1), 0, mapNone, 0);
		Unit1.SetFactoryCargo(0, mapCommonOreSmelter, mapNone);
		Unit1.SetFactoryCargo(1, mapVehicleFactory, mapNone);
		Unit1.SetFactoryCargo(2, mapRareOreSmelter, mapNone);
		Unit1.SetFactoryCargo(3, mapAdvancedLab, mapNone);
		Unit1.SetFactoryCargo(4, mapSpaceport, mapNone);
	TethysGame::CreateUnit(Unit1, mapCommonOreSmelter, LOCATION(30 + 31, 22 - 1), 0, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapCommonOreSmelter, LOCATION(18 + 31, 33 - 1), 0, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapCommonOreSmelter, LOCATION(13 + 31, 33 - 1), 0, mapNone, 0);
	CreateTubeOrWallLine(18 + 31, 22 - 1, 18 + 31, 31 - 1, mapTube);
	TethysGame::CreateUnit(Unit1, mapTokamak, LOCATION(8 + 31, 6 - 1), 0, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapTokamak, LOCATION(8 + 31, 3 - 1), 0, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapCommonOreMine, LOCATION(17 + 31, 36 - 1), 0, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapAgridome, LOCATION(18 + 31, 17 - 1), 0, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapAgridome, LOCATION(18 + 31, 20 - 1), 0, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapAdvancedResidence, LOCATION(18 + 31, 10 - 1), 0, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapAdvancedResidence, LOCATION(18 + 31, 6 - 1), 0, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapResidence, LOCATION(14 + 31, 17 - 1), 0, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapResidence, LOCATION(14 + 31, 20 - 1), 0, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapAgridome, LOCATION(23 + 31, 7 - 1), 0, mapNone, 0);
	TethysGame::CreateWallOrTube(16 + 31, 17 - 1, 0, mapTube);
	TethysGame::CreateUnit(Unit1, mapStandardLab, LOCATION(23 + 31, 10 - 1), 0, mapNone, 0);
	CreateTubeOrWallLine(26 + 31, 13 - 1, 27 + 31, 21 - 1, mapTube);
	TethysGame::CreateUnit(Unit1, mapNursery, LOCATION(27 + 31, 10 - 1), 0, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapUniversity, LOCATION(27 + 31, 7 - 1), 0, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapMedicalCenter, LOCATION(30 + 31, 10 - 1), 0, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapMedicalCenter, LOCATION(30 + 31, 7 - 1), 0, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapMedicalCenter, LOCATION(33 + 31, 7 - 1), 0, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapMedicalCenter, LOCATION(36 + 31, 7 - 1), 0, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapRecreationFacility, LOCATION(33 + 31, 10 - 1), 0, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapRecreationFacility, LOCATION(36 + 31, 10 - 1), 0, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapRecreationFacility, LOCATION(39 + 31, 10 - 1), 0, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapRobotCommand, LOCATION(39 + 31, 7 - 1), 0, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapVehicleFactory, LOCATION(29 + 31, 13 - 1), 0, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapVehicleFactory, LOCATION(34 + 31, 13 - 1), 0, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapGORF, LOCATION(23 + 31, 4 - 1), 0, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapDIRT, LOCATION(27 + 31, 4 - 1), 0, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapRareOreSmelter, LOCATION(63 + 31, 18 - 1), 0, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapCommonOreMine, LOCATION(67 + 31, 17 - 1), 0, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapGeothermalPlant, LOCATION(95 + 31, 12 - 1), 0, mapNone, 0);

	// Vehicles
	TethysGame::CreateUnit(Unit1, mapConVec, LOCATION(24 + 31, 14 - 1), 0, mapNone, 4);
	Unit1.DoSetLights(1);
	TethysGame::CreateUnit(Unit1, mapConVec, LOCATION(24 + 31, 15 - 1), 0, mapNone, 4);
	Unit1.DoSetLights(1);
	TethysGame::CreateUnit(Unit1, mapConVec, LOCATION(25 + 31, 15 - 1), 0, mapNone, 4);
	Unit1.DoSetLights(1);

	TethysGame::CreateUnit(Unit1, mapCargoTruck, LOCATION(28 + 31, 25 - 1), 0, mapNone, 2);
	Unit1.DoSetLights(1);
	TethysGame::CreateUnit(Unit1, mapCargoTruck, LOCATION(22 + 31, 27 - 1), 0, mapNone, 3);
	Unit1.DoSetLights(1);
	TethysGame::CreateUnit(Unit1, mapCargoTruck, LOCATION(29 + 31, 30 - 1), 0, mapNone, 4);
	Unit1.DoSetLights(1);
	TethysGame::CreateUnit(Unit1, mapCargoTruck, LOCATION(20 + 31, 34 - 1), 0, mapNone, 4);
	Unit1.DoSetLights(1);
	TethysGame::CreateUnit(Unit1, mapCargoTruck, LOCATION(60 + 31, 22 - 1), 0, mapNone, 4);
	Unit1.DoSetLights(1);
	TethysGame::CreateUnit(Unit1, mapCargoTruck, LOCATION(61 + 31, 22 - 1), 0, mapNone, 4);
	Unit1.DoSetLights(1);
	TethysGame::CreateUnit(Unit1, mapEarthworker, LOCATION(15 + 31, 15 - 1), 0, mapNone, 1);
	Unit1.DoSetLights(1);
	TethysGame::CreateUnit(Unit1, mapEarthworker, LOCATION(16 + 31, 15 - 1), 0, mapNone, 1);
	Unit1.DoSetLights(1);
	TethysGame::CreateUnit(Unit1, mapRoboSurveyor, LOCATION(29 + 31, 16 - 1), 0, mapNone, 7);
	Unit1.DoSetLights(1);
	TethysGame::CreateUnit(Unit1, mapRoboMiner, LOCATION(30 + 31, 16 - 1), 0, mapNone, 7);
	Unit1.DoSetLights(1);

	TethysGame::CreateUnit(Unit1, mapTiger, LOCATION(59 + 31, 29 - 1), 0, mapThorsHammer, 0);
	Unit1.DoSetLights(1);
	TethysGame::CreateUnit(Unit1, mapTiger, LOCATION(59 + 31, 30 - 1), 0, mapThorsHammer, 0);
	Unit1.DoSetLights(1);
	TethysGame::CreateUnit(Unit1, mapTiger, LOCATION(59 + 31, 31 - 1), 0, mapThorsHammer, 0);
	Unit1.DoSetLights(1);
	TethysGame::CreateUnit(Unit1, mapTiger, LOCATION(59 + 31, 32 - 1), 0, mapThorsHammer, 0);
	Unit1.DoSetLights(1);

	CreateTubeOrWallLine(41 + 31, 10 - 1, 63 + 31, 21 - 1, mapTube);

}

void SetupVictoryDefeat()
{
	// Evacuation Module
	SD.EvacModule = CreateCountTrigger(1, 0, 0, mapEvacuationModule, mapNone, 1, cmpGreaterEqual, "None");
	CreateVictoryCondition(1, 0, SD.EvacModule, "Evacuate 200 colonists to the starship.");

	// Food Cargo
	SD.FoodCarg = CreateCountTrigger(1, 0, 0, mapFoodCargo, mapNone, 1, cmpGreaterEqual, "None");
	CreateVictoryCondition(1, 0, SD.FoodCarg, "Evacuate 10000 units of food to the starship.");

	// Rare Metals Cargo
	SD.RareCarg = CreateCountTrigger(1, 0, 0, mapRareMetalsCargo, mapNone, 1, cmpGreaterEqual, "None");
	CreateVictoryCondition(1, 0, SD.RareCarg, "Evacuate 10000 units of Rare Metals to the starship.");

	// Common Metals Cargo
	SD.ComCarg = CreateCountTrigger(1, 0, 0, mapCommonMetalsCargo, mapNone, 1, cmpGreaterEqual, "None");
	CreateVictoryCondition(1, 0, SD.ComCarg, "Evacuate 10000 units of Common Metals to the starship.");

	// Phoenix Module
	SD.PhnxMod = CreateCountTrigger(1, 0, 0, mapPhoenixModule, mapNone, 1, cmpGreaterEqual, "None");
	CreateVictoryCondition(1, 0, SD.PhnxMod, "Launch the Phoenix Module.");

	// Orbital Package
	SD.OrbPack = CreateCountTrigger(1, 0, 0, mapOrbitalPackage, mapNone, 1, cmpGreaterEqual, "None");
	CreateVictoryCondition(1, 0, SD.OrbPack, "Launch the Orbital Package.");

	// Stasis Systems
	SD.StasSys = CreateCountTrigger(1, 0, 0, mapStasisSystems, mapNone, 1, cmpGreaterEqual, "None");
	CreateVictoryCondition(1, 0, SD.StasSys, "Launch the Stasis Systems.");

	// Sensor Package
	SD.SnsPack = CreateCountTrigger(1, 0, 0, mapSensorPackage, mapNone, 1, cmpGreaterEqual, "None");
	CreateVictoryCondition(1, 0, SD.SnsPack, "Launch the Sensor Package.");

	// Habitat Ring
	SD.HabRing = CreateCountTrigger(1, 0, 0, mapHabitatRing, mapNone, 1, cmpGreaterEqual, "None");
	CreateVictoryCondition(1, 0, SD.HabRing, "Launch the Habitat Ring.");

	// Command Module
	SD.CommMod = CreateCountTrigger(1, 0, 0, mapCommandModule, mapNone, 1, cmpGreaterEqual, "None");
	CreateVictoryCondition(1, 0, SD.CommMod, "Launch the Command Module.");

	// Fueling Systems
	SD.FuelSys = CreateCountTrigger(1, 0, 0, mapFuelingSystems, mapNone, 1, cmpGreaterEqual, "None");
	CreateVictoryCondition(1, 0, SD.FuelSys, "Launch the Fueling Systems.");

	// Fusion Drive
	SD.FusDriv = CreateCountTrigger(1, 0, 0, mapFusionDriveModule, mapNone, 1, cmpGreaterEqual, "None");
	CreateVictoryCondition(1, 0, SD.FusDriv, "Launch the Fusion Drive Module.");

	// Ion Drive
	SD.IonDriv = CreateCountTrigger(1, 0, 0, mapIonDriveModule, mapNone, 1, cmpGreaterEqual, "None");
	CreateVictoryCondition(1, 0, SD.IonDriv, "Launch the Ion Drive Module.");

	// Skydock
	SD.Skydock = CreateCountTrigger(1, 0, 0, mapSkydock, mapNone, 1, cmpGreaterEqual, "None");
	CreateVictoryCondition(1, 0, SD.Skydock, "Place the Skydock in orbit.");

	// Failure condition
	SD.NoCC = CreateOperationalTrigger(1, 1, 0, mapCommandCenter, 0, cmpEqual, "None");
	CreateFailureCondition(1, 0, SD.NoCC, "");
}

void SetupAI()
{
	Player[1].SetColorNumber(5);
	Player[1].GoEden();
	Player[1].GoAI();
	Player[1].SetSolarSat(2);	// Yeah it's cheap, but this is a quick project I threw together in a few hours!
	Player[1].SetOre(5600);
	Player[1].SetTechLevel(4);

	TethysGame::CreateBeacon(mapMiningBeacon, 118 + 31, 121 - 1, 0, 0, 0);
	TethysGame::CreateBeacon(mapMiningBeacon, 122 + 31, 92 - 1, 1, 0, 0);

	Unit Unit1,
		Smelter,
		Mine;

	// Create default groups
	SD.SF = CreateBuildingGroup(1);
	SD.Earthworker = CreateBuildingGroup(1);
	SD.Common = CreateMiningGroup(1);
	SD.Rare = CreateMiningGroup(1);
	

	// Structures
	TethysGame::CreateUnit(Unit1, mapCommandCenter, LOCATION(117 + 31, 106 - 1), 1, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapSolarPowerArray, LOCATION(125 + 31, 124 - 1), 1, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapSolarPowerArray, LOCATION(107 + 31, 118 - 1), 1, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapStructureFactory, LOCATION(113 + 31, 106 - 1), 1, mapNone, 0);
		SD.SF.TakeUnit(Unit1);
	TethysGame::CreateUnit(Mine, mapCommonOreMine, LOCATION(118 + 31, 121 - 1), 1, mapNone, 0);
	TethysGame::CreateUnit(Smelter, mapCommonOreSmelter, LOCATION(121 + 31, 114 - 1), 1, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapAgridome, LOCATION(113 + 31, 103 - 1), 1, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapAgridome, LOCATION(113 + 31, 100 - 1), 1, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapStandardLab, LOCATION(117 + 31, 103 - 1), 1, mapNone, 0);
	TethysGame::CreateUnit(Unit1, mapResidence, LOCATION(110 + 31, 103 - 1), 1, mapNone, 0);

	// Vehicles
	TethysGame::CreateUnit(Unit1, mapConVec, LOCATION(111 + 31, 109 - 1), 1, mapNone, 0);
		Unit1.DoSetLights(1);
		SD.SF.TakeUnit(Unit1);
	TethysGame::CreateUnit(Unit1, mapConVec, LOCATION(112 + 31, 109 - 1), 1, mapNone, 0);
		Unit1.DoSetLights(1);
		SD.SF.TakeUnit(Unit1);
	TethysGame::CreateUnit(Unit1, mapConVec, LOCATION(113 + 31, 109 - 1), 1, mapNone, 0);
		Unit1.DoSetLights(1);
		SD.SF.TakeUnit(Unit1);

	TethysGame::CreateUnit(Unit1, mapCargoTruck, LOCATION(118 + 31, 118 - 1), 1, mapNone, 0);
		Unit1.DoSetLights(1);
		SD.Common.TakeUnit(Unit1);
	TethysGame::CreateUnit(Unit1, mapCargoTruck, LOCATION(119 + 31, 118 - 1), 1, mapNone, 0);
		Unit1.DoSetLights(1);
		SD.Common.TakeUnit(Unit1);
	TethysGame::CreateUnit(Unit1, mapCargoTruck, LOCATION(120 + 31, 118 - 1), 1, mapNone, 0);
		Unit1.DoSetLights(1);
		SD.Common.TakeUnit(Unit1);

	TethysGame::CreateUnit(Unit1, mapEarthworker, LOCATION(118 + 31, 110 - 1), 1, mapNone, 0);
		Unit1.DoSetLights(1);
		SD.Earthworker.TakeUnit(Unit1);

	CreateTubeOrWallLine(120 + 31, 106 - 1, 120 + 31, 112 - 1, mapTube);

	// Setup groups
	SD.SF.SetRect(MAP_RECT(111 + 31, 106 - 1, 115 + 31, 109 - 1));
	SD.SF.SetTargCount(mapConVec, mapNone, 3);
	SD.Earthworker.SetRect(MAP_RECT(105 + 31, 108 - 1, 114 + 31, 111 - 1));
	SD.Earthworker.SetTargCount(mapEarthworker, mapNone, 1);
	SD.Earthworker.RecordTubesTouching(LOCATION(120 + 31, 106 - 1));
	for (int x = 113; x >= 105; x--)
	{
		SD.Earthworker.RecordTube(LOCATION(x + 31, 109 - 1));
	}
	SD.VF[0] = CreateBuildingGroup(1);
	SD.VF[0].RecordVehReinforceGroup(SD.Common, 9999);
	SD.VF[0].RecordVehReinforceGroup(SD.SF, 700);
	SD.VF[0].RecordVehReinforceGroup(SD.Earthworker, 300);
	SD.VF[0].RecordBuilding(LOCATION(118 + 31, 121 - 1), mapCommonOreMine, mapNone, SD.Common);

	SD.Common.Setup(Mine, Smelter, MAP_RECT(116 + 31, 114 - 1, 123 + 31, 124 - 1));
	SD.Common.SetTargCount(mapCargoTruck, mapNone, 4);

	// Build orders
	SD.SF.RecordBuilding(LOCATION(125 + 31, 124 - 1), mapSolarPowerArray, mapNone);
	SD.SF.RecordBuilding(LOCATION(107 + 31, 118 - 1), mapSolarPowerArray, mapNone);
	SD.SF.RecordBuilding(LOCATION(103 + 31, 109 - 1), mapVehicleFactory, mapNone, SD.VF[0]);
	SD.SF.RecordBuilding(LOCATION(117 + 31, 103 - 1), mapStandardLab, mapNone);
	SD.SF.RecordBuilding(LOCATION(121 + 31, 103 - 1), mapUniversity, mapNone);
	SD.SF.RecordBuilding(LOCATION(124 + 31, 103 - 1), mapNursery, mapNone);
	SD.SF.RecordBuilding(LOCATION(110 + 31, 103 - 1), mapResidence, mapNone);
	SD.SF.RecordBuilding(LOCATION(110 + 31, 125 - 1), mapAdvancedLab, mapNone);
	SD.SF.RecordBuilding(LOCATION(113 + 31, 103 - 1), mapAgridome, mapNone);
	SD.SF.RecordBuilding(LOCATION(113 + 31, 100 - 1), mapAgridome, mapNone);
	SD.SF.RecordBuilding(LOCATION(107 + 31, 103 - 1), mapRobotCommand, mapNone);

	// Triggers
	SD.Trig_TubeToAdvLav = CreateCountTrigger(1, 1, 1, mapNursery, mapNone, 1, cmpEqual, "AI_StartTubingToAdvLab");
	SD.Trig_ArmyStart = CreateTimeTrigger(1, 1, 14000, "AI_StartBuildingAnArmy");
	SD.Trig_Attacks[0] = CreateTimeTrigger(1, 1, 21000, "AI_Attack1");
	SD.Trig_Attacks[1] = CreateTimeTrigger(1, 1, 29000, "AI_Attack2");
	SD.Trig_Attacks[2] = CreateTimeTrigger(1, 1, 37000, "AI_Attack3");
	SD.Trig_Attacks[3] = CreateTimeTrigger(1, 1, 46000, "AI_Attack3"); // Repeat
	SD.Trig_Attacks[4] = CreateTimeTrigger(1, 1, 52000, "AI_Attack4");
	SD.Trig_Attacks[5] = CreateTimeTrigger(1, 1, 61000, "AI_Attack4"); // Repeat
	SD.Trig_Attacks[6] = CreateTimeTrigger(1, 1, 70000, "AI_Attack5");
	SD.Trig_Attacks[7] = CreateTimeTrigger(1, 1, 81000, "AI_Attack6");
	SD.Trig_MoreTech[0] = CreateTimeTrigger(1, 1, 34000, "AI_MoreTech1");
	SD.Trig_MoreTech[1] = CreateTimeTrigger(1, 1, 51000, "AI_MoreTech2");
	SD.Trig_MoreTech[1] = CreateTimeTrigger(1, 1, 73000, "AI_MoreTech3");
	SD.Trig_MoreTech[1] = CreateTimeTrigger(1, 1, 90700, "AI_MoreTech4");
	SD.Trig_BuiltRareSmelter = CreateTimeTrigger(1, 0, 300, "AI_RareOre1Ready");
}

Export void AI_StartTubingToAdvLab()
{
	// When the Nursery gets deployed, the AI should start tubing to the site of the Advanced Lab.
	SD.Earthworker.SetRect(MAP_RECT(109 + 31, 116 - 1, 123 + 31, 127 - 1));
	for (int y = 117; y <= 125; y++)
	{
		SD.Earthworker.RecordTube(LOCATION(121 + 31, y - 1));
	}
	for (int x = 120; x >= 112; x--)
	{
		SD.Earthworker.RecordTube(LOCATION(x + 31, 125 - 1));
	}

	for (int y = 127; y >= 122; y--)
	{
		SD.Earthworker.RecordWall(LOCATION(107 + 31, y - 1), mapWall);
	}

	for (int x = 108; x <= 112; x++)
	{
		SD.Earthworker.RecordWall(LOCATION(x + 31, 122 - 1), mapWall);
	}

	SD.Earthworker.RecordWall(LOCATION(113 + 31, 122 - 1), mapWall);
	SD.Earthworker.RecordWall(LOCATION(113 + 31, 123 - 1), mapWall);
	SD.Earthworker.RecordWall(LOCATION(113 + 31, 124 - 1), mapWall);
	SD.Earthworker.RecordWall(LOCATION(113 + 31, 126 - 1), mapWall);
	SD.Earthworker.RecordWall(LOCATION(113 + 31, 127 - 1), mapWall);
}

Export void AI_StartBuildingAnArmy()
{
	// Setup defense and reinforce fight group
	SD.Defense = CreateFightGroup(1);
	SD.Defense.SetRect(MAP_RECT(107 + 31, 91 - 1, 114 + 31, 96 - 1));
	SD.Defense.SetTargCount(mapLynx, mapLaser, 5);
	SD.Defense.AddGuardedRect(MAP_RECT(105 + 31, 95 - 1, 128 + 31, 127 - 1));
	SD.Defense.AddGuardedRect(MAP_RECT(96 + 31, 104 - 1, 105 + 31, 117 - 1));
	SD.VF[0].RecordVehReinforceGroup(SD.Defense, 4000);
}

Export void AI_Attack1()
{
	// Setup attack group
	SD.Offense = CreateFightGroup(1);
	SD.Offense.SetAttackType(mapCommandCenter);
	SD.Offense.DoAttackEnemy();
	SD.Offense.TakeAllUnits(SD.Defense);

	// Upgrade the defense group
	SD.Defense.SetTargCount(mapLynx, mapLaser, 9);

	// Have the AI keep pretending it needs to manage morale and stuff.
	SD.SF.RecordBuilding(LOCATION(117 + 31, 100 - 1), mapDIRT, mapNone);
	SD.SF.RecordBuilding(LOCATION(110 + 31, 100 - 1), mapResidence, mapNone);
	SD.SF.RecordBuilding(LOCATION(121 + 31, 100 - 1), mapMedicalCenter, mapNone);
	SD.SF.RecordBuilding(LOCATION(124 + 31, 100 - 1), mapMedicalCenter, mapNone);
}

Export void AI_Attack2()
{
	// Setup attack group
	SD.Offense.SetAttackType(mapCommandCenter);
	SD.Offense.DoAttackEnemy();
	SD.Offense.TakeAllUnits(SD.Defense);

	// Upgrade the defense group
	SD.Defense.SetTargCount(mapLynx, mapLaser, 6);
	SD.Defense.SetTargCount(mapLynx, mapRailGun, 3);
	SD.Defense.SetTargCount(mapLynx, mapEMP, 1);
}

Export void AI_Attack3()
{
	// Setup attack group
	SD.Offense.SetAttackType(mapCommandCenter);
	SD.Offense.DoAttackEnemy();
	SD.Offense.TakeAllUnits(SD.Defense);

	// Upgrade the defense group
	SD.Defense.SetTargCount(mapLynx, mapLaser, 4);
	SD.Defense.SetTargCount(mapLynx, mapRailGun, 6);
	SD.Defense.SetTargCount(mapLynx, mapEMP, 4);
}

Export void AI_Attack4()
{
	// Setup attack group
	SD.Offense.SetAttackType(mapCommandCenter);
	SD.Offense.DoAttackEnemy();
	SD.Offense.TakeAllUnits(SD.Defense);

	// Upgrade the defense group
	SD.Defense.SetTargCount(mapPanther, mapLaser, 4);
	SD.Defense.SetTargCount(mapPanther, mapRailGun, 7);
	SD.Defense.SetTargCount(mapPanther, mapEMP, 5);
}

Export void AI_Attack5()
{
	// Setup attack group
	SD.Offense.SetAttackType(mapCommandCenter);
	SD.Offense.DoAttackEnemy();
	SD.Offense.TakeAllUnits(SD.Defense);

	// Upgrade the defense group
	SD.Defense.SetTargCount(mapPanther, mapLaser, 4);
	SD.Defense.SetTargCount(mapPanther, mapRailGun, 4);
	SD.Defense.SetTargCount(mapPanther, mapThorsHammer, 3);
	SD.Defense.SetTargCount(mapPanther, mapEMP, 4);
}

Export void AI_Attack6()
{
	// Setup attack group
	SD.Offense.SetAttackType(mapCommandCenter);
	SD.Offense.DoAttackEnemy();
	SD.Offense.TakeAllUnits(SD.Defense);

	// Upgrade the defense group
	SD.Defense.SetTargCount(mapPanther, mapLaser, 4);
	SD.Defense.SetTargCount(mapPanther, mapThorsHammer, 8);
	SD.Defense.SetTargCount(mapPanther, mapEMP, 6);

	SD.Trig_Attacks[7] = CreateTimeTrigger(1, 0, 6000, 8000, "AI_AttackEnd");
}

// This attack will repeat every 60-80 marks
Export void AI_AttackEnd()
{
	// Setup attack group
	SD.Offense.SetAttackType(mapCommandCenter);
	SD.Offense.DoAttackEnemy();
	SD.Offense.TakeAllUnits(SD.Defense);

	// Upgrade the defense group
	SD.Defense.SetTargCount(mapTiger, mapLaser, 3);
	SD.Defense.SetTargCount(mapTiger, mapAcidCloud, 6 + TethysGame::GetRand(4));
	SD.Defense.SetTargCount(mapTiger, mapThorsHammer, 10 + TethysGame::GetRand(4));
	SD.Defense.SetTargCount(mapTiger, mapEMP, 8 + TethysGame::GetRand(3));

	// Cheat
	Player[1].SetOre(10000);
	Player[1].SetRareOre(10000);
}

Export void AI_MoreTech1()
{
	Player[1].SetTechLevel(4);

	// Setup rare ore stuff
	SD.SF.RecordBuilding(LOCATION(118 + 31, 89 - 1), mapRareOreSmelter, mapNone, SD.Rare);
	SD.VF[0].RecordBuilding(LOCATION(122 + 31, 92 - 1), mapRareOreMine, mapNone, SD.Rare);

	// Tube stuff
	SD.Earthworker.SetRect(MAP_RECT(116 + 31, 91 - 1, 122 + 31, 99 - 1));
	for (int y = 98; y >= 91; y--)
	{
		SD.Earthworker.RecordTube(LOCATION(118 + 31, y - 1));
	}

	// Add rare mine area to defense zone
	SD.Defense.AddGuardedRect(MAP_RECT(116 + 31, 79 - 1, 128 + 31, 95 - 1));
	SD.Defense.AddGuardedRect(MAP_RECT(105 + 31, 81 - 1, 122 + 31, 92 - 1));
}

Export void AI_MoreTech2()
{
	Player[1].SetTechLevel(6);
}

Export void AI_MoreTech3()
{
	Player[1].SetTechLevel(9);
}

Export void AI_MoreTech4()
{
	Player[1].SetTechLevel(12);
}

Export void AI_RareOre1Ready()
{
	Unit NextUnit,
		Smelter,
		Mine;
	GroupEnumerator rare1(SD.Rare);
	while (rare1.GetNext(NextUnit))
	{
		if (NextUnit.GetType() == mapRareOreSmelter &&
			NextUnit.IsLive() &&
			!NextUnit.IsBusy())
		{
			Smelter = NextUnit;
		}
		else if (NextUnit.GetType() == mapRareOreMine &&
			NextUnit.IsLive() &&
			!NextUnit.IsBusy())
		{
			Mine = NextUnit;
		}
	}

	if (Smelter.unitID != 0 && Mine.unitID != 0)
	{
		SD.Rare.Setup(Mine, Smelter, MAP_RECT(118 + 31, 89 - 1, 123 + 31, 93 - 1));
		SD.Rare.SetTargCount(mapCargoTruck, mapNone, 2);
		SD.VF[0].RecordVehReinforceGroup(SD.Rare, 6000);
		SD.Trig_BuiltRareSmelter.Destroy();
	}
}

// ------------------------------------------------------------------
// The usual OP2 stuff
Export void AIProc()
{
	//
}

Export void __cdecl GetSaveRegions(struct BufferDesc &bufDesc)
{
	bufDesc.bufferStart = &SD;	// Pointer to a buffer that needs to be saved
	bufDesc.length = sizeof(SD);			// sizeof(buffer)
}

// I use "None" instead of "NoResponseToTrigger" because it's shorter
Export void None()
{
}