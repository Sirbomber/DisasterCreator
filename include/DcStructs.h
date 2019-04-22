#pragma once

#include <Outpost2DLL.h>

#include "DcEnums.h"

struct DisasterZone
{
	MAP_RECT zoneRect;
	disType zoneType;
};

struct Volcano
{
	LOCATION eruptAt;		  // Location for the eruption to occur
	int warnTime = 0;			  // Tick that the volcano eruption should start playing.
	int	eruptTime = 0;			  // Time of first warning (erupts 10 marks later)
	bool animStarted = false;		  // Set once the lava flow animation on the volcano has started playing.
	bool eruptionSet = false;		  // Set once TethysGame::SetEruption has been called.
	disVolcanoDir direction = disVolcanoDir::volNone;
	disSpeed speed = disSpeed::spdStopped;
};
