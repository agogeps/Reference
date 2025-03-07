[EntityEditorProps("Escapists/Location", description: "Weapon rack manager in locations.", color: "0 0 255 255")]
class ESCT_CivilianPresenceComponentClass : ESCT_LocationComponentClass
{
}

class ESCT_CivilianPresenceComponent : ESCT_LocationComponent
{
	protected ESCT_CivilianManagerComponent m_CivManager;
	protected RoadNetworkManager m_RoadNetworkManager;
	protected ESCT_WaypointManagerComponent m_WaypointManager;

	private ref array<IEntity> m_aPedestrians;
	private ref array<IEntity> m_aVehicles;

	private int m_iCarQuantity;
	private int m_iPedestrianQuantity;

	private static const float CIV_SPAWN_RADIUS = 50.0;
	private static const float CIV_QUERY_SPACE = 3.0;
	private static const float ROAD_SEARCH_RADIUS = 500.0;

	private static const string CIV_GROUP = "{464DE4EEF0F155CE}Prefabs/AI/Groups/Group_CIV.et";

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		if (aiWorld)
			m_RoadNetworkManager = aiWorld.GetRoadNetworkManager();

		m_WaypointManager = ESCT_WaypointManagerComponent.GetInstance();
		m_CivManager = ESCT_CivilianManagerComponent.GetInstance();
	}

	override void Spawn()
	{
		ESCT_EscapistsConfigComponent gameConfig = ESCT_EscapistsConfigComponent.GetInstance();
		if (!gameConfig || !gameConfig.GetCivilianPresence())
			return;

		if (!m_Location.IsVisited())
		{
			//TODO: allow other locations too?
			ESCT_City city = ESCT_City.Cast(m_Location);
			if (city)
			{
				m_iPedestrianQuantity = Math.RandomInt(1, city.GetMaxPedestrians());
				m_iCarQuantity = Math.RandomInt(1, city.GetMaxCars());
			}
		}

		if (!m_WaypointManager || !m_CivManager)
			return;

		CreatePedestrians();
		CreateCars();
	}

	override void Despawn()
	{
		ESCT_EscapistsConfigComponent gameConfig = ESCT_EscapistsConfigComponent.GetInstance();
		if (!gameConfig || !gameConfig.GetCivilianPresence())
			return;

		if (m_aPedestrians)
		{
			foreach (IEntity pedestrian : m_aPedestrians)
			{
				SCR_EntityHelper.DeleteEntityAndChildren(pedestrian);
			}
			m_aPedestrians.Clear();
		}

		if (m_aVehicles)
		{
			foreach (IEntity vehicle : m_aVehicles)
			{
				SCR_EntityHelper.DeleteEntityAndChildren(vehicle);
			}
			m_aVehicles.Clear();
		}
	}

	private void CreatePedestrians()
	{
		if (m_iPedestrianQuantity == 0)
			return;

		if (!m_aPedestrians)
			m_aPedestrians = {};

		for (int i = 0; i < m_iPedestrianQuantity; i++)
		{
			vector spawnPosition = ESCT_Misc.GetRandomValidPosition(m_Location.GetOrigin(), 0, 100, CIV_SPAWN_RADIUS, CIV_QUERY_SPACE);
			if (spawnPosition == vector.Zero)
				continue;

			IEntity group = ESCT_SpawnHelpers.SpawnEntityPrefab(CIV_GROUP, spawnPosition);
			SCR_AIGroup aiGroup = SCR_AIGroup.Cast(group);

			IEntity civilian = SpawnCivilian(spawnPosition, aiGroup);
			if (!civilian)
				continue;

			m_WaypointManager.Patrol(aiGroup, m_Location.GetOrigin(), Math.RandomFloat(30, 90));
		}
	}

	private void CreateCars()
	{
		if (m_iCarQuantity == 0 || !m_RoadNetworkManager)
			return;

		if (!m_aVehicles)
			m_aVehicles = {};

		SCR_EntityCatalogEntry vehicleEntry = null;

		for (int i = 0; i < m_iCarQuantity; i++)
		{
			vector initialPosition = m_Location.GetOrigin();
			vector spawnPosition, direction;
			ESCT_RoadNavigation.SelectRoadSegment(initialPosition, spawnPosition, direction);

			if (!spawnPosition || spawnPosition == vector.Zero || !direction)
			{
				ESCT_Logger.ErrorFormat("Couldn't find valid road vector for %1 location, on to the next car it goes.", m_Location.ToString());
				continue;
			}

			vehicleEntry = m_CivManager.GetRandomVehicle();
			if (!vehicleEntry)
				continue;

			IEntity vehicle = ESCT_SpawnHelpers.SpawnVehiclePrefab(vehicleEntry.GetPrefab(), spawnPosition, direction);
			if (!vehicle)
				continue;

			m_aVehicles.Insert(vehicle);

			EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(vehicle.FindComponent(EventHandlerManagerComponent));
			if (eventHandlerManager)
			{
				eventHandlerManager.RegisterScriptHandler("OnCompartmentEntered", vehicle, OnCompartmentEntered, false, false);
				eventHandlerManager.RegisterScriptHandler("OnDestroyed", vehicle, OnDestroyed);
			}

			IEntity group = ESCT_SpawnHelpers.SpawnEntityPrefab(CIV_GROUP, spawnPosition);
			if (!group)
				continue;

			SCR_AIGroup aiGroup = SCR_AIGroup.Cast(group);
			if (!aiGroup)
				continue;

			IEntity civilian = SpawnCivilianForVehicle(spawnPosition, aiGroup, vehicle);
			if (!civilian)
				continue;

			AICharacterMovementComponent movementComponent = AICharacterMovementComponent.Cast(civilian.FindComponent(AICharacterMovementComponent));
			if (movementComponent)
				movementComponent.SetMovementTypeWanted(EMovementType.WALK);

			AICarMovementComponent carMovementComp = AICarMovementComponent.Cast(vehicle.FindComponent(AICarMovementComponent));
			if (carMovementComp)
				carMovementComp.SetCruiseSpeed(Math.RandomFloat(35, 70));

			m_WaypointManager.VehiclePatrol(aiGroup, m_Location.GetOrigin(), Math.RandomFloat(50, 500));
		}
	}

	private IEntity SpawnCivilian(vector spawnPosition, SCR_AIGroup group, IEntity vehicle = null)
	{
		SCR_EntityCatalogEntry catEntry = m_CivManager.GetRandomCharacter();
		IEntity civilian = ESCT_CharacterHelper.SpawnCharacterForGroup(catEntry.GetPrefab(), group, spawnPosition);
		if (!civilian)
			return null;

		m_aPedestrians.Insert(civilian);

		return civilian;
	}

	private IEntity SpawnCivilianForVehicle(vector spawnPosition, SCR_AIGroup group, notnull IEntity vehicle)
	{
		SCR_EntityCatalogEntry catEntry = m_CivManager.GetRandomCharacter();
		if (!catEntry)
			return null;

		IEntity civilian = ESCT_CharacterHelper.SpawnCharacterForGroup(catEntry.GetPrefab(), group, spawnPosition);
		if (!civilian)
			return null;

		m_aPedestrians.Insert(civilian);

		ChimeraCharacter civilianCharacter = ChimeraCharacter.Cast(civilian);
		if (!civilianCharacter)
			return null;

		CompartmentAccessComponent compartmentAccess = civilianCharacter.GetCompartmentAccessComponent();
		if (compartmentAccess)
		{
			BaseCompartmentSlot slot = compartmentAccess.FindFreeCompartment(vehicle, ECompartmentType.PILOT);
			if (slot)
				compartmentAccess.GetInVehicle(vehicle, slot, true, 0, ECloseDoorAfterActions.CLOSE_DOOR, false);
		}

		return civilian;
	}

	private void OnCompartmentEntered(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		if (!vehicle)
			return;

		if (!EntityUtils.IsPlayer(occupant))
			return;

		//untie vehicle from location so it wouldn't be despawned if players are inside it, abandoned vehicles will be dealt by garbage collector
		m_aVehicles.RemoveItem(vehicle);

		if (m_iCarQuantity > 0)
			m_iCarQuantity--;

		EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(vehicle.FindComponent(EventHandlerManagerComponent));
		if (eventHandlerManager)
		{
			eventHandlerManager.RemoveScriptHandler("OnCompartmentEntered", vehicle, OnCompartmentEntered, false);
			eventHandlerManager.RemoveScriptHandler("OnDestroyed", vehicle, OnDestroyed, false);
		}
	}

	private void OnDestroyed(IEntity ent)
	{
		if (m_iCarQuantity > 0)
			m_iCarQuantity--;

		//untie vehicle from location so it wouldn't be despawned
		if (ent)
			m_aVehicles.RemoveItem(ent);
	}
}
