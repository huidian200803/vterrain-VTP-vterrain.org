//
// Enumerations for the Enviro application.
//

#ifndef ENVIROENUMH
#define ENVIROENUMH

// States in which the application can be in:
enum AppState {
	AS_Initializing,	// still setting up
	AS_Neutral,			// not doing anything
	AS_Orbit,			// full view of the earth
	AS_FlyingIn,		// moving in toward a specific terrain
	AS_SwitchToTerrain,	// ready to switch
	AS_Terrain,			// inside a terrain
	AS_MovingOut,		// moving out to orbit
	AS_Error
};

/// Navigation types
enum NavType {
	NT_Normal,
	NT_Velo,
	NT_Grab,
	NT_Dummy,	// takes place of historical removed style
	NT_Pano
};

/// Mouse modes
enum MouseMode
{
	MM_NAVIGATE,
	MM_SELECT,
	MM_SELECTBOX,
	MM_SELECTMOVE,
	MM_LINEARS,
	MM_BUILDINGS,
	MM_POWER,
	MM_PLANTS,
	MM_ADDPOINTS,
	MM_INSTANCES,
	MM_VEHICLES,
	MM_MOVE,
	MM_MEASURE,
	MM_SLOPE,

	MM_NONE
};

#endif	// ENVIROENUMH

