[ComponentEditorProps(category: "Escapists/GameMode/Components", description: "Handles visibility of various UI screens and menus.")]
class ESCT_StartupUIHandlerComponentClass: ESCT_GameModeBaseComponentClass
{
}

sealed class ESCT_StartupUIHandlerComponent: ESCT_GameModeBaseComponent
{
	protected PlayerManager m_PlayerManager;
	
	private ref array<int> m_aPlayerIds;
	private MenuBase m_WaitScreen;
	
	private static ESCT_StartupUIHandlerComponent s_Instance = null;
	static ESCT_StartupUIHandlerComponent GetInstance()
	{
		if (!s_Instance)
		{
			BaseGameMode gameMode = GetGame().GetGameMode();
			if (gameMode)
			{
				s_Instance = ESCT_StartupUIHandlerComponent.Cast(gameMode.FindComponent(ESCT_StartupUIHandlerComponent));
			}
		}

		return s_Instance;
	}
	
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		if (SCR_Global.IsEditMode()) return;
		
		SetEventMask(owner, EntityEvent.INIT);
	}

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (SCR_Global.IsEditMode()) return;
		
		ESCT_Logger.Info("Initializing startup UI handler...");

		m_PlayerManager = GetGame().GetPlayerManager();
		
//#ifdef WORKBENCH
//		ESCT_ScreenFadeComponent.GetInstance().FadeToBlackToStart();
//		return;
//#endif
		
		if (Replication.IsServer())
		{
			ESCT_MissionHeader header = GetGame().GetEscapistsMissionHeader();
			
			//dedicated server may use just mission header for setting up mission settings so no need to activate setup menu handover
			if (RplSession.Mode() == RplMode.Dedicated && !header.m_bUseSetupMenu)
				return;
			
			SetEventMask(owner, EntityEvent.FIXEDFRAME);
			return;
		}

		ESCT_EscapistsGameMode gameMode = GetEscapistsGameMode();
		if (gameMode && gameMode.GetState() == SCR_EGameModeState.PREGAME)
		{
			//Seems that HUD manager is not being present on game mode's EOnInit (so some components will not work in menu despite it beign fully shown as intended),
			//so we should call it in next frame
			GetGame().GetCallqueue().Call(SetWaitScreenVisibilityForClients, true);
		}
		
		ESCT_Logger.Info("Startup UI handler initialized.");
	}

	override void EOnFixedFrame(IEntity owner, float timeSlice)
	{
		super.EOnFixedFrame(owner, timeSlice);

		if (!m_aPlayerIds)
			m_aPlayerIds = {};
		else
			m_aPlayerIds.Clear();
		
		m_PlayerManager.GetPlayers(m_aPlayerIds);
		if (m_aPlayerIds.IsEmpty())
			return;

		int adminPid;
		foreach (int pid : m_aPlayerIds)
		{
			if (SCR_Global.IsAdmin(pid))
			{
				adminPid = pid;
				break;
			}
		}

		if (!adminPid)
			return;

		ShowSetupMenuToAdmin(adminPid);
		
		ClearEventMask(owner, EntityEvent.FIXEDFRAME);
		m_aPlayerIds.Clear();
		delete m_aPlayerIds;
	}
		
	override void OnGameStateChanged(SCR_EGameModeState state)
	{
		super.OnGameStateChanged(state);

		if (state != SCR_EGameModeState.GAME || Replication.IsServer())
			return;

		ESCT_Logger.Info("Hiding wait screen for clients...");
		
		//JIPs are automatically handled as OnGameStateChanged is an RplProp event handler
		//so this menu would be closed for JIPs too
		GetGame().GetCallqueue().Call(SetWaitScreenVisibilityForClients, false);
	}
	
	void SetWaitScreenVisibilityForClients(bool isVisible)
	{
		if (isVisible)
		{
			m_WaitScreen = GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.EscapistsClientWaitScreen);
		}
		else if (m_WaitScreen)
		{
			GetGame().GetMenuManager().CloseMenu(m_WaitScreen);
		}
	}

	protected void ShowSetupMenuToAdmin(int adminPid)
	{
		if (m_WaitScreen)
			GetGame().GetMenuManager().CloseMenu(m_WaitScreen);
		
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return;
		
		PlayerController pc = playerManager.GetPlayerController(adminPid);
		if (!pc)
			return;
		
		ESCT_EscapistsNetworkComponent networkComponent = ESCT_EscapistsNetworkComponent.Cast(pc.FindComponent(ESCT_EscapistsNetworkComponent));
		if (!networkComponent)
			return;
		
		networkComponent.ShowSetupMenu();
	}
}	
