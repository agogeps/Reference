[EntityEditorProps(category: "Escapists/Entities", description: "Vehicle Patrol ambient event.", visible: false)]
class ESCT_VehiclePatrolEventClass : ESCT_AmbientEventBaseClass
{
}

[BaseContainerProps(), BaseContainerCustomStringTitleField("DO NOT USE BASE CLASS!")]
sealed class ESCT_VehiclePatrolEvent : ESCT_AmbientEventBase
{
	[Attribute("0", UIWidgets.ComboBox, "Select Entity Labels which you want to optionally include to random spawn. If you want to spawn everything, you can leave it out empty.", "", ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected ref array<EEditableEntityLabel> m_aIncludedEditableEntityLabels;

	[Attribute("0", UIWidgets.ComboBox, "Select Entity Labels which you want to exclude from random spawn.", "", ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected ref array<EEditableEntityLabel> m_aExcludedEditableEntityLabels;

	[Attribute("0", desc: "If true, only assets with ALL of provided included labels will be used.")]
	protected bool m_bRequireAllIncludedLabels;

	[Attribute(defvalue: "0.35", uiwidget: UIWidgets.Slider, params: "0 1 0.05", precision: 2, desc: "Passenger seat fill chance.")]
	protected float m_fPassengerSeatFillChance;

	protected AIGroup m_PatrolGroup;
	protected IEntity m_PatrolVehicle;

	private SCR_BaseCompartmentManagerComponent m_PatrolVehicleCompartmentManager;
	private ESCT_Faction m_Faction;

	override bool Launch()
	{
		if (!super.Launch())
			return false;

		vector spawnPosition, spawnDirection;
		if (!GetInitialVehiclePatrolPosition(spawnPosition, spawnDirection))
			return false;

		array<ESCT_Faction> factions = {};
		array<float> weights = {};
		GetFactionsAndWeights(factions, weights);

		if (factions.IsEmpty())
			return false;

		int factionIndex = SCR_ArrayHelper.GetWeightedIndex(weights, Math.RandomFloat01());
		m_Faction = factions[factionIndex];

		SCR_EntityCatalog entityCatalog = m_Faction.GetFactionEntityCatalogOfType(EEntityCatalogType.VEHICLE);
		if (!entityCatalog)
			return false;

		array<SCR_EntityCatalogEntry> data = {};
		entityCatalog.GetFullFilteredEntityListWithLabels(data, m_aIncludedEditableEntityLabels, m_aExcludedEditableEntityLabels, needsAllIncluded: m_bRequireAllIncludedLabels);
		if (data.IsEmpty())
			return false;

		Math.Randomize(-1);
		SCR_EntityCatalogEntry catalogEntry = data.GetRandomElement();
		ResourceName vehiclePrefab = catalogEntry.GetPrefab();
		if (!vehiclePrefab)
			return false;

		m_PatrolVehicle = ESCT_SpawnHelpers.SpawnVehiclePrefab(vehiclePrefab, spawnPosition, spawnDirection);
		if (!m_PatrolVehicle)
			return false;

		m_PatrolVehicleCompartmentManager = SCR_BaseCompartmentManagerComponent.Cast(m_PatrolVehicle.FindComponent(SCR_BaseCompartmentManagerComponent));
		if (!m_PatrolVehicleCompartmentManager)
			return false;

		AICarMovementComponent carMovementComp = AICarMovementComponent.Cast(m_PatrolVehicle.FindComponent(AICarMovementComponent));
		if (carMovementComp)
			carMovementComp.SetCruiseSpeed(Math.RandomFloat(25, 70));

		EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(m_PatrolVehicle.FindComponent(EventHandlerManagerComponent));
		if (eventHandlerManager)
		{
			eventHandlerManager.RegisterScriptHandler("OnCompartmentEntered", m_PatrolVehicle, OnCompartmentEntered, false, false);
			eventHandlerManager.RegisterScriptHandler("OnCompartmentLeft", m_PatrolVehicle, OnCompartmentLeft, false, false);
			eventHandlerManager.RegisterScriptHandler("OnDestroyed", m_PatrolVehicle, OnDestroyed);
		}

		if (!SpawnVehicleOccupants())
			return false;

		if (!AssignWaypoints())
			return false;

		ESCT_Logger.InfoFormat("[ESCT_VehiclePatrolEvent] Spawned %1 vehicle patrol on %2 position, origin position %3.",
			ESCT_Misc.GetPrefabName(m_PatrolVehicle),
			m_PatrolVehicle.GetOrigin().ToString(),
			GetOrigin().ToString()
		);

		return true;
	}

	override bool CheckDespawn()
	{
		if (HasGracePeriod())
			return false;

		if (super.CheckDespawn())
			return true;

		if (!m_PatrolGroup)
			return true;

		if (!IsAnyPlayerNearPatrolVehicle())
			return true;

		return false;
	}

	override void Despawn()
	{
		if (m_PatrolGroup)
		{
			array<AIAgent> agents = {};
			m_PatrolGroup.GetAgents(agents);

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

			delete m_PatrolGroup;
		}

		if (m_PatrolVehicle && !IsAnyPlayerNearPatrolVehicle())
			SCR_EntityHelper.DeleteEntityAndChildren(m_PatrolVehicle);

		super.Despawn();
	}

	protected bool GetInitialVehiclePatrolPosition(out vector spawnPosition, out vector spawnDirection)
	{
		ESCT_EscapistsGameMode gameMode = ESCT_EscapistsGameMode.GetGameMode();
		vector initialPosition = gameMode.GetRandomGenerator().GenerateRandomPointInRadius(300, 1200, GetOrigin());
		initialPosition[1] = SCR_TerrainHelper.GetTerrainY(initialPosition);

		ESCT_RoadNavigation.SelectRoadSegment(initialPosition, spawnPosition, spawnDirection, facePosition: false);
		if (spawnPosition == vector.Zero || spawnDirection == vector.Zero || vector.Distance(spawnPosition, GetOrigin()) < 50)
		{
			spawnPosition = ESCT_Misc.GetRandomPositionForVehicle(initialPosition, 300, 600);
			spawnDirection = vector.Direction(spawnPosition, GetOrigin());
		}
		else
		{
			vector roadToDestination = (GetOrigin() - spawnPosition);
			roadToDestination.Normalize();
			if (vector.Dot(spawnDirection, roadToDestination) < 0) spawnDirection = -spawnDirection;
		}

		if (spawnPosition == vector.Zero)
			return false;

		return true;
	}

	protected void GetFactionsAndWeights(out array<ESCT_Faction> factions, out array<float> weights)
	{
		ESCT_FactionManager factionManager = ESCT_FactionManager.GetInstance();
		if (!factionManager)
			return;

		factions = {factionManager.GetEnemyFaction()};
		weights = {50};
		if (ESCT_EscapistsConfigComponent.GetInstance().IsInsurgencyEnabled())
		{
			factions.Insert(factionManager.GetInsurgentFaction());
			weights.Insert(10);
		}
	}

	protected bool SpawnVehicleOccupants()
	{
		array<BaseCompartmentSlot> compartments = {};
		m_PatrolVehicleCompartmentManager.GetFreeCompartmentsOfType(compartments, ECompartmentType.PILOT);
		m_PatrolVehicleCompartmentManager.GetFreeCompartmentsOfType(compartments, ECompartmentType.TURRET);
		if (compartments.IsEmpty())
			return false;

		m_PatrolGroup = ESCT_CharacterHelper.CreateGroup(m_Faction.GetFactionKey(), m_Faction.GetBaseFactionRadioGroup());
		SCR_AIGroup group = SCR_AIGroup.Cast(m_PatrolGroup);
		if (group)
			group.GetOnAgentAdded().Insert(RegisterAgent);

		foreach (BaseCompartmentSlot slot : compartments)
		{
			ResourceName charPrefab = m_Faction.GetRandomCharacterOfBranch(ESCT_EMilitaryBranchType.Military);
			if (!charPrefab || charPrefab == ResourceName.Empty)
				continue;

			slot.SpawnCharacterInCompartment(charPrefab, m_PatrolGroup, m_Faction.GetBaseFactionRadioGroup());
		}

		compartments.Clear();
		m_PatrolVehicleCompartmentManager.GetFreeCompartmentsOfType(compartments, ECompartmentType.CARGO);
		foreach (BaseCompartmentSlot slot : compartments)
		{
			if (Math.RandomFloat01() > m_fPassengerSeatFillChance)
				continue;

			ResourceName charPrefab = m_Faction.GetRandomCharacterOfBranch(ESCT_EMilitaryBranchType.Military);
			if (!charPrefab || charPrefab == ResourceName.Empty)
				continue;

			slot.SpawnCharacterInCompartment(charPrefab, m_PatrolGroup, m_Faction.GetBaseFactionRadioGroup());
		}

		return true;
	}

	protected bool AssignWaypoints()
	{
		ESCT_WaypointManagerComponent waypointManager = ESCT_WaypointManagerComponent.GetInstance();
		if (!waypointManager)
			return false;

		float distanceToRoad;
		vector closestRoadPosition = ESCT_RoadNavigation.GetClosestRoadPosition(GetOrigin(), distanceToRoad);
		if (closestRoadPosition == vector.Zero || distanceToRoad > 500)
			closestRoadPosition = ESCT_Misc.GetRandomPositionForVehicle(GetOrigin(), 0, 300);

		if (closestRoadPosition == vector.Zero)
			closestRoadPosition = GetOrigin();

		waypointManager.Move(m_PatrolGroup, closestRoadPosition, 35.0, 30);

		array<ESCT_Location> locations = {};
		ESCT_LocationSystem.GetSystem().GetLocations(locations);
		ESCT_Location selectedLocation = locations.GetRandomElement();

		vector locationPosition = ESCT_RoadNavigation.GetClosestRoadPosition(selectedLocation.GetOrigin(), distanceToRoad);
		if (locationPosition == vector.Zero || distanceToRoad > 400)
			locationPosition = ESCT_Misc.GetRandomPositionForVehicle(selectedLocation.GetOrigin(), 0, 400);

		if (locationPosition != vector.Zero)
			waypointManager.Move(m_PatrolGroup, locationPosition, 35.0, 30);

		waypointManager.Cycle(m_PatrolGroup, 35.0);

		return true;
	}

	private void OnCompartmentEntered(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		if (!EntityUtils.IsPlayer(occupant) || !m_PatrolVehicle)
			return;

		EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(vehicle.FindComponent(EventHandlerManagerComponent));
		if (eventHandlerManager)
		{
			eventHandlerManager.RemoveScriptHandler("OnCompartmentEntered", m_PatrolVehicle, OnCompartmentEntered, false);
			eventHandlerManager.RemoveScriptHandler("OnCompartmentLeft", m_PatrolVehicle, OnCompartmentLeft, false);
			eventHandlerManager.RemoveScriptHandler("OnDestroyed", m_PatrolVehicle, OnDestroyed, false);
		}

		//untie vehicle from task so it wouldn't be despawned if players are inside it, abandoned vehicles will be dealt by garbage collector
		m_PatrolVehicle = null;
	}

	private void OnCompartmentLeft(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		if (!m_PatrolVehicleCompartmentManager || !m_PatrolVehicle)
			return;

		array<IEntity> occupants = {};
		m_PatrolVehicleCompartmentManager.GetOccupants(occupants);

		bool hasPlayer = false;
		foreach (IEntity occ : occupants)
		{
			if (EntityUtils.IsPlayer(occ))
			{
				hasPlayer = true;
				break;
			}
		}

		if (!hasPlayer && m_PatrolVehicleCompartmentManager.GetOccupantCount() == 0)
		{
			EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(vehicle.FindComponent(EventHandlerManagerComponent));
			if (eventHandlerManager)
			{
				eventHandlerManager.RemoveScriptHandler("OnCompartmentEntered", m_PatrolVehicle, OnCompartmentEntered, false);
				eventHandlerManager.RemoveScriptHandler("OnCompartmentLeft", m_PatrolVehicle, OnCompartmentEntered, false);
				eventHandlerManager.RemoveScriptHandler("OnDestroyed", m_PatrolVehicle, OnDestroyed, false);
			}

			m_PatrolVehicle = null;
		}
	}

	private void OnDestroyed(IEntity ent)
	{
		if (!m_PatrolVehicle)
			return;

		//garbage collector will take care of it
		m_PatrolVehicle = null;
	}

	private void RegisterAgent(AIAgent agent)
	{
		if (!agent)
			return;

		//so it won't despawn by engine means
		agent.SetPermanentLOD(0);
	}

	private bool IsAnyPlayerNearPatrolVehicle()
	{
		if (!m_PatrolVehicle)
			return false;

		bool isPlayerNear = false;

		array<IEntity> players = {};
		ESCT_Player.GetAlivePlayers(players, true);

		for (int i = 0; i < players.Count(); i++)
		{
			if (ESCT_Math.IsPointInRadiusXZ(players[i].GetOrigin(), m_PatrolVehicle.GetOrigin(), m_fDespawnDistance))
			{
				isPlayerNear = true;
				break;
			}
		}

		return isPlayerNear;
	}
}
