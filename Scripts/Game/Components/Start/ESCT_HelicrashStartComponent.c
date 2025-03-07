[EntityEditorProps("Prison", description: "Component that handles helicrash starts.", color: "0 0 255 255")]
class ESCT_HelicrashStartComponentClass : ESCT_StartAreaComponentClass
{
}

sealed class ESCT_HelicrashStartComponent : ESCT_StartAreaComponent
{
	private ref array<IEntity> m_aHelicopters;

	private IEntity m_Helicopter;

	IEntity GetHelicopter()
	{
		return m_Helicopter;
	}

	override void Setup()
	{
		super.Setup();

		SetHelicopter();

		GetGame().GetCallqueue().CallLater(SetRadioTask, Math.RandomInt(8000, 16000), false);
	}

	private void SetRadioTask()
	{
		ESCT_GameStateManagerComponent.GetInstance().SetGameState(ESCT_EGameState.RadioStationSearch);
		GetGame().GetCallqueue().CallLater(ProvideFeedback, Math.RandomInt(30000, 60000), false);

		ESCT_Location location = ESCT_Location.Cast(GetOwner());
		if (location)
			location.RemoveSpawnReason(ESCT_ELocationSpawnReason.Start);
	}

	private void ProvideFeedback()
	{
		ESCT_EscapistsGameMode gameMode = ESCT_EscapistsGameMode.GetGameMode();
		if (!gameMode)
			return;

		ESCT_EscapistsManager manager = gameMode.GetEscapistsManager();
		if (!manager)
			return;

		manager.GetOnRunStart().Invoke();
	}

	private void SetHelicopter()
	{
		//unfortunately adding hierarchy component to heli completely breaks it so we have to query it in the game world
		IEntity composition = GetOwner();
		if (!composition)
			return;

		m_aHelicopters = {};

		BaseWorld baseWorld = composition.GetWorld();
		baseWorld.QueryEntitiesBySphere(composition.GetOrigin(), 10, InsertEntity, FilterEntities, queryFlags: EQueryEntitiesFlags.DYNAMIC);

		m_Helicopter = m_aHelicopters.GetRandomElement();
		if (!m_Helicopter)
			ESCT_Logger.ErrorFormat("[ESCT_HelicrashStartComponent] Failed to find helicopter entity for %1 location.", GetOwner().ToString());

		delete m_aHelicopters;
	}

	private bool InsertEntity(IEntity entity)
	{
		if (entity.FindComponent(ESCT_HelicrashHelicopterComponent))
			m_aHelicopters.Insert(entity);

		return true;
	}

	private bool FilterEntities(IEntity entity)
	{
		return true;
	}
}
