class ESCT_EntitySearch
{	
	private ref array<IEntity> m_aEntities = {};
	private typename m_Type;
	private ResourceName m_PrefabName;
	
	array<IEntity> FindEntitiesByPrefab(ResourceName prefabName, EQueryEntitiesFlags flags = EQueryEntitiesFlags.ALL)
	{
		if (!m_aEntities.IsEmpty())
			m_aEntities.Clear();

		m_PrefabName = prefabName;
		vector minPos, maxPos;
		
		BaseWorld baseWorld = GetGame().GetWorld();
		baseWorld.GetBoundBox(minPos, maxPos);
		baseWorld.QueryEntitiesByAABB(minPos, maxPos, InsertEntity, FilterEntitiesByPrefab, queryFlags: flags);
		
		return m_aEntities;
	}
	
	array<IEntity> FindEntitiesByType(typename type, EQueryEntitiesFlags flags = EQueryEntitiesFlags.ALL)
	{
		if (!m_aEntities.IsEmpty())
			m_aEntities.Clear();

		m_Type = type;
		vector minPos, maxPos;
		
		BaseWorld baseWorld = GetGame().GetWorld();
		baseWorld.GetBoundBox(minPos, maxPos);
		baseWorld.QueryEntitiesByAABB(minPos, maxPos, InsertEntity, FilterEntitiesByType, queryFlags: flags);
		
		return m_aEntities;
	}
	
	array<IEntity> FindEntitiesByTypeInRadius(vector position, float radius, typename type, EQueryEntitiesFlags flags = EQueryEntitiesFlags.ALL)
	{
		if (!m_aEntities.IsEmpty())
			m_aEntities.Clear();

		m_Type = type;
		
		BaseWorld baseWorld = GetGame().GetWorld();
		baseWorld.QueryEntitiesBySphere(position, radius, InsertEntity, FilterEntitiesByType, queryFlags: flags);
		
		return m_aEntities;
	}
	
	array<IEntity> FindEntitiesByPrefabInRadius(vector position, float radius, ResourceName prefabName, EQueryEntitiesFlags flags = EQueryEntitiesFlags.ALL)
	{
		if (!m_aEntities.IsEmpty())
			m_aEntities.Clear();

		m_PrefabName = prefabName;
		
		BaseWorld baseWorld = GetGame().GetWorld();
		baseWorld.QueryEntitiesBySphere(position, radius, InsertEntity, FilterEntitiesByPrefab, queryFlags: flags);
		
		return m_aEntities;
	}
	
	private bool InsertEntity(IEntity entity)
	{
		if (entity)
			m_aEntities.Insert(entity);
		return true;
	}
	
	private bool FilterEntitiesByType(IEntity entity)
	{
		if(entity.Type() == m_Type) 
			return true;
		
		return false;
	}
	
	private bool FilterEntitiesByPrefab(IEntity entity)
	{
		EntityPrefabData data = entity.GetPrefabData();
		if (!data)
			return false;
		
		ResourceName prefabName = data.GetPrefabName();
		
		if (prefabName == m_PrefabName)
			return true;
			
		return false;
	}
}
