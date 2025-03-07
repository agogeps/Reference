class ESCT_MilBuildingSearch
{
	private ref array<IEntity> m_aEntities = {};

	void FindNearMilitaryBuildings(vector position, notnull out array<IEntity> entities)
	{
		if (!m_aEntities.IsEmpty())
			m_aEntities.Clear();

		BaseWorld baseWorld = GetGame().GetWorld();
		baseWorld.QueryEntitiesBySphere(position, 250, InsertEntity, FilterEntities, queryFlags: EQueryEntitiesFlags.ALL);
		
		entities.Copy(m_aEntities);
	}
	
	private bool InsertEntity(IEntity entity)
	{
		if (entity)
			m_aEntities.Insert(entity);
		return true;
	}
	
	private bool FilterEntities(IEntity entity)
	{
		if(entity.Type() == SCR_DestructibleBuildingEntity && ESCT_PopulatedBuildingComponent.Cast(entity.FindComponent(ESCT_PopulatedBuildingComponent)))
			return true;
		
		return false;
	}
}
