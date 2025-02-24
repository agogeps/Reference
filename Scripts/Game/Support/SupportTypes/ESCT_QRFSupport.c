[EntityEditorProps(category: "Escapists/Entities", description: "QRF support.")]
class ESCT_QRFSupportClass : ESCT_SupportBaseClass
{
}

sealed class ESCT_QRFSupport : ESCT_SupportBase
{
	protected SCR_AIGroup m_QrfGroup;

	private bool m_bShouldFollowTarget = false;
	private ResourceName m_GroupPrefab;
	private vector m_CurrentTargetPos;

	private static const float QRF_SPAWN_RADIUS = 50.0;
	private static const float QRF_QUERY_SPACE = 2.5;

	override bool Setup(IEntity target, vector targetPosition = vector.Zero, IEntity caller = null, bool followTarget = false)
	{
		if (!super.Setup(target, targetPosition, caller, followTarget))
			return false;
		
		if (m_eSupportState != ESCT_ESupportState.New)
			return false;

		ESCT_EGroupSize groupSize = ESCT_GroupDirector.GetGroupSize();
		if (groupSize == ESCT_EGroupSize.Undefined)
			return false;

		ESCT_Faction faction = ESCT_FactionManager.GetInstance().GetEnemyFaction();
		if (!faction)
			return false;

		array<ESCT_EMilitaryBranchType> branches = {ESCT_EMilitaryBranchType.SF, ESCT_EMilitaryBranchType.Military};
		int index = SCR_ArrayHelper.GetWeightedIndex({10, 25}, Math.RandomFloat01());
		
		m_GroupPrefab = faction.GetRandomGroupOfBranchAndSize(branches[index], groupSize);
		if (!m_GroupPrefab || m_GroupPrefab == ResourceName.Empty)
			return false;

		m_bShouldFollowTarget = followTarget;
		m_eSupportState = ESCT_ESupportState.Ready;

		return true;
	}

	override bool Launch()
	{
		if (!super.Launch())
			return false;

		ESCT_EscapistsGameMode gameMode = ESCT_EscapistsGameMode.Cast(GetGame().GetGameMode());

		vector supportPosition;
		if (m_Target)
		{
			supportPosition = m_Target.GetOrigin();
		}
		else
		{
			supportPosition = m_vTargetPosition;
		}

		vector qrfPosition = ESCT_Misc.GetRandomValidPosition(supportPosition, 250, 350, QRF_SPAWN_RADIUS, QRF_QUERY_SPACE);
		if (qrfPosition == vector.Zero)
		{
			//FIXME: may spawn in water, it's bad, but it seems to be a last resort
			qrfPosition = gameMode.GetRandomGenerator().GenerateRandomPointInRadius(250, 350, supportPosition);
			qrfPosition[1] = SCR_TerrainHelper.GetTerrainY(qrfPosition);
		}

		if (qrfPosition == vector.Zero)
			return false;

		IEntity patrol = ESCT_SpawnHelpers.SpawnEntityPrefab(m_GroupPrefab, qrfPosition);
		m_QrfGroup = SCR_AIGroup.Cast(patrol);
		if (!m_QrfGroup)
			return false;
		
		m_QrfGroup.GetOnAgentAdded().Insert(RegisterAgent);

		ESCT_Logger.InfoFormat("Spawned %1 QRF group on %2 position.", m_QrfGroup.GetPrefabData().GetPrefabName(), m_QrfGroup.GetOrigin().ToString());

		ESCT_WaypointManagerComponent waypointManager = ESCT_WaypointManagerComponent.GetInstance();
		if (!waypointManager)
			return false;

		vector position = ESCT_EscapistsGameMode.GetGameMode().GetRandomGenerator().GenerateRandomPointInRadius(5, 10, supportPosition, false);
		position[1] = SCR_TerrainHelper.GetTerrainY(position, noUnderwater: true);
		AIWaypoint moveWaypoint = waypointManager.Move(m_QrfGroup, position);
		AIWaypoint sndWaypoint = waypointManager.SearchAndDestroy(m_QrfGroup, position, holdingTime: Math.RandomFloat(60.0, 180.0));
		
		if (m_bShouldFollowTarget && m_Target && m_Target.Type() == Vehicle)
		{
			m_CurrentTargetPos = m_Target.GetOrigin();

			//subscribe for subsequent car alarm triggers
			ESCT_CarAlarmComponent carAlarm = ESCT_CarAlarmComponent.Cast(m_Target.FindComponent(ESCT_CarAlarmComponent));
			if (carAlarm)
			{
				carAlarm.GetOnAlarmTriggered().Insert(FollowVehicle);
			}

			FollowVehicle();
		}
		
		return true;
	}

