#include <Tethys/API/API.h>
#include "DisasterCreator.h"

using namespace Tethys;
using namespace Tethys::TethysAPI;

void DisasterCreator::DefineVolcano(Location volcLoc, int lavaAnimTime, int eruptTime, VolcanoDirection dir, BlightLavaSpeed speed)
{
	// Limit check.
	if (numVolcanoesDefined >= MAX_SIZE)
	{
		Game::AddMessage("DC Error: Too many volcanoes!", SoundID::Dirt);
		return;
	}

	// Sanity checks.
	if (lavaAnimTime > eruptTime ||
		dir < VolcanoDirection::South || dir > VolcanoDirection::None ||
		speed < BlightLavaSpeed::Stopped || speed > BlightLavaSpeed::Instant)
	{
		Game::AddMessage("DC Error: Bad volcano definition!", SoundID::Dirt);
		return;
	}

	// Record new volcano data to list
	AllVolcanoes[numVolcanoesDefined].animStarted = false;
	AllVolcanoes[numVolcanoesDefined].eruptionSet = false;
	AllVolcanoes[numVolcanoesDefined].eruptAt = volcLoc;
	AllVolcanoes[numVolcanoesDefined].warnTime = lavaAnimTime;
	AllVolcanoes[numVolcanoesDefined].eruptTime = eruptTime;
	AllVolcanoes[numVolcanoesDefined].direction = dir;
	AllVolcanoes[numVolcanoesDefined].speed = speed;

	// Set tiles as lava-possible.
	GameMap::SetLavaPossible(Location(volcLoc.x, volcLoc.y), 1);
	GameMap::SetLavaPossible(Location(volcLoc.x, volcLoc.y+1), 1);
	GameMap::SetLavaPossible(Location(volcLoc.x, volcLoc.y+2), 1);
	GameMap::SetLavaPossible(Location(volcLoc.x, volcLoc.y+3), 1);
	GameMap::SetLavaPossible(Location(volcLoc.x, volcLoc.y+4), 1);

	// If current game time has surpassed the animation start time, automatically start the eruption animation (but let the regular CheckVolcanoes function handle setting the eruption).
	if (lavaAnimTime <= Game::Tick())
	{
		AnimateVolcano(&AllVolcanoes[numVolcanoesDefined]);
	}


	// Increment volcano count
	numVolcanoesDefined++;
}

void DisasterCreator::CheckVolcanoes()
{
	// Iterate through the list of volcanoes.
	int i = 0;
	while (i < numVolcanoesDefined)
	{
		// Check if the warn time has passed.
		if (!AllVolcanoes[i].animStarted &&
			Game::Tick() >= AllVolcanoes[i].warnTime)
		{
			AnimateVolcano(&AllVolcanoes[i]);
			i++;
		}

		// Check if the eruption time has passed.
		else if (!AllVolcanoes[i].eruptionSet &&
			     Game::Tick() >= AllVolcanoes[i].eruptTime)
		{
			EruptVolcano(&AllVolcanoes[i]);
			i++;
		}

		// Check if we should stop the animation.
		// Once we do that, we don't care about this volcano anymore, so we can remove it from the list.
		else if (AllVolcanoes[i].eruptionSet &&
			Game::Tick() >= AllVolcanoes[i].eruptTime + 980)
		{
			StopVolcano(&AllVolcanoes[i]);
			EraseVolcano(i);
		}

		else
		{
			i++;
		}
	}
}

void DisasterCreator::EraseVolcano(int i)
{
	// Push every volcano further down the array up one slot.
	for (i; i < numVolcanoesDefined-1; i++)
	{
		AllVolcanoes[i] = AllVolcanoes[i + 1];
	}

	numVolcanoesDefined--;
}

void DisasterCreator::AnimateVolcano(Volcano *v)
{
	v->animStarted = true;
	switch (v->direction)
	{
		case VolcanoDirection::South:
			AnimateFlowS(Location(v->eruptAt.x, v->eruptAt.y - 1));
			break;
		case VolcanoDirection::SouthEast:
			AnimateFlowSE(Location(v->eruptAt.x - 1, v->eruptAt.y - 1));
			break;
		case VolcanoDirection::SouthWest:
			AnimateFlowSW(Location(v->eruptAt.x, v->eruptAt.y - 1));
			break;
	}
}

void DisasterCreator::EruptVolcano(Volcano *v)
{
	v->eruptionSet = true;
	Game::CreateEruption(v->eruptAt, GetSpreadSpeed(v->speed));
}

void DisasterCreator::StopVolcano(Volcano *v)
{
	switch (v->direction)
	{
	case VolcanoDirection::South:
		FreezeFlowS(Location(v->eruptAt.x, v->eruptAt.y - 1));
		break;
	case VolcanoDirection::SouthEast:
		FreezeFlowSE(Location(v->eruptAt.x - 1, v->eruptAt.y - 1));
		break;
	case VolcanoDirection::SouthWest:
		FreezeFlowSW(Location(v->eruptAt.x, v->eruptAt.y - 1));
		break;
	}
}

void DisasterCreator::SetLavaSpeed(BlightLavaSpeed newSpeed)
{
	if (newSpeed < BlightLavaSpeed::Stopped || newSpeed > BlightLavaSpeed::Instant)
	{
		Game::AddMessage("DC Error: Invalid lava spread speed!", Tethys::SoundID::Dirt);
	}
	else
	{
		Game::SetLavaSpeed(GetSpreadSpeed(newSpeed));
	}
}

