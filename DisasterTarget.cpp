#include "DisasterCreator.h"

/*
  Code for the various methods a disaster finds its target.
*/

LOCATION DisasterCreator::GetDisasterTarget(disTarget trgType, disType type)
{
	// Return value
	LOCATION tLocation;

	switch (trgType)
	{
		case trgRandom:
			tLocation = TargetRandomLocation();
			break;
		case trgZone:
			tLocation = TargetLocationInZone(type);
			break;
		case trgPlayer:
			tLocation = TargetPlayer();
			break;
	}

	return tLocation;
}

LOCATION DisasterCreator::TargetRandomLocation()
{
	// Just pick a random spot on the map.  Easy.
	LOCATION tLocation;
	tLocation.x = TethysGame::GetRand(mapSize.x) + mapXOffset + 1;	// +1 so disasters don't spawn at x = 0
	tLocation.y = TethysGame::GetRand(mapSize.y) - 1;
	return tLocation;
}

LOCATION DisasterCreator::TargetLocationInZone(disType type)
{
	// Return value.
	LOCATION tLocation;

	// Iterate through the zone list for each zone defined for the current disaster type.  Pick one at random.
	int zoneScore = 0;
	DisasterZone *toUse = nullptr;
	list<DisasterZone>::iterator znIt = AllZones.begin(),
					            znEnd = AllZones.end();
	for (znIt; znIt != znEnd; znIt++)
	{
		if (znIt->zoneType == type)
		{
			int tScore = TethysGame::GetRand(100) + 1;	// +1 so it will always be > 0
			if (tScore > zoneScore)
			{
				zoneScore = tScore;
				toUse = &(*znIt);
			}
		}
	}

	// Check if we actually found a zone; if not get a random location
	if (toUse == nullptr)
	{
		tLocation = TargetRandomLocation();
	}
	else
	{
		// Get a random point inside this zone.
		tLocation = LOCATION(TethysGame::GetRand(toUse->zoneRect.Width()) + toUse->zoneRect.x1,
			                 TethysGame::GetRand(toUse->zoneRect.Height()) + toUse->zoneRect.y1);
	}

	return tLocation;
}

LOCATION DisasterCreator::TargetPlayer()
{
	// Return value.
	LOCATION tLocation;

	// Pick a player at random to torment.
	int tPlayer = TethysGame::GetRand(numPlayers);

	// Flip a coin: heads, we target a random vehicle belonging to this player.
	// Tails, we find the middle of their base and target that.
	if (TethysGame::GetRand(2) == 0)
	{
		// Random unit mode.  Give each unit a random score, 0-99.  The highest scoring unit "wins".
		Unit targUnit, tempUnit;
		int targScore = -1,
			tempScore;
		PlayerVehicleEnum FindTarget(tPlayer);
		while (FindTarget.GetNext(tempUnit))
		{
			if (!tempUnit.IsLive())
			{
				continue;
			}

			tempScore = TethysGame::GetRand(100);
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
			tLocation = TargetWithinRadius(targUnit.Location());
		}
		//  Fall back to a purely random location.
		else
		{
			tLocation = TargetRandomLocation();
		}

	}

	else
	{
		// Fancypants mode.  Find the "midpoint" location for each of this player's structures.
		int x = 0, y = 0, total = 0;
		Unit tempUnit;
		PlayerBuildingEnum FindTarget(tPlayer, mapNone);
		while (FindTarget.GetNext(tempUnit))
		{
			if (!tempUnit.IsLive() ||
				!tempUnit.IsBuilding())
			{
				continue;
			}

			total++;
			x += tempUnit.Location().x;
			y += tempUnit.Location().y;
		}

		// Get base midpoint (unless no buildings found)
		if (total > 0)
		{
			tLocation = TargetWithinRadius(LOCATION(x / total, y / total));
		}
		// Fall back to a purely random location.
		else
		{
			tLocation = TargetRandomLocation();
		}
	}

	return tLocation;
}

LOCATION DisasterCreator::GetVortexDestination(LOCATION source)
{
	// Vortexes should rampage through player bases. >:D
	// To that end, the destination should be within a (relatively) small radius from the source.
	double t = ((double)TethysGame::GetRand(101) / 100.00) * 3.14 * 2;
	LOCATION retLoc = LOCATION((int)(cos(t) * 9), (int)(sin(t) * 9));	// Radius is 9
	retLoc.Clip();

	return retLoc;
}

LOCATION DisasterCreator::GetStormDestination(LOCATION source)
{
	// Storms should move towards the edges of the map.
	double t = ((double)TethysGame::GetRand(101) / 100.00) * 3.14 * 2;
	LOCATION retLoc = LOCATION((int)(cos(t) * 150), (int)(sin(t) * 150));	// Radius is 150, which is actually bigger than the map, but good thing for .Clip()
	retLoc.Clip();

	// Magic time: if we create a storm too close to the source, try again.
	if (ApproxDistance(source, retLoc) < 30)
	{
		retLoc = GetStormDestination(source);
	}

	return retLoc;
}

LOCATION DisasterCreator::TargetWithinRadius(LOCATION target)
{
	double r = 30 * sqrt(((double)TethysGame::GetRand(101) / 100.00));
	double t = ((double)TethysGame::GetRand(101) / 100) * 3.14 * 2;

	LOCATION retLoc = LOCATION((int)(target.x + r * cos(t)), (int)(target.y + r * sin(t)));
	retLoc.Clip();
	return retLoc;
}

// Derived from http://www.flipcode.com/archives/Fast_Approximate_Distance_Functions.shtml
int DisasterCreator::ApproxDistance(LOCATION s, LOCATION d)
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
