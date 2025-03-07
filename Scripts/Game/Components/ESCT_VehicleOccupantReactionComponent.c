[EntityEditorProps("Escapists", description: "Component that handles various reactions of vehicle occupants.", color: "0 0 255 255")]
class ESCT_VehicleOccupantReactionComponentClass : ScriptComponentClass
{
}

sealed class ESCT_VehicleOccupantReactionComponent : ScriptComponent
{
	protected RoadNetworkManager m_RoadNetworkManager;
	protected SCR_WheeledDamageManagerComponent m_DamageManager;
	protected IEntity m_Vehicle;
	
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (SCR_Global.IsEditMode() || !Replication.IsServer()) return;

		SetEventMask(owner, EntityEvent.INIT);
	}
	
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (SCR_Global.IsEditMode() || !Replication.IsServer()) return;
		
		m_Vehicle = owner;

		SCR_VehicleFactionAffiliationComponent factionComponent = SCR_VehicleFactionAffiliationComponent.Cast(owner.FindComponent(SCR_VehicleFactionAffiliationComponent));
		if (factionComponent && factionComponent.GetDefaultFactionKey() != "CIV")
			return;
		
		GetGame().GetCallqueue().Call(LateApply);
	}
	
	private void LateApply()
	{
		SCR_BaseCompartmentManagerComponent compManager = SCR_BaseCompartmentManagerComponent.Cast(m_Vehicle.FindComponent(SCR_BaseCompartmentManagerComponent));
		if (!compManager)
			return;
		
		array<IEntity> occupants = {};
		compManager.GetOccupantsOfType(occupants, ECompartmentType.PILOT);
		
		if (occupants.IsEmpty())
			return;
		
		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		if (aiWorld)
			m_RoadNetworkManager = aiWorld.GetRoadNetworkManager();
		
		m_DamageManager = SCR_WheeledDamageManagerComponent.Cast(m_Vehicle.FindComponent(SCR_WheeledDamageManagerComponent));
		if (!m_DamageManager)
			return;
		
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
	}
	
	private void OnHit(IEntity vehicle, BaseDamageContext damageContext)
	{
		if (!m_Vehicle || damageContext.damageType == EDamageType.COLLISION)
			return;

		Vehicle veh = Vehicle.Cast(m_Vehicle);
		if (!veh)
			return;
		
		RemoveEventHandler();
		
		IEntity driver = veh.GetPilot();
		if (!driver || EntityUtils.IsPlayer(driver) || m_DamageManager.IsDestroyed())
			return;
		
		SCR_AIGroup group = ESCT_CharacterHelper.GetAIGroup(driver);
		if (!group)
			return;
		
		array<AIWaypoint> waypoints = {};
		group.GetWaypoints(waypoints);
		for (int i = 0; i < waypoints.Count(); i++)
		{
			group.RemoveWaypointAt(0);
		}	
		
		AICharacterMovementComponent movementComponent = AICharacterMovementComponent.Cast(driver.FindComponent(AICharacterMovementComponent));
		if (movementComponent)
			movementComponent.SetMovementTypeWanted(EMovementType.SPRINT);
		
		AICarMovementComponent carMovementComp = AICarMovementComponent.Cast(vehicle.FindComponent(AICarMovementComponent));
		if (carMovementComp)
			carMovementComp.ResetCruiseSpeed();
		
		RandomGenerator gen = ESCT_EscapistsGameMode.GetGameMode().GetRandomGenerator();
		
		ESCT_WaypointManagerComponent waypointManager = ESCT_WaypointManagerComponent.GetInstance();
		
		if (gen.RandInt(0, 100 < 45))
		{
			//run by vehicle
			vector initialPosition = gen.GenerateRandomPointInRadius(300, 1000, veh.GetOrigin());
			vector vehicleRetreatPosition = ESCT_RoadNavigation.GetClosestRoadPosition(initialPosition);
			if (vehicleRetreatPosition != vector.Zero)
			{
				waypointManager.Move(group, vehicleRetreatPosition);
				waypointManager.VehiclePatrol(group, veh.GetOrigin(), Math.RandomFloat(50, 500));
			}
		}
		else 
		{
			//run by feet
			CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(driver.FindComponent(CompartmentAccessComponent));
			if (compartmentAccess)
				compartmentAccess.GetOutVehicle(EGetOutType.ANIMATED, 0, ECloseDoorAfterActions.LEAVE_OPEN, false);	
			
			vector retreatPosition = ESCT_Misc.GetRandomValidPosition(veh.GetOrigin(), 300, 1000, 100, 0);
			if (retreatPosition != vector.Zero)
				AIWaypoint moveWaypoint = ESCT_WaypointManagerComponent.GetInstance().Move(group, retreatPosition);
		}
	}

	private void RemoveEventHandler()
	{
		array<HitZone> hitZones = {};
		m_DamageManager.GetAllHitZones(hitZones);
		
		SCR_VehicleHitZone vehHitZone;
		for (int i = 0; i < hitZones.Count(); i++)
		{
			vehHitZone = SCR_VehicleHitZone.Cast(hitZones[i]);
			if (!vehHitZone)
				continue;
			
			vehHitZone.GetOnHit().Remove(OnHit);		
		}
	}
}
