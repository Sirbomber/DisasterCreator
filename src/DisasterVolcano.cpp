#include "DisasterCreator.h"

/*
  Code that assists level creators with the eruption of volcanoes.
*/

void DisasterCreator::DefineVolcano(LOCATION volcLoc, int lavaAnimTime, int eruptTime, disVolcanoDir dir, disSpeed speed)
{
	// Limit check.
	if (numVolcanoesDefined >= MAX_SIZE)
	{
		TethysGame::AddMessage(-1, -1, "DC Error: Too many volcanoes!", -1, sndDirt);
	}

	// Sanity checks.
	if (lavaAnimTime > eruptTime ||
		dir < volSouth || dir > volNone ||
		speed < spdStopped || speed > spdInstant)
	{
		TethysGame::AddMessage(-1, -1, "DC Error: Bad volcano definition!", -1, sndDirt);
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
	numVolcanoesDefined++;


	// Set tiles as lava-possible.
	GameMap::SetLavaPossible(LOCATION(volcLoc.x, volcLoc.y), 1);
	GameMap::SetLavaPossible(LOCATION(volcLoc.x, volcLoc.y+1), 1);
	GameMap::SetLavaPossible(LOCATION(volcLoc.x, volcLoc.y+2), 1);
}

void DisasterCreator::CheckVolcanoes()
{
	// Iterate through the list of volcanoes.
	int i = 0;
	while (i < numVolcanoesDefined)
	{
		// Check if the warn time has passed.
		if (!AllVolcanoes[i].animStarted &&
			TethysGame::Tick() >= AllVolcanoes[i].warnTime)
		{
			AnimateVolcano(&AllVolcanoes[i]);
			i++;
		}

		// Check if the eruption time has passed.
		else if (!AllVolcanoes[i].eruptionSet &&
			     TethysGame::Tick() >= AllVolcanoes[i].eruptTime)
		{
			EruptVolcano(&AllVolcanoes[i]);
			i++;
		}

		// Check if we should stop the animation.
		// Once we do that, we don't care about this volcano anymore, so we can remove it from the list.
		else if (!AllVolcanoes[i].eruptionSet &&
			TethysGame::Tick() >= AllVolcanoes[i].eruptTime + 1000)
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
		case volSouth:
			AnimateFlowS(LOCATION(v->eruptAt.x, v->eruptAt.y - 1));
			break;
		case volSouthEast:
			AnimateFlowSE(LOCATION(v->eruptAt.x - 1, v->eruptAt.y - 1));
			break;
		case volSouthWest:
			AnimateFlowSW(LOCATION(v->eruptAt.x, v->eruptAt.y - 1));
			break;
	}
}

void DisasterCreator::EruptVolcano(Volcano *v)
{
	v->eruptionSet = true;
	TethysGame::SetEruption(v->eruptAt.x, v->eruptAt.y, GetSpreadSpeed(v->speed));
}

void DisasterCreator::StopVolcano(Volcano *v)
{
	switch (v->direction)
	{
	case volSouth:
		FreezeFlowS(LOCATION(v->eruptAt.x, v->eruptAt.y - 1));
		break;
	case volSouthEast:
		FreezeFlowSE(LOCATION(v->eruptAt.x - 1, v->eruptAt.y - 1));
		break;
	case volSouthWest:
		FreezeFlowSW(LOCATION(v->eruptAt.x, v->eruptAt.y - 1));
		break;
	}
}

int DisasterCreator::GetSpreadSpeed(disSpeed speed)
{
	// To-Do: Make these a function of map size
	int actualSpeed;
	switch (speed)
	{
	case spdStopped:
		actualSpeed = 0;
		break;
	case spdSlow:
		actualSpeed = 15;
		break;
	case spdMedium:
		actualSpeed = 45;
		break;
	case spdFast:
		actualSpeed = 85;
		break;
	case spdInstant:
		actualSpeed = 4096;
		break;
	default:
		TethysGame::AddMessage(-1, -1, "DC Error: Unknown spread speed!", -1, sndDirt);
		actualSpeed = 0;
		break;
	}

	return actualSpeed;
}

void DisasterCreator::SetLavaTiles()
{
	SetLavaTiles({});
}

void DisasterCreator::SetLavaTiles(const vector<CellTypes>& optionalTypes)
{
	for (int x = 0; x < mapSize.x; x++)
	{
		for (int y = 0; y < mapSize.y; y++)
		{
			// Check if black rock.  Note that gray/black border tiles show lava correctly; orange/black do not however.
			int curTile = GameMap::GetTile(LOCATION(x + mapXOffset, y - 1));
			if ((curTile >= 439 && curTile <= 536) ||
				(curTile >= 559 && curTile <= 606) ||
				curTile == 718 || curTile == 547 || curTile == 698 || curTile == 683 || 
				(curTile >= 798 && curTile <= 944) ||   // Walls, tubes, dozed terrain, wreckage
				(curTile >= 951 && curTile <= 998))     // Gray/black border tiles
			{
				// If no additional celltype checking, set lava possible.
				if (optionalTypes.size() == 0)
				{
					GameMap::SetLavaPossible(LOCATION(x + mapXOffset, y - 1), true);
				}
				// Else we need to check if the current tile's celltype is one of those passed in.
				for (int i = 0; i < (int)optionalTypes.size(); i++)
				{
					if (GameMap::GetCellType(LOCATION(x + mapXOffset, y - 1)) == optionalTypes[i])
					{
						GameMap::SetLavaPossible(LOCATION(x + mapXOffset, y - 1), true);
					}
				}
			}
		}
	}
}
