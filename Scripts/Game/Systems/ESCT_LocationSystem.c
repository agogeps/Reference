sealed class ESCT_LocationSystem : GameSystem
{
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Start area prefab.", params: "et", category: "Escapists")]
	protected ResourceName m_StartAreaLocationPrefab;

	protected ref array<ESCT_Location> m_aLocations = {};
	protected ref array<IEntity> m_aStartAreas = {};
	protected ref array<IEntity> m_aStaticLocations = {};
	protected ref array<ESCT_RandomLocationHolder> m_aRandomLocationHolders = {};

	protected IEntity m_StartingPrison = null;

	private PlayerManager m_PlayerManager;
	private ref array<IEntity> m_aPlayers;
	private float m_fLastExecutionTime = 0;

	private static const float CHECK_INTERVAL = 1000.0;

	static ESCT_LocationSystem GetSystem()
	{
		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return null;

		return ESCT_LocationSystem.Cast(world.FindSystem(ESCT_LocationSystem));
	}

	protected override void OnInit()
	{
		super.OnInit();

		if (SCR_Global.IsEditMode() || !Replication.IsServer())
			return;

		ESCT_Logger.Info("Initializing Location System.");

		m_aPlayers = {};
		m_PlayerManager = GetGame().GetPlayerManager();

		ESCT_GameStateManagerComponent gameStateManager = ESCT_GameStateManagerComponent.GetInstance();
		if (gameStateManager)
			gameStateManager.GetOnGameStateChanged().Insert(HandleGameStateChanged);

		ESCT_EscapistsGameMode gameMode = ESCT_EscapistsGameMode.GetGameMode();
		if (gameMode)
			gameMode.GetOnGameStateChanged().Insert(OnGameStateChanged);

		Enable(false);

		ESCT_Logger.Info("Location Manager initialized.");
	}

	protected override void OnUpdate(ESystemPoint point)
	{
		super.OnUpdate(point);

		float currentTime = GetWorld().GetWorldTime();
		if (currentTime - m_fLastExecutionTime < 1)
			return;

		m_fLastExecutionTime = currentTime + CHECK_INTERVAL;

		if (m_PlayerManager.GetPlayerCount() == 0)
			return;

		m_aPlayers.Clear();
		ESCT_Player.GetPlayers(m_aPlayers);

		ESCT_Location location;
		IEntity player;
		ESCT_ELocationSpawnReason spawnReasons;
		ESCT_ESpawnState spawnState;

		for (int i = 0; i < m_aLocations.Count(); i++)
		{
			location = m_aLocations[i];

			bool isIntersected = false;
			for (int j = 0; j < m_aPlayers.Count(); j++)
			{
				player = m_aPlayers[j];
				if (ESCT_Math.IsPointInRadiusXZ(player.GetOrigin(), location.GetOrigin(), location.GetSpawnDistance()))
				{
					isIntersected = true;
					break;
				}
			}

			spawnReasons = location.GetSpawnReasons();
			spawnState = location.GetSpawnState();

			bool hasPlayerFlag = SCR_Enum.HasPartialFlag(spawnReasons, ESCT_ELocationSpawnReason.PlayerInDistance);
			bool shouldAddDistanceFlag = isIntersected && !hasPlayerFlag;
			bool shouldRemoveDistanceFlag = !isIntersected && hasPlayerFlag;

			if (shouldAddDistanceFlag)
			{
				location.AddSpawnReason(ESCT_ELocationSpawnReason.PlayerInDistance);
			}
			else if (shouldRemoveDistanceFlag)
			{
				location.RemoveSpawnReason(ESCT_ELocationSpawnReason.PlayerInDistance);
			}

			//already spawned locations live in the "gap" between these two conditions (do nothing if both conditions equal false)
			bool shouldSpawn = spawnReasons != 0 && spawnState == ESCT_ESpawnState.Idle;
			bool shouldDespawn = spawnReasons == 0 && spawnState == ESCT_ESpawnState.Spawned;

			if (shouldSpawn)
			{
				location.Spawn();
			}
			else if (shouldDespawn)
			{
				location.Despawn();
			}
		}
	}

	void GetLocations(notnull out array<ESCT_Location> locations)
	{
		locations.Copy(m_aLocations);
	}

	void AddStartArea(notnull IEntity startArea)
	{
		m_aStartAreas.Insert(startArea);
	}

	void AddStaticLocation(notnull IEntity location)
	{
		m_aStaticLocations.Insert(location);
	}

	void AddRandomLocationHolder(notnull ESCT_RandomLocationHolder locationHolder)
	{
		m_aRandomLocationHolders.Insert(locationHolder)
	}

	bool IsSomeLocationNear(vector position, float distance)
	{
		bool isSomeLocationNear = false;
		for (int i = 0; i < m_aLocations.Count(); i++)
		{
			if (ESCT_Math.IsPointInRadiusXZ(m_aLocations[i].GetOrigin(), position, distance))
			{
				isSomeLocationNear = true;
				break;
			}
		}

		return isSomeLocationNear;
	}

	IEntity GetStartArea()
	{
		return m_StartingPrison;
	}

	//------------------------------------------------------------------------------------------------
	//! \param component must not be null
	void Register(notnull ESCT_Location location)
	{
		//About to be deleted
		if (location.IsDeleted() || (location.GetFlags() & EntityFlags.USER5))
			return;

		if (m_aLocations.Find(location) != -1)
			return;

		m_aLocations.Insert(location);
	}

	void DespawnLocations()
	{
		foreach (ESCT_Location location : m_aLocations)
		{
			if (location.GetSpawnState() == ESCT_ESpawnState.Spawned)
			{
				location.Despawn();
			}
		}
	}

	void PrepareLocations()
	{
		ESCT_LocationPreparer preparer = new ESCT_LocationPreparer(m_aLocations, m_aStaticLocations, m_aRandomLocationHolders, m_StartingPrison);
		preparer.InititalizeLocations();

		delete m_aStaticLocations;
		delete m_aRandomLocationHolders;
	}

	void SelectStartArea()
	{
		ESCT_StartAreaSelector areaSelector = new ESCT_StartAreaSelector(m_aStartAreas, m_StartAreaLocationPrefab);
		m_StartingPrison = areaSelector.SelectStartArea();

		//no longer needed
		delete m_aStartAreas;
	}

	void SelectHelipad()
	{
		ESCT_Logger.Info("Selecting escape vehicle location.");

		array<IEntity> players = {};
		ESCT_Player.GetAlivePlayers(players, true);

		array<ESCT_Helipad> helipads = {};
		foreach (ESCT_Location location : m_aLocations)
		{
			foreach (IEntity player : players)
			{
				//ignore close locations
				if (ESCT_Math.IsPointInRadiusXZ(player.GetOrigin(), location.GetOrigin(), location.GetSpawnDistance() / 2))
					continue;
			}

			if (location.GetLocationType() == ESCT_ELocationType.Helipad && location.GetSpawnState() == ESCT_ESpawnState.Disabled)
				helipads.Insert(ESCT_Helipad.Cast(location));
		}

		if (helipads.IsEmpty())
		{
			ESCT_Logger.Info("No more locations, forcing ending.");
			ESCT_WinLoseHandlerComponent winLoseHandler = ESCT_WinLoseHandlerComponent.GetInstance();
			if (winLoseHandler)
			{
				winLoseHandler.SetNoMoreVehicles();
			}
			return;
		}

		ESCT_Helipad activeHelipad = helipads[helipads.GetRandomIndex()];
		activeHelipad.Initialize();

		ESCT_TaskAssignerComponent taskAssigner = ESCT_TaskAssignerComponent.GetInstance();
		if (taskAssigner)
			GetGame().GetCallqueue().CallLater(taskAssigner.CreateExtractionPointTask, Math.RandomInt(1000, 4000), false, activeHelipad.GetOrigin());

		ESCT_Logger.InfoFormat("Escape vehicle location at %1 position selected.", param1: activeHelipad.GetOrigin().ToString());
	}

	private void HandleGameStateChanged(ESCT_EGameState gameState)
	{
		if (gameState != ESCT_EGameState.Transport)
			return;

		SelectHelipad();

		ESCT_GameStateManagerComponent.GetInstance().GetOnGameStateChanged().Remove(HandleGameStateChanged);
	}

	private void OnGameStateChanged(SCR_EGameModeState state)
	{
		switch (state)
		{
			case SCR_EGameModeState.GAME:
				Enable(true);
				break;
			case SCR_EGameModeState.POSTGAME:
				Enable(false);
				break;
		}
	}
}
