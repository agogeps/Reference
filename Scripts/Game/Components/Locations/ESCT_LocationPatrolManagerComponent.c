[EntityEditorProps("Escapists/Location", description: "This component handles location patrols.", color: "0 0 255 255")]
class ESCT_LocationPatrolManagerComponentClass : ESCT_LocationComponentClass
{
}

class ESCT_LocationPatrolManagerComponent : ESCT_LocationComponent
{
	[Attribute("0", UIWidgets.Auto, desc: "If not ticked, the ordinary group will try to use location patrol points, if yes - additional group will be spawned for that.", category: "Escapists")]
	protected bool m_bSpawnAdditionalGroupForPoints;
	
	[Attribute(defvalue: "", uiwidget: UIWidgets.Auto, desc: "Custom patrol waypoints for location.", category: "Escapists")]
	protected ref array<ref ESCT_LocationPatrolPoint> m_aLocationPatrolPoints;

	[Attribute(defvalue: "", uiwidget: UIWidgets.Auto, desc: "Custom defend waypoint for location.", category: "Escapists")]
	protected ref ESCT_LocationPatrolPoint m_DefendPoint;

	[Attribute(defvalue: "2", uiwidget: UIWidgets.Slider, desc: "How many units needs to be downed before location will be persistently cleared.", params: "1 30 1", category: "Escapists")]
	protected int m_iClearThreshold;

	protected bool m_bIsCleared = false;
	protected ref array<SCR_AIGroup> m_aGroups = {};

	private static const float PATROL_SPAWN_RADIUS = 50.0;
	private static const float PATROL_QUERY_SPACE = 1.5;

	void SetSpawnAdditionalGroup(bool value)
	{
		m_bSpawnAdditionalGroupForPoints = value;
	} 
	
	void SetDefendPoint(ESCT_LocationPatrolPoint defendPoint)
	{
		m_DefendPoint = defendPoint;
	}

	void SetLocationPatrolPoints(array<ref ESCT_LocationPatrolPoint> points)
	{
		m_aLocationPatrolPoints = points;
	}

	int GetClearThreshold()
	{
		return m_iClearThreshold;
	}

	void ClearGroup(SCR_AIGroup group)
	{
		if (!group)
		{
			m_Location.GetOnEntityChange().Invoke(-1);
			return;
		}

		m_aGroups.RemoveItem(group);
		m_Location.GetOnEntityChange().Invoke(-1);

		if (m_aGroups.IsEmpty())
			m_bIsCleared = true;
	}

	override void Spawn()
	{
		if (m_bIsCleared)
			return;

		ESCT_WaypointManagerComponent waypointManager = ESCT_WaypointManagerComponent.GetInstance();
		if (!waypointManager)
			return;

		vector locationPosition = m_Location.GetOrigin();

		if (m_bSpawnAdditionalGroupForPoints)
		{
			PatrolLocation(waypointManager, locationPosition);
			PatrolPoints(waypointManager);
			PatrolDefend(waypointManager);
		}
		else
		{
			if (!m_aLocationPatrolPoints.IsEmpty())
			{
				PatrolPoints(waypointManager);
			}
			else if (m_DefendPoint)
			{
				PatrolDefend(waypointManager);
			}
			else 
			{
				PatrolLocation(waypointManager, locationPosition);
			}
		}
	}

	override void Despawn()
	{
		if (m_bIsCleared)
			return;

		foreach (SCR_AIGroup group : m_aGroups)
		{
			if (!group)
				continue;

			SCR_EntityHelper.DeleteEntityAndChildren(group);
		}
		m_aGroups.Clear();
	}
	
	protected ResourceName SelectGroupPrefab(notnull ESCT_Faction faction)
	{
		ESCT_EGroupSize groupSize = ESCT_GroupDirector.GetGroupSize(m_Location.GetLocationType());
		if (groupSize == ESCT_EGroupSize.Undefined)
			return ResourceName.Empty;
		
		array<ESCT_EMilitaryBranchType> branches = {ESCT_EMilitaryBranchType.SF, ESCT_EMilitaryBranchType.Military};
		int index = SCR_ArrayHelper.GetWeightedIndex({10, 35}, Math.RandomFloat01());

		return faction.GetRandomGroupOfBranchAndSize(branches[index], groupSize);
	}

	private SCR_AIGroup CreatePatrolGroup()
	{
		ESCT_EGroupSize groupSize = ESCT_GroupDirector.GetGroupSize(m_Location.GetLocationType());
		if (groupSize == ESCT_EGroupSize.Undefined)
			return null;

		ESCT_Faction faction = m_Location.GetOwnerFaction();
		if (!faction)
			return null;
		
		ResourceName groupPrefab = SelectGroupPrefab(faction);
		if (!groupPrefab || groupPrefab == ResourceName.Empty)
			return null;

		vector patrolPosition = ESCT_Misc.GetRandomValidPosition(m_Location.GetOrigin(), 0, 30, PATROL_SPAWN_RADIUS, PATROL_QUERY_SPACE);
		if (patrolPosition == vector.Zero)
		{
			patrolPosition = ESCT_EscapistsGameMode.GetGameMode().GetRandomGenerator().GenerateRandomPointInRadius(0, 25, m_Location.GetOrigin());
			patrolPosition[1] = SCR_TerrainHelper.GetTerrainY(patrolPosition, noUnderwater: false);
		}

		IEntity patrol = ESCT_SpawnHelpers.SpawnEntityPrefab(groupPrefab, patrolPosition);
		SCR_AIGroup group = SCR_AIGroup.Cast(patrol);
		if (!group)
			return null;

		group.GetOnAgentAdded().Insert(RegisterPatroller);
		
		m_aGroups.Insert(group);
		m_Location.ChangeEntitiesToClear(1);

		return group;
	}

	private void PatrolPoints(ESCT_WaypointManagerComponent waypointManager)
	{
		if (m_aLocationPatrolPoints.IsEmpty())
			return;
		
		SCR_AIGroup patrolGroup = CreatePatrolGroup();
		if (!patrolGroup)
			return;

		foreach (ESCT_LocationPatrolPoint point : m_aLocationPatrolPoints)
		{
			vector waypointPosition = point.GetWaypointWorldPosition(GetOwner());
			AIWaypoint patrol = waypointManager.PatrolSingle(patrolGroup, waypointPosition);
			if (patrol && point.GetWaypointRadius() != -1)
				patrol.SetCompletionRadius(point.GetWaypointRadius());
		}

		waypointManager.Cycle(patrolGroup);
	}

	private void PatrolLocation(ESCT_WaypointManagerComponent waypointManager, vector locationPosition)
	{
		if (m_Location.GetLocationType() == ESCT_ELocationType.Patrol)
			return;
		
		SCR_AIGroup group = CreatePatrolGroup();
		if (!group)
			return;
		
		if (m_Location.GetLocationType() == ESCT_ELocationType.City)
			waypointManager.Patrol(group, locationPosition, Math.RandomFloat(50, 100));
		else
		{
			float defendRadius = GetDefendRadius();
			waypointManager.DefendPosition(group, locationPosition, defendRadius, priority: 30);
		}
	}

	private void PatrolDefend(ESCT_WaypointManagerComponent waypointManager)
	{
		if (!m_DefendPoint)
			return;
		
		SCR_AIGroup defendGroup = CreatePatrolGroup();
		if (!defendGroup)
			return;

		vector waypointPosition = m_DefendPoint.GetWaypointWorldPosition(GetOwner());
		AIWaypoint defend = waypointManager.DefendPosition(defendGroup, waypointPosition, priority: 30);
		if (defend && m_DefendPoint.GetWaypointRadius() != -1)
			defend.SetCompletionRadius(m_DefendPoint.GetWaypointRadius());
	}

	private float GetDefendRadius()
	{
		float defendRadius = 0;

		switch (m_Location.GetLocationType())
		{
			case ESCT_ELocationType.Checkpoint:
			{
				defendRadius = 10;
				break;
			}
			case ESCT_ELocationType.City:
			{
				defendRadius = Math.RandomFloat(50, 150);
				break;
			}
			default:
			{
				defendRadius = Math.RandomFloat(20, 50);
				break;
			}
		}

		return defendRadius;
	}

	private void RegisterPatroller(AIAgent aiAgent)
	{
		if (!aiAgent)
			return;

		IEntity charEntity = aiAgent.GetControlledEntity();
		if (!charEntity)
			return;

		ESCT_LocationPatrollerComponent patrollerComponent = ESCT_LocationPatrollerComponent.Cast(charEntity.FindComponent(ESCT_LocationPatrollerComponent));
		if (!patrollerComponent)
			return;

		SCR_AIGroup group = ESCT_CharacterHelper.GetAIGroup(charEntity);
		if (!group)
			return;

		patrollerComponent.Register(this, group);
	}
	
#ifdef WORKBENCH
	protected const float _DRAW_DISTANCE = 500 * 500;

	override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
	{
		super._WB_AfterWorldUpdate(owner, timeSlice);

		if (!m_aLocationPatrolPoints || !m_Location)
			return;

		GenericEntity genericOwner = GenericEntity.Cast(owner);
		if (!genericOwner)
			return;

		WorldEditorAPI api = genericOwner._WB_GetEditorAPI();
		if (!api)
			return;

		IEntitySource src = api.EntityToSource(owner);
		if (!src)
			return;
//
//		if (!api.IsEntityVisible(src) || !api.IsEntityLayerVisible(src.GetSubScene(), src.GetLayerID()))
//			return;
//
		vector cameraTransform[4];
		owner.GetWorld().GetCurrentCamera(cameraTransform);

		int colorBackground = 0x00000000;
		if (api.IsEntitySelected(src))
		{
			colorBackground = ARGBF(1, 0, 0, 0);
		}
		else
		{
			//--- Draw only if near when not selected
			if (vector.DistanceSq(owner.GetOrigin(), cameraTransform[3]) > _DRAW_DISTANCE)
				return;
		}

		Color color;

		switch (m_Location.GetLocationType())
		{
			case ESCT_ELocationType.Checkpoint:
			{
				color = Color.DarkYellow;
				break;
			}
			case ESCT_ELocationType.Helipad:
			{
				color = Color.Blue;
				break;
			}
			case ESCT_ELocationType.RadioStation:
			{
				color = Color.Yellow;
				break;
			}
			case ESCT_ELocationType.StartArea:
			{
				color = Color.Cyan;
				break;
			}
			case ESCT_ELocationType.City:
			{
				color = Color.Pink;
				break;
			}
			case ESCT_ELocationType.Hospital:
			{
				color = Color.Red;
				break;
			}
			case ESCT_ELocationType.Outpost:
			{
				color = Color.Violet;
				break;
			}
			case ESCT_ELocationType.Hideout:
			{
				color = Color.Green;
				break;
			}
			case ESCT_ELocationType.Patrol:
			{
				color = Color.DodgerBlue;
				break;
			}
			case ESCT_ELocationType.VehicleDepot:
			{
				color = Color.Chartreuse;
				break;
			}
			default:
			{
				color = Color.White;
				break;
			}
		}

		int index = 1;
		foreach (ESCT_LocationPatrolPoint point : m_aLocationPatrolPoints)
		{
			string text = string.Format("Patrol #%1", index);
			CreateWaypointPresentation(point, owner, color, colorBackground, text);
			index++;
		}

		if (m_DefendPoint)
			CreateWaypointPresentation(m_DefendPoint, owner, color, colorBackground, "Defend");

	}

	private void CreateWaypointPresentation(ESCT_LocationPatrolPoint point, IEntity owner, Color color, int colorBackground, string text)
	{
		vector pointPosition = point.GetWaypointWorldPosition(GetOwner());
		string pointText = text;

		Color newColor = new Color(color.R(), color.G(), color.B(), 0.75);

		ref DebugTextWorldSpace textShape = DebugTextWorldSpace.Create(
			owner.GetWorld(),
			pointText,
			DebugTextFlags.ONCE | DebugTextFlags.CENTER | DebugTextFlags.FACE_CAMERA,
			pointPosition[0],
			pointPosition[1] + 5,
			pointPosition[2],
			18.0,
			newColor.PackToInt(),
			colorBackground
		);

		Color transparentColor = new Color(color.R(), color.G(), color.B(), 0.3);
		float radius;
		if (point.GetWaypointRadius() != -1) {radius = point.GetWaypointRadius()} else {radius = 4}

		ref Shape dbgShapeCenter = Shape.CreateCylinder(
			transparentColor.PackToInt(),
			ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOZWRITE | ShapeFlags.ONCE | ShapeFlags.NOOUTLINE,
			pointPosition,
			0.25,
			10
		);

		ref Shape dbgShapeOuter = Shape.CreateCylinder(
			transparentColor.PackToInt(),
			ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOZWRITE | ShapeFlags.ONCE | ShapeFlags.NOOUTLINE,
			pointPosition,
			radius,
			2
		);
	}

#endif
}
