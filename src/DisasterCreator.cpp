#include "DisasterCreator.h"

using namespace Tethys;
using namespace Tethys::TethysAPI;

/*
   Constructor/Destructor + basic disaster creation functions.
*/

DisasterCreator::DisasterCreator()
{
	lastTick = Game::Tick();
	minWait = 0;

	quakesEnabled = false;
	stormsEnabled = false;
	vortexEnabled = false;
	meteorEnabled = false;

	quakesWeight = 0;
	stormsWeight = 0;
	vortexWeight = 0;
	meteorWeight = 0;
	noneWeight = 0;

	lowWeight = 0;
	mediumWeight = 0;
	highWeight = 0;
	apocWeight = 0;

	randWeight = 0;
	zoneWeight = 0;
	plyrWeight = 0;

	ignoreMinTimeChance = 0;

	numZonesDefined = 0;
	numVolcanoesDefined = 0;
	lastLavaSpeed = 0;

	for (int i = 0; i < 7; i++)
	{
		if (Player[i].IsHuman())
		{
			ignorePlayer[i] = false;
		}
	}

	numQueuedDisastersDefined = 0;
}

DisasterCreator::~DisasterCreator()
{
	if (disCheck.GetID() != 255)
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
		Game::AddMessage("DC Error: No weights defined!", Tethys::SoundID::Dirt);
		return;
	}

	// Get a random number from 0 to (total power weights)
	DisasterPower power = DisasterPower::Low;
	int rv = Game::GetRand(lowWeight + mediumWeight + highWeight + apocWeight + 1);
	if (rv < lowWeight)
	{
		power = DisasterPower::Low;
	}
	else if (rv < lowWeight + mediumWeight)
	{
		power = DisasterPower::Medium;
	}
	else if (rv < lowWeight + mediumWeight + highWeight)
	{
		power = DisasterPower::High;
	}
	else if (rv < lowWeight + mediumWeight + highWeight + apocWeight)
	{
		power = DisasterPower::Apocalyptic;
	}
	// else should never happen, but it will default to low power anyways

	// Get a random number from 0 to (total disaster targeting type weights)
	DisasterTarget target = DisasterTarget::Random;
	rv = Game::GetRand(randWeight + zoneWeight + plyrWeight + 1);
	if (rv < randWeight)
	{
		target = DisasterTarget::Random;
	}
	else if (rv < randWeight + zoneWeight)
	{
		target = DisasterTarget::Zone;
	}
	else if (rv < randWeight + zoneWeight + plyrWeight)
	{
		target = DisasterTarget::Player;
	}
	// else should never happen, but it will default to random targeting anyways

	// Get a random number from 0 to (total disaster weights)
	rv = Game::GetRand(quakesWeight + stormsWeight + vortexWeight + meteorWeight + noneWeight + 1);
	if (rv < quakesWeight)
	{
		DoDisaster(DisasterType::Quake, power, target);
	}
	else if (rv < quakesWeight + stormsWeight)
	{
		DoDisaster(DisasterType::Storm, power, target);
	}
	else if (rv < quakesWeight + stormsWeight + vortexWeight)
	{
		DoDisaster(DisasterType::Vortex, power, target);
	}
	else if (rv < quakesWeight + stormsWeight + vortexWeight + meteorWeight)
	{
		DoDisaster(DisasterType::Meteor, power, target);
	}
	// else nothing
}

void DisasterCreator::DoDisaster(DisasterType type, DisasterPower power, DisasterTarget target, bool force, bool instant)
{
	// Check if the minimum time between disasters has elapsed.
	if (!force &&
		(Game::Tick() - lastTick < minWait) &&
		(Game::GetRand(101) >= ignoreMinTimeChance))
	{
		return;
	}

	// Update last disaster time (even if we roll no disaster - consider it a reprieve!)
	lastTick = Game::Tick();

	// Invoke the appropriate disaster.
	switch (type)
	{
		case DisasterType::Quake:
			if (quakesEnabled)
			{
				DoQuake(power, target, instant);
			}
			break;
		case DisasterType::Storm:
			if (stormsEnabled)
			{
				DoStorm(power, target, instant);
			}
			break;
		case DisasterType::Vortex:
			if (vortexEnabled)
			{
				DoVortex(power, target, instant);
			}
			break;
		case DisasterType::Meteor:
			if (meteorEnabled)
			{
				DoMeteor(power, target, instant);
			}
			break;
		case DisasterType::None:
			// Lucky you!
			break;
		default:
			Game::AddMessage("DC Error: Unknown disaster type invoked!", Tethys::SoundID::Dirt);
			break;
	}
}

