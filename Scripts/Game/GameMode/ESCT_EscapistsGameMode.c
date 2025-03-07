class ESCT_EscapistsGameModeClass : SCR_BaseGameModeClass
{
}

void OnGameStateChangedDelegate(SCR_EGameModeState gameState);
typedef func OnGameStateChangedDelegate;
typedef ScriptInvokerBase<OnGameStateChangedDelegate> OnGameStateChangedInvoker;

sealed class ESCT_EscapistsGameMode : SCR_BaseGameMode
{
	[RplProp()]
	protected ESCT_EStartType m_StartType;
	
	protected ref OnGameStateChangedInvoker m_OnGameStateChanged;
	
	protected ESCT_EscapistsConfigComponent m_Config;
	protected ESCT_GameStateManagerComponent m_GameState;
	protected SCR_PlayerSpawnPointManagerComponent m_PlayerSpawnPointManager;
	
	protected ref ESCT_EscapistsManager m_EscapistsManager = new ESCT_EscapistsManager(this);

	private bool m_bIsGameLaunched = false;
	private ref RandomGenerator randomGen;
	
	static ESCT_EscapistsGameMode GetGameMode()
	{
		return ESCT_EscapistsGameMode.Cast(GetGame().GetGameMode());
	}
	
	OnGameStateChangedInvoker GetOnGameStateChanged()
	{
		if (!m_OnGameStateChanged)
		{
			m_OnGameStateChanged = new OnGameStateChangedInvoker();
		}

		return m_OnGameStateChanged;
	}
	
	ESCT_EscapistsManager GetEscapistsManager()
	{
		return m_EscapistsManager;
	}

	RandomGenerator GetRandomGenerator()
	{
		if (!randomGen)
		{
			randomGen = new RandomGenerator();
			Math.Randomize(-1);	
		}	
		
		return randomGen;
	}

	bool IsGameLaunched()
	{
		return m_bIsGameLaunched;
	}
	
	void SetStartType(ESCT_EStartType startType)
	{
		m_StartType = startType;
		Replication.BumpMe();
	}
	
	ESCT_EStartType GetStartType()
	{
		return m_StartType;
	}

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (SCR_Global.IsEditMode()) return;

		ESCT_Logger.Info("Game Mode Init process starts.");
		ESCT_Logger.InfoFormat("Escapists version: %1", ESCT_Constants.ESCAPISTS_VERSION);

		m_Config = ESCT_EscapistsConfigComponent.Cast(FindComponent(ESCT_EscapistsConfigComponent));
		m_GameState = ESCT_GameStateManagerComponent.Cast(FindComponent(ESCT_GameStateManagerComponent));
		m_PlayerSpawnPointManager = SCR_PlayerSpawnPointManagerComponent.Cast(FindComponent(SCR_PlayerSpawnPointManagerComponent));

		if (IsMaster())
		{
			m_GameState.GetOnGameStateChanged().Insert(HandleGameStateChanged);
		}

		if (RplSession.Mode() == RplMode.Dedicated)
			GetGame().GetCallqueue().CallLater(DetectPlayersToStart, 1000, true);

		ESCT_Logger.Info("Game Mode initialized.");
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerSpawnOnPoint_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, IEntity entity, SCR_SpawnPointSpawnData spawnPointData)
	{
		super.OnPlayerSpawnOnPoint_S(requestComponent, handlerComponent, entity, spawnPointData);

		// Location popup for player
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(requestComponent.GetPlayerId());
		if (!playerController)
			return;
			
		ESCT_EscapistsNetworkComponent networkComponent = ESCT_EscapistsNetworkComponent.Cast(playerController.FindComponent(ESCT_EscapistsNetworkComponent));
		if (networkComponent)
			networkComponent.RespawnLocationPopup();
	}
	
	void InitiateGameStart()
	{
		if (Replication.IsClient())
			return;

		//ESCT_PreGameGameModeStateComponent tracks this variable and changes game mode state to GAME as soon as it is launched, then StartServer kicks in
		m_bIsGameLaunched = true;
	}

	protected override void OnGameStateChanged()
	{
		super.OnGameStateChanged();
		
		if (GetState() != SCR_EGameModeState.GAME || !Replication.IsServer())
			return;
		
		SCR_EGameModeState currentState = GetState();
		
		StartServer();
		
		GetOnGameStateChanged().Invoke(currentState);
	}

	private void StartServer()
	{
		if (GetState() != SCR_EGameModeState.GAME)
			return;

		ESCT_GameStateManagerComponent.GetInstance().SetGameState(ESCT_EGameState.Start);

		SCR_AmbientVehicleSystem vehiclesManager = SCR_AmbientVehicleSystem.GetInstance();
		if (vehiclesManager)
			vehiclesManager.GetOnVehicleSpawned().Insert(OnAmbientVehicleSpawned);
	}

	private void HandleGameStateChanged(ESCT_EGameState gameState)
	{
		if (gameState != ESCT_EGameState.RadioStationSearch)
			return;

		//JIPs will spawn on other players
		m_PlayerSpawnPointManager.EnablePlayerSpawnPoints(true);
		m_GameState.GetOnGameStateChanged().Remove(HandleGameStateChanged);
	}

	private void OnAmbientVehicleSpawned(SCR_AmbientVehicleSpawnPointComponent spawnpoint, Vehicle vehicle)
	{
		array<SCR_FuelManagerComponent> fuelManagers = {};
		array<BaseFuelNode> fuelNodes = {};
		SCR_FuelManagerComponent.GetAllFuelManagers(vehicle, fuelManagers);

		// Remove fuel
		foreach (SCR_FuelManagerComponent fuelManager : fuelManagers)
		{
			fuelNodes.Clear();
			fuelManager.GetFuelNodesList(fuelNodes);

			foreach (BaseFuelNode fuelNode : fuelNodes)
			{
				fuelNode.SetFuel(fuelNode.GetMaxFuel() * Math.RandomFloat(0.05, 0.35));
			}
		}
		
		ESCT_CarAlarmComponent carAlarm = ESCT_CarAlarmComponent.Cast(vehicle.FindComponent(ESCT_CarAlarmComponent));
		if (carAlarm && Math.RandomFloat01() <= m_Config.GetCarAlarmChance())
			carAlarm.ArmAlarm();	
	}
	
	private void DetectPlayersToStart()
	{
		if (m_bIsGameLaunched)
		{
			GetGame().GetCallqueue().Remove(DetectPlayersToStart);
			return;
		}

		ESCT_MissionHeader header = GetGame().GetEscapistsMissionHeader();
		if (header && header.m_bUseSetupMenu)
		{
			ESCT_Logger.Info("Setup UI admin handover enabled, aborting player polling...");
			GetGame().GetCallqueue().Remove(DetectPlayersToStart);
			return;
		}
		
		ESCT_Logger.Info("Waiting for players to start...");

		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return;

		int playerCount = playerManager.GetPlayerCount();
		if (playerCount == 0)
			return;
		
		ESCT_Logger.Info("Players found, proceeding to game start!");

		ESCT_EscapistsConfigComponent gameConfig = ESCT_EscapistsConfigComponent.GetInstance();
		if (gameConfig)
			gameConfig.LoadHeaderSettings();

		ESCT_ScreenFadeComponent screenFade = ESCT_ScreenFadeComponent.Cast(FindComponent(ESCT_ScreenFadeComponent));
		if (screenFade)
			screenFade.FadeToBlackToStart();

		GetGame().GetCallqueue().Remove(DetectPlayersToStart);
	}
}
