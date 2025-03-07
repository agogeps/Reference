[EntityEditorProps(category: "Escapists/Entities/Locations", description: "Radio station location that will be used for distress signals.")]
class ESCT_RadioStationClass : ESCT_LocationClass
{
}

sealed class ESCT_RadioStation : ESCT_Location
{
	private ESCT_RadioState m_RadioState;
	private ESCT_Radio m_Radio = null;

	ESCT_Radio GetRadio()
	{
		return m_Radio;
	}

	override bool Spawn()
	{
		bool baseSpawn = super.Spawn();

		if (!Replication.IsServer() || !baseSpawn)
			return false;

		FindRadio();

		//keep radio state consistent between spawns
		if (m_Radio && m_Radio.GetRadioState() != m_RadioState)
		{
			m_Radio.SetRadioState(m_RadioState);
		}

		if (ESCT_GameStateManagerComponent.GetInstance().GetGameState() != ESCT_EGameState.RadioStationSearch)
			return false;

		//early spawned radio stations will subscribe later via ESCT_RadioTaskSupportEntity - SubscribeEarlyRadioStationSpawns method
		SubscribeToRadioTask();

		return true;
	}

	override bool Despawn()
	{
		if (!Replication.IsServer())
			return false;

		if (m_Radio)
			m_RadioState = m_Radio.GetRadioState();

		bool baseDespawn = super.Despawn();
		if (!baseDespawn)
			return false;

		if (ESCT_GameStateManagerComponent.GetInstance().GetGameState() != ESCT_EGameState.RadioStationSearch)
			return false;

		//no need to unsubscribe as radio entity despawns

		return true;
	}

	private void SubscribeToRadioTask()
	{
		if (!m_Radio)
		{
			ESCT_Logger.ErrorFormat("For some reason %1 Radio Station has no ESCT_Radio, couldn't subscribe to task!", ToString());
			return;
		}

		if (m_Radio.GetIsSubscribedToTask())
			return;

		ESCT_RadioTaskSupportEntity supportEntity = ESCT_RadioTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(ESCT_RadioTaskSupportEntity));
		if (!supportEntity)
			return;

		ESCT_RadioTask radioTask = ESCT_RadioTask.Cast(GetTaskManager().GetTask(supportEntity.GetTaskIdByType(ESCT_RadioTask)));
		if (!radioTask)
			return;

		m_Radio.GetOnSignalSent().Insert(radioTask.OnSignalSent);
	}

	private void FindRadio()
	{
		if (m_Radio)
			return;

		array<IEntity> children = {};
		ESCT_EntityHelper.GetAllChildren(GetComposition(), children);

		foreach (IEntity entity : children)
		{
			if (entity.Type() == ESCT_Radio)
			{
				m_Radio = ESCT_Radio.Cast(entity);
				return;
			}
		}
	}

	private void HandleGameStateChanged(ESCT_EGameState gameState)
	{
		if (gameState != ESCT_EGameState.RadioStationSearch)
			return;

		SubscribeToRadioTask();

		ESCT_GameStateManagerComponent.GetInstance().GetOnGameStateChanged().Remove(HandleGameStateChanged);
	}
}
