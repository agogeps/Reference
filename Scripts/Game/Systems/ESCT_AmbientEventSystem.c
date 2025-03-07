sealed class ESCT_AmbientEventSystem : GameSystem
{
	[Attribute("", UIWidgets.Auto, category: "Escapists")]
	protected ref ESCT_AmbientSystemConfig m_AmbientSystemConfig;

	protected ref array<ESCT_AmbientEventBase> m_aActiveEvents = {};
	protected ref map<ESCT_EAmbientEventType, ref ESCT_AmbientEventInfo> m_mInfoMap = new map<ESCT_EAmbientEventType, ref ESCT_AmbientEventInfo>();

	private PlayerManager m_PlayerManager;
	private ESCT_GameStateManagerComponent m_gameStateManager;

	private ref array<int> m_aPlayers;
	private float m_fEventChance = 0;
	private float m_fLastExecutionTime = 0;
	private ref ESCT_AmbientEventPicker m_EventPicker;

	private static const float CHECK_TICK = 5000;

	static ESCT_AmbientEventSystem GetSystem()
	{
		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return null;

		return ESCT_AmbientEventSystem.Cast(world.FindSystem(ESCT_AmbientEventSystem));
	}

	bool AddExternalEvent(notnull ESCT_AmbientEventBase ambientEvent)
	{
		return m_aActiveEvents.Insert(ambientEvent);
	}

	array<ESCT_AmbientEventBase> GetActiveEvents()
	{
		return m_aActiveEvents;
	}

	void IncreaseEventFrequency()
	{
		ESCT_EscapistsConfigComponent config = ESCT_EscapistsConfigComponent.GetInstance();
		if (!config)
			return;

		int tick = config.GetAmbientEventsFrequency();
		if (tick == -1)
		{
			ESCT_Logger.Info("[ESCT_AmbientEventSystem] Frequency of ambient events will be not increased as the entire ambient events system is disabled.");
			return;
		}
		
		int moreFrequentTick = (tick / 2) * 1000;
		if (moreFrequentTick < 20)
		{
			ESCT_Logger.InfoFormat(
				"[ESCT_AmbientEventSystem] Ambient events will be not increased as it's not supposed to be THAT frequent. Discarded tick value : %1", 
				moreFrequentTick.ToString()
			);
			return;
		}

		GetGame().GetCallqueue().Remove(AmbientEventCreationLoop);
		GetGame().GetCallqueue().CallLater(AmbientEventCreationLoop, moreFrequentTick, true);
	}

	protected override void OnInit()
	{
		super.OnInit();

		if (SCR_Global.IsEditMode() || !Replication.IsServer())
			return;

		ESCT_Logger.Info("[ESCT_AmbientEventSystem] Initializing Ambient Event System.");

		m_aPlayers = {};

		FillEventsMap();

		m_PlayerManager = GetGame().GetPlayerManager();

		ESCT_EscapistsGameMode gameMode = ESCT_EscapistsGameMode.GetGameMode();
		if (gameMode)
			gameMode.GetOnGameStateChanged().Insert(OnGameStateChanged);

		m_EventPicker = new ESCT_AmbientEventPicker(m_aActiveEvents, m_mInfoMap);

		Enable(false);

		ESCT_Logger.Info("[ESCT_AmbientEventSystem] Ambient Event System initialized.");
	}

	protected override void OnUpdate(ESystemPoint point)
	{
		super.OnUpdate(point);

		float currentTime = GetWorld().GetWorldTime();

		if (currentTime - m_fLastExecutionTime < 1)
			return;

		m_fLastExecutionTime = currentTime + CHECK_TICK;

		ESCT_AmbientEventBase ambientEvent = null;
		for (int i = m_aActiveEvents.Count() - 1; i >= 0; i--)
		{
			ambientEvent = m_aActiveEvents[i];
			if (!ambientEvent || !ambientEvent.CheckDespawn())
				continue;

			ESCT_Logger.InfoFormat("%1 event will be despawned - %2 type.",
				ambientEvent.ToString(),
				typename.EnumToString(ESCT_EAmbientEventType, ambientEvent.GetAmbientEventType())
			);

			ambientEvent.Despawn();
			m_aActiveEvents.Remove(i);
		}
	}

	protected void AmbientEventCreationLoop()
	{
		if (m_PlayerManager.GetPlayerCount() == 0)
			return;

		if (Math.RandomFloat01() > m_fEventChance)
		{
			m_fEventChance += Math.RandomFloat(-0.01, 0.075);
			ESCT_Logger.InfoFormat("[ESCT_AmbientEventSystem] Idle tick - event %1 chance.", m_fEventChance.ToString(len: 2));
			return;
		}

		ESCT_Logger.InfoFormat("[ESCT_AmbientEventSystem] Trying to create ambient event, chance was %1", m_fEventChance.ToString(len: 2));
		m_fEventChance = 0;

		CreateAmbientEvent();
	}

	private void CreateAmbientEvent()
	{
		m_PlayerManager.GetPlayers(m_aPlayers);
		if (m_aPlayers.IsEmpty())
			return;

		IEntity randomPlayer = m_PlayerManager.GetPlayerControlledEntity(m_aPlayers.GetRandomElement());
		if (!randomPlayer)
			return;

		vector targetPosition = randomPlayer.GetOrigin();

		ESCT_AmbientEventInfo ambientEventInfo = m_EventPicker.PickEvent();
		if (!ambientEventInfo)
			return;

		ESCT_Logger.InfoFormat("[ESCT_AmbientEventSystem] Picked event of %1 to create on %2 position.", typename.EnumToString(ESCT_EAmbientEventType, ambientEventInfo.GetAmbientEventType()), targetPosition.ToString());
		ResourceName eventPrefab = ambientEventInfo.GetEventPrefab();

		if (!eventPrefab)
		{
			ESCT_Logger.ErrorFormat("[ESCT_AmbientEventSystem] Failed to select event type at %1 position!", targetPosition.ToString());
			return;
		}

		IEntity eventEntity = ESCT_SpawnHelpers.SpawnEntityPrefab(eventPrefab, targetPosition, global: false);
		if (!eventEntity)
		{
			ESCT_Logger.ErrorFormat("[ESCT_AmbientEventSystem] Failed to spawn %1 ambient event prefab at %2 position!", eventPrefab, targetPosition.ToString());
			return;
		}

		ESCT_AmbientEventBase ambientEvent = ESCT_AmbientEventBase.Cast(eventEntity);
		if (!ambientEvent)
		{
			ESCT_Logger.ErrorFormat("[ESCT_AmbientEventSystem] Failed to cast %1 support prefab at %2 position!", eventPrefab, targetPosition.ToString());
			return;
		}

		ambientEvent.Launch();

		m_aActiveEvents.Insert(ambientEvent);
	}

	private void FillEventsMap()
	{
		if (!m_AmbientSystemConfig)
			return;

		array<ref ESCT_AmbientEventInfo> ambientEvents = {};
		m_AmbientSystemConfig.GetAmbientEvents(ambientEvents);

		foreach (ESCT_AmbientEventInfo eventInfo : ambientEvents)
		{
			ESCT_EAmbientEventType type = eventInfo.GetAmbientEventType();
			m_mInfoMap.Insert(type, eventInfo);
			ESCT_Logger.InfoFormat("[ESCT_AmbientEventSystem] %1 ambient event added to event types map.", typename.EnumToString(ESCT_EAmbientEventType, type));
		}
	}

	private void OnGameStateChanged(SCR_EGameModeState state)
	{
		switch (state)
		{
			case SCR_EGameModeState.GAME:
				EnableAmbientEvents();
				break;
			case SCR_EGameModeState.POSTGAME:
				Enable(false);
				GetGame().GetCallqueue().Remove(AmbientEventCreationLoop);
				break;
		}
	}

	private void EnableAmbientEvents()
	{
		ESCT_EscapistsConfigComponent config = ESCT_EscapistsConfigComponent.GetInstance();
		if (!config)
			return;

		int tick = config.GetAmbientEventsFrequency();
		if (tick == -1)
		{
			ESCT_Logger.Info("[ESCT_AmbientEventSystem] Ambient events system will be not enabled as there's no tick.");
			return;
		}

		GetGame().GetCallqueue().CallLater(AmbientEventCreationLoop, tick * 1000, true);
		Enable(true);

		ESCT_Logger.Info("[ESCT_AmbientEventSystem] Ambient events system has been enabled.");
	}

	void ~ESCT_AmbientEventSystem()
	{
		GetGame().GetCallqueue().Remove(AmbientEventCreationLoop);
	}
}
