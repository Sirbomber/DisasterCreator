#include <Tethys/API/API.h>
#include "DisasterCreator.h"

using namespace Tethys;
using namespace Tethys::TethysAPI;

/*
  Code for the various methods a disaster finds its target.
*/

Location DisasterCreator::GetDisasterTarget(DisasterTarget trgType, DisasterType type)
{
	// Return value
	Location tLocation;

	switch (trgType)
	{
		case DisasterTarget::Random:
			tLocation = TargetRandomLocation();
			break;
		case DisasterTarget::Zone:
			tLocation = TargetLocationInZone(type);
			break;
		case DisasterTarget::Player:
			tLocation = TargetPlayer();
			break;
	}

	return tLocation;
}

Location DisasterCreator::TargetRandomLocation()
{
	// Just pick a random spot on the map.  Easy.
	int x, y;
	x = Game::GetRand(GameMap::GetWidth() + 1);	// +1 so disasters don't spawn at x = 0
	y = Game::GetRand(GameMap::GetHeight());
	return Location(x, y);
}

Location DisasterCreator::TargetLocationInZone(DisasterType type)
{
	// Return value.
	Location tLocation;

	// Iterate through the zone list for each zone defined for the current disaster type.  Pick one at random.
	int zoneScore = 0;
	DisasterZone *toUse = nullptr;
	for (int i = 0; i < numZonesDefined; i++)
	{
		if (AllZones[i].zoneType == type)
		{
			int tScore = Game::GetRand(100) + 1;	// +1 so it will always be > 0
			if (tScore > zoneScore)
			{
				zoneScore = tScore;
				toUse = &(AllZones[i]);
			}
		}
	}

	// Check if we actually found a zone; if not get a random Location
	if (toUse == nullptr)
	{
		tLocation = TargetRandomLocation();
	}
	else
	{
		// Get a random point inside this zone.
		tLocation = Location(Game::GetRand(toUse->zoneRect.Width()) + toUse->zoneRect.x1,
			                    Game::GetRand(toUse->zoneRect.Height()) + toUse->zoneRect.y1);
	}

	return tLocation;
}

Location DisasterCreator::TargetPlayer()
{
	// Return value.
	Location tLocation;

	// Pick a player at random to torment from the list of non-ignored players.
	int tPlayer = -1;

	// Randomly reorder the list (backwards)
	int next, start;
	start = next = Game::GetRand(7);
	do
	{
		if (!ignorePlayer[next])
		{
			tPlayer = next;
			break;
		}
		else
		{
			next++;
			next %= 7;
		}
	} while (next != start);

	// All players are marked as ignored.  That's not my problem!
	if (tPlayer == -1)
	{
		tPlayer = Game::GetRand(Game::NumPlayers());
	}

	// Flip a coin: heads, we target a random vehicle belonging to this player.
	// Tails, we find the middle of their base and target that.
	if (Game::GetRand(2) == 0)
	{
		// Random unit mode.  Give each unit a random score, 0-99.  The highest scoring unit "wins".
		Unit targUnit;
		int targScore = -1,
			tempScore;
		for (Unit tempUnit : PlayerVehicleEnum(tPlayer, MapID::Any))
		{
			if (!tempUnit.IsLive())
			{
				continue;
			}

			tempScore = Game::GetRand(100);
			if (tempScore > targScore)
			{
				targScore = tempScore;
				targUnit = tempUnit;
			}

			// If we get a really high score, save some CPU time and just exit early.
			// It's unlikely we'll find a unit with a higher score anyways.
			if (targScore > 90)
			{
				break;
			}
		}

		// If target score is still -1, it means we never ran the loop, and this player
		// has no units to target.
		if (targScore >= 0)
		{
			tLocation = TargetWithinRadius(targUnit.GetLocation());
		}
		//  Fall back to a purely random Location.
		else
		{
			tLocation = TargetRandomLocation();
		}
	}

	else
	{
		// Fancypants mode.  Find the "midpoint" Location for each of this player's structures.
		int x = 0, y = 0, total = 0;
		Unit tempUnit;
		for (Unit tempUnit : PlayerBuildingEnum(tPlayer, MapID::Any))
		{
			if (!tempUnit.IsLive() ||
				!tempUnit.IsBuilding())
			{
				continue;
			}

			total++;
			x += tempUnit.GetLocation().x;
			y += tempUnit.GetLocation().y;
		}

		// Get base midpoint (unless no buildings found)
		if (total > 0)
		{
			tLocation = TargetWithinRadius(Location(x / total, y / total));
		}
		// Fall back to a purely random Location.
		else
		{
			tLocation = TargetRandomLocation();
		}
	}

	return tLocation;
}

Location DisasterCreator::GetVortexDestination(Location source)
{
	// Vortexes should rampage through player bases. >:D
	// To that end, the destination should be within a (relatively) small radius from the source.
	double t = ((double)Game::GetRand(101) / 100.00) * 3.14 * 2;
	Location retLoc = Location((int)(cos(t) * 9), (int)(sin(t) * 9));	// Radius is 9
	retLoc.Clip();

	return retLoc;
}

Location DisasterCreator::GetStormDestination(Location source)
{
	// Storms should move towards the edges of the map.
	double t = ((double)Game::GetRand(101) / 100.00) * 3.14 * 2;
	Location retLoc = Location((int)(cos(t) * 150), (int)(sin(t) * 150));	// Radius is 150, which is actually bigger than the map, but good thing for .Clip()
	retLoc.Clip();

	// Magic time: if we create a storm too close to the source, try again.
	if (ApproxDistance(source, retLoc) < 30)
	{
		retLoc = GetStormDestination(source);
	}

	return retLoc;
}

Location DisasterCreator::TargetWithinRadius(Location target)
{
	double r = 30 * sqrt(((double)Game::GetRand(101) / 100.00));
	double t = ((double)Game::GetRand(101) / 100) * 3.14 * 2;

	Location retLoc = Location((int)(target.x + r * cos(t)), (int)(target.y + r * sin(t)));
	retLoc.Clip();
	return retLoc;
}

// Derived from http://www.flipcode.com/archives/Fast_Approximate_Distance_Functions.shtml
int DisasterCreator::ApproxDistance(Location s, Location d)
{
	int min,
		max,
		dX,
		dY,
		approxDist;

	dX = s.x - d.x;
	if (dX < 0) { dX *= -1; }
	dY = s.y - d.y;
	if (dY < 0) { dY *= -1; }


	if (dX < dY)
	{
		min = dX;
		max = dY;
	}
	else
	{
		min = dY;
		max = dX;
	}

	// Tweak these magic numbers later for better accuracy?
	approxDist = (max * 1007) + (min * 441);
	if (max < (min << 4))
	{
		approxDist -= (max * 40);
	}

	// Add 512 for rounding and then divide by 1024
	return ((approxDist + 512) >> 10);
}
