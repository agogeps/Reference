[EntityEditorProps(category: "Escapists/Entities/Locations", description: "Possible escape helicopter location.")]
class ESCT_HelipadClass : ESCT_LocationClass
{
}

sealed class ESCT_Helipad : ESCT_Location
{
	protected ESCT_WinLoseHandlerComponent m_WinLoseHandler;

	private IEntity m_Helicopter;
	private bool b_mIsHelicopterTaken = false;
	private bool b_mIsFeedbackProvided = false;

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (!GetGame().InPlayMode())
			return;

		m_WinLoseHandler = ESCT_WinLoseHandlerComponent.GetInstance();
	}

	override void Initialize()
	{
		if (ESCT_GameStateManagerComponent.GetInstance().GetGameState() != ESCT_EGameState.Transport)
			return;

		SetSpawnState(ESCT_ESpawnState.Idle);

		m_FactionAffiliation.SetAffiliatedFactionByKey(ESCT_FactionManager.GetInstance().GetEnemyFactionKey());

		if (!m_MapMarker)
			return;

		m_MapMarker.Init();
		m_MapMarker.Reveal();
	}

	override bool Spawn()
	{
		bool baseSpawn = super.Spawn();

		if (!baseSpawn)
			return false;

		if (ESCT_GameStateManagerComponent.GetInstance().GetGameState() != ESCT_EGameState.Transport)
			return false;

		if (b_mIsHelicopterTaken)
		{
			return false;
		}

		CreateHelicopter();

		return true;
	}

	override bool Despawn()
	{
		bool baseDespawn = super.Despawn();

		if (!Replication.IsServer() || !baseDespawn || !m_Helicopter)
			return false;

		//players leave location on helicopter, so it needs to be unlinked from location
		if (b_mIsHelicopterTaken)
		{
			EventHandlerManagerComponent ev = EventHandlerManagerComponent.Cast(m_Helicopter.FindComponent(EventHandlerManagerComponent));
			if (ev)
			{
				ev.RemoveScriptHandler("OnCompartmentEntered", m_Helicopter, OnCompartmentEntered, false);
				ev.RemoveScriptHandler("OnCompartmentLeft", m_Helicopter, OnCompartmentLeft, false);
			}

			m_Helicopter = null;

			return false;
		}

		m_WinLoseHandler.RemoveEscapeVehicle(m_Helicopter);
		SCR_EntityHelper.DeleteEntityAndChildren(m_Helicopter);

		return true;
	}

	private void CreateHelicopter()
	{
		ESCT_Faction faction = ESCT_FactionManager.GetInstance().GetEnemyFaction();
		if (!faction)
			return;

		SCR_EntityCatalog entityCatalog = faction.GetFactionEntityCatalogOfType(EEntityCatalogType.VEHICLE);
		if (!entityCatalog)
			return;

		array<SCR_EntityCatalogEntry> helicopters = {};
		entityCatalog.GetFullFilteredEntityListWithLabels(helicopters, { EEditableEntityLabel.VEHICLE_HELICOPTER });
		if (helicopters.IsEmpty())
			return;

		SCR_EntityCatalogEntry entry = helicopters.GetRandomElement();
		ResourceName helicopterResource = entry.GetPrefab();

		IEntity composition = GetComposition();
		if (composition)
		{
			vector mat[4];
			composition.GetTransform(mat);
			mat[3] = mat[3] + "0 1 0";

			m_Helicopter = ESCT_SpawnHelpers.SpawnEntityPrefabMatrix(helicopterResource, mat);
		}
		else
		{
			m_Helicopter = ESCT_SpawnHelpers.SpawnEntityPrefab(helicopterResource, GetOrigin() + "0 1 0", GetAngles());
		}

		//some vehicles are frozen in air when spawned
		Physics physics = m_Helicopter.GetPhysics();
		if (physics)
		{
			physics.ApplyImpulse(vector.Up * physics.GetMass() * 0.00001);
		}

		EventHandlerManagerComponent ev = EventHandlerManagerComponent.Cast(m_Helicopter.FindComponent(EventHandlerManagerComponent));
		if (ev)
		{
			ev.RegisterScriptHandler("OnCompartmentEntered", m_Helicopter, OnCompartmentEntered, false, false);
			ev.RegisterScriptHandler("OnCompartmentLeft", m_Helicopter, OnCompartmentLeft, false, false);
		}
		
		SCR_HelicopterDamageManagerComponent helicopterDamageManager = SCR_HelicopterDamageManagerComponent.Cast(m_Helicopter.FindComponent(SCR_HelicopterDamageManagerComponent));
		if (helicopterDamageManager)
		{
			helicopterDamageManager.GetOnDamageStateChanged().Insert(OnHeliDestroyed);
		}

		m_WinLoseHandler.AddEscapeVehicle(m_Helicopter);
	}

	private void OnCompartmentEntered(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		if (b_mIsHelicopterTaken)
			return;

		b_mIsHelicopterTaken = true;

		if (b_mIsFeedbackProvided)
			return;

		b_mIsFeedbackProvided = true;

		if (RplSession.Mode() != RplMode.Dedicated)
		{
			Rpc_DoTriggerFeedback();
		}
		Rpc(Rpc_DoTriggerFeedback);
	}

	private void OnCompartmentLeft(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		array<BaseCompartmentSlot> outCompartments = {};
		mgr.GetCompartments(outCompartments);

		bool isOccupied = false;
		foreach (BaseCompartmentSlot slot : outCompartments)
		{
			if (slot.IsOccupied())
			{
				isOccupied = true;
				break;
			}
		}

		b_mIsHelicopterTaken = isOccupied;
	}

	protected void OnHeliDestroyed(EDamageState state)
	{
		if (state != EDamageState.DESTROYED || !m_Helicopter)
			return;
		
		// helicopter should be "wasted" if it was destroyed
		if (!b_mIsHelicopterTaken)
			b_mIsHelicopterTaken = true;

		if (m_Helicopter)
		{
			EventHandlerManagerComponent ev = EventHandlerManagerComponent.Cast(m_Helicopter.FindComponent(EventHandlerManagerComponent));
			if (ev)
			{
				ev.RemoveScriptHandler("OnCompartmentEntered", m_Helicopter, OnCompartmentEntered, false);
				ev.RemoveScriptHandler("OnCompartmentLeft", m_Helicopter, OnCompartmentLeft, false);
			}
		}
		
		FailExtractionTask();

		ESCT_LocationSystem locationSystem = ESCT_LocationSystem.GetSystem();
		if (!locationSystem)
			return;

		//assign another helipad location
		GetGame().GetCallqueue().CallLater(locationSystem.SelectHelipad, Math.RandomInt(2000, 10000), false);
	}

	private void FailExtractionTask()
	{
		ESCT_MoveTaskSupportEntity supportEntity = ESCT_MoveTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(ESCT_MoveTaskSupportEntity));
		if (!supportEntity)
			return;

		ESCT_MoveTask pointTask = ESCT_MoveTask.Cast(GetTaskManager().GetTask(supportEntity.GetTaskIdByType(ESCT_MoveTask)));
		if (!pointTask)
			return;

		supportEntity.FailTask(pointTask);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	private void Rpc_DoTriggerFeedback()
	{
		ESCT_EscapistsGameMode gameMode = ESCT_EscapistsGameMode.GetGameMode();
		if (!gameMode)
			return;

		gameMode.GetEscapistsManager().GetOnHelicopterEntered().Invoke();
	}
}
