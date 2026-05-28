#pragma once

// Disaster type enum
enum class DisasterType : int
{
	Quake = 0,
	Storm,
	Vortex,
	Meteor,
	None,

	Count
};

// Relative disaster power enum
enum class DisasterPower : int
{
	Low = 0,
	Medium,
	High,
	Apocalyptic,

	Count
};

// Disaster targeting method enum
enum class DisasterTarget : int
{
	Random = 0,
	Zone,
	Player,
	Location,

	Count
};

// Volcano direction enum.  Used to play the "volcano" animation before the volcano actually erupts.
// Look at the volcano in the top right corner of Plymouth Starship 1 if you don't know what I'm talking about.
enum class VolcanoDirection : int
{
	South = 0,
	SouthEast,
	SouthWest,
	None,				// Use this if you don't want to play the volcano animation, for whatever reason.

	volCount
};

// Lava/Blight spread speed enum.
enum class BlightLavaSpeed : int
{
	Stopped = 0,
	NoChange,
	VerySlow,
	Slower,
	Slow,
	MediumSlow,
	Medium,
	MediumFast,
	Fast,
	Faster,
	VeryFast,
	Fastest,
	Instant,

	Count
};

// Blight spawn shape
enum class BlightShape : int
{
	Square = 0,
	Diamond,
	TriangleUp,
	TriangleDown,

	Count
};