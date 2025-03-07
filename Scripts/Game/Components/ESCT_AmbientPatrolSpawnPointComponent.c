class ESCT_AmbientPatrolSpawnPointComponentClass : SCR_AmbientPatrolSpawnPointComponentClass
{
}

class ESCT_AmbientPatrolSpawnPointComponent : SCR_AmbientPatrolSpawnPointComponent
{
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode() || !Replication.IsServer()) return;

		if (ESCT_GameStateManagerComponent.GetInstance().GetGameState() <= ESCT_EGameState.Start)
		{
			ESCT_GameStateManagerComponent.GetInstance().GetOnGameStateChanged().Insert(HandleGameStateChanged);
		}
	}

	void Register()
	{
		ESCT_EscapistsConfigComponent config = ESCT_EscapistsConfigComponent.GetInstance();
		if (!config)
			return;

		SCR_AmbientPatrolSystem manager = SCR_AmbientPatrolSystem.GetInstance();
		if (!manager)
			return;

		ESCT_FactionManager factionManager = ESCT_FactionManager.GetInstance();
		if (!factionManager)
			return;

		SCR_FactionAffiliationComponent factionComponent = SCR_FactionAffiliationComponent.Cast(GetOwner().FindComponent(SCR_FactionAffiliationComponent));
		if (!factionComponent)
			return;

		//skip FIA points if faction is not enabled in game settings
		if (factionComponent.GetDefaultFactionKey() == factionManager.GetInsurgentFaction().GetFactionKey())
		{
			if (!config.IsInsurgencyEnabled()) 
				return;
			
			manager.RegisterPatrol(this);
			return;
		}
		
		string factionKey = factionManager.GetEnemyFactionKey();
		factionComponent.SetAffiliatedFactionByKey(factionKey);
		manager.RegisterPatrol(this);
	}

	private void HandleGameStateChanged(ESCT_EGameState gameState)
	{
		if (gameState != ESCT_EGameState.RadioStationSearch)
			return;

		Register();

		ESCT_GameStateManagerComponent.GetInstance().GetOnGameStateChanged().Remove(HandleGameStateChanged);
	}
}
