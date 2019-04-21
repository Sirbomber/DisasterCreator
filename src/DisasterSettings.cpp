#include "DisasterCreator.h"

/*
  Anything that allows level creators to define or tweak Disaster Creator parameters goes here.
*/

void DisasterCreator::SetMapSize(int width, int height)
{
	mapSize = LOCATION(width, height);
	if (width == 512)
	{
		mapXOffset = -1;
	}
	else
	{
		mapXOffset = +31;
	}
}

void DisasterCreator::SetMinimumWait(int wait)
{
	minWait = wait;
}

void DisasterCreator::SetNumPlayers(int numPl)
{
	// Sanity check
	if (numPl > TethysGame::NoPlayers())
	{
		TethysGame::AddMessage(-1, -1, "DC Error: SetNumPlayers cannot be higher than TethysGame::NoPlayers()!", -1, sndDirt);
		return;
	}
	numPlayers = numPl;
}

void DisasterCreator::EnableDisaster(disType type)
{
	switch (type)
	{
		case disQuake:
			quakesEnabled = true;
			break;
		case disStorm:
			stormsEnabled = true;
			break;
		case disVortex:
			vortexEnabled = true;
			break;
		case disMeteor:
			meteorEnabled = true;
			break;
		case disNone:
			break;
		default:
			TethysGame::AddMessage(-1, -1, "DC Error: Tried to enable unknown disaster type!", -1, sndDirt);
			break;
	}
}

void DisasterCreator::DisableDisaster(disType type)
{
	switch (type)
	{
	case disQuake:
		quakesEnabled = false;
		quakesWeight = 0;
		break;
	case disStorm:
		stormsEnabled = false;
		stormsWeight = 0;
		break;
	case disVortex:
		vortexEnabled = false;
		vortexWeight = 0;
		break;
	case disMeteor:
		meteorEnabled = false;
		meteorWeight = 0;
		break;
	case disNone:
		break;
	default:
		TethysGame::AddMessage(-1, -1, "DC Error: Tried to disable unknown disaster type!", -1, sndDirt);
		break;
	}
}
void DisasterCreator::SetDisasterTypeWeight(disType type, int weight)
{
	switch (type)
	{
		case disQuake:
			quakesWeight = weight;
			break;
		case disStorm:
			stormsWeight = weight;
			break;
		case disVortex:
			vortexWeight = weight;
			break;
		case disMeteor:
			meteorWeight = weight;
			break;
		case disNone:
			noneWeight = weight;
			break;
		default:
			TethysGame::AddMessage(-1, -1, "DC Error: Tried to set weight for unknown disaster type!", -1, sndDirt);
			break;
	}
}

void DisasterCreator::SetDisasterPowerWeight(disPower power, int weight)
{
	switch (power)
	{
		case pwrLow:
			lowWeight = weight;
			break;
		case pwrMedium:
			mediumWeight = weight;
			break;
		case pwrHigh:
			highWeight = weight;
			break;
		case pwrApocalyptic:
			apocWeight = weight;
			break;
		default:
			TethysGame::AddMessage(-1, -1, "DC Error: Tried to set weight for unknown power type!", -1, sndDirt);
			break;
	}
}

void DisasterCreator::SetDisasterTargetWeight(disTarget target, int weight)
{
	switch (target)
	{
	case trgRandom:
		randWeight = weight;
		break;
	case trgZone:
		zoneWeight = weight;
		break;
	case trgPlayer:
		plyrWeight = weight;
		break;
	default:
		TethysGame::AddMessage(-1, -1, "DC Error: Tried to set weight for unknown target type!", -1, sndDirt);
		break;
	}
}

void DisasterCreator::SetIgnoreChance(int weight)
{
	// Chance to ignore the minimum disaster wait time must be between 0-100 inclusive
	if (weight < 0 || weight > 100)
	{
		TethysGame::AddMessage(-1, -1, "DC Error: SetIgnoreChance out of bounds!", -1, sndDirt);
	}
	else
	{
		ignoreMinTimeChance = weight;
	}
}

void DisasterCreator::AddDisasterZone(disType type, MAP_RECT zone)
{
	// Limit check
	if (numZonesDefined >= MAX_SIZE)
	{
		TethysGame::AddMessage(-1, -1, "DC Error: Zone limit reached!", -1, sndDirt);
		return;
	}

	// Sanity check
	if (type < disQuake || type > disNone)
	{
		TethysGame::AddMessage(-1, -1, "DC Error: Tried to define zone for unknown disaster type!", -1, sndDirt);
		return;
	}

	// Don't bother with no disaster.
	if (type == disNone)
	{
		return;
	}

	// Add the zone to our list.
	AllZones[numZonesDefined].zoneType = type;
	AllZones[numZonesDefined].zoneRect = zone;
	numZonesDefined++;
}

Trigger DisasterCreator::SetCallbackTrigger(char const *callback, int minDelay, int maxDelay)
{
	if (disCheck.IsInitialized())
	{
		disCheck.Destroy();
	}

	return disCheck = CreateTimeTrigger(1, 0, minDelay, maxDelay, callback);
}

Trigger DisasterCreator::SetCallbackTrigger(char const *callback, int delay)
{
	if (disCheck.IsInitialized())
	{
		disCheck.Destroy();
	}

	return disCheck = CreateTimeTrigger(1, 0, delay, callback);
}

/*
  Sample callback function.
  Export void DisasterCreator_Callback()
  {
	DC.RollRandom();
  }
*/
