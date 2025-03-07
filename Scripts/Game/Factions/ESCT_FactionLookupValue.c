class ESCT_FactionLookupValue
{
	protected ResourceName m_rPrefab;
	protected float m_fWeight;
	
	void ESCT_FactionLookupValue(ResourceName prefab, float weight)
	{
		m_rPrefab = prefab;
		m_fWeight = weight;
	}
	
	ResourceName GetPrefab()
	{
		return m_rPrefab; 
	}
	
	float GetWeight()
	{
		return m_fWeight;
	}
}
