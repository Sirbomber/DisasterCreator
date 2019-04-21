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
*/

#ifndef DISASTERCREATOR
#define DISASTERCREATOR

#include <Outpost2DLL.h>
#include <OP2Helper.h>
#include <math.h>
#include <vector>

#include "DcEnums.h"
#include "DcStructs.h"

using namespace std;

// ----------------------------------------------

const int MAX_SIZE = 20;			// Maximum number of volcanoes/disaster zones a level creator may define.

class DisasterCreator
{
	public:
		DisasterCreator();
		~DisasterCreator();

		// Disaster invocation
		void RollRandom();
		void DoDisaster(disType type, disPower power, disTarget target);

		// Disaster Creator controls
		void SetMapSize(int width, int height);											// Required.  Tell DC the map dimensions.
		void SetMinimumWait(int wait);													// Set the minimum amount of time after a disaster the engine will wait before creating another one.
		void SetNumPlayers(int numPl);													// Allows level creator to control which players can be targetted by disasters.  Useful for protecting AI players.
		void EnableDisaster(disType type);												// Allows the specified disaster to happen.  At least one disaster must be enabled.
		void DisableDisaster(disType type);												// The specified disaster will no longer happen.  Also sets spawn chance to zero.
		void SetDisasterTypeWeight(disType type, int weight);							// Adjusts the chance of the specified disaster occurring.  At least one must be set.
		void SetDisasterPowerWeight(disPower power, int weight);						// Adjusts the chance of disasters occuring with the specified power.  At least one must be set.
		void SetDisasterTargetWeight(disTarget target, int weight);						// Adjusts the chance of disasters using the specified targetting type.  At least one must be set.
		void SetIgnoreChance(int weight);												// Adjusts the chance of disasters ignoring the minimum wait time before spawning.
		void AddDisasterZone(disType type, MAP_RECT zone);								// Defines a disaster spawn zone.
		Trigger SetCallbackTrigger(char const *callback, int Delay);					// Creates a time trigger, using the specified delay and callback function.
		Trigger SetCallbackTrigger(char const *callback, int minDelay, int maxDelay);	// Creates a time trigger, using the specified delay and callback function.

		// Volcano controls
		void CheckVolcanoes();
		void DefineVolcano(LOCATION volcLoc, int lavaAnimTime, int eruptTime, disVolcanoDir dir, disSpeed speed);
		void SetLavaTiles();
		void SetLavaTiles(const vector<CellTypes>& optionalTypes);			// Sets all "black rock" tiles to lava-possible.  Optionally, a list of tile types to check for may also be passed.  If so, only black rock tiles that are also one of those tile types will be set as lava possible.

	private:
		// Disaster creation functions
        void DoQuake(disPower power, disTarget target);
		void DoStorm(disPower power, disTarget target);
		void DoVortex(disPower power, disTarget target);
		void DoMeteor(disPower power, disTarget target);

		// Volcano helper functions
		void AnimateVolcano(Volcano *v);
		void EruptVolcano(Volcano *v);
		void StopVolcano(Volcano *v);
		void EraseVolcano(int i);
		int GetSpreadSpeed(disSpeed speed);

		// Disaster targetting
		LOCATION GetDisasterTarget(disTarget trgType, disType type);
		LOCATION TargetRandomLocation();
		LOCATION TargetLocationInZone(disType type);
		LOCATION TargetPlayer();
		LOCATION GetVortexDestination(LOCATION source);
		LOCATION GetStormDestination(LOCATION source);
		LOCATION TargetWithinRadius(LOCATION target);

		int ApproxDistance(LOCATION s, LOCATION d);

		// Data
		int lastTick,			  // Records the last time a disaster was successfully rolled for.
			minWait,			  // Minimum time (in ticks) to wait between disasters
			ignoreMinTimeChance;  // Chance to ignore the minimum wait timer.

		Trigger disCheck;		  // Pointer to callback trigger

		LOCATION mapSize;		  // Stores x/y size of map
		int mapXOffset;			  // This will be either +31 (regular maps) or -1 (world maps)

		int numPlayers;

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
};

#endif
