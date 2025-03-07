class ESCT_StorageState
{
	protected int m_Id;
	protected IEntity m_Container;
	protected ref array<ResourceName> m_aStoredItems = {};
	
	int GetId()
	{
		return m_Id;
	}
	
	IEntity GetContainer()
	{
		return m_Container;
	}
	
	array<ResourceName> GetStoredItems()
	{
		return m_aStoredItems;
	}
	
	bool UpdateStorage(ResourceName prefab)
	{
		if (!m_aStoredItems || m_aStoredItems.IsEmpty() || !prefab)
			return false;
		
		int index = m_aStoredItems.Find(prefab);
		if (index == -1)
			return false;
		
		m_aStoredItems.Remove(index);
		
		//ESCT_Logger.InfoFormat("StorageState %1 item count: %2", m_Id.ToString(), m_aStoredItems.Count().ToString());
		
		return true;
	}
	
	void ESCT_StorageState(int id, array<ResourceName> items, IEntity container)
	{
		m_Id = id;
		m_aStoredItems = items;
		m_Container = container;
	}
}
