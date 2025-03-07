class ESCT_SpawnPositionComponentClass : SCR_SpawnPositionComponentClass
{
}

sealed class ESCT_SpawnPositionComponent : SCR_SpawnPositionComponent
{
	protected bool m_bIsClaimed = false;
	
	bool IsClaimed()
	{
		return m_bIsClaimed;
	}
	
	void Claim()
	{
		if (m_bIsClaimed)
		{
			ESCT_Logger.Warning("Trying to claim already claimed spawn position, aborting!");
			return;
		}
		
		m_bIsClaimed = true;
	}
}
