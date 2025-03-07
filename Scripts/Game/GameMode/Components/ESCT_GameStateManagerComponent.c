[ComponentEditorProps(category: "Escapists/GameMode/Components", description: "Components that handles progression of Escapists game mode.")]
class ESCT_GameStateManagerComponentClass: ESCT_GameModeBaseComponentClass
{
}

void ScriptInvokerGameStateMethod(ESCT_EGameState gameState);
typedef func ScriptInvokerGameStateMethod;
typedef ScriptInvokerBase<ScriptInvokerGameStateMethod> ScriptInvokerGameState;

sealed class ESCT_GameStateManagerComponent: ESCT_GameModeBaseComponent
{
	protected ref ScriptInvokerGameState m_OnGameStateChanged;
	
	[RplProp()]
	protected ESCT_EGameState m_GameState = ESCT_EGameState.Undefined;
	
	private static ESCT_GameStateManagerComponent s_Instance = null;
	static ESCT_GameStateManagerComponent GetInstance()
	{
		if (!s_Instance)
		{
			BaseGameMode gameMode = GetGame().GetGameMode();
			if (gameMode)
			{
				s_Instance = ESCT_GameStateManagerComponent.Cast(gameMode.FindComponent(ESCT_GameStateManagerComponent));
			}
		}

		return s_Instance;
	}
	
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (SCR_Global.IsEditMode()) return;
		if (!Replication.IsServer()) return;

		SetEventMask(owner, EntityEvent.INIT);
	}
	
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		if (SCR_Global.IsEditMode() || !Replication.IsServer()) return;
		
		ESCT_Logger.Info("Initializing Game State Manager.");
		
		m_GameState = ESCT_EGameState.PreStart;
		
		ESCT_Logger.Info("Game State Manager initialized.");
	}
		
	ScriptInvokerGameState GetOnGameStateChanged()
	{
		if (!m_OnGameStateChanged)
		{
			m_OnGameStateChanged = new ScriptInvokerGameState();
		}
		
		return m_OnGameStateChanged;
	}
	
	ESCT_EGameState GetGameState()
	{
		return m_GameState;
	}
	
	void SetGameState(ESCT_EGameState newGameState)
	{
		ESCT_Logger.InfoFormat("Received new game state: %1", typename.EnumToString(ESCT_EGameState, newGameState));
		
		// going backwards in terms of game progression is not possible
		if (m_GameState >= newGameState)
		{
			return;
		}
		
		m_GameState = newGameState;
		Replication.BumpMe();
		
		//subscribers handle their own systems to tune themeselves to game state stage changes (start menu->prison->comm center search->helcopter search)
		GetOnGameStateChanged().Invoke(m_GameState);
	}
	
	bool ShouldEnableAI()
	{
		return m_GameState > ESCT_EGameState.Start;
	}
}
