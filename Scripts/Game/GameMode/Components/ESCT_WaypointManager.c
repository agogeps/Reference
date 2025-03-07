[ComponentEditorProps(category: "Escapists/GameMode/Components", description: "Component that holds waypoint configs.")]
class ESCT_WaypointManagerComponentClass : ESCT_GameModeBaseComponentClass
{
}

sealed class ESCT_WaypointManagerComponent: ESCT_GameModeBaseComponent
{
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Defend waypoint type.", params: "et", category: "Escapists")]
	protected ResourceName m_DefendWaypoint;

	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Patrol waypoint type.", params: "et", category: "Escapists")]
	protected ResourceName m_PatrolWaypoint;

	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Wait waypoint type.", params: "et", category: "Escapists")]
	protected ResourceName m_WaitWaypoint;

	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Cycle waypoint type.", params: "et", category: "Escapists")]
	protected ResourceName m_CycleWaypoint;

	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Search and destroy waypoint type.", params: "et", category: "Escapists")]
	protected ResourceName m_SearchAndDestroyWaypoint;
	
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Suppression waypoint type.", params: "et", category: "Escapists")]
	protected ResourceName m_SuppressionWaypoint;

	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Move waypoint type.", params: "et", category: "Escapists")]
	protected ResourceName m_MoveWaypoint;
	
	private static const float WAYPOINT_EMPTY_RADIUS = 15.0;
	private static const float WAYPOINT_QUERY_SPACE = 2.0; //usually the same as default completion radius so waypoint could be approached

	private static ESCT_WaypointManagerComponent s_Instance = null;
	static ESCT_WaypointManagerComponent GetInstance()
	{
		if (!s_Instance)
		{
			BaseGameMode gameMode = GetGame().GetGameMode();
			if (gameMode)
			{
				s_Instance = ESCT_WaypointManagerComponent.Cast(gameMode.FindComponent(ESCT_WaypointManagerComponent));
			}
		}

		return s_Instance;
	}

	AIWaypoint DefendPosition(AIGroup group, vector position, int radius = 50.0, int defendPreset = 0, int priority = -1)
	{
		AIWaypoint waypoint = SpawnWaypoint(m_DefendWaypoint, position);
		if (!waypoint)
			return null;
		
		SCR_DefendWaypoint defendWaypoint = SCR_DefendWaypoint.Cast(waypoint);
		if (!defendWaypoint)
			return null;
		
		defendWaypoint.SetCompletionRadius(radius);
		defendWaypoint.SetCurrentDefendPreset(defendPreset);
		
		if (priority != -1)
			defendWaypoint.SetPriorityLevel(priority);

		group.AddWaypoint(defendWaypoint);

		return waypoint;
	}

	SCR_AIWaypoint Move(AIGroup group, vector position, int radius = 4.0, int priority = -1)
	{
		AIWaypoint waypoint = SpawnWaypoint(m_MoveWaypoint, position);
		SCR_AIWaypoint moveWaypoint = SCR_AIWaypoint.Cast(waypoint);
		
		moveWaypoint.SetCompletionRadius(radius);
		if (priority != -1)
			moveWaypoint.SetPriorityLevel(priority); 

		group.AddWaypoint(moveWaypoint);

		return moveWaypoint;
	}
	
	SCR_AIWaypoint PatrolSingle(AIGroup group, vector position, int radius = 4.0, int priority = -1)
	{
		AIWaypoint waypoint = SpawnWaypoint(m_PatrolWaypoint, position);
		SCR_AIWaypoint patrolWaypoint = SCR_AIWaypoint.Cast(waypoint);
		patrolWaypoint.SetCompletionRadius(radius);
		if (priority != -1)
			patrolWaypoint.SetPriorityLevel(priority); 

		group.AddWaypoint(patrolWaypoint);
		
		SCR_TimedWaypoint waitWaypoint = SCR_TimedWaypoint.Cast(SpawnWaypoint(m_WaitWaypoint, position));
		waitWaypoint.SetHoldingTime(Math.RandomFloat(5, 15));
		group.AddWaypoint(waitWaypoint);

		return patrolWaypoint;
	}
	
	AIWaypointCycle Cycle(AIGroup group, int radius = 4.0)
	{
		AIWaypointCycle cycleWaypoint = AIWaypointCycle.Cast(SpawnWaypoint(m_CycleWaypoint, group.GetOrigin()));
		if (!cycleWaypoint)
			return null;
		
		array<AIWaypoint> waypoints = {};
		group.GetWaypoints(waypoints);
		cycleWaypoint.SetWaypoints(waypoints);
		cycleWaypoint.SetCompletionRadius(radius);
		group.AddWaypoint(cycleWaypoint);

		return cycleWaypoint;
	}
	
	SCR_TimedWaypoint Suppress(AIGroup group, vector position, int radius = 4.0)
	{
		AIWaypoint waypoint = SpawnWaypoint(m_SuppressionWaypoint, position);
		SCR_SuppressWaypoint suppressWaypoint = SCR_SuppressWaypoint.Cast(waypoint);
		suppressWaypoint.SetCompletionRadius(radius);

		group.AddWaypoint(suppressWaypoint);

		return suppressWaypoint;
	}

