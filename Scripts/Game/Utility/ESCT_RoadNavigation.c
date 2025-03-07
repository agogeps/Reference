class ESCT_RoadNavigation
{
	private static const float m_fEpsilon = 0.0001;

	static vector GetClosestRoadPosition(vector position, out float distanceToRoad = 0)
	{
		if (position == vector.Zero)
			return vector.Zero;

		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld)
			return vector.Zero;

		RoadNetworkManager roadNetworkManager = aiWorld.GetRoadNetworkManager();
		if (!roadNetworkManager)
			return vector.Zero;

		BaseRoad road;
		roadNetworkManager.GetClosestRoad(position, road, distanceToRoad, skipNavlinks: true);
		if (!road)
			return vector.Zero;

		array<vector> points = {};
		road.GetPoints(points);

		if (points.IsEmpty())
			return vector.Zero;

		vector waypointPosition;
		int iteration = 30;
		while (iteration > 0)
		{
			waypointPosition = points.GetRandomElement();
			if (SCR_WorldTools.TraceCylinder(waypointPosition))
			{
				break;
			}

			iteration--;
			waypointPosition = vector.Zero;
		}

		if (waypointPosition == vector.Zero)
			ESCT_Logger.ErrorFormat("Failed to find road position for %1 position", position.ToString());

		return waypointPosition;

	}

	static void SelectRoadSegment(vector position, out vector spawnPosition, out vector direction, bool facePosition = true, float distanceToRoad = 600)
	{
		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld)
			return;

		RoadNetworkManager roadNetworkManager = aiWorld.GetRoadNetworkManager();
		if (!roadNetworkManager)
			return;

		BaseRoad road;
		float distance;
		roadNetworkManager.GetClosestRoad(position, road, distance, skipNavlinks: true);
		if (!road || distance > distanceToRoad)
			return;

		array<vector> points = {};
		road.GetPoints(points);

		if (points.IsEmpty())
			return;

		vector firstPosition = vector.Zero;
		vector secondPosition = vector.Zero;

		const float cylinderHeight = 1;
		const float cylinderRadius = 4;
		vector cylinderOffset = Vector(0, cylinderHeight, 0);

		int iteration = 30;
		while (iteration > 0)
		{
			int positionIndex = points.GetRandomIndex();
			if (points.IsIndexValid(positionIndex + 1))
			{
				firstPosition = points[positionIndex];
				secondPosition = points[positionIndex + 1];
			}
			else
			{
				firstPosition = points[positionIndex - 1];
				secondPosition = points[positionIndex];
			}

			//second position needed only for direction
			if (SCR_WorldTools.TraceCylinder(firstPosition + cylinderOffset, cylinderRadius, cylinderHeight))
			{
				break;
			}

			iteration--;
		}

		spawnPosition = firstPosition;
		direction = vector.Direction(firstPosition, secondPosition);
		direction.Normalize();

		if (facePosition)
		{
			vector roadToDestination = (position - spawnPosition);
			roadToDestination.Normalize();
			if (vector.Dot(direction, roadToDestination) < 0) direction = -direction;
		}
	}
}
