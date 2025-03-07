[EntityEditorProps(category: "Escapists/PlayerController", description: "Handles client > server communication in Escapists. Should be attached to PlayerController.", color: "0 0 255 255")]
class ESCT_EscapistsNetworkComponentClass : ScriptComponentClass
{
}

enum ESCT_EEscapistsClientNotificationID
{
	SUPPORT_INCOMING,
	RESPAWN
}

//! Takes care of Escapists-specific server <> client communication and requests
class ESCT_EscapistsNetworkComponent : ScriptComponent
{
	protected SCR_PlayerController m_PlayerController;
	protected RplComponent m_RplComponent;
	protected bool m_bFirstSpawn = true;

	//------------------------------------------------------------------------------------------------
	//! \return
	static ESCT_EscapistsNetworkComponent GetInstance()
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return null;

		return ESCT_EscapistsNetworkComponent.Cast(pc.FindComponent(ESCT_EscapistsNetworkComponent));
	}

	//------------------------------------------------------------------------------------------------
	// Init
	override void EOnInit(IEntity owner)
	{
		m_PlayerController = SCR_PlayerController.Cast(PlayerController.Cast(owner));
		if (!m_PlayerController)
		{
			ESCT_Logger.Error("SCR_CampaignNetworkComponent must be attached to PlayerController!");
			return;
		}

		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
	}

	void RespawnLocationPopup()
	{
		Rpc(RpcDo_PlayerFeedbackBase, ESCT_EEscapistsClientNotificationID.RESPAWN);
	}

	//------------------------------------------------------------------------------------------------
	//! Sends request to start the game to server.
	void StartGame(notnull ESCT_SetupConfig config)
	{
		Rpc(Rpc_DoStartGame_SA, config);
	}

	void AddUnconsciousDisplay()
	{
		Rpc(Rpc_AddUnconsciousDisplay_CA);
	}

	//------------------------------------------------------------------------------------------------
	//! Shows setup menu to owner.
	void ShowSetupMenu()
	{
		Rpc(Rpc_ShowSetupMenu_CA);
	}

	//------------------------------------------------------------------------------------------------
	//! Shows support notification to owner.
	void ShowSupportNotification(ESCT_ESupportType type)
	{
		Rpc(RpcDo_PlayerFeedbackIntValueBase, ESCT_EEscapistsClientNotificationID.SUPPORT_INCOMING, type);
	}

	//------------------------------------------------------------------------------------------------
	//! \return if the session is run as client
	protected bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void Rpc_DoStartGame_SA(ESCT_SetupConfig config)
	{
		ESCT_EscapistsConfigComponent configManager = ESCT_EscapistsConfigComponent.GetInstance();
		if (!configManager)
			return;

		configManager.ApplyConfig(config, "setup menu");

		ESCT_ScreenFadeComponent screenFade = ESCT_ScreenFadeComponent.GetInstance();
		if (!screenFade)
			return;

		screenFade.FadeToBlackToStart();
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void Rpc_ShowSetupMenu_CA()
	{
		ESCT_EscapistsFeedbackComponent feedbackComponent = ESCT_EscapistsFeedbackComponent.GetInstance();
		if (!feedbackComponent)
			return;

		feedbackComponent.SetSetupMenuVisibility(true);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void Rpc_AddUnconsciousDisplay_CA()
	{
		//should work only on listen servers and clients
		if (RplSession.Mode() == RplMode.Dedicated)
			return;
		
		ESCT_SelfReviveComponent selfRevive = ESCT_SelfReviveComponent.GetLocalInstance();
		if (!selfRevive)
			return;

		//doesn't work at all unless we wait for 1 frame
		GetGame().GetCallqueue().Call(selfRevive.TryAddSelfRevive);
	}

	//------------------------------------------------------------------------------------------------
	//! Show notification about request result to the requester
	//! \param[in] msgID Message ID (see ESCT_EEscapistsClientNotificationID)
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_PlayerFeedback(int msgID)
	{
		PlayerFeedbackImpl(msgID);
	}

	//------------------------------------------------------------------------------------------------
	//! Show notification about request result to the requester
	//! \param[in] msgID Message ID (see ESCT_EEscapistsClientNotificationID)
	//! \param[in] value
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_PlayerFeedbackIntValueBase(int msgID, int value)
	{
		// Short delay so replicated values have time to catch up on client's machine
		GetGame().GetCallqueue().CallLater(PlayerFeedbackImpl, SCR_GameModeCampaign.MINIMUM_DELAY, false, msgID, 0, value);
	}

	//------------------------------------------------------------------------------------------------
	//! Show notification about request result to the requester
	//! \param[in] msgID Message ID (see ESCT_EEscapistsClientNotificationID)
	//! \param[in] value
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_PlayerFeedbackValueBase(int msgID, float value)
	{
		// Short delay so replicated values have time to catch up on client's machine
		GetGame().GetCallqueue().CallLater(PlayerFeedbackImpl, SCR_GameModeCampaign.MINIMUM_DELAY, false, msgID, value, -1);
	}

	//------------------------------------------------------------------------------------------------
	//! Show notification about request result to the requester
	//! \param[in] msgID Message ID (see ESCT_EEscapistsClientNotificationID)
	//! \param[in] baseID
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_PlayerFeedbackBase(int msgID)
	{
		PlayerFeedbackImpl(msgID, 0, -1);
	}

	protected void PlayerFeedbackImpl(int msgID, float value = 0, int intValue = 0)
	{
		ESCT_EscapistsGameMode escapists = ESCT_EscapistsGameMode.GetGameMode();
		ESCT_EscapistsFeedbackComponent manager = ESCT_EscapistsFeedbackComponent.GetInstance();

		if (!escapists)
			return;

		switch (msgID)
		{
			case ESCT_EEscapistsClientNotificationID.RESPAWN:
			{
				if (manager)
					manager.OnRespawn();

				break;
			};

			case ESCT_EEscapistsClientNotificationID.SUPPORT_INCOMING:
			{
				ESCT_ESupportType type = intValue;

				LocalizedString subtitle;
				string iconName;
				string imageSet;
				int scale;

				switch (type)
				{
					case ESCT_ESupportType.QRF_VEHICLE:
					{
						imageSet = "{2B4F198AB5937F32}UI/Imagesets/Editor/Icons/EditableEntities.imageset";
						subtitle = "#Escapists-Support_QRF_VEHICLE_Subtitle";
						iconName = "EditableEntity_Vehicle_Apc";
						scale = 3;
						break;
					}
					case ESCT_ESupportType.QRF:
					{
						imageSet = "{12BA7AE37121F768}UI/Textures/Icons/icons_wrapperUI-96.imageset";
						subtitle = "#Escapists-Support_QRF_Subtitle";
						iconName = "characters";
						scale = 2;
						break;
					}
					default:
					{
						imageSet = "{12BA7AE37121F768}UI/Textures/Icons/icons_wrapperUI-96.imageset";
						subtitle = "#Escapists-Support_Generic_Subtitle";
						iconName ="death";
						scale = 2;
						break;
					}
				}

				ESCT_Faction faction = ESCT_FactionManager.GetInstance().GetEnemyFaction();
				string title = WidgetManager.Translate("#Escapists-Support_Generic_Title", faction.GetFactionName());
				title = string.Format("<color rgba='196,2,52,255'>%1</color>", title);

				LocalizedString resultSubtitle = subtitle + "<br/><br/>" + string.Format("<image set='%1' name='%2' scale='%3'/>", imageSet, iconName, scale);

				SCR_PopUpNotification.GetInstance().PopupMsg(title, 8, sound: SCR_SoundEvent.SOUND_SIREN, text2: resultSubtitle);

				break;
			};

			default:
			{
				return;
			};
		}

		//SCR_PopUpNotification.GetInstance().PopupMsg(msg, duration, msg2, param1: msg1param1, text2param1: msg2param1, text2param2: msg2param2);
	}
}