void DisasterCreator::DoDisasterAt(DisasterType type, DisasterPower power, Tethys::Location target, Tethys::Location stormVortexEndPt, bool force, bool instant)
{
	// Check if the minimum time between disasters has elapsed.
	if (!force &&
		(Game::Tick() - lastTick < minWait) &&
		(Game::GetRand(101) >= ignoreMinTimeChance))
	{
		return;
	}

	// Update last disaster time (even if we roll no disaster - consider it a reprieve!)
	lastTick = Game::Tick();

	// Invoke the appropriate disaster.
	switch (type)
	{
	case DisasterType::Quake:
		if (quakesEnabled || force)
		{
			DoQuake(power, DisasterTarget::Location, instant, target);
		}
		break;
	case DisasterType::Storm:
		if (stormsEnabled || force)
		{
			DoStorm(power, DisasterTarget::Location, instant, target, stormVortexEndPt);
		}
		break;
	case DisasterType::Vortex:
		if (vortexEnabled || force)
		{
			DoVortex(power, DisasterTarget::Location, instant, target, stormVortexEndPt);
		}
		break;
	case DisasterType::Meteor:
		if (meteorEnabled || force)
		{
			DoMeteor(power, DisasterTarget::Location, instant, target);
		}
		break;
	case DisasterType::None:
		// Lucky you!
		break;
	default:
		Game::AddMessage("DC Error: Unknown disaster type invoked!", Tethys::SoundID::Dirt);
		break;
	}
}

void DisasterCreator::QueueDisaster(DisasterType type, DisasterPower power, DisasterTarget target, Tethys::Location at, Tethys::Location stormVortexEndPt, int tick, bool instant)
{
	// Limit check.
	if (numQueuedDisastersDefined >= MAX_SIZE)
	{
		Game::AddMessage("DC Error: Too many queued disasters!", SoundID::Dirt);
		return;
	}

	// Silently discard a queued type of none
	if (type == DisasterType::None)
	{
		return;
	}

	AllQueuedDisasters[numQueuedDisastersDefined].type = type;
	AllQueuedDisasters[numQueuedDisastersDefined].power = power;
	AllQueuedDisasters[numQueuedDisastersDefined].target = target;
	AllQueuedDisasters[numQueuedDisastersDefined].startAt = at;
	AllQueuedDisasters[numQueuedDisastersDefined].endAt = stormVortexEndPt;
	AllQueuedDisasters[numQueuedDisastersDefined].triggerTime = tick;
	AllQueuedDisasters[numQueuedDisastersDefined].instant = instant;

	// Increment queue count
	numQueuedDisastersDefined++;
}

void DisasterCreator::CheckQueuedDisasters()
{
	// Iterate through the list of queued disasters.
	QueuedDisaster* q;
	int i = 0;
	while (i < numQueuedDisastersDefined)
	{
		// Check if the trigger time has passed.
		if (Game::Tick() >= AllQueuedDisasters[i].triggerTime)
		{
			// Trigger the disaster
			q = &AllQueuedDisasters[i];
			if (q->target == DisasterTarget::Location)
			{
				DoDisasterAt(q->type, q->power, q->startAt, q->endAt, true, q->instant);
			}
			else
			{
				DoDisaster(q->type, q->power, q->target, true, q->instant);
			}

			// Erase this disaster from the list
			EraseQueuedDisaster(i);
		}
		else
		{
			i++;
		}
	}
}

void DisasterCreator::EraseQueuedDisaster(int i)
{
	// Push every volcano further down the array up one slot.
	for (i; i < numQueuedDisastersDefined - 1; i++)
	{
		AllQueuedDisasters[i] = AllQueuedDisasters[i + 1];
	}

	numQueuedDisastersDefined--;
}

// ----------------------------------------------

