#include "DisasterCreator.h"

/*
   Constructor/Destructor + basic disaster creation functions.
*/

DisasterCreator::DisasterCreator()
{
	lastTick = TethysGame::Tick();
	numPlayers = TethysGame::NoPlayers();
}

DisasterCreator::~DisasterCreator()
{
	if (disCheck.IsInitialized())
	{
		disCheck.Destroy();
	}
}

// ----------------------------------------------

void DisasterCreator::RollRandom()
{
	// Safety!
	if (lowWeight + mediumWeight + highWeight + apocWeight == 0 ||
		quakesWeight + stormsWeight + vortexWeight + meteorWeight + noneWeight == 0 ||
		randWeight + zoneWeight + plyrWeight == 0)
	{
		TethysGame::AddMessage(-1, -1, "DC Error: No weights defined!", -1, sndDirt);
		return;
	}

	// Get a random number from 0 to (total power weights)
	disPower power = pwrLow;
	int rv = TethysGame::GetRand(lowWeight + mediumWeight + highWeight + apocWeight + 1);
	if (rv < lowWeight)
	{
		power = pwrLow;
	}
	else if (rv < lowWeight + mediumWeight)
	{
		power = pwrMedium;
	}
	else if (rv < lowWeight + mediumWeight + highWeight)
	{
		power = pwrHigh;
	}
	else if (rv < lowWeight + mediumWeight + highWeight + apocWeight)
	{
		power = pwrApocalyptic;
	}
	// else should never happen, but it will default to low power anyways

	// Get a random number from 0 to (total disaster targeting type weights)
	disTarget target = trgRandom;
	rv = TethysGame::GetRand(randWeight + zoneWeight + plyrWeight + 1);
	if (rv < randWeight)
	{
		target = trgRandom;
	}
	else if (rv < randWeight + zoneWeight)
	{
		target = trgZone;
	}
	else if (rv < randWeight + zoneWeight + plyrWeight)
	{
		target = trgPlayer;
	}
	// else should never happen, but it will default to random targetting anyways

	// Get a random number from 0 to (total disaster weights)
	rv = TethysGame::GetRand(quakesWeight + stormsWeight + vortexWeight + meteorWeight + noneWeight + 1);
	if (rv < quakesWeight)
	{
		DoDisaster(disQuake, power, target);
	}
	else if (rv < quakesWeight + stormsWeight)
	{
		DoDisaster(disStorm, power, target);
	}
	else if (rv < quakesWeight + stormsWeight + vortexWeight)
	{
		DoDisaster(disVortex, power, target);
	}
	else if (rv < quakesWeight + stormsWeight + vortexWeight + meteorWeight)
	{
		DoDisaster(disMeteor, power, target);
	}
	// else nothing
}

void DisasterCreator::DoDisaster(disType type, disPower power, disTarget target)
{
	// Check if the minimum time between disasters has elapsed.
	if ((TethysGame::Tick() - lastTick < minWait) &&
		(TethysGame::GetRand(101) >= ignoreMinTimeChance))
	{
		return;
	}

	// Update last disaster time (even if we roll no disaster - consider it a reprieve!)
	lastTick = TethysGame::Tick();

	// Invoke the appropriate disaster.
	switch (type)
	{
		case disQuake:
			if (quakesEnabled)
			{
				DoQuake(power, target);
			}
			break;
		case disStorm:
			if (stormsEnabled)
			{
				DoStorm(power, target);
			}
			break;
		case disVortex:
			if (vortexEnabled)
			{
				DoVortex(power, target);
			}
			break;
		case disMeteor:
			if (meteorEnabled)
			{
				DoMeteor(power, target);
			}
			break;
		case disNone:
			// Lucky you!
			break;
		default:
			TethysGame::AddMessage(-1, -1, "DC Error: Unknown disaster type invoked!", -1, sndDirt);
			break;
	}
}

// ----------------------------------------------

