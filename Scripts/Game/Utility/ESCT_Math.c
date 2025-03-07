class ESCT_Math
{
	//------------------------------------------------------------------------------------------------
	//! Checks if a point is within a certain radius in the XZ plane.
	//! \param[in] p1 The first point.
	//! \param[in] p2 The second point.
	//! \param[in] radius The radius to check against.
	//! \return True if the distance between the points is less than or equal to the radius, false otherwise.
    static bool IsPointInRadiusXZ(vector point, vector center, float radius)
    {
        float distance = vector.DistanceSqXZ(point, center);
        return distance <= radius * radius;
    }
	
	//! Returns world size, radius and center of the world.
	static Tuple3<int, int, vector> GetWorldSize()
	{
		int worldSize = 0;
		int radius = 0;
		vector worldCenter = "0 0 0";
		
		if(g_Game.GetWorldEntity())
		{	
			vector min, max;
			g_Game.GetWorldEntity().GetWorldBounds(min, max);
			
			worldCenter = vector.Lerp(min, max, 0.5);
			worldCenter[1] = GetGame().GetWorld().GetSurfaceY(worldCenter[0], worldCenter[2]);
			
			worldSize = Math.Max(max[0] - min[0], max[2] - min[2]);
			radius = Math.Sqrt(2 * (worldSize / 2));
		}
		
		return new Tuple3<int, int, vector>(worldSize, radius, worldCenter);
	}
}
