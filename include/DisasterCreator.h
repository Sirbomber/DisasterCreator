/*
  Version history
   Version 0.01 -- 3/1/19
	- Initial effort.  All 4 "regular" disasters supported.  Disasters target players, and then get an
	  "error" applied to them (within a certain radius).  This keeps players on their toes and reduces
	  the amount of pointless alerts for disasters on the other side of the map.
   Version 0.02 -- 4/9/19
	- Added disaster zones as a targeting type, which allow level creators to set specific areas a disaster should
	  spawn in.
	- Added weights for the different disaster targeting methods.
	- Added support for volcanoes.
   Version 1.00 -- 4/12/19
	- Initial (unannounced) release.
	- Finished work on disaster targeting.
	- Added SetLavaTiles function.
   Version 1.10 -- 4/21/19
   - Replaced STL lists with static arrays so saving will work.
   - Finalized sample level.
   Version 2.00 -- 5/28/2026
   - Updated to use the new TethysAPI.
   - Switched enums over to enum classes.
   - Made some quality-of-life changes.
   - Added Blight support.
   - Removed SetNumPlayers.  Use SetIgnorePlayer to protect specific players when using player-target mode for disasters.
   - Removed SetMapSize as it is no longer needed.
   - Updated sample level code to reflect changes to DisasterCreator parameter setup.
   - Added more lava/Blight spread speed options.
   - Added Set[Lava/Blight]PreciseSpeed.
   - Added DoDisasterAt to allow precise targetting of disasters.  Takes the same arguments as DoDisaster, but takes a map location instead of a targetting method.
   - Added support for queued disasters, allowing level creators to define specific disasters ahead of time.
*/

#pragma once

#include <Tethys/API/API.h>
#include <math.h>
#include <vector>

#include "DcEnums.h"
#include "DcStructs.h"

using namespace std;

// ----------------------------------------------

const int MAX_SIZE = 20;			// Maximum number of volcanoes/disaster zones/queued disasters a level creator may define.

class DisasterCreator
{
public:
	DisasterCreator();
	~DisasterCreator();

	// Disaster invocation
	void RollRandom();																									// Invokes a random disaster, based on current parameters.
	void DoDisaster(DisasterType type, DisasterPower power, DisasterTarget target, bool force = false, bool instant = false);			// This can be called to forcibly create a disaster.  Set the instant flag to skip the usual 10-mark delay + warning messages.
	void DoDisasterAt(DisasterType type, DisasterPower power, Tethys::Location target,
		              Tethys::Location stormVortexEndLoc, bool force = false, bool instant = false);					// This can be called to forcibly create a disaster at a specific map location.  Set the instant flag to skip the usual 10-mark delay + warning messages.
	void QueueDisaster(DisasterType type, DisasterPower power, DisasterTarget target, Tethys::Location at,
		               Tethys::Location stormVortexEndPt, int tick, bool instant);										// Use this to cause a disaster to happen at a specific time.
	void CheckQueuedDisasters();																						// Checks the list of queued disasters to see if any are ready to be triggered.

	// Disaster Creator controls
	void SetMinimumWait(int wait);																		// Set the minimum amount of time after a disaster the engine will wait before creating another one.
	void EnableDisaster(DisasterType type);																	// Allows the specified disaster to happen.  At least one disaster must be enabled for the DisasterCreator to work.
	void DisableDisaster(DisasterType type);																	// The specified disaster will no longer happen.  Also sets spawn chance to zero.
	void SetDisasterTypeWeight(DisasterType type, int weight);												// Adjusts the chance of the specified disaster occurring.  At least one must be set.
	void SetDisasterPowerWeight(DisasterPower power, int weight);											// Adjusts the chance of disasters occuring with the specified power.  At least one must be set.
	void SetDisasterTargetWeight(DisasterTarget target, int weight);											// Adjusts the chance of disasters using the specified targeting type.  At least one must be set.
	void SetIgnoreChance(int weight);																	// Adjusts the chance of disasters ignoring the minimum wait time before spawning.
	void SetIgnorePlayer(int player, bool ignore);														// Specify which players should be ignored if disasters are set to target players.
	void AddDisasterZone(DisasterType type, Tethys::MapRect zone);											// Defines a disaster spawn zone.
	Tethys::TethysAPI::Trigger SetCallbackTrigger(char const *callback, int Delay);						// Creates a time trigger, using the specified delay and callback function.  This must be called at least once for th
	Tethys::TethysAPI::Trigger SetCallbackTrigger(char const *callback, int minDelay, int maxDelay);	// Creates a time trigger, using the specified delay and callback function.

	// Volcano controls
	void CheckVolcanoes();
	void DefineVolcano(Tethys::Location volcLoc, int lavaAnimTime, int eruptTime, VolcanoDirection dir, BlightLavaSpeed speed = BlightLavaSpeed::NoChange);	// Note that animation/eruption time are absolute, not relative to the time the volcano was defined (in other words, if you define a volcano at tick 40000, and want it to erupt at tick 60000, you'd pass in 60000 for eruptTime, not 20000).  Also note that lava spread speed is global, not unique to each volcano.
	void SetLavaSpeed(BlightLavaSpeed newSpeed);
	void SetLavaTiles();														// Note that lava spread speed is global for all volcanoes.
	void SetLavaTiles(const vector<Tethys::CellType>& optionalTypes);			// Sets all "black rock" tiles to lava-possible.  Optionally, a list of tile types to check for may also be passed.  If so, only black rock tiles that are also one of those tile types will be set as lava possible.
	void SetLavaPreciseSpeed(int newSpeed);

	// Blight helper
	void SetBlight(Tethys::Location spawnAt, BlightShape shape, int size, BlightLavaSpeed speed = BlightLavaSpeed::NoChange, bool infect = true, bool noWarning = false);	// Spawn location should be the midpoint of the area you want to infect.  Set infect to false to "despawn" Blight in an area.
	void SetBlightSpeed(BlightLavaSpeed newSpeed);
	void SetBlightPreciseSpeed(int newSpeed);

private:
	// Disaster creation functions
	void DoQuake(DisasterPower power, DisasterTarget target, bool instant = false, Tethys::Location at = { -1,-1 });
	void DoStorm(DisasterPower power, DisasterTarget target, bool instant = false, Tethys::Location start = { -1,-1 }, Tethys::Location end = { -1,-1 });
	void DoVortex(DisasterPower power, DisasterTarget target, bool instant = false, Tethys::Location start = { -1,-1 }, Tethys::Location end = { -1,-1 });
	void DoMeteor(DisasterPower power, DisasterTarget target, bool instant = false, Tethys::Location at = { -1,-1 });

	// Volcano helper functions
	void AnimateVolcano(Volcano *v);
	void EruptVolcano(Volcano *v);
	void StopVolcano(Volcano *v);
	void EraseVolcano(int i);
	int GetSpreadSpeed(BlightLavaSpeed speed);							// Note that lava spread speed is global for all volcanoes.

	// Queued disaster helper functions
	void EraseQueuedDisaster(int i);

	// Disaster targeting
	Tethys::Location GetDisasterTarget(DisasterTarget trgType, DisasterType type);
	Tethys::Location TargetRandomLocation();
	Tethys::Location TargetLocationInZone(DisasterType type);
	Tethys::Location TargetPlayer();
	Tethys::Location GetVortexDestination(Tethys::Location source);
	Tethys::Location GetStormDestination(Tethys::Location source);
	Tethys::Location TargetWithinRadius(Tethys::Location target);

	int ApproxDistance(Tethys::Location s, Tethys::Location d);

	// Data
	int lastTick,			  // Records the last time a disaster was successfully rolled for.
		minWait,			  // Minimum time (in ticks) to wait between disasters
		ignoreMinTimeChance;  // Chance to ignore the minimum wait timer.

	Tethys::TethysAPI::Trigger disCheck;		  // Pointer to callback trigger

	bool ignorePlayer[7] = { false,false,false,false,false,false,true };

	// These let us control which disasters are allowed...
	bool quakesEnabled,
		stormsEnabled,
		vortexEnabled,
		meteorEnabled;

	// ...and how often they get rolled.
	int quakesWeight,
		stormsWeight,
		vortexWeight,
		meteorWeight,
		noneWeight;

	// Weights for disaster power.
	int lowWeight,
		mediumWeight,
		highWeight,
		apocWeight;

	// Weights for disaster targeting type.
	int randWeight,		// Chance to target a completely random point on the map.
		zoneWeight,		// Chance to target one of the defined disaster zones.
		plyrWeight;		// Chance to target a point near a player's base or units

	// Disaster zones
	DisasterZone AllZones[MAX_SIZE];
	int numZonesDefined;

	// Defined volcanoes
	Volcano AllVolcanoes[MAX_SIZE];
	int numVolcanoesDefined;
	int lastLavaSpeed;					// Used to preserve lava spread rate when "no change" passed in.

	// Queued disasters
	QueuedDisaster AllQueuedDisasters[MAX_SIZE];
	int numQueuedDisastersDefined;

	// These members are copied from OP2Helper, but have been rewritten to use TethysAPI
	void AnimateFlowSW(const Tethys::Location& loc);
	void AnimateFlowS(const Tethys::Location& loc);
	void AnimateFlowSE(const Tethys::Location& loc);
	void FreezeFlowSW(const Tethys::Location& loc);
	void FreezeFlowS(const Tethys::Location& loc);
	void FreezeFlowSE(const Tethys::Location& loc);

};