void DisasterCreator::DoQuake(disPower power, disTarget target)
{
	// Translate the arbitrary power level into something that makes sense for this disaster
	int actualPower = 1;
	switch (power)
	{
		case pwrLow:
			// Low power quake: magnitude 1 or 2
			actualPower = TethysGame::GetRand(2) + 1;
			break;
		case pwrMedium:
			// Medium power quake: magnitude 1 - 4
			actualPower = TethysGame::GetRand(4) + 1;
			break;
		case pwrHigh:
			// High power quake: magnitude 3 - 7
			actualPower = TethysGame::GetRand(5) + 3;
			break;
		case pwrApocalyptic:
			// Apocalyptic power quake: magnitude 6 - 10
			actualPower = TethysGame::GetRand(5) + 6;
			break;
	}

	LOCATION targetLoc = GetDisasterTarget(target, disQuake);
	TethysGame::SetEarthquake(targetLoc.x, targetLoc.y, actualPower);
}

void DisasterCreator::DoStorm(disPower power, disTarget target)
{
	// Translate the arbitrary power level into something that makes sense for this disaster
	int actualPower = 1;
	switch (power)
	{
		case pwrLow:
			// Low power storm: power 3 - 8
			actualPower = TethysGame::GetRand(6) + 3;
			break;
		case pwrMedium:
			// Medium power storm: power 9 - 14
			actualPower = TethysGame::GetRand(6) + 9;
			break;
		case pwrHigh:
			// High power storm: power 17 - 27
			actualPower = TethysGame::GetRand(11) + 17;
			break;
		case pwrApocalyptic:
			// Apocalyptic power storm: power 30 - 60
			actualPower = TethysGame::GetRand(31) + 30;
			break;
	}

	LOCATION targetLoc = GetDisasterTarget(target, disStorm);
	LOCATION destination = GetStormDestination(targetLoc);
	TethysGame::SetLightning(targetLoc.x, targetLoc.y, actualPower, destination.x, destination.y);
}

void DisasterCreator::DoVortex(disPower power, disTarget target)
{
	// Translate the arbitrary power level into something that makes sense for this disaster
	int actualPower = 1;
	switch (power)
	{
		case pwrLow:
			// Low power vortex: duration 6 - 9
			actualPower = TethysGame::GetRand(4) + 6;
			break;
		case pwrMedium:
			// Medium power vortex: duration 7 - 12
			actualPower = TethysGame::GetRand(6) + 7;
			break;
		case pwrHigh:
			// High power vortex: duration 14 - 22
			actualPower = TethysGame::GetRand(9) + 14;
			break;
		case pwrApocalyptic:
			// Apocalyptic power vortex: duration 25 - 35
			actualPower = TethysGame::GetRand(11) + 25;
			break;
	}

	LOCATION targetLoc = GetDisasterTarget(target, disVortex);
	LOCATION destination = GetVortexDestination(targetLoc);
	TethysGame::SetTornado(targetLoc.x, targetLoc.y, actualPower, destination.x, destination.y, false);
}

void DisasterCreator::DoMeteor(disPower power, disTarget target)
{
	// Translate the arbitrary power level into something that makes sense for this disaster
	int actualPower = 1;
	switch (power)
	{
		case pwrLow:
			// Low power meteor: size 1
			actualPower = 1;
			break;
		case pwrMedium:
			// Medium power meteor: size 1 or 2;
			actualPower = TethysGame::GetRand(2) + 1;
			break;
		case pwrHigh:
			// High power meteor: size 2-4
			actualPower = TethysGame::GetRand(3) + 2;
			break;
		case pwrApocalyptic:
			// Apocalyptic power meteor: Screw it, size 10
			actualPower = 10;
			break;
	}

	LOCATION targetLoc = GetDisasterTarget(target, disMeteor);
	TethysGame::SetMeteor(targetLoc.x, targetLoc.y, actualPower);
}