void DisasterCreator::DoQuake(DisasterPower power, DisasterTarget target, bool instant, Tethys::Location at)
{
	// Translate the arbitrary power level into something that makes sense for this disaster
	int actualPower = 1;
	switch (power)
	{
		case DisasterPower::Low:
			// Low power quake: magnitude 1 or 2
			actualPower = Game::GetRand(2) + 1;
			break;
		case DisasterPower::Medium:
			// Medium power quake: magnitude 1 - 4
			actualPower = Game::GetRand(4) + 1;
			break;
		case DisasterPower::High:
			// High power quake: magnitude 3 - 7
			actualPower = Game::GetRand(5) + 3;
			break;
		case DisasterPower::Apocalyptic:
			// Apocalyptic power quake: magnitude 6 - 10
			actualPower = Game::GetRand(5) + 6;
			break;
	}
	
	Location targetLoc;
	if (target == DisasterTarget::Location && at.x != -1 && at.y != -1)
	{
		targetLoc = at;
	}
	else
	{
		targetLoc = GetDisasterTarget(target, DisasterType::Quake);
	}
	Game::CreateEarthquake(targetLoc, actualPower, instant);
}

void DisasterCreator::DoStorm(DisasterPower power, DisasterTarget target, bool instant, Tethys::Location start, Tethys::Location end)
{
	// Translate the arbitrary power level into something that makes sense for this disaster
	int actualPower = 1;
	switch (power)
	{
		case DisasterPower::Low:
			// Low power storm: power 3 - 8
			actualPower = Game::GetRand(6) + 3;
			break;
		case DisasterPower::Medium:
			// Medium power storm: power 9 - 14
			actualPower = Game::GetRand(6) + 9;
			break;
		case DisasterPower::High:
			// High power storm: power 17 - 27
			actualPower = Game::GetRand(11) + 17;
			break;
		case DisasterPower::Apocalyptic:
			// Apocalyptic power storm: power 30 - 60
			actualPower = Game::GetRand(31) + 30;
			break;
	}

	Location targetLoc;
	Location destination;
	if (target == DisasterTarget::Location && start.x != -1 && start.y != -1 && end.x != -1 && end.y != -1)
	{
		targetLoc = start;
		destination = end;
	}
	else
	{
		targetLoc = GetDisasterTarget(target, DisasterType::Storm);
		destination = GetStormDestination(targetLoc);
	}

	Game::CreateLightning(targetLoc, destination, actualPower, instant);
}

void DisasterCreator::DoVortex(DisasterPower power, DisasterTarget target, bool instant, Tethys::Location start, Tethys::Location end)
{
	// Translate the arbitrary power level into something that makes sense for this disaster
	int actualPower = 1;
	switch (power)
	{
		case DisasterPower::Low:
			// Low power vortex: duration 6 - 9
			actualPower = Game::GetRand(4) + 6;
			break;
		case DisasterPower::Medium:
			// Medium power vortex: duration 7 - 12
			actualPower = Game::GetRand(6) + 7;
			break;
		case DisasterPower::High:
			// High power vortex: duration 14 - 22
			actualPower = Game::GetRand(9) + 14;
			break;
		case DisasterPower::Apocalyptic:
			// Apocalyptic power vortex: duration 25 - 35
			actualPower = Game::GetRand(11) + 25;
			break;
	}

	Location targetLoc;
	Location destination;
	if (target == DisasterTarget::Location && start.x != -1 && start.y != -1 && end.x != -1 && end.y != -1)
	{
		targetLoc = start;
		destination = end;
	}
	else
	{
		targetLoc = GetDisasterTarget(target, DisasterType::Vortex);
		destination = GetVortexDestination(targetLoc);
	}
	Game::CreateTornado(targetLoc, destination, actualPower, instant);
}

void DisasterCreator::DoMeteor(DisasterPower power, DisasterTarget target, bool instant, Tethys::Location at)
{
	// Translate the arbitrary power level into something that makes sense for this disaster
	int actualPower = 1;
	switch (power)
	{
		case DisasterPower::Low:
			// Low power meteor: size 1
			actualPower = 1;
			break;
		case DisasterPower::Medium:
			// Medium power meteor: size 1 or 2;
			actualPower = Game::GetRand(2) + 1;
			break;
		case DisasterPower::High:
			// High power meteor: size 2-4
			actualPower = Game::GetRand(3) + 2;
			break;
		case DisasterPower::Apocalyptic:
			// Apocalyptic power meteor: Screw it, size 10
			actualPower = 10;
			break;
	}

	Location targetLoc;
	if (target == DisasterTarget::Location && at.x != -1 && at.y != -1)
	{
		targetLoc = at;
	}
	else
	{
		targetLoc = GetDisasterTarget(target, DisasterType::Meteor);
	}
	Game::CreateMeteor(targetLoc, (MeteorSize)actualPower, instant);
}

