#pragma once

#include <Tethys/API/API.h>

#include "DcEnums.h"

struct DisasterZone
{
	Tethys::MapRect zoneRect;
	DisasterType zoneType;
};

struct Volcano
{
	Tethys::Location eruptAt;  // Location for the eruption to occur
	int warnTime,			   // Tick that the volcano eruption should start playing.
		eruptTime;			   // Time of first warning (erupts 10 marks later)
	bool animStarted,		   // Set once the lava flow animation on the volcano has started playing.
		 eruptionSet;		   // Set once Game::SetEruption has been called.
	VolcanoDirection direction;
	BlightLavaSpeed speed;
};

struct QueuedDisaster
{
	DisasterType type;
	DisasterPower power;
	DisasterTarget target;
	int triggerTime = -1;
	Tethys::Location startAt,
		endAt;
	bool instant;
};