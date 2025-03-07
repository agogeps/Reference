class ESCT_LocationCatalog
{
	protected SCR_Faction m_Faction;
	protected ref array<SCR_EntityCatalogEntry> m_aLocations;
	protected ESCT_ELocationType m_CachedLocationType;
	
	void ESCT_LocationCatalog(SCR_Faction faction)
	{
		m_Faction = faction;
	}
	
	SCR_EntityCatalogEntry GetRandomLocation(ESCT_ELocationType locationType)
	{
		if (!m_Faction)
			return null;
		
		if (!m_aLocations || m_CachedLocationType != locationType)
		{
			SCR_EntityCatalog catalog = m_Faction.GetFactionEntityCatalogOfType(EEntityCatalogType.LOCATION);
			m_aLocations = {};
			catalog.GetEntityList(m_aLocations);
			
			SCR_EntityCatalogEntry locationEntry;
			array<SCR_BaseEntityCatalogData> entityDataList = {};
		
			for (int i = m_aLocations.Count() - 1; i >= 0; i--)
			{					
				locationEntry = m_aLocations[i];
				if (!locationEntry)
					continue;
				
				ESCT_LocationData locationData;
				locationEntry.GetEntityDataList(entityDataList);
				foreach (SCR_BaseEntityCatalogData data: entityDataList)
				{
					if (data.Type() == ESCT_LocationData)
					{
						locationData = ESCT_LocationData.Cast(data);
						break;
					}
				}
				
			    if (!locationData || locationData.GetLocationType() != locationType)
			    {
			        m_aLocations.Remove(i);
			    }
			}
			
			m_CachedLocationType = locationType;
		}
		
		
		if (m_aLocations.IsEmpty())
			return null;
		
		int locationCount = m_aLocations.Count();
		
		// Use a single loop to create the weights array and find the weighted index
		float totalWeight = 0;
		for (int j = 0; j < locationCount; j++)
		{
		    float weight = m_aLocations[j].GetWeight() + (Math.RandomFloat01() * 0.01);
		    totalWeight += weight;
		}
		
		float randomValue;
		float cumulativeWeight;
		SCR_EntityCatalogEntry entry = null;
		
		randomValue = Math.RandomFloat01() * totalWeight;
		cumulativeWeight = 0;
		for (int j = 0; j < locationCount; j++)
		{
			cumulativeWeight += m_aLocations[j].GetWeight();
			if (randomValue <= cumulativeWeight)
			{
				entry = m_aLocations[j];
				break;
			}
		}
		
		return entry;
	}
}