void DisasterCreator::SetBlight(Location spawnAt, BlightShape shape, int size, BlightLavaSpeed speed, bool infect, bool noWarning)
{
	// Sanity checks.
	if (size < 1)
	{
		Game::AddMessage("DC Error: Invalid Blight spawn size!", Tethys::SoundID::Dirt);
	}
	else if (speed < BlightLavaSpeed::Stopped || speed > BlightLavaSpeed::Instant)
	{
		Game::AddMessage("DC Error: Invalid Blight spread speed!", Tethys::SoundID::Dirt);
	}

	else
	{
		int startX, startY;
		switch (shape)
		{
		case BlightShape::Square:
			startX = spawnAt.x - (size / 2);
			startY = spawnAt.y - (size / 2);
			for (int x = startX; x <= spawnAt.x + ((size / 2) - (size % 2)); x++)
			{
				for (int y = startY; y <= spawnAt.y + ((size / 2) - (size % 2)); y++)
				{
					Game::CreateBlight(Location(x, y));
				}
			}
			break;
		case BlightShape::Diamond:
			for (int y = spawnAt.y - size;
				y <= spawnAt.y + size;
				y++)
			{
				for (int x = spawnAt.x - (size - abs(y - spawnAt.y));
					x <= spawnAt.x + (size - abs(y - spawnAt.y));
					x++)
				{
					Game::CreateBlight(Location(x, y));
				}
			}
			break;
		case BlightShape::TriangleUp:
			for (int y = spawnAt.y - size;
				y <= spawnAt.y;
				y++)
			{
				for (int x = spawnAt.x - (size - abs(y - spawnAt.y));
					x <= spawnAt.x + (size - abs(y - spawnAt.y));
					x++)
				{
					Game::CreateBlight(Location(x, y));
				}
			}
			break;
		case BlightShape::TriangleDown:
			for (int y = spawnAt.y;
				y <= spawnAt.y + size;
				y++)
			{
				for (int x = spawnAt.x - (size - abs(y - spawnAt.y));
					x <= spawnAt.x + (size - abs(y - spawnAt.y));
					x++)
				{
					Game::CreateBlight(Location(x, y));
				}
			}
			break;
		default:
			Game::AddMessage("DC Error: Invalid Blight spawn shape!", Tethys::SoundID::Dirt);
			break;
		}

		switch (speed)
		{
		case BlightLavaSpeed::Stopped:
			Game::SetBlightSpeed(0);
			break;
		case BlightLavaSpeed::VerySlow:
			Game::SetBlightSpeed(10);
			break;
		case BlightLavaSpeed::Slower:
			Game::SetBlightSpeed(24);
			break;
		case BlightLavaSpeed::Slow:
			Game::SetBlightSpeed(41);
			break;
		case BlightLavaSpeed::MediumSlow:
			Game::SetBlightSpeed(63);
			break;
		case BlightLavaSpeed::Medium:
			Game::SetBlightSpeed(81);
			break;
		case BlightLavaSpeed::MediumFast:
			Game::SetBlightSpeed(99);
			break;
		case BlightLavaSpeed::Fast:
			Game::SetBlightSpeed(132);
			break;
		case BlightLavaSpeed::Faster:
			Game::SetBlightSpeed(157);
			break;
		case BlightLavaSpeed::VeryFast:
			Game::SetBlightSpeed(180);
			break;
		case BlightLavaSpeed::Fastest:
			Game::SetBlightSpeed(210);
			break;
		case BlightLavaSpeed::Instant:
			Game::SetBlightSpeed(2000);
			break;
		case BlightLavaSpeed::NoChange:
		default:
			break;
		}
	}

	if (infect && !noWarning)
	{
		Game::AddMessage("Microbe growth detected!", SoundID::Savnt278, -1, Location(spawnAt.x, spawnAt.y));
	}
}

void DisasterCreator::SetBlightSpeed(BlightLavaSpeed speed)
{
	if (speed < BlightLavaSpeed::Stopped || speed > BlightLavaSpeed::Instant)
	{
		Game::AddMessage("DC Error: Invalid Blight spread speed!", Tethys::SoundID::Dirt);
	}
	else
	{
		Game::SetBlightSpeed(GetSpreadSpeed(speed));
	}
}

void DisasterCreator::SetBlightPreciseSpeed(int newSpeed)
{
	if (newSpeed < 0)
	{
		Game::AddMessage("DC Error: Precise speed must be non-negative!", Tethys::SoundID::Dirt);
	}
	else
	{
		Game::SetBlightSpeed(newSpeed);
	}
}
