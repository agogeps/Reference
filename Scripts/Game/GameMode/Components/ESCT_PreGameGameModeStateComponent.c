[ComponentEditorProps(category: "Escapists/GameMode/Components", description: "Game Mode component responsible for adding a 'pre-game' period to the game mode.")]
class ESCT_PreGameGameModeStateComponentClass : SCR_BaseGameModeStateComponentClass
{
}

//------------------------------------------------------------------------------------------------
/*!
	Game mode component that handles the pre-game period.
*/
sealed class ESCT_PreGameGameModeStateComponent : SCR_BaseGameModeStateComponent
{
	[Attribute("0", uiwidget: UIWidgets.CheckBox, "Disables player controls in this state if checked.", category: "Game Mode")]
	protected bool m_bDisableControls;
	
	private ESCT_EscapistsGameMode m_gameMode;
	
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (SCR_Global.IsEditMode() || !Replication.IsServer()) return;

		SetEventMask(owner, EntityEvent.INIT);
	}

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (SCR_Global.IsEditMode() || !Replication.IsServer()) return;

		m_gameMode = ESCT_EscapistsGameMode.Cast(GetGameMode());
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Called on authority to check if transition to next state can occur in current step.
		\param nextState The state to transition into.
	*/
	override bool CanAdvanceState(SCR_EGameModeState nextState)
	{
		if (!m_gameMode)
			return false;
		
		return m_gameMode.IsGameLaunched();
	}
	
	override bool GetAllowControls()
	{
		return !m_bDisableControls;
	}
	
	override SCR_EGameModeState GetAffiliatedState()
	{
		return SCR_EGameModeState.PREGAME;
	}
}