	override bool CheckDespawn()
	{
		if (HasGracePeriod())
			return false;
		
		if (super.CheckDespawn())
			return true;

		if (m_bShouldFollowTarget)
		{
			if (!m_Target)
				return true;
			
			DamageManagerComponent damageComponent = DamageManagerComponent.Cast(m_Target.FindComponent(DamageManagerComponent));
			if (damageComponent && damageComponent.GetState() == EDamageState.DESTROYED)
				return true;
		}

		if (!m_QrfGroup)
			return true;

		return false;
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
		
		super.Despawn();
	}

	private void FollowVehicle()
	{
		//TODO: just link the waypoint to entity (.SetEntity()?)	
		if (!m_Target)
			return;

		if (m_eSupportState == ESCT_ESupportState.Finished)
			return;

		DamageManagerComponent damageComponent = DamageManagerComponent.Cast(m_Target.FindComponent(DamageManagerComponent));
		if (damageComponent && damageComponent.GetState() == EDamageState.DESTROYED)
			return;

		ESCT_CarAlarmComponent carAlarm = ESCT_CarAlarmComponent.Cast(m_Target.FindComponent(ESCT_CarAlarmComponent));
		if (carAlarm && carAlarm.GetAlarmState() != ESCT_CarAlarmState.Active)
			return;

		if (vector.Distance(m_CurrentTargetPos, m_Target.GetOrigin()) < 35.0)
		{
			//position hasn't changed, check later again
			GetGame().GetCallqueue().CallLater(FollowVehicle, 1000, false);
			return;
		}

		m_CurrentTargetPos = m_Target.GetOrigin();

		array<AIWaypoint> waypoints = {};
		m_QrfGroup.GetWaypoints(waypoints);

		if (waypoints.IsEmpty())
		{
			ESCT_WaypointManagerComponent waypointManager = ESCT_WaypointManagerComponent.GetInstance();
			if (waypointManager)
			{
				AIWaypoint sndWaypoint = waypointManager.SearchAndDestroy(m_QrfGroup, m_CurrentTargetPos, holdingTime: Math.RandomFloat(60.0, 180.0));
				waypoints.Insert(sndWaypoint);
			}
		}
		else 
		{
			for (int i = 0; i < waypoints.Count(); i++)
			{
				m_QrfGroup.RemoveWaypointAt(0);
			}
			
			vector position = ESCT_EscapistsGameMode.GetGameMode().GetRandomGenerator().GenerateRandomPointInRadius(5, 10, m_CurrentTargetPos, false);
			position[1] = SCR_TerrainHelper.GetTerrainY(position, noUnderwater: true);
			
			ESCT_WaypointManagerComponent waypointManager = ESCT_WaypointManagerComponent.GetInstance();
			AIWaypoint moveWaypoint = waypointManager.Move(m_QrfGroup, position);
			AIWaypoint sndWaypoint = waypointManager.SearchAndDestroy(m_QrfGroup, position, holdingTime: Math.RandomFloat(60.0, 180.0));
		}

		GetGame().GetCallqueue().CallLater(FollowVehicle, 1000, false);
	}
	
	private void RegisterAgent(AIAgent agent)
	{
		if (!agent)
			return;
		
		agent.SetPermanentLOD(0);
	}
}