	SCR_TimedWaypoint SearchAndDestroy(AIGroup group, vector position, int radius = 20.0, float holdingTime = 60.0)
	{
		AIWaypoint waypoint = SpawnWaypoint(m_SearchAndDestroyWaypoint, position);
		SCR_TimedWaypoint sndWaypoint = SCR_TimedWaypoint.Cast(waypoint);
		sndWaypoint.SetCompletionRadius(radius);
		sndWaypoint.SetHoldingTime(holdingTime);

		group.AddWaypoint(sndWaypoint);

		return sndWaypoint;
	}

	array<AIWaypoint> Patrol(AIGroup group, vector center, float patrolRadius, string formationType = "Column")
	{
		AIFormationComponent formation = AIFormationComponent.Cast(group.FindComponent(AIFormationComponent));
		if (!formation)
			return null;

		formation.SetFormation(formationType);

		RandomGenerator gen = ESCT_EscapistsGameMode.GetGameMode().GetRandomGenerator();
		array<float> radians = {gen.RandFloatXY(0, 45.0) * Math.DEG2RAD, gen.RandFloatXY(60.0, 120.0) * Math.DEG2RAD, gen.RandFloatXY(150.0, 210.0) * Math.DEG2RAD, gen.RandFloatXY(240.0, 300.0) * Math.DEG2RAD};
		array<vector> vectors = {};
		for (int i = 0; i < radians.Count(); i++)
		{
			float radius = gen.RandFloatXY(patrolRadius - 50, patrolRadius + 50);
			float radian = radians[i];
			vectors.Insert(Vector(center[0] + radius * Math.Cos(radian), 15.0, center[2] + radius * Math.Sin(radian)));
		}

		array<AIWaypoint> waypoints = {};
		foreach (vector position : vectors)
		{
			position[1] = SCR_TerrainHelper.GetTerrainY(position);

			vector waypointPosition;
			SCR_WorldTools.FindEmptyTerrainPosition(waypointPosition, position, WAYPOINT_EMPTY_RADIUS, WAYPOINT_QUERY_SPACE);

			AIWaypoint patrolWaypoint = SpawnWaypoint(m_PatrolWaypoint, waypointPosition);
			waypoints.Insert(patrolWaypoint);
			group.AddWaypoint(patrolWaypoint);

			SCR_TimedWaypoint waitWaypoint = SCR_TimedWaypoint.Cast(SpawnWaypoint(m_WaitWaypoint, waypointPosition));
			waitWaypoint.SetHoldingTime(gen.RandFloatXY(5, 15));
			waypoints.Insert(waitWaypoint);
			group.AddWaypoint(waitWaypoint);
		}

		AIWaypointCycle cycleWaypoint = AIWaypointCycle.Cast(SpawnWaypoint(m_CycleWaypoint, group.GetOrigin()));
		cycleWaypoint.SetWaypoints(waypoints);
		group.AddWaypoint(cycleWaypoint);
		waypoints.Insert(cycleWaypoint);

		return waypoints;
	}

	array<AIWaypoint> VehiclePatrol(AIGroup group, vector center, float patrolRadius)
	{
		RandomGenerator gen = ESCT_EscapistsGameMode.GetGameMode().GetRandomGenerator();
		array<float> radians =
		{
			gen.RandFloatXY(0, 45.0) * Math.DEG2RAD,
			gen.RandFloatXY(60.0, 120.0) * Math.DEG2RAD,
			gen.RandFloatXY(150.0, 210.0) * Math.DEG2RAD,
			gen.RandFloatXY(240.0, 300.0) * Math.DEG2RAD
		};

		array<vector> vectors = {};
		for (int i = 0; i < radians.Count(); i++)
		{
			float radius = gen.RandFloatXY(patrolRadius - 50, patrolRadius + 50);
			float radian = radians[i];
			vectors.Insert(Vector(center[0] + radius * Math.Cos(radian), 15.0, center[2] + radius * Math.Sin(radian)));
		}

		array<AIWaypoint> waypoints = {};
		foreach(vector position : vectors)
		{
			vector waypointPosition = ESCT_RoadNavigation.GetClosestRoadPosition(position);
			if (waypointPosition == vector.Zero)
				continue;
			
			AIWaypoint patrolWaypoint = SpawnWaypoint(m_PatrolWaypoint, waypointPosition);
			if (!patrolWaypoint)
				continue;
			waypoints.Insert(patrolWaypoint);
			group.AddWaypoint(patrolWaypoint);
			
			SCR_TimedWaypoint waitWaypoint = SCR_TimedWaypoint.Cast(SpawnWaypoint(m_WaitWaypoint, waypointPosition));
			if (!waitWaypoint)
				continue;
			waitWaypoint.SetHoldingTime(gen.RandFloatXY(5, 15));
			waypoints.Insert(waitWaypoint);
			group.AddWaypoint(waitWaypoint);
		}
		
		if (waypoints.IsEmpty())
			return waypoints;
		
		AIWaypointCycle cycleWaypoint = AIWaypointCycle.Cast(SpawnWaypoint(m_CycleWaypoint, group.GetOrigin()));
		cycleWaypoint.SetWaypoints(waypoints);
		group.AddWaypoint(cycleWaypoint);
		waypoints.Insert(cycleWaypoint);

		return waypoints;
	}

	private AIWaypoint SpawnWaypoint(ResourceName resource, vector position)
	{
		return AIWaypoint.Cast(ESCT_SpawnHelpers.SpawnEntityPrefab(resource, position));
	}
}
