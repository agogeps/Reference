[EntityEditorProps("Escapists", description: "Component that inits helicopter for fake fly-by.", color: "0 0 255 255")]
class ESCT_HeliFlybyComponentClass : ScriptComponentClass
{
}

sealed class ESCT_HeliFlybyComponent : ScriptComponent
{
	[Attribute(defvalue: "35", desc: "Speed in m/s")]
	protected float m_fSpeed;

	protected SCR_HelicopterDamageManagerComponent m_DamageManager;
	protected TimeAndWeatherManagerEntity m_TimeAndWeatherManager;
	protected vector m_vVelocity;

	private IEntity m_Helicopter;
	private AIGroup m_HeliCrewGroup;
	private float m_Timer = 0.0;

	private ref array<IEntity> m_aTargets;
	private ref TraceParam m_Trace;

	AIGroup GetHeliCrewGroup()
	{
		if (!m_HeliCrewGroup)
			return null;

		return m_HeliCrewGroup;
	}

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (SCR_Global.IsEditMode() || !Replication.IsServer()) return;

		Physics physics = owner.GetPhysics();
		if (physics)
			physics.SetActive(ActiveState.ACTIVE);

		vector transform[4];
		owner.GetWorldTransform(transform);
		m_vVelocity = m_fSpeed * transform[2];

		m_aTargets = {};
		m_Trace = new TraceParam();
		m_Trace.Flags = TraceFlags.ENTS | TraceFlags.WORLD | TraceFlags.ANY_CONTACT;

		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME | EntityEvent.SIMULATE);
	}

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (SCR_Global.IsEditMode() || !Replication.IsServer()) return;

		m_Helicopter = owner;

		ChimeraWorld world = ChimeraWorld.CastFrom(owner.GetWorld());
		if (world)
			m_TimeAndWeatherManager = world.GetTimeAndWeatherManager();

		EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(m_Helicopter.FindComponent(EventHandlerManagerComponent));
		if (eventHandlerManager)
			eventHandlerManager.RegisterScriptHandler("OnDestroyed", m_Helicopter, OnDestroyed);

		m_DamageManager = SCR_HelicopterDamageManagerComponent.Cast(m_Helicopter.FindComponent(SCR_HelicopterDamageManagerComponent));
		if (m_DamageManager)
		{
			array<HitZone> hitZones = {};
			m_DamageManager.GetAllHitZones(hitZones);

			for (int i = 0; i < hitZones.Count(); i++)
			{
				SCR_VehicleHitZone vehHitZone = SCR_VehicleHitZone.Cast(hitZones[i]);
				if (vehHitZone && !vehHitZone.IsOnHitSubscribed())
				{
					//FIXME: it conflicts with other EHs as they can't subscribe as flag already set
					vehHitZone.SetSubscribe(true);
					vehHitZone.GetOnHit().Insert(OnHit);
				}
			}

			m_DamageManager.GetOnDamage().Insert(OnHelicopterDamage);
		}

		EnableHelicopterFly();
		GetGame().GetCallqueue().Call(SpawnCrew);
	}

	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);

		m_Timer += timeSlice;
		if (m_Timer < 1.0)
			return;

		m_Timer = 0;

		if (!owner)
		{
			return;
		}

		m_aTargets.Clear();
		ESCT_Player.GetPlayers(m_aTargets);
		if (m_aTargets.IsEmpty())
			return;

		bool hasVisibleTarget = false;
		float traceScore;
		IEntity target = null;
		m_Trace.End = owner.GetOrigin() - "0 2 0";

		float visibilityValue = 0.75;
		int visibilityDistance = 500;
		if (m_TimeAndWeatherManager && m_TimeAndWeatherManager.IsSunSet())
		{
			visibilityValue = 0.998;
			visibilityDistance = 50;
		}

		bool isNight = m_TimeAndWeatherManager.IsSunSet();

		for (int i = 0; i < m_aTargets.Count(); i++)
		{
			target = m_aTargets[i];
			if (!target)
				continue;

			m_Trace.Start = target.GetOrigin();
			if (vector.DistanceXZ(m_Trace.Start, m_Trace.End) > visibilityDistance)
				continue;

			traceScore = GetGame().GetWorld().TraceMove(m_Trace, null);
			if (traceScore <= visibilityValue)
				continue;

			//ESCT_Logger.InfoFormat("Trace Target Position result: %1, distance: %2", traceScore.ToString(), vector.Distance(trace.Start, trace.End).ToString());
			hasVisibleTarget = true;
			break;
		}

		if (!hasVisibleTarget)
			return;

		CallForSupport(target);
	}

	override void EOnSimulate(IEntity owner, float timeSlice)
	{
		Physics physics = owner.GetPhysics();
		if (!physics)
			return;
		
		physics.SetVelocity(m_vVelocity);
	}

	private void CallForSupport(IEntity target)
	{
		if (!target)
			return;

		ESCT_SupportSystem supportSystem = ESCT_SupportSystem.GetSystem();
		if (!supportSystem)
		{
			ESCT_Logger.Error("[ESCT_HeliFlyByComponent] Can't call for support - Support System wasn't found! Check if call was made by the server!");
			return;
		}

		ESCT_Logger.InfoFormat("[ESCT_HeliFlyByComponent] Helicopter at %1 position noticed some of the players and called for support!", m_Helicopter.GetOrigin().ToString());

		supportSystem.CreateSupport(target, target.GetOrigin());
		ClearEventMask(m_Helicopter, EntityEvent.FRAME);
	}

	private void EnableHelicopterFly()
	{
		if (!m_Helicopter)
			return;

		VehicleHelicopterSimulation heliSim = VehicleHelicopterSimulation.Cast(m_Helicopter.FindComponent(VehicleHelicopterSimulation));
		if (!heliSim)
			return;

		heliSim.EngineStart();
		heliSim.SetThrottle(1);
		heliSim.RotorSetForceScaleState(0, 2);
		heliSim.RotorSetForceScaleState(1, 1);

		vector velOrig = m_Helicopter.GetPhysics().GetVelocity();
		vector rotVector = m_Helicopter.GetAngles();
		vector vel = {velOrig[0] + Math.Sin(rotVector[1] * Math.DEG2RAD) * 50, velOrig[1], velOrig[2] + Math.Cos(rotVector[1] * Math.DEG2RAD) * 50 };
		m_Helicopter.GetPhysics().SetVelocity(vel);
	}

	private void SpawnCrew()
	{
		FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(m_Helicopter.FindComponent(FactionAffiliationComponent));
		if (!factionAffiliation)
			return;

		SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(m_Helicopter.FindComponent(SCR_BaseCompartmentManagerComponent));
		if (!compartmentManager)
			return;

		ESCT_FactionManager factionManager = ESCT_FactionManager.GetInstance();
		if (!factionManager)
			return;

		ESCT_Faction faction = factionManager.TryGetFactionByKey(factionAffiliation.GetDefaultFactionKey());
		if (!faction)
			return;

		m_HeliCrewGroup = ESCT_CharacterHelper.CreateGroup(faction.GetFactionKey(), faction.GetBaseFactionGroup());
		SCR_AIGroup group = SCR_AIGroup.Cast(m_HeliCrewGroup);
		if (group)
			group.GetOnAgentAdded().Insert(RegisterPilot);

		ResourceName pilotPrefab = faction.GetRandomCharacterOfRole(ESCT_EUnitRole.HeliPilot);
		if (pilotPrefab)
		{
			array<BaseCompartmentSlot> compartments = {};
			compartmentManager.GetFreeCompartmentsOfType(compartments, ECompartmentType.PILOT);

			foreach (BaseCompartmentSlot slot : compartments)
			{
				slot.SpawnCharacterInCompartment(pilotPrefab, m_HeliCrewGroup, faction.GetBaseFactionGroup());
			}
		}

		ResourceName crewPrefab = faction.GetRandomCharacterOfRole(ESCT_EUnitRole.HeliCrew);
		if (crewPrefab)
		{
			array<BaseCompartmentSlot> compartments = {};
			compartmentManager.GetFreeCompartmentsOfType(compartments, ECompartmentType.TURRET);

			foreach (BaseCompartmentSlot slot : compartments)
			{
				slot.SpawnCharacterInCompartment(crewPrefab, m_HeliCrewGroup, faction.GetBaseFactionGroup());
			}
		}
	}

	private void RegisterPilot(AIAgent aiAgent)
	{
		//AI might not spawn inside vehicle if they're far away from players
		aiAgent.SetPermanentLOD(0);

		//pilots are not yet in heli by the time of this event
		GetGame().GetCallqueue().CallLater(RegisterPilotLate, 200, false, aiAgent);
	}

	private void RegisterPilotLate(AIAgent aiAgent)
	{
		if (!aiAgent)
			return;

		IEntity charEntity = aiAgent.GetControlledEntity();
		if (!charEntity)
			return;

		ChimeraCharacter character = ChimeraCharacter.Cast(charEntity);
		if (!character)
			return;

		// We cannot be pilot nor interior, if we are not seated in vehicle at all.
		if (!character.IsInVehicle())
			return;

		CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
		if (!compartmentAccess)
			return;

		BaseCompartmentSlot slot = compartmentAccess.GetCompartment();
		if (!slot || slot.GetType() != ECompartmentType.PILOT)
			return;

		SCR_DamageManagerComponent damageManager = character.GetDamageManager();
		if (!damageManager)
			return;

		damageManager.GetOnDamage().Insert(OnPilotDamage);
	}

	protected void OnPilotDamage(BaseDamageContext damageContext)
	{
		if (!damageContext || !damageContext.hitEntity)
			return;

		IEntity damagedEntity = damageContext.hitEntity;

		if (ESCT_CharacterHelper.CanFight(damagedEntity))
			return;

		SCR_AIGroup group = ESCT_CharacterHelper.GetAIGroup(damagedEntity);
		if (!group)
			return;

		array<AIAgent> agents = {};
		group.GetAgents(agents);

		bool otherPilotDeceased = false;

		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			IEntity controlledEntity = agent.GetControlledEntity();
			if (!controlledEntity || controlledEntity == damagedEntity)
				continue;

			ChimeraCharacter character = ChimeraCharacter.Cast(controlledEntity);
			if (!character)
				continue;

			CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
			if (!compartmentAccess)
				continue;

			BaseCompartmentSlot slot = compartmentAccess.GetCompartment();
			if (!slot || slot.GetType() != ECompartmentType.PILOT)
				continue;

			if (!ESCT_CharacterHelper.CanFight(damagedEntity))
			{
				otherPilotDeceased = true;
				break;
			}
		}

		if (otherPilotDeceased)
		{
			foreach (AIAgent agent : agents)
			{
				agent.SetPermanentLOD(-1);
			}

			//stop helicopter movement as both pilots can't pilot heli anymore
			ClearEventMask(m_Helicopter, EntityEvent.SIMULATE | EntityEvent.FRAME);
		}

		ChimeraCharacter character = ChimeraCharacter.Cast(damagedEntity);
		if (!character)
			return;

		SCR_DamageManagerComponent damageManager = character.GetDamageManager();
		if (!damageManager)
			return;

		damageManager.GetOnDamage().Remove(OnPilotDamage);
	}

	private void OnDestroyed(IEntity ent)
	{
		if (!m_Helicopter)
			return;

		if (m_HeliCrewGroup)
		{
			array<AIAgent> agents = {};
			m_HeliCrewGroup.GetAgents(agents);

			foreach (AIAgent agent : agents)
			{
				agent.SetPermanentLOD(-1);
			}
		}

		//stop helicopter movement as helicopter got destroyed
		ClearEventMask(m_Helicopter, EntityEvent.SIMULATE | EntityEvent.FRAME);
	}

	protected void OnHelicopterDamage(BaseDamageContext damageContext)
	{
		if (damageContext.damageType == EDamageType.COLLISION || damageContext.damageType == EDamageType.FIRE || damageContext.damageType == EDamageType.REGENERATION)
			return;

		if (!m_Helicopter || m_DamageManager.IsDestroyed() || !damageContext.instigator)
			return;

		//unsubscribe
		array<HitZone> hitZones = {};
		m_DamageManager.GetAllHitZones(hitZones);
		m_DamageManager.GetOnDamage().Remove(OnHelicopterDamage);

		for (int i = 0; i < hitZones.Count(); i++)
		{
			SCR_VehicleHitZone vehHitZone = SCR_VehicleHitZone.Cast(hitZones[i]);
			if (!vehHitZone)
				continue;

			vehHitZone.GetOnHit().Remove(OnHit);
		}
	}

	private void OnHit(IEntity vehicle, BaseDamageContext damageContext)
	{
		if (!m_Helicopter || m_DamageManager.IsDestroyed() || !damageContext.instigator)
			return;

		if (damageContext.damageType == EDamageType.COLLISION || damageContext.damageType == EDamageType.FIRE)
			return;

		IEntity entity = damageContext.instigator.GetInstigatorEntity();
		if (!entity)
			return;

		CallForSupport(entity);

		//unsubscribe
		array<HitZone> hitZones = {};
		m_DamageManager.GetAllHitZones(hitZones);
		m_DamageManager.GetOnDamage().Remove(OnHelicopterDamage);

		for (int i = 0; i < hitZones.Count(); i++)
		{
			SCR_VehicleHitZone vehHitZone = SCR_VehicleHitZone.Cast(hitZones[i]);
			if (!vehHitZone)
				continue;

			vehHitZone.GetOnHit().Remove(OnHit);
		}
	}
}
