[EntityEditorProps("Prison", description: "Component that attaches to prison door and tracks it's lockpicking state.")]
class ESCT_PrisonDoorComponentClass : ScriptComponentClass
{
}

sealed class ESCT_PrisonDoorComponent : ScriptComponent
{
	[RplProp()]
	protected bool m_bIsUnlocked = false;

	protected ref ESCT_LockpickDoor m_LockpickAction;
	
	private ref ScriptInvokerVoid m_OnDoorLockpick;
	
	ScriptInvokerVoid GetOnDoorLockpick()
	{
		if (!m_OnDoorLockpick)
		{
			m_OnDoorLockpick = new ScriptInvokerVoid();
		}

		return m_OnDoorLockpick;
	}
	
	void UnlockDoor(IEntity pUserEntity)
	{
		SetDoorUnlockState(true);
		
		ESCT_GameStateManagerComponent.GetInstance().SetGameState(ESCT_EGameState.RadioStationSearch);
		
		if (m_OnDoorLockpick)
			m_OnDoorLockpick.Invoke();
		
		ProvideFeedback();
	}
	
	void SetDoorUnlockState(bool value)
	{
		m_bIsUnlocked = value;
		Replication.BumpMe();
	}
	
	bool GetDoorUnlockState()
	{
		return m_bIsUnlocked;
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
}
