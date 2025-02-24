[EntityEditorProps(category: "Escapists/Entities", description: "QRF support.")]
class ESCT_QRFVehicleSupportClass : ESCT_SupportBaseClass
{
}

sealed class ESCT_QRFVehicleSupport : ESCT_SupportBase
{
	[Attribute("0", UIWidgets.ComboBox, "Select Entity Labels which you want to optionally include to random spawn. If you want to spawn everything, you can leave it out empty.", "", ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected ref array<EEditableEntityLabel> m_aIncludedEditableEntityLabels;

	[Attribute("0", UIWidgets.ComboBox, "Select Entity Labels which you want to exclude from random spawn.", "", ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected ref array<EEditableEntityLabel> m_aExcludedEditableEntityLabels;

	[Attribute("0", desc: "If true, only assets with ALL of provided included labels will be used.")]
	protected bool m_bRequireAllIncludedLabels;

	protected SCR_AIGroup m_FireteamGroup;
	protected AIGroup m_QrfGroup;
	protected IEntity m_Vehicle;

	private ESCT_Faction m_Faction;
	private ResourceName m_vehiclePrefab;
	private vector m_CurrentTargetPos;
	private IEntity m_Gunner;
	private SCR_AIWaypoint m_MoveWaypoint;

	private SCR_BaseCompartmentManagerComponent m_VehicleCompartmentManager;
	private ESCT_WaypointManagerComponent m_WaypointManager;
	private BaseWorld m_BaseWorld;

	private ref array<IEntity> m_aTargets = {};

	override bool Setup(IEntity target, vector targetPosition = vector.Zero, IEntity caller = null, bool followTarget = false)
	{
		if (!super.Setup(target, targetPosition, caller, followTarget))
			return false;

		if (m_eSupportState != ESCT_ESupportState.New)
			return false;

		ESCT_EscapistsConfigComponent gameConfig = ESCT_EscapistsConfigComponent.GetInstance();
		if (!gameConfig)
			return false;

		m_WaypointManager = ESCT_WaypointManagerComponent.GetInstance();
		if (!m_WaypointManager)
			return false;

		m_BaseWorld = GetGame().GetWorld();
		if (!m_BaseWorld)
			return false;

		m_Faction = ESCT_FactionManager.GetInstance().GetEnemyFaction();

		SCR_EntityCatalog entityCatalog = m_Faction.GetFactionEntityCatalogOfType(EEntityCatalogType.VEHICLE);
		if (!entityCatalog)
			return false;

		array<SCR_EntityCatalogEntry> data = {};
		entityCatalog.GetFullFilteredEntityListWithLabels(data, m_aIncludedEditableEntityLabels, m_aExcludedEditableEntityLabels, needsAllIncluded: m_bRequireAllIncludedLabels);
		if (data.IsEmpty())
			return false;

		Math.Randomize(-1);
		SCR_EntityCatalogEntry catalogEntry = data.GetRandomElement();
		m_vehiclePrefab = catalogEntry.GetPrefab();

		m_eSupportState = ESCT_ESupportState.Ready;

		return true;
	}

	override bool Launch()
	{
		if (!super.Launch())
			return false;

		ESCT_EscapistsGameMode gameMode = ESCT_EscapistsGameMode.Cast(GetGame().GetGameMode());
		vector initialSearchPosition = gameMode.GetRandomGenerator().GenerateRandomPointInRadius(300, 600, m_vTargetPosition);
		initialSearchPosition[1] = SCR_TerrainHelper.GetTerrainY(initialSearchPosition, noUnderwater: false);

		vector spawnPosition, spawnDirection;
		ESCT_RoadNavigation.SelectRoadSegment(initialSearchPosition, spawnPosition, spawnDirection, facePosition: false);
		if (spawnPosition == vector.Zero || spawnDirection == vector.Zero || vector.Distance(spawnPosition, m_vTargetPosition) < 100)
		{
			spawnPosition = ESCT_Misc.GetRandomPositionForVehicle(initialSearchPosition, 300, 600);
			spawnDirection = vector.Direction(spawnPosition, m_vTargetPosition);
		}
		else
		{
			vector roadToDestination = (m_vTargetPosition - spawnPosition);
			roadToDestination.Normalize();
			if (vector.Dot(spawnDirection, roadToDestination) < 0) spawnDirection = -spawnDirection;
		}

		if (spawnPosition == vector.Zero)
			return false;

		m_Vehicle = ESCT_SpawnHelpers.SpawnVehiclePrefab(m_vehiclePrefab, spawnPosition, spawnDirection);
		if (!m_Vehicle)
			return false;

		m_VehicleCompartmentManager = SCR_BaseCompartmentManagerComponent.Cast(m_Vehicle.FindComponent(SCR_BaseCompartmentManagerComponent));
		if (!m_VehicleCompartmentManager)
			return false;

		SpawnVehicleOccupants();
		GetGame().GetCallqueue().CallLater(AssignWaypoints, 4000, false);

		return true;
	}

	override void Despawn()
	{
		if (m_QrfGroup)
		{
			array<AIAgent> agents = {};
			m_QrfGroup.GetAgents(agents);

			for (int i = 0; i < agents.Count(); i++)
			{
				IEntity agentEntity = agents[i].GetControlledEntity();
				if (!agentEntity)
					continue;

				SCR_CharacterControllerComponent charController = SCR_CharacterControllerComponent.Cast(agentEntity.FindComponent(SCR_CharacterControllerComponent));
				if (!charController)
					continue;

				//bodies will be handled by garbage collector, alive soldiers should be explicitly deleted
				ECharacterLifeState lifeState = charController.GetLifeState();
				if (lifeState != ECharacterLifeState.DEAD)
					SCR_EntityHelper.DeleteEntityAndChildren(agentEntity);
			}

			delete m_QrfGroup;
		}

		if (m_FireteamGroup)
		{
			array<AIAgent> agents = {};
			m_FireteamGroup.GetAgents(agents);

			for (int i = 0; i < agents.Count(); i++)
			{
				IEntity agentEntity = agents[i].GetControlledEntity();
				if (!agentEntity)
					continue;

				SCR_CharacterControllerComponent charController = SCR_CharacterControllerComponent.Cast(agentEntity.FindComponent(SCR_CharacterControllerComponent));
				if (!charController)
					continue;

				//bodies will be handled by garbage collector, alive soldiers should be explicitly deleted
				ECharacterLifeState lifeState = charController.GetLifeState();
				if (lifeState != ECharacterLifeState.DEAD)
					SCR_EntityHelper.DeleteEntityAndChildren(agentEntity);
			}

			delete m_QrfGroup;
		}

		super.Despawn();
	}

	override bool CheckDespawn()
	{
		if (HasGracePeriod())
			return false;

		if (super.CheckDespawn())
			return true;

		if (!m_QrfGroup && !m_FireteamGroup)
			return true;

		return false;
	}

	private void AssignWaypoints()
	{
		if (!m_Vehicle || !m_QrfGroup)
			return;

		//QRF group should move with SND purpose until they see the point, then things happen
		SCR_AIGroup qrfGroup = SCR_AIGroup.Cast(m_QrfGroup);
		m_MoveWaypoint = m_WaypointManager.Move(qrfGroup, m_vTargetPosition, 10, priority: 2000);
		qrfGroup.GetOnWaypointCompleted().Insert(OnWaypointCompleted);

		array<IEntity> gunners = {};
		m_VehicleCompartmentManager.GetOccupantsOfType(gunners, ECompartmentType.TURRET);
		if (!gunners.IsEmpty())
			m_Gunner = gunners[0];

		GetGame().GetCallqueue().CallLater(CheckForSuppression, 4000, repeat: true);
	}

	private void CheckForSuppression()
	{
		if (!m_Vehicle || !m_QrfGroup || !m_Gunner || GetSupportState() != ESCT_ESupportState.InProgress)
		{
			GetGame().GetCallqueue().Remove(CheckForSuppression);
			return;
		}

		if (!ESCT_CharacterHelper.CanFight(m_Gunner))
		{
			GetGame().GetCallqueue().Remove(CheckForSuppression);
			return;
		}

		CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(m_Gunner.FindComponent(CompartmentAccessComponent));
		if (compartmentAccess && !compartmentAccess.IsInCompartment())
		{
			GetGame().GetCallqueue().Remove(CheckForSuppression);
			return;
		}

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(m_Gunner);
		if (!character)
		{
			GetGame().GetCallqueue().Remove(CheckForSuppression);
			return;
		}

		if (ESCT_Math.IsPointInRadiusXZ(m_vTargetPosition, m_Vehicle.GetOrigin(), 35))
		{
			GetGame().GetCallqueue().Remove(CheckForSuppression);
			return;
		}

		array<IEntity> occupants = {};
		m_VehicleCompartmentManager.GetOccupants(occupants);
		if (occupants.IsEmpty())
		{
			GetGame().GetCallqueue().Remove(CheckForSuppression);
			return;
		}

		m_aTargets.Clear();
		m_BaseWorld.QueryEntitiesBySphere(m_vTargetPosition, 35, InsertEntity, FilterEntities, queryFlags: EQueryEntitiesFlags.DYNAMIC);

		if (m_aTargets.IsEmpty())
			return;

		bool hasVisibleTarget = false;
		float traceScore;
		IEntity target = null;
		autoptr TraceParam trace = new TraceParam();
		trace.Flags = TraceFlags.WORLD | TraceFlags.OCEAN | TraceFlags.ENTS | TraceFlags.VISIBILITY;
		trace.Start = character.EyePosition();

		for (int i = 0; i < m_aTargets.Count(); i++)
		{
			target = m_aTargets[i];
			if (!target)
				continue;

			trace.End = target.GetOrigin();

			if (vector.DistanceXZ(trace.Start, trace.End) > 200)
				continue;

			traceScore = GetGame().GetWorld().TraceMove(trace, null);
			//ESCT_Logger.InfoFormat("Trace Target Position result: %1, distance: %2", traceScore.ToString(), vector.Distance(trace.Start, trace.End).ToString());

			if (traceScore <= 0.75) //0.998?
				continue;

			hasVisibleTarget = true;
			break;
		}

		if (!hasVisibleTarget)
			return;

		array<AIWaypoint> waypoints = {};
		m_QrfGroup.GetWaypoints(waypoints);
		for (int i = 0; i < waypoints.Count(); i++)
		{
			m_QrfGroup.RemoveWaypointAt(0);
		}

		//wait for driver to hit the brakes
		GetGame().GetCallqueue().CallLater(GetOutOccupants, 3000);
		GetGame().GetCallqueue().Remove(CheckForSuppression);
	}

	private void GetOutOccupants()
	{
		if (!m_Vehicle || !m_QrfGroup || GetSupportState() != ESCT_ESupportState.InProgress)
			return;

		//radio group because there might be random guys with RTO capabilities
		m_FireteamGroup = ESCT_CharacterHelper.CreateGroup(m_Faction.GetFactionKey(), m_Faction.GetBaseFactionRadioGroup());
		if (!m_FireteamGroup)
			return;

		array<IEntity> occupants = {};
		m_VehicleCompartmentManager.GetOccupantsOfType(occupants, ECompartmentType.CARGO);

		foreach (IEntity occupant : occupants)
		{
			if (!ESCT_CharacterHelper.CanFight(occupant))
				continue;

			ChimeraAIAgent agent = ESCT_CharacterHelper.GetAIAgent(occupant);
			if (!agent)
				continue;

			m_FireteamGroup.AddAgent(agent);

			CompartmentAccessComponent occAccess = CompartmentAccessComponent.Cast(occupant.FindComponent(CompartmentAccessComponent));
			if (!occAccess || !occAccess.IsInCompartment() || !occAccess.CanGetOutVehicle())
				continue;

			occAccess.GetOutVehicle(EGetOutType.ANIMATED, -1, ECloseDoorAfterActions.CLOSE_DOOR, true);
		}

		GetGame().GetCallqueue().CallLater(AssignBattleWaypoints, 4600);
	}

	private void AssignBattleWaypoints()
	{
		if (!m_Vehicle || !m_QrfGroup || GetSupportState() != ESCT_ESupportState.InProgress)
			return;

		SCR_AIGroup qrfGroup = SCR_AIGroup.Cast(m_QrfGroup);

		m_WaypointManager.Suppress(qrfGroup, m_vTargetPosition);
		m_WaypointManager.SearchAndDestroy(qrfGroup, m_vTargetPosition, holdingTime: Math.RandomFloat(60.0, 180.0));

		if (m_FireteamGroup)
		{
			vector movePosition = (m_Vehicle.GetOrigin() + m_vTargetPosition) * 0.5;
			movePosition[1] = SCR_TerrainHelper.GetTerrainY(movePosition, GetWorld(), noUnderwater: true);
			if (!ESCT_Misc.IsPositionInWater(movePosition) && movePosition != vector.Zero)
				m_WaypointManager.Move(m_FireteamGroup, movePosition, 10);

			m_WaypointManager.Suppress(m_FireteamGroup, m_vTargetPosition);
			m_WaypointManager.SearchAndDestroy(m_FireteamGroup, m_vTargetPosition, holdingTime: Math.RandomFloat(60.0, 180.0));
		}
	}

	private void SpawnVehicleOccupants()
	{
		if (!m_Vehicle || !m_VehicleCompartmentManager)
			return;

		array<BaseCompartmentSlot> compartments = {};
		m_VehicleCompartmentManager.GetFreeCompartmentsOfType(compartments, ECompartmentType.CARGO);
		m_VehicleCompartmentManager.GetFreeCompartmentsOfType(compartments, ECompartmentType.PILOT);
		m_VehicleCompartmentManager.GetFreeCompartmentsOfType(compartments, ECompartmentType.TURRET);

		if (compartments.IsEmpty())
			return;

		m_QrfGroup = ESCT_CharacterHelper.CreateGroup(m_Faction.GetFactionKey(), m_Faction.GetBaseFactionRadioGroup());
		SCR_AIGroup qrfGroup = SCR_AIGroup.Cast(m_QrfGroup);
		if (qrfGroup)
			qrfGroup.GetOnAgentAdded().Insert(RegisterAgent);

		foreach (BaseCompartmentSlot slot : compartments)
		{
			ResourceName characterPrefab = m_Faction.GetRandomCharacterOfBranch(ESCT_EMilitaryBranchType.Military);
			if (!characterPrefab || characterPrefab == ResourceName.Empty)
				continue;

			slot.SpawnCharacterInCompartment(characterPrefab, m_QrfGroup, m_Faction.GetBaseFactionGroup());
		}
	}

	private void OnWaypointCompleted(AIWaypoint waypoint)
	{
		if (!m_Vehicle)
			return;

		if (m_MoveWaypoint && m_MoveWaypoint == waypoint)
		{
			//radio group because there might be random guys with RTO capabilities
			m_FireteamGroup = ESCT_CharacterHelper.CreateGroup(m_Faction.GetBaseFactionRadioGroup());
			if (!m_FireteamGroup)
				return;

			array<IEntity> occupants = {};
			m_VehicleCompartmentManager.GetOccupantsOfType(occupants, ECompartmentType.CARGO);

			foreach (IEntity occupant : occupants)
			{
				if (!ESCT_CharacterHelper.CanFight(occupant))
					continue;

				ChimeraAIAgent agent = ESCT_CharacterHelper.GetAIAgent(occupant);
				if (!agent)
					continue;

				m_FireteamGroup.AddAgent(agent);

				CompartmentAccessComponent occAccess = CompartmentAccessComponent.Cast(occupant.FindComponent(CompartmentAccessComponent));
				if (!occAccess || !occAccess.IsInCompartment() || !occAccess.CanGetOutVehicle())
					continue;

				occAccess.GetOutVehicle(EGetOutType.ANIMATED, -1, ECloseDoorAfterActions.CLOSE_DOOR, true);
			}

			GetGame().GetCallqueue().CallLater(SearchAndDestroy, 4600);
		}
	}

	private void SearchAndDestroy()
	{
		if (!m_FireteamGroup || GetSupportState() != ESCT_ESupportState.InProgress)
			return;

		m_WaypointManager.SearchAndDestroy(m_FireteamGroup, m_vTargetPosition, holdingTime: Math.RandomFloat(60.0, 180.0));
	}

	private bool InsertEntity(IEntity entity)
	{
		if (entity.FindComponent(FactionAffiliationComponent))
			m_aTargets.Insert(entity);

		return true;
	}

	private bool FilterEntities(IEntity entity)
	{
		FactionAffiliationComponent fa = FactionAffiliationComponent.Cast(entity.FindComponent(FactionAffiliationComponent));
		if (fa && fa.GetAffiliatedFactionKey() != "CIV" && fa.GetAffiliatedFactionKey() != m_Faction.GetFactionKey())
		{
			return true;
		}

		return false;
	}

	private void RegisterAgent(AIAgent agent)
	{
		if (!agent)
			return;

		agent.SetPermanentLOD(0);
	}
}
