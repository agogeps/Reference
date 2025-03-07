class ESCT_FactionManagerClass : SCR_FactionManagerClass
{
}

class ESCT_FactionManager : SCR_FactionManager
{
	protected ref ESCT_Faction m_EnemyFaction;
	protected ref ESCT_Faction m_PlayerFaction;
	protected ref ESCT_Faction m_InsurgentFaction;

	ESCT_Faction GetEnemyFaction()
	{
		return m_EnemyFaction;
	}

	ESCT_Faction GetInsurgentFaction()
	{
		return m_InsurgentFaction;
	}

	ESCT_Faction GetPlayerFaction()
	{
		return m_PlayerFaction;
	}

	string GetEnemyFactionKey()
	{
		return m_EnemyFaction.GetFactionKey();
	}

	string GetPlayerFactionKey()
	{
		return m_PlayerFaction.GetFactionKey();
	}

	string GetInsurgentFactionKey()
	{
		return m_InsurgentFaction.GetFactionKey();
	}

	ESCT_Faction TryGetFactionByKey(string key)
	{
		return ESCT_Faction.Cast(GetFactionByKey(key));
	}

	static ESCT_FactionManager GetInstance()
	{
		return ESCT_FactionManager.Cast(GetGame().GetFactionManager());
	}

	void TrySetEnemyFaction_CA(string key)
	{
		if (!key || key == string.Empty)
			return;

		Faction faction = GetFactionByKey(key);
		if (!faction)
		{
			ESCT_Logger.ErrorFormat("[ESCT_FactionManager] Unable to find %1 enemy faction in faction manager!", key);
			return;
		}

		m_EnemyFaction = ESCT_Faction.Cast(faction);
	}

	void TrySetPlayerFaction_CA(string key)
	{
		if (!key || key == string.Empty)
			return;

		Faction faction = GetFactionByKey(key);
		if (!faction)
		{
			ESCT_Logger.ErrorFormat("[ESCT_FactionManager] Unable to find %1 player faction in faction manager!", key);
			return;
		}

		m_PlayerFaction = ESCT_Faction.Cast(faction);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets the faction data off all the faction for both client and server, should called by the game setup only.
	//! \param[in] occKey Enemy faction identifier.
	//! \param[in] invKey Player faction identifier.
	void SetFactions(string enemyKey, string playerKey)
	{
		if (!Replication.IsServer())
			return;

		RpcDo_SetFactions(enemyKey, playerKey);
		Rpc(RpcDo_SetFactions, enemyKey, playerKey);
	}

	void SetupFactionGroups()
	{
		if (!Replication.IsServer())
			return;

		m_EnemyFaction.BuildFactionLookups();
		if (ESCT_EscapistsConfigComponent.GetInstance().IsInsurgencyEnabled())
			m_InsurgentFaction.BuildFactionLookups();
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_SetFactions(string enemyKey, string playerKey)
	{
		m_EnemyFaction = SetupFaction(enemyKey);
		m_PlayerFaction = SetupFaction(playerKey);
		m_InsurgentFaction = ESCT_Faction.Cast(GetFactionByKey("FIA"));

		if (!m_EnemyFaction || !m_PlayerFaction || !m_InsurgentFaction)
		{
			ESCT_Logger.ErrorFormat("[ESCT_FactionManager] Failed to obtain one of the crucial factions by provided key. Keys: %1, %2, %3.", enemyKey, playerKey, "FIA");
			return;
		}

		ESCT_Logger.InfoFormat("[ESCT_FactionManager] Faction data set | Enemy: %1 | Player: %2 | Insurgents: %3",
			m_EnemyFaction.GetFactionKey(),
			m_PlayerFaction.GetFactionKey(),
			m_InsurgentFaction.GetFactionKey()
		);
	}

	private ESCT_Faction SetupFaction(string key)
	{
		ESCT_Faction faction = ESCT_Faction.Cast(GetFactionByKey(key));
		if (!faction)
		{
			ESCT_Logger.ErrorFormat("[ESCT_FactionManager] Failed to find faction under %1 key!", key);
			return null;
		}

		ESCT_PlayerFactionInfo info = faction.GetPlayerFactionInfo();
		if (!info)
		{
			ESCT_Logger.ErrorFormat("[ESCT_FactionManager] %1 faction has no faction data, check faction config!", key);
			return null;
		}

		info.SetFaction(faction);

		return faction;
	}
}
