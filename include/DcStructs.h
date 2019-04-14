#ifndef DC_STRUCTS
#define DC_STRUCTS

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
	int warnTime,			  // Tick that the volcano eruption should start playing.
		eruptTime;			  // Time of first warning (erupts 10 marks later)
	bool animStarted,		  // Set once the lava flow animation on the volcano has started playing.
		 eruptionSet;		  // Set once TethysGame::SetEruption has been called.
	disVolcanoDir direction;
	disSpeed speed;
};

#endif
