[ComponentEditorProps(category: "Escapists/GameMode/Components", description: "Component that handles initial screen fade.")]
class ESCT_ScreenFadeComponentClass: ESCT_GameModeBaseComponentClass
{
}

sealed class ESCT_ScreenFadeComponent: ESCT_GameModeBaseComponent
{
	[Attribute(category: "Escapists")]
	protected ref ESCT_UiContext m_preStartScreenEffectsLayout;
	
	protected ref ESCT_FadeEffect m_FadeEffect;
	
	[RplProp()]
	private bool m_bIsStarting = false;
	
	private bool m_bIsEnding = false;
	
	private static ESCT_ScreenFadeComponent s_Instance = null;
	static ESCT_ScreenFadeComponent GetInstance()
	{
		if (!s_Instance)
		{
			BaseGameMode gameMode = GetGame().GetGameMode();
			if (gameMode)
			{
				s_Instance = ESCT_ScreenFadeComponent.Cast(gameMode.FindComponent(ESCT_ScreenFadeComponent));
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
		
		Print("[Escapists] Initializing Screen Fade.");
		
		m_preStartScreenEffectsLayout.Init(owner);
		m_preStartScreenEffectsLayout.ShowLayout();
		
		Print("[Escapists] Screen Fade initalized.");
	}
	
	void FadeToBlackToStart()
	{
		if (!Replication.IsServer())
		{
			Rpc(Rpc_DoFadeToBlackToStartSA);
			return;
		}
		
		Rpc_DoFadeToBlackToStartSA();
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void Rpc_DoFadeToBlackToStartSA()
	{
		if (m_bIsStarting)
			return;
		
		m_bIsStarting = true;
		Replication.BumpMe();
		
#ifdef WORKBENCH
		GetEscapistsGameMode().InitiateGameStart();
		Rpc(RpcDo_FadeToBlackToStartClients);
		return;
#endif
		
		m_FadeEffect = new ESCT_FadeEffect(m_preStartScreenEffectsLayout.GetRootWidget());
		m_FadeEffect.FadeToBlack();
		m_FadeEffect.m_eFadeEvent.GetOnScreenFade().Insert(StartServer);
		
		Rpc(RpcDo_FadeToBlackToStartClients);
	}
	
	//TODO: when delegates support will be there - remove this delicious copypasta in favour of unified method that accepts func as argument
	void FadeToBlackToEnd()
	{
		if (!Replication.IsServer())
			return;
		
		if (m_bIsEnding)
			return;
		
		m_bIsEnding = true;
		
		m_FadeEffect = new ESCT_FadeEffect(m_preStartScreenEffectsLayout.GetRootWidget());
		m_FadeEffect.FadeToBlack();
		m_FadeEffect.m_eFadeEvent.GetOnScreenFade().Insert(EndServer);
		
		Rpc(RpcDo_FadeToBlackToEndClients);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	private void RpcDo_FadeToBlackToStartClients()
	{
		ESCT_StartupUIHandlerComponent startupUI = ESCT_StartupUIHandlerComponent.GetInstance();
		if (startupUI) 
			startupUI.SetWaitScreenVisibilityForClients(false);	
		
		m_FadeEffect = new ESCT_FadeEffect(m_preStartScreenEffectsLayout.GetRootWidget());
		m_FadeEffect.FadeToBlack();
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	private void RpcDo_FadeToBlackToEndClients()
	{
		m_FadeEffect = new ESCT_FadeEffect(m_preStartScreenEffectsLayout.GetRootWidget());
		m_FadeEffect.FadeToBlack();
	}
	
	private void StartServer()
	{
		GetEscapistsGameMode().InitiateGameStart();
		m_FadeEffect.m_eFadeEvent.GetOnScreenFade().Remove(StartServer);
	}
	
	private void EndServer()
	{	
		ESCT_GameStateManagerComponent gameStateManager = ESCT_GameStateManagerComponent.GetInstance();
		if (gameStateManager)
		{
			gameStateManager.SetGameState(ESCT_EGameState.Ending);
		}
		
		ESCT_LocationSystem locSystem = ESCT_LocationSystem.GetSystem();
		if (locSystem)
		{
			locSystem.DespawnLocations();
		}
		
		map<int, IEntity> idToEntitiesMap = ESCT_Player.GetPlayerMap();
		if (idToEntitiesMap)
		{
			for (int i=0; i < idToEntitiesMap.Count(); i++)
			{
				int playerId = idToEntitiesMap.GetKey(i);
				IEntity player = idToEntitiesMap.Get(playerId);
				
				Vehicle vehicle = Vehicle.Cast(CompartmentAccessComponent.GetVehicleIn(player));
				if (vehicle)
				{
					CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(player.FindComponent(CompartmentAccessComponent));
					if (compartmentAccess)
					{
						compartmentAccess.GetOutVehicle(EGetOutType.TELEPORT, -1, ECloseDoorAfterActions.INVALID, false);	
					}
				}
				
				DamageManagerComponent damageManager = DamageManagerComponent.Cast(player.FindComponent(DamageManagerComponent));
				if (damageManager)
				{
					damageManager.SetHealthScaled(1);
					damageManager.EnableDamageHandling(false);
				}
				
				GetGame().GetCallqueue().CallLater(TeleportPlayerToSafeLocation, 100, false, playerId);
			}
		}
		
		ESCT_WinLoseHandlerComponent winLoseHandler = ESCT_WinLoseHandlerComponent.GetInstance();
		if (!winLoseHandler)
			return;
		
		winLoseHandler.ForceEndGame();
		m_FadeEffect.m_eFadeEvent.GetOnScreenFade().Remove(EndServer);
	}
	
	private void TeleportPlayerToSafeLocation(int playerId)
	{
		Tuple3<int, int, vector> worldMeasures = ESCT_Math.GetWorldSize();
		SCR_Global.TeleportPlayer(playerId, worldMeasures.param3 + "0 0.125 0");
	}
}
