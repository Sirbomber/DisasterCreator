#include <Tethys/API/API.h>
#include "DisasterCreator.h"

using namespace Tethys;
using namespace Tethys::TethysAPI;

/*
  Anything that allows level creators to define or tweak Disaster Creator parameters goes here.
*/

void DisasterCreator::SetMinimumWait(int wait)
{
	minWait = wait;
}

void DisasterCreator::EnableDisaster(DisasterType type)
{
	switch (type)
	{
		case DisasterType::Quake:
			quakesEnabled = true;
			break;
		case DisasterType::Storm:
			stormsEnabled = true;
			break;
		case DisasterType::Vortex:
			vortexEnabled = true;
			break;
		case DisasterType::Meteor:
			meteorEnabled = true;
			break;
		case DisasterType::None:
			break;
		default:
			Game::AddMessage("DC Error: Tried to enable unknown disaster type!", Tethys::SoundID::Dirt);
			break;
	}
}

void DisasterCreator::DisableDisaster(DisasterType type)
{
	switch (type)
	{
	case DisasterType::Quake:
		quakesEnabled = false;
		quakesWeight = 0;
		break;
	case DisasterType::Storm:
		stormsEnabled = false;
		stormsWeight = 0;
		break;
	case DisasterType::Vortex:
		vortexEnabled = false;
		vortexWeight = 0;
		break;
	case DisasterType::Meteor:
		meteorEnabled = false;
		meteorWeight = 0;
		break;
	case DisasterType::None:
		break;
	default:
		Game::AddMessage("DC Error: Tried to disable unknown disaster type!", Tethys::SoundID::Dirt);
		break;
	}
}
void DisasterCreator::SetDisasterTypeWeight(DisasterType type, int weight)
{
	switch (type)
	{
		case DisasterType::Quake:
			quakesWeight = weight;
			break;
		case DisasterType::Storm:
			stormsWeight = weight;
			break;
		case DisasterType::Vortex:
			vortexWeight = weight;
			break;
		case DisasterType::Meteor:
			meteorWeight = weight;
			break;
		case DisasterType::None:
			noneWeight = weight;
			break;
		default:
			Game::AddMessage("DC Error: Tried to set weight for unknown disaster type!", Tethys::SoundID::Dirt);
			break;
	}
}

void DisasterCreator::SetDisasterPowerWeight(DisasterPower power, int weight)
{
	switch (power)
	{
		case DisasterPower::Low:
			lowWeight = weight;
			break;
		case DisasterPower::Medium:
			mediumWeight = weight;
			break;
		case DisasterPower::High:
			highWeight = weight;
			break;
		case DisasterPower::Apocalyptic:
			apocWeight = weight;
			break;
		default:
			Game::AddMessage("DC Error: Tried to set weight for unknown power type!", Tethys::SoundID::Dirt);
			break;
	}
}

void DisasterCreator::SetDisasterTargetWeight(DisasterTarget target, int weight)
{
	switch (target)
	{
	case DisasterTarget::Random:
		randWeight = weight;
		break;
	case DisasterTarget::Zone:
		zoneWeight = weight;
		break;
	case DisasterTarget::Player:
		plyrWeight = weight;
		break;
	default:
		Game::AddMessage("DC Error: Tried to set weight for unknown target type!", Tethys::SoundID::Dirt);
		break;
	}
}

void DisasterCreator::SetIgnoreChance(int weight)
{
	// Chance to ignore the minimum disaster wait time must be between 0-100 inclusive
	if (weight < 0 || weight > 100)
	{
		Game::AddMessage("DC Error: SetIgnoreChance out of bounds!", Tethys::SoundID::Dirt);
	}
	else
	{
		ignoreMinTimeChance = weight;
	}
}

void DisasterCreator::SetIgnorePlayer(int player, bool ignore)
{
	// Make sure the passed-in player ID is valid.
	if (player < 0 || player > 7)
	{
		Game::AddMessage("DC Error: Ignore player ID out of bounds!", Tethys::SoundID::Dirt);
	}
	else
	{
		ignorePlayer[player] = ignore;
	}
}

void DisasterCreator::AddDisasterZone(DisasterType type, MapRect zone)
{
	// Limit check
	if (numZonesDefined >= MAX_SIZE)
	{
		Game::AddMessage("DC Error: Zone limit reached!", Tethys::SoundID::Dirt);
		return;
	}

	// Sanity check
	if (type < DisasterType::Quake || type > DisasterType::None)
	{
		Game::AddMessage("DC Error: Tried to define zone for unknown disaster type!", Tethys::SoundID::Dirt);
		return;
	}

	// Don't bother with no disaster.
	if (type == DisasterType::None)
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
	// Destroy any existing callback trigger
	if (disCheck.GetID() != 255)
	{
		disCheck.Destroy();
	}

	return disCheck = CreateTimeTrigger(minDelay, maxDelay, callback, false, true);
}

Trigger DisasterCreator::SetCallbackTrigger(char const *callback, int delay)
{
	// Destroy any existing callback trigger
	if (disCheck.GetID() != 255)
	{
		disCheck.Destroy();
	}

	return disCheck = CreateTimeTrigger(delay, callback, false, true);
}

/*
  Sample callback function.
  Export void DisasterCreator_Callback()
  {
	DC.RollRandom();
  }
*/
