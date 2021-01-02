#ifndef DC_ENUMS
#define DC_ENUMS

// Disaster type enum
enum disType
{
	disQuake = 0,
	disStorm,
	disVortex,
	disMeteor,
	disNone
};

// Relative disaster power enum
enum disPower
{
	pwrLow = 0,
	pwrMedium,
	pwrHigh,
	pwrApocalyptic
};

// Disaster targeting method enum
enum disTarget
{
	trgRandom = 0,
	trgZone,
	trgPlayer
};

// Volcano direction enum.  Used to play the "volcano" animation before the volcano actually erupts.
// Look at the volcano in the top right corner of Plymouth Starship 1 if you don't know what I'm talking about.
enum disVolcanoDir
{
	volSouth = 0,
	volSouthEast,
	volSouthWest,
	volNone				// Use this if you don't want to play the volcano animation, for whatever reason.
};

// Lava spread speed enum.
enum disSpeed
{
	spdStopped = 0,
	spdSlow,
	spdMedium,
	spdFast,
	spdInstant
};

#endif