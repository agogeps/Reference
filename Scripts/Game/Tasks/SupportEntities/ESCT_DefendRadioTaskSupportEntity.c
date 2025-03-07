[EntityEditorProps(category: "Escapists/Tasks/SupportEntities", description: "Support entity for defend radio task.", color: "0 0 255 255")]
class ESCT_DefendRadioTaskSupportEntityClass : ESCT_BaseTaskSupportEntityClass
{
}

sealed class ESCT_DefendRadioTaskSupportEntity : ESCT_BaseTaskSupportEntity
{
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Task presentation prefab.", params: "et", category: "Escapists")]
	protected ResourceName m_TaskPresentationPrefab;
	
	protected ESCT_DefendRadioTaskPresentation m_TaskPresentation;
	
	protected ESCT_Radio m_Radio;
	protected vector m_vTaskPosition;
	protected ref ESCT_Faction m_Faction;

	protected ESCT_WaypointManagerComponent m_WaypointManager;

	private ref array<SCR_AIGroup> m_aGroups = {};
	private SCR_AIGroup m_SuppressionGroup;

	private static const float PATROL_SPAWN_RADIUS = 50.0;
	private static const float PATROL_QUERY_SPACE = 5;
	private static const int DESPAWN_DISTANCE = 1000;
	
	ESCT_Radio GetRadio()
	{
		return m_Radio;
	}

	vector GetRadioPosition()
	{
		if (!m_Radio)
			return vector.Zero;

		return m_Radio.GetOrigin();
	}

	SCR_BaseTask CreateDefendTask(ESCT_Radio radio, vector position, bool shouldAssign = true)
	{
		m_Radio = radio;
		m_vTaskPosition = position;
		
		ESCT_DefendRadioTask defendTask = ESCT_DefendRadioTask.Cast(super.CreateTask(radio, shouldAssign));
		if (!defendTask)
			return null;

		m_WaypointManager = ESCT_WaypointManagerComponent.GetInstance();

		AssignFaction();
		SetupAttackers();
		
		if (m_TaskPresentationPrefab)
		{
	
			m_TaskPresentation = ESCT_DefendRadioTaskPresentation.Cast(GetGame().SpawnEntityPrefab(Resource.Load(m_TaskPresentationPrefab), GetGame().GetWorld()));
			m_TaskPresentation.Setup();
		}
		
		return defendTask;
	}

	override void FinishTask(notnull SCR_BaseTask task)
	{
		super.FinishTask(task);
		
		ClearPresentation();

		//stakes should a bit higher and patrols and events should be more frequent
		ESCT_AmbientEventSystem.GetSystem().IncreaseEventFrequency();

		GetGame().GetCallqueue().CallLater(CheckDespawn, 10000, true);
	}
	
	private void ClearPresentation()
	{
		if (!m_TaskPresentation)
			return;
		
		m_TaskPresentation.Clear();
	}

	private void AssignFaction()
	{
		ESCT_EscapistsConfigComponent config = ESCT_EscapistsConfigComponent.GetInstance();
		if (!config)
			return;

		m_Faction = ESCT_FactionManager.GetInstance().GetEnemyFaction();
	}

	private void SetupAttackers()
	{
		if (!m_Faction)
			return;

		ESCT_EscapistsGameMode gameMode = ESCT_EscapistsGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;

		vector radioPosition = GetRadioPosition();
		if (radioPosition == vector.Zero)
			return;

		vector spawnPosition = ESCT_Misc.GetRandomValidPosition(radioPosition, 200, 350, PATROL_SPAWN_RADIUS, PATROL_QUERY_SPACE);
		if (spawnPosition == vector.Zero)
		{
			spawnPosition = gameMode.GetRandomGenerator().GenerateRandomPointInRadius(200, 350, radioPosition);
			spawnPosition[1] = SCR_TerrainHelper.GetTerrainY(spawnPosition);
		}

		vector movePosition = (spawnPosition + radioPosition) * 0.5;
		movePosition[1] = SCR_TerrainHelper.GetTerrainY(movePosition, GetWorld(), noUnderwater: true);
		
		GetGame().GetCallqueue().CallLater(CreateAttackersByFrameSequence, param1: spawnPosition, param2: movePosition, param3: radioPosition);
	}

	private SCR_AIGroup CreateGroup(vector position)
	{
		if (position == vector.Zero)
			return null;

		ESCT_EGroupSize groupSize = ESCT_GroupDirector.GetGroupSize(ESCT_ELocationType.Helipad, true);
		if (groupSize == ESCT_EGroupSize.Undefined)
			return null;

		array<ESCT_EMilitaryBranchType> branchTypes = { ESCT_EMilitaryBranchType.Military, ESCT_EMilitaryBranchType.SF };
		int index = SCR_ArrayHelper.GetWeightedIndex({20, 25}, Math.RandomFloat01());

		ResourceName groupPrefab = m_Faction.GetRandomGroupOfBranchAndSize(branchTypes[index], groupSize);
		if (!groupPrefab || groupPrefab == ResourceName.Empty)
			return null;

		IEntity entity = ESCT_SpawnHelpers.SpawnEntityPrefab(groupPrefab, position);
		SCR_AIGroup group = SCR_AIGroup.Cast(entity);
		if (!group)
			return null;

		m_aGroups.Insert(group);

		return group;
	}
	
	private void CreateAttackersByFrameSequence(vector spawnPosition, vector movePosition, vector radioPosition)
	{
		m_SuppressionGroup = CreateGroup(spawnPosition);
		if (m_SuppressionGroup)
		{
			if (!ESCT_Misc.IsPositionInWater(movePosition) && movePosition != vector.Zero)
				m_WaypointManager.Move(m_SuppressionGroup, movePosition, 15);

			m_WaypointManager.SearchAndDestroy(m_SuppressionGroup, radioPosition);
			GetGame().GetCallqueue().CallLater(CheckForSuppressionInfantry, 4000, repeat: true);
		}
		
		GetGame().GetCallqueue().CallLater(CreateSndGroup, param1: spawnPosition, param2: movePosition, param3: radioPosition);
	}
	
	private void CreateSndGroup(vector spawnPosition, vector movePosition, vector radioPosition)
	{
		SCR_AIGroup sndGroup = CreateGroup(spawnPosition);
		if (sndGroup)
		{
			if (!ESCT_Misc.IsPositionInWater(movePosition) && movePosition != vector.Zero)
				m_WaypointManager.Move(sndGroup, movePosition, 15);

			m_WaypointManager.SearchAndDestroy(sndGroup, radioPosition);
		}
		
		GetGame().GetCallqueue().CallLater(CreateSupport);
	}
	
	private void CreateSupport()
	{
		ESCT_SupportSystem supportSystem = ESCT_SupportSystem.GetSystem();
		if (!supportSystem)
		{
			ESCT_Logger.Error("Can't call for support on defend radio task - Support System wasn't found! Check if call was made by the server!");
			return;
		}
		
		supportSystem.CreateSupport(null, GetRadioPosition(), desiredSupportType: ESCT_ESupportType.QRF_VEHICLE, isSilent: true);
	}

	private void CheckForSuppressionInfantry()
	{
		if (!m_SuppressionGroup)
		{
			GetGame().GetCallqueue().Remove(CheckForSuppressionInfantry);
			return;
		}

		vector radioPosition = GetRadioPosition();
		if (radioPosition == vector.Zero)
			return;

		if (ESCT_Math.IsPointInRadiusXZ(radioPosition, m_SuppressionGroup.GetOrigin(), 35))
		{
			GetGame().GetCallqueue().Remove(CheckForSuppressionInfantry);
			return;
		}

		IEntity leader = m_SuppressionGroup.GetLeaderEntity();
		if (!leader || !ESCT_CharacterHelper.CanFight(leader))
		{
			GetGame().GetCallqueue().Remove(CheckForSuppressionInfantry);
			return;
		}

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(leader);
		if (!character)
		{
			GetGame().GetCallqueue().Remove(CheckForSuppressionInfantry);
			return;
		}

		autoptr TraceParam trace = new TraceParam();
		trace.Flags = TraceFlags.WORLD | TraceFlags.OCEAN;
		trace.Start = character.EyePosition();
		trace.End = radioPosition;

		if (vector.DistanceXZ(trace.Start, trace.End) > 200)
			return;

		float traceScore = GetGame().GetWorld().TraceMove(trace, null);
		ESCT_Logger.InfoFormat("Defend Radio Task suppression trace result: %1, distance: %2", traceScore.ToString(), vector.Distance(trace.Start, trace.End).ToString());
		if (traceScore <= 0.75) //0.998?
			return;

		array<AIWaypoint> waypoints = {};
		m_SuppressionGroup.GetWaypoints(waypoints);
		for (int i = 0; i < waypoints.Count(); i++)
		{
			m_SuppressionGroup.RemoveWaypointAt(0);
		}

		AssignInfantryWaypoints();

		GetGame().GetCallqueue().Remove(CheckForSuppressionInfantry);
	}

	private void AssignInfantryWaypoints()
	{
		if (!m_SuppressionGroup)
			return;

		vector radioPosition = GetRadioPosition();

		m_WaypointManager.Suppress(m_SuppressionGroup, radioPosition);
		m_WaypointManager.SearchAndDestroy(m_SuppressionGroup, radioPosition, holdingTime: Math.RandomFloat(60.0, 180.0));
	}

	private void CheckDespawn()
	{
		bool isPlayerNear = false;

		array<IEntity> players = {};
		ESCT_Player.GetAlivePlayers(players, true);

		for (int i = 0; i < players.Count(); i++)
		{
			if (ESCT_Math.IsPointInRadiusXZ(players[i].GetOrigin(), m_vTaskPosition, DESPAWN_DISTANCE))
			{
				isPlayerNear = true;
				break;
			}
		}

		if (isPlayerNear)
			return;

		foreach (SCR_AIGroup group : m_aGroups)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(group);
		}
		m_aGroups.Clear();
		
		if (m_TaskPresentation)
			SCR_EntityHelper.DeleteEntityAndChildren(m_TaskPresentation);

		GetGame().GetCallqueue().Remove(CheckDespawn);
	}
}
