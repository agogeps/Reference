[EntityEditorProps(category: "Escapists/Entities", description: "Random vehicle spawn point.")]
class ESCT_VehicleSpawnPointClass : GenericEntityClass
{
}

sealed class ESCT_VehicleSpawnPoint : GenericEntity
{
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Probability of spawn will be not checked and vehicle will be always present on this point.", category: "Escapists")]
	bool m_bIsSpawnGuaranteed;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Use player faction storages.", category: "Escapists")]
	bool m_bUsePlayerFaction;
	
	[Attribute("0", UIWidgets.ComboBox, "Select Entity Labels which you want to optionally include to random spawn. If you want to spawn everything, you can leave it out empty.", "", ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected ref array<EEditableEntityLabel> m_aIncludedEditableEntityLabels;

	[Attribute("0", UIWidgets.ComboBox, "Select Entity Labels which you want to exclude from random spawn.", "", ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected ref array<EEditableEntityLabel> m_aExcludedEditableEntityLabels;

	[Attribute("0", desc: "If true, only assets with ALL of provided included labels will be used.")]
	protected bool m_bRequireAllIncludedLabels;
	
	[Attribute("0", UIWidgets.Auto, desc: "Populate gunner slot on vehicle on spawn.", category: "Escapists")]
	protected bool m_bPopulateGunnerSlot;
	
	protected ESCT_Faction m_Faction;
	protected AIGroup m_GunnerGroup;
	protected IEntity m_Vehicle;
	protected int m_SpawnPointId;
	protected ESCT_LocationVehicleManagerComponent m_VehicleManager;
	
#ifdef WORKBENCH
	private float m_fRefreshRate = 0.1;
	private float m_fTime;
	private ref Shape m_Shape;
#endif
	
	int GetSpawnPointId()
	{
		return m_SpawnPointId;
	}
	
	IEntity GetVehicle()
	{
		return m_Vehicle;
	}
	
 	void SetVehicleManager(notnull ESCT_LocationVehicleManagerComponent vehicleManager)
	{
		m_VehicleManager = vehicleManager;
	}
	
	void ESCT_VehicleSpawnPoint(IEntitySource src, IEntity parent)
	{
		if (SCR_Global.IsEditMode() || Replication.IsServer())
		{
			SetEventMask(EntityEvent.INIT);
		}
	}
	
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
#ifdef WORKBENCH
		if (SCR_Global.IsEditMode())
		{
			vector transform[4];
			owner.GetTransform(transform);
	
			m_Shape = Shape.Create(
				ShapeType.BBOX, ARGB(100, 0x99, 0x10, 0xF2), 
				ShapeFlags.VISIBLE | ShapeFlags.TRANSP | ShapeFlags.NOOUTLINE | ShapeFlags.ADDITIVE | ShapeFlags.DOUBLESIDE, 
				"-1.5 0 -2", "1.5 2 2"
			);
			m_Shape.SetMatrix(transform);
			
			return;
		}
#endif
			
		if (!Replication.IsServer()) return;
		
		m_SpawnPointId = ESCT_IdentifierGenerator.GenerateIntId();
	}
	
	void FirstSetup(notnull ESCT_Faction faction)
	{
		if (!m_VehicleManager)
			return;
		
		SCR_EntityCatalogEntry entry = AssignCatalogEntry(faction);
		if (!entry)
			return;
		
		ESCT_VehicleSpawnPointState state = new ESCT_VehicleSpawnPointState(m_SpawnPointId, entry, true);
		m_VehicleManager.AddToSlotStateMap(m_SpawnPointId, state);
	}
	
	IEntity Spawn()
	{
		if (!m_VehicleManager)
			return null;

		map<int, ref ESCT_VehicleSpawnPointState> stateMap = m_VehicleManager.GetSpawnPointStates();
		ESCT_VehicleSpawnPointState state = stateMap.Get(m_SpawnPointId);
		if (!state || state.b_mIsUsed)
			return null;

		ResourceName prefab = state.GetVehicleCatalogEntry().GetPrefab();
		
		vector mat[4];
		GetTransform(mat);
		
		m_Vehicle = ESCT_SpawnHelpers.SpawnEntityPrefabMatrix(prefab, mat);
		if (!m_Vehicle)
			return null;
		
		CarControllerComponent carController = CarControllerComponent.Cast(m_Vehicle.FindComponent(CarControllerComponent));
		if (carController)
			carController.SetPersistentHandBrake(true);
		
		Physics physicsComponent = m_Vehicle.GetPhysics();
		if (physicsComponent)
			physicsComponent.SetVelocity("0 -0.1 0");
		
		EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(m_Vehicle.FindComponent(EventHandlerManagerComponent));
		if (eventHandlerManager)
		{
			eventHandlerManager.RegisterScriptHandler("OnCompartmentEntered", m_Vehicle, OnCompartmentEntered, false, false);
			eventHandlerManager.RegisterScriptHandler("OnDestroyed", m_Vehicle, OnDestroyed);	
		}
		
		if (m_bPopulateGunnerSlot)
			SpawnTurretGunner(state);

		return m_Vehicle;
	}
	
	private SCR_EntityCatalogEntry AssignCatalogEntry(ESCT_Faction faction)
	{
		if (!m_bIsSpawnGuaranteed)
		{
			float spawnChance = ESCT_EscapistsConfigComponent.GetInstance().GetVehicleSpawnChance();
			if (Math.RandomFloat01() > spawnChance)
				return null;
		}
		
		ESCT_FactionManager factionManager = ESCT_FactionManager.GetInstance();
		if (!factionManager)
			return null;
		
		if (m_bUsePlayerFaction)
			m_Faction = factionManager.GetPlayerFaction();
		else 
			m_Faction = faction;

		if (!m_Faction)
			return null;
		
		SCR_EntityCatalog entityCatalog = m_Faction.GetFactionEntityCatalogOfType(EEntityCatalogType.VEHICLE);
		if (!entityCatalog)
			return null;

		array<SCR_EntityCatalogEntry> data = {};
		entityCatalog.GetFullFilteredEntityListWithLabels(data, m_aIncludedEditableEntityLabels, m_aExcludedEditableEntityLabels, m_bRequireAllIncludedLabels);

		if (data.IsEmpty())
			return null;

		Math.Randomize(-1);
		return data.GetRandomElement();
	}
	
	private void SpawnTurretGunner(ESCT_VehicleSpawnPointState state)
	{
		if (!state.IsGunnerAlive())
			return;
		
		SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(m_Vehicle.FindComponent(SCR_BaseCompartmentManagerComponent));
		if (!compartmentManager)
			return;

		array<BaseCompartmentSlot> compartments = {};
		compartmentManager.GetFreeCompartmentsOfType(compartments, ECompartmentType.TURRET);
		
		foreach (BaseCompartmentSlot slot : compartments)
		{
			ResourceName characterPrefab = m_Faction.GetRandomCharacterOfBranch(ESCT_EMilitaryBranchType.Military);
			if (!characterPrefab || characterPrefab == ResourceName.Empty)
				continue;

			slot.SpawnCharacterInCompartment(characterPrefab, m_GunnerGroup, m_Faction.GetBaseFactionGroup());
		}
		
		if (m_GunnerGroup)
		{
			SCR_AIGroup group = SCR_AIGroup.Cast(m_GunnerGroup);
			group.GetOnAgentAdded().Insert(RegisterGunner);
		}
	}
	
	private void OnTurretOccupantDeath(EDamageState state)
	{
		m_VehicleManager.UpdateState(new ESCT_VehicleSpawnPointState(m_SpawnPointId, null, false, isUsed: false));
		
		if (!m_Vehicle)
			return;
		
		EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(m_Vehicle.FindComponent(EventHandlerManagerComponent));
		if (eventHandlerManager)
		{
			eventHandlerManager.RemoveScriptHandler("OnCompartmentEntered", m_Vehicle, OnCompartmentEntered, false);
			eventHandlerManager.RemoveScriptHandler("OnDestroyed", m_Vehicle, OnDestroyed, false);	
		}
	}
	
	private void OnCompartmentEntered(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		if (!EntityUtils.IsPlayer(occupant))
			return;
		
		//remove vehicle from state manager
		m_VehicleManager.UpdateState(new ESCT_VehicleSpawnPointState(m_SpawnPointId, null, false, isUsed: true));
		
		//untie vehicle from location so it wouldn't be despawned
		m_Vehicle = null;
		
		EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(vehicle.FindComponent(EventHandlerManagerComponent));
		if (eventHandlerManager)
		{
			eventHandlerManager.RemoveScriptHandler("OnCompartmentEntered", vehicle, OnCompartmentEntered, false);
			eventHandlerManager.RemoveScriptHandler("OnDestroyed", vehicle, OnDestroyed, false);	
		}
	}
	
	private void OnDestroyed(IEntity ent)
	{	
		if (!m_VehicleManager)
			return;
		
		//remove vehicle from state manager
		m_VehicleManager.UpdateState(new ESCT_VehicleSpawnPointState(m_SpawnPointId, null, false, isUsed: true));
		
		//untie vehicle from location so it wouldn't be despawned
		m_Vehicle = null;
	}
	
	private void RegisterGunner(AIAgent aiAgent)
	{
		if (!aiAgent)
			return;
				
		IEntity charEntity = aiAgent.GetControlledEntity();
		if (!charEntity)
			return;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(charEntity);
		if (!character)
			return;	
		
		SCR_CharacterDamageManagerComponent damageMan = SCR_CharacterDamageManagerComponent.Cast(character.GetDamageManager());
		if (damageMan)
			damageMan.GetOnDamageStateChanged().Insert(OnTurretOccupantDeath);
	}
	
#ifdef WORKBENCH
	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		super._WB_AfterWorldUpdate(this, timeSlice);
		
		if (!m_Shape)
			return;
		
		if (m_fTime < m_fRefreshRate)
		{
			m_fTime += timeSlice;
			return;
		}

		vector transform[4];
		GetTransform(transform);
		m_Shape.SetMatrix(transform);
	}
#endif
}
