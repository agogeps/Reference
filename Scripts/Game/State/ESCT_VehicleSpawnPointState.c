class ESCT_VehicleSpawnPointState
{
	bool b_mIsUsed;
	bool m_bIsGunnerAlive;

	protected int m_Id;
	protected SCR_EntityCatalogEntry m_rVehicleCatalogEntry;

	int GetId()
	{
		return m_Id;
	}
	
	bool IsGunnerAlive()
	{
		return m_bIsGunnerAlive;
	}

	SCR_EntityCatalogEntry GetVehicleCatalogEntry()
	{
		return m_rVehicleCatalogEntry;
	}

	void ESCT_VehicleSpawnPointState(int id, SCR_EntityCatalogEntry entry, bool isGunnerAlive, bool isUsed = false)
	{
		m_Id = id;
		m_bIsGunnerAlive = isGunnerAlive;
		m_rVehicleCatalogEntry = entry;
		b_mIsUsed = isUsed;
	}
}
