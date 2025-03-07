class ESCT_AmbientVehicleSpawnPointComponentClass : SCR_AmbientVehicleSpawnPointComponentClass
{
}

//------------------------------------------------------------------------------------------------
class ESCT_AmbientVehicleSpawnPointComponent : SCR_AmbientVehicleSpawnPointComponent
{
	[Attribute("0", uiwidget: UIWidgets.CheckBox, "Will use enemy faction vehicle pools is checked.", category: "Escapists")]
	protected bool m_bBelongsToEnemy;
	
	private bool m_bIsRegistered = false;
			
#ifdef WORKBENCH
	private float m_fRefreshRate = 0.1;
	private float m_fTime;
	private ref Shape m_Shape;
#endif
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
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
		}
#endif
		
		if (SCR_Global.IsEditMode() || !Replication.IsServer()) return;	
		
		if (ESCT_GameStateManagerComponent.GetInstance().GetGameState() <= ESCT_EGameState.Start)
			ESCT_GameStateManagerComponent.GetInstance().GetOnGameStateChanged().Insert(HandleGameStateChanged);
	}
	
	void Register()
	{
		if (m_bIsRegistered)
			return;
		
		if (m_bBelongsToEnemy)
		{			
			ESCT_FactionManager factionManager = ESCT_FactionManager.GetInstance();
			if (!factionManager)
				return;
			
			SCR_FactionAffiliationComponent factionAffiliaton = SCR_FactionAffiliationComponent.Cast(GetOwner().FindComponent(SCR_FactionAffiliationComponent));
			if (!factionAffiliaton)
				return;
			
			ESCT_EscapistsConfigComponent gameConfig = ESCT_EscapistsConfigComponent.GetInstance();
			if (!gameConfig)
				return;
			
			bool isInsurgentFaction = factionAffiliaton.GetAffiliatedFactionKey() == factionManager.GetInsurgentFaction().GetFactionKey();

			//ignore FIA locations if FIA is not present in the run
			if (isInsurgentFaction && !gameConfig.IsInsurgencyEnabled())
				return;	
			
			//do not register locations that doesn't have something location near them
			if (!IsSomeLocationClose())
				return;
			
			if (!isInsurgentFaction)
				UpdateFaction();
		}
		
		SCR_AmbientVehicleSystem manager = SCR_AmbientVehicleSystem.GetInstance();
		if (!manager)
			return;

		manager.RegisterSpawnpoint(this);
		m_bIsRegistered = true;
	}
	
	private void UpdateFaction()
	{
		SCR_FactionAffiliationComponent factionAffiliaton = SCR_FactionAffiliationComponent.Cast(GetOwner().FindComponent(SCR_FactionAffiliationComponent));
		if (!factionAffiliaton)
			return;
		
		ESCT_Faction enemyFaction = ESCT_FactionManager.GetInstance().GetEnemyFaction();	
		factionAffiliaton.SetFaction(GetOwner(), enemyFaction);
	}
	
	private void HandleGameStateChanged(ESCT_EGameState gameState)
	{
		if (gameState < ESCT_EGameState.Start)
			return;
		
		ESCT_EscapistsConfigComponent gameConfig = ESCT_EscapistsConfigComponent.GetInstance();
		if (gameConfig) 
		{
			if (Math.RandomFloat(0,1) <= gameConfig.GetVehicleSpawnChance())
				Register();				
		}

		ESCT_GameStateManagerComponent.GetInstance().GetOnGameStateChanged().Remove(HandleGameStateChanged);
	}
	
	private bool IsSomeLocationClose()
	{
		ESCT_LocationSystem locationSystem = ESCT_LocationSystem.GetSystem();
		if (!locationSystem)
			return false;
	
		return locationSystem.IsSomeLocationNear(GetOwner().GetOrigin(), 100);
	}
	
#ifdef WORKBENCH
	override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
	{
		super._WB_AfterWorldUpdate(owner, timeSlice);
		
		if (!m_Shape)
			return;
		
		if (m_fTime < m_fRefreshRate)
		{
			m_fTime += timeSlice;
			return;
		}

		vector transform[4];
		owner.GetTransform(transform);
		m_Shape.SetMatrix(transform);
	}
#endif
}
