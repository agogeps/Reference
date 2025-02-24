[EntityEditorProps(category: "Escapists/Entities", description: "Ambient helicopter flyby.", visible: false)]
class ESCT_HeliFlybyEventClass : ESCT_AmbientEventBaseClass
{
}

[BaseContainerProps(), BaseContainerCustomStringTitleField("DO NOT USE BASE CLASS!")]
sealed class ESCT_HeliFlybyEvent : ESCT_AmbientEventBase
{
	[Attribute(defvalue: "40", UIWidgets.Slider, desc: "Flying height of helicopter realtive to terrain", params: "10 1000 1")]
	protected float m_fHeight;

	protected AIGroup m_HeliCrewGroup;
	protected IEntity m_Helicopter;

	private ESCT_Faction m_Faction;

	override bool Launch()
	{
		if (!super.Launch())
			return false;

		ESCT_EGroupSize groupSize = ESCT_GroupDirector.GetGroupSize();
		if (groupSize == ESCT_EGroupSize.Undefined)
			return false;

		m_Faction = ESCT_FactionManager.GetInstance().GetEnemyFaction();
		if (!m_Faction)
			return false;

		array<ResourceName> resources = m_Faction.GetFlybyHelicopterPrefabs();
		if (!resources || resources.IsEmpty())
			return false;

		vector transform[4];
		GetIntialHelicopterTransform(transform);

		m_Helicopter = ESCT_SpawnHelpers.SpawnEntityPrefabMatrix(resources.GetRandomElement(), transform);
		if (!m_Helicopter)
			return false;

		ESCT_HeliFlybyComponent heliComponent = ESCT_HeliFlybyComponent.Cast(m_Helicopter.FindComponent(ESCT_HeliFlybyComponent));
		if (!heliComponent)
			return false;

		m_HeliCrewGroup = heliComponent.GetHeliCrewGroup();

		EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(m_Helicopter.FindComponent(EventHandlerManagerComponent));
		if (eventHandlerManager)
			eventHandlerManager.RegisterScriptHandler("OnDestroyed", m_Helicopter, OnDestroyed);

		return true;
	}

	override bool CheckDespawn()
	{
		if (HasGracePeriod())
			return false;

		if (super.CheckDespawn())
			return true;

		if (!m_Helicopter)
			return true;
		
		if (m_Helicopter)
		{
			bool isPlayerNear = false;
			
			array<IEntity> players = {};
			ESCT_Player.GetAlivePlayers(players, true);
			for (int i = 0; i < players.Count(); i++)
			{
				if (ESCT_Math.IsPointInRadiusXZ(players[i].GetOrigin(), m_Helicopter.GetOrigin(), m_fDespawnDistance))
				{
					isPlayerNear = true;
					break;
				}
			}
			
			if (!isPlayerNear)
				return true;
		}

		return false;
	}

	override void Despawn()
	{
		if (m_HeliCrewGroup)
		{
			array<AIAgent> agents = {};
			m_HeliCrewGroup.GetAgents(agents);

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

			delete m_HeliCrewGroup;
		}

		if (m_Helicopter)
			SCR_EntityHelper.DeleteEntityAndChildren(m_Helicopter);

		super.Despawn();
	}

	private void GetIntialHelicopterTransform(out vector transform[4])
	{
		//Kex's algorhitm to set max height based on height of terrain of near the target position
		const float TERRAIN_Y_SCAN_STEP_SIZE = 100;

		ESCT_EscapistsGameMode gameMode = ESCT_EscapistsGameMode.Cast(GetGame().GetGameMode());
		vector targetPosition = gameMode.GetRandomGenerator().GenerateRandomPointInRadius(0, 350, GetOrigin());
		vector initialPosition = gameMode.GetRandomGenerator().GenerateRandomPointInRadius(1200, 2000, targetPosition);

		transform[3] = initialPosition;
		vector direction = vector.Direction(initialPosition, targetPosition);
		direction.Normalize();
		Math3D.AnglesToMatrix(Vector(direction.ToYaw(), 0, 0), transform);

		float maxTerrainY = 0;
		for (int i = 0; i < Math.Ceil(m_fDespawnDistance / TERRAIN_Y_SCAN_STEP_SIZE); i++)
		{
			maxTerrainY = Math.Max(maxTerrainY, SCR_TerrainHelper.GetTerrainY(transform[3] + i * TERRAIN_Y_SCAN_STEP_SIZE * transform[2]));
		}

		transform[3][1] = transform[3][1] + m_fHeight + maxTerrainY;
	}

	private void OnDestroyed(IEntity ent)
	{
		if (!m_Helicopter)
			return;

		//garbage collector will take care of it
		m_Helicopter = null;
	}
}