int DisasterCreator::GetSpreadSpeed(BlightLavaSpeed speed)
{
	int actualSpeed;
	switch (speed)
	{
	case BlightLavaSpeed::Stopped:
		actualSpeed = 0;
		break;
	case BlightLavaSpeed::VerySlow:
		actualSpeed = 15;
		break;
	case BlightLavaSpeed::Slow:
		actualSpeed = 45;
		break;
	case BlightLavaSpeed::Slower:
		actualSpeed = 85;
		break;
	case BlightLavaSpeed::MediumSlow:
		actualSpeed = 135;
		break;
	case BlightLavaSpeed::Medium:
		actualSpeed = 170;
		break;
	case BlightLavaSpeed::MediumFast:
		actualSpeed = 210;
		break;
	case BlightLavaSpeed::Fast:
		actualSpeed = 270;
		break;
	case BlightLavaSpeed::Faster:
		actualSpeed = 360;
		break;
	case BlightLavaSpeed::VeryFast:
		actualSpeed = 440;
		break;
	case BlightLavaSpeed::Fastest:
		actualSpeed = 510;
		break;
	case BlightLavaSpeed::Instant:
		actualSpeed = 4096;
		break;
	case BlightLavaSpeed::NoChange:
		actualSpeed = lastLavaSpeed;
		break;
	default:
		Game::AddMessage("DC Error: Unknown spread speed!", Tethys::SoundID::Dirt);
		actualSpeed = 0;
		break;
	}

	return actualSpeed;
}

void DisasterCreator::SetLavaPreciseSpeed(int newSpeed)
{
	if (newSpeed < 0)
	{
		Game::AddMessage("DC Error: Precise speed must be non-negative!", Tethys::SoundID::Dirt);
	}
	else
	{
		Game::SetLavaSpeed(newSpeed);
	}
}

void DisasterCreator::SetLavaTiles()
{
	SetLavaTiles({});
}

void DisasterCreator::SetLavaTiles(const vector<CellType>& optionalTypes)
{
	for (int x = 0; x < GameMap::GetWidth(); x++)
	{
		for (int y = 0; y < GameMap::GetHeight(); y++)
		{
			// Check if black rock.  Note that gray/black border tiles show lava correctly; orange/black do not however.
			int curTile = GameMap::GetTile(Location(x, y));
			if ((curTile >= 439 && curTile <= 536) ||
				(curTile >= 559 && curTile <= 606) ||
				curTile == 718 || curTile == 547 || curTile == 698 || curTile == 683 || 
				(curTile >= 798 && curTile <= 944) ||   // Walls, tubes, dozed terrain, wreckage
				(curTile >= 951 && curTile <= 998))     // Gray/black border tiles
			{
				// If no additional celltype checking, set lava possible.
				if (optionalTypes.size() == 0)
				{
					GameMap::SetLavaPossible(Location(x, y), true);
				}
				// Else we need to check if the current tile's celltype is one of those passed in.
				for (int i = 0; i < (int)optionalTypes.size(); i++)
				{
					if (GameMap::GetCellType(Location(x, y)) == optionalTypes[i])
					{
						GameMap::SetLavaPossible(Location(x, y), true);
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------
// Copied from OP2Helper, but rewritten to use TethysAPI.  If OP2Helper ever gets rewritten to use TethysAPI, these members can be removed.
// ----------------------------------------------------------------------------

void DisasterCreator::AnimateFlowSW(const Location& loc)
{
	GameMap::SetTile(loc + Location(1, 0), 0x453);
	GameMap::SetTile(loc, 0x447);
	GameMap::SetTile(loc + Location(0, 1), 0x45E);
	GameMap::SetTile(loc + Location(1, 1), 0x469);
}

void DisasterCreator::AnimateFlowS(const Location& loc)
{
	GameMap::SetTile(loc, 0x474);
	GameMap::SetTile(loc + Location(0, 1), 0x47E);
}

void DisasterCreator::AnimateFlowSE(const Location& loc)
{
	GameMap::SetTile(loc, 0x489);
	GameMap::SetTile(loc + Location(0, 1), 0x4A0);
	GameMap::SetTile(loc + Location(1, 1), 0x4AB);
	GameMap::SetTile(loc + Location(1, 0), 0x494);
}


void DisasterCreator::FreezeFlowSW(const Location& loc)
{
	GameMap::SetTile(loc + Location(1, 0), 0x45A);
	GameMap::SetTile(loc, 0x44F);
	GameMap::SetTile(loc + Location(0, 1), 0x465);
	GameMap::SetTile(loc + Location(1, 1), 0x470);
}

void DisasterCreator::FreezeFlowS(const Location& loc)
{
	GameMap::SetTile(loc, 0x47B);
	GameMap::SetTile(loc + Location(0, 1), 0x486);
}

void DisasterCreator::FreezeFlowSE(const Location& loc)
{
	GameMap::SetTile(loc, 0x490);
	GameMap::SetTile(loc + Location(0, 1), 0x4A8);
	GameMap::SetTile(loc + Location(1, 1), 0x4B2);
	GameMap::SetTile(loc + Location(1, 0), 0x49C);
}
