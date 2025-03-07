[ComponentEditorProps(category: "Escapists/GameMode/Components", description: "Components that handles popup messages.")]
class ESCT_PopupMessageHandlerComponentClass: ESCT_GameModeBaseComponentClass
{
}

sealed class ESCT_PopupMessageHandlerComponent: ESCT_GameModeBaseComponent
{
	private static ESCT_PopupMessageHandlerComponent s_Instance = null;
	
	static ESCT_PopupMessageHandlerComponent GetInstance()
	{
		if (!s_Instance)
		{
			BaseGameMode gameMode = GetGame().GetGameMode();
			if (gameMode)
			{
				s_Instance = ESCT_PopupMessageHandlerComponent.Cast(gameMode.FindComponent(ESCT_PopupMessageHandlerComponent));
			}
		}

		return s_Instance;
	}
	
	//------------------------------------------------------------------------------------------------
	void PopUpMessage(string sTitle, string sSubtitle, string sound = string.Empty, FactionKey factionKey = "")
	{
		//For some reason it works without it, and uncommenting this line multiplies notifications on unusual manner
		//Rpc(RpcDo_PopUpMessage, sTitle, sSubtitle, sound, factionKey);
		
		if (factionKey != "")
			if (SCR_FactionManager.SGetLocalPlayerFaction() != GetGame().GetFactionManager().GetFactionByKey(factionKey))
				return;
		
		SCR_PopUpNotification.GetInstance().PopupMsg(sTitle, text2: sSubtitle, prio: SCR_ECampaignPopupPriority.TASK_DONE, sound: sound);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_PopUpMessage(string sTitle, string sSubtitle, string sound, FactionKey factionKey)
	{
		if (factionKey != "")
			if (SCR_FactionManager.SGetLocalPlayerFaction() != GetGame().GetFactionManager().GetFactionByKey(factionKey))
				return;
		
		SCR_PopUpNotification.GetInstance().PopupMsg(sTitle, text2: sSubtitle, prio: SCR_ECampaignPopupPriority.TASK_DONE, sound: sound);
	}
}
