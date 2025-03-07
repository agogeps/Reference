class ESCT_Misc
{
	static bool HasNearEntities(IEntity radiusEntity, notnull array<IEntity> entities, float checkDistance = 100.0)
	{
		if (entities.IsEmpty())
			return false;
		
		bool isInteresected = false;
		
		foreach (IEntity entity : entities)
		{
			if (ESCT_Math.IsPointInRadiusXZ(radiusEntity.GetOrigin(), entity.GetOrigin(), checkDistance))
			{
				isInteresected = true;
				break;
			}
		}
		
		return isInteresected;
	}
	
	static vector GetRandomValidPosition(vector center, float minRadius, float maxRadius, float areaRadius = 50.0, float querySpace = 0.5)
	{	
		ESCT_EscapistsGameMode gameMode = ESCT_EscapistsGameMode.Cast(GetGame().GetGameMode());
		vector initialPosition = gameMode.GetRandomGenerator().GenerateRandomPointInRadius(minRadius, maxRadius, center);
		initialPosition[1] = SCR_TerrainHelper.GetTerrainY(initialPosition, noUnderwater: false);
		
		int initialIterations = 30;
		bool isNotInWater = false;
							
		while(initialIterations > 0)
		{
			if (!IsPositionInWater(initialPosition))
			{
				isNotInWater = true;
				break;
			}
				
			initialPosition = gameMode.GetRandomGenerator().GenerateRandomPointInRadius(minRadius, maxRadius, center);
			initialPosition[1] = SCR_TerrainHelper.GetTerrainY(initialPosition, noUnderwater: false);
				
			initialIterations -= 1;
		}
		
		vector finalPosition;
		if (isNotInWater)
		{
			SCR_WorldTools.FindEmptyTerrainPosition(finalPosition, initialPosition, areaRadius, querySpace);
		}
		else
		{
			SCR_WorldTools.FindEmptyTerrainPosition(finalPosition, center, areaRadius, querySpace);
		}
		
		//SCR_TerrainHelper.GetTerrainY(finalPosition, GetWorld(), noUnderwater: true);
		
		const float cylinderHeight = 0.5;
		vector cylinderOffset = Vector(0, cylinderHeight * 0.5, 0);

		//FIXME: seems that trace cylinder also checks for ocean (water?)?
		if (SCR_WorldTools.TraceCylinder(finalPosition + cylinderOffset, 2, 2) && !IsPositionInWater(finalPosition))
			return finalPosition;
	
		float radius = areaRadius + 15;	
		bool isSuccessful = false;
		int iterations = 30;
							
		while(iterations > 0)
		{
			if (SCR_WorldTools.TraceCylinder(finalPosition + cylinderOffset, 2, 2) && !IsPositionInWater(finalPosition))	
			{
				isSuccessful = true;
				break;
			}
			
			SCR_WorldTools.FindEmptyTerrainPosition(finalPosition, initialPosition, radius, querySpace);	
				
			radius += 15;
			iterations -= 1;
		}
		
		if (!isSuccessful)
		{
			ESCT_Logger.ErrorFormat("Failed to find valid position for %1 center!", center.ToString());
			return vector.Zero;
		}
		
		return finalPosition;
	}
	
	static vector GetRandomPositionForVehicle(vector center, float minRadius, float maxRadius, float areaRadius = 75.0, float querySpace = 4)
	{	
		vector initialPosition = FindRandomPointNotInWater(center, minRadius, maxRadius);
		if (initialPosition == vector.Zero)
			return vector.Zero;
		
		vector resultPosition;
		SCR_WorldTools.FindEmptyTerrainPosition(resultPosition, initialPosition, areaRadius, querySpace);
		if (resultPosition == vector.Zero)
			return vector.Zero;
		
		const float cylinderHeight = 0.5;
		vector cylinderOffset = Vector(0, cylinderHeight * 0.5, 0);

		//FIXME: seems that trace cylinder also checks for ocean (water?)?
		if (SCR_WorldTools.TraceCylinder(resultPosition + cylinderOffset, 2, 2) && !IsPositionInWater(resultPosition))
			return resultPosition;
	
		float radius = areaRadius + 15;	
		bool isSuccessful = false;
		int iterations = 30;
							
		while(iterations > 0)
		{
			if (SCR_WorldTools.TraceCylinder(resultPosition + cylinderOffset, 2, 2) && !IsPositionInWater(resultPosition))	
			{
				isSuccessful = true;
				break;
			}
			
			SCR_WorldTools.FindEmptyTerrainPosition(resultPosition, initialPosition, radius, querySpace);	
				
			radius += 15;
			iterations -= 1;
		}
		
		if (!isSuccessful)
		{
			ESCT_Logger.ErrorFormat("Failed to find valid position for %1 center!", center.ToString());
			return vector.Zero;
		}
		
		return resultPosition;
	}
	
	static bool IsPositionInWater(vector position)
	{
		if (position == vector.Zero)
			return false;
		
		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return false;
		
		position[1] = world.GetSurfaceY(position[0], position[2]);
		return ChimeraWorldUtils.TryGetWaterSurfaceSimple(world, position);
	}
	
	static string GetPrefabName(IEntity entity)
	{
		if (!entity) 
			return string.Empty;
		
		string prefabName = SCR_BaseContainerTools.GetPrefabResourceName(entity.GetPrefabData().GetPrefab());
		if (!prefabName)
			return string.Empty;
		
		array<string> splits = {};
		prefabName.Split("/", splits, true);
		
		return splits.Get(splits.Count() - 1);
	}
	
	static string GetPrefabName(ResourceName prefabName)
	{
		if (!prefabName || prefabName == ResourceName.Empty) 
			return string.Empty;
		
		array<string> splits = {};
		prefabName.Split("/", splits, true);
		
		return splits.Get(splits.Count() - 1);
	}
	
	static vector FindRandomPointNotInWater(vector center, float minRadius, float maxRadius, float maxDeviation = 25)
	{
		ESCT_EscapistsGameMode gameMode = ESCT_EscapistsGameMode.Cast(GetGame().GetGameMode());
		RandomGenerator rnd = gameMode.GetRandomGenerator();
		vector position = gameMode.GetRandomGenerator().GenerateRandomPointInRadius(minRadius, maxRadius, center);
		position[1] = SCR_TerrainHelper.GetTerrainY(position, noUnderwater: false);
		
		int initialIterations = 40;
		bool isNotInWater = false;
							
		while(initialIterations > 0)
		{
			if (!IsPositionInWater(position))
			{
				isNotInWater = true;
				break;
			}
			
			//deviate center position a bit
			position[0] = rnd.RandFloatXY(-maxDeviation, maxDeviation);
			position[2] = rnd.RandFloatXY(-maxDeviation, maxDeviation);
			position = gameMode.GetRandomGenerator().GenerateRandomPointInRadius(minRadius, maxRadius, center);
			position[1] = SCR_TerrainHelper.GetTerrainY(position, noUnderwater: false);
				
			initialIterations -= 1;
		}
		
		if (!isNotInWater)
			return vector.Zero;
		
		return position;
	}
}
