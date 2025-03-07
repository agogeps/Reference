[ComponentEditorProps(category: "Escapists/GameMode/Components", description: "Customized reconnect component.")]
class ESCT_ReconnectComponentClass : SCR_ReconnectComponentClass
{
}

//! Takes care of managing player reconnects in case of involuntary disconnect
//! Authority-only component attached to gamemode prefab
sealed class ESCT_ReconnectComponent : SCR_ReconnectComponent
{
 	override bool OnPlayerDC(int playerId, KickCauseCode cause)
	{
		KickCauseGroup2 groupInt = KickCauseCodeAPI.GetGroup(cause);
		int reasonInt = KickCauseCodeAPI.GetReason(cause);
				
		if (groupInt != RplKickCauseGroup.REPLICATION)
			return false;
		
//		else if (reasonInt == RplError.SHUTDOWN)
//			return false;
				
		bool addEntry = true;
		
		if (!m_ReconnectPlayerList.IsEmpty())
		{
			int count = m_ReconnectPlayerList.Count();
			for (int i; i < count; i++)
			{
				if (m_ReconnectPlayerList[i].m_iPlayerId == playerId)
				{
					addEntry = false;
					break;
				}
			}
		}
		
		if (addEntry)
		{
			IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			if (!ent)
				return false;
			
			SCR_ReconnectData newEntry = new SCR_ReconnectData(playerId, ent);
			m_ReconnectPlayerList.Insert(newEntry);
			if (m_OnAddedToReconnectList)
				m_OnAddedToReconnectList.Invoke(newEntry);
		}
		
		return true;
	}
}
