modded class SCR_AmbientVehicleSystem : GameSystem
{
	protected static const int SPAWN_RADIUS_MIN_SQ = 375 * 375;		// Square value for distance checks
	protected static const int SPAWN_RADIUS_MAX_SQ = 750 * 750;	// Square value for distance checks
	protected static const int DESPAWN_RADIUS_DIFF_SQ = 150 * 150;	// Square value for distance checks
}