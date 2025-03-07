[EntityEditorProps("Escapists", description: "Storage for military buildings around the location.", color: "0 0 255 255")]
class ESCT_MilitaryBuildingStorageComponentClass : ESCT_LocationComponentClass
{
}

sealed class ESCT_MilitaryBuildingStorageComponent : ESCT_LocationComponent
{
	protected ref array<ESCT_PopulatedBuildingComponent> m_aBuildings;
	
	void GetMilitaryBuildings(notnull out array<ESCT_PopulatedBuildingComponent> buildings)
	{
		if (!m_aBuildings)
		{
			TryGetUnclaimedBuildings();
		}
		
		buildings.Copy(m_aBuildings);
	}
	
	private void TryGetUnclaimedBuildings()
	{	
		m_aBuildings = {};
		array<IEntity> buildingsToCheck = {};
		ESCT_MilBuildingSearch buildingSearch = new ESCT_MilBuildingSearch();
		buildingSearch.FindNearMilitaryBuildings(m_Location.GetOrigin(), buildingsToCheck);
		
		ESCT_PopulatedBuildingComponent buildingComponent;
		for (int i = 0; i < buildingsToCheck.Count(); i++)
		{	
			buildingComponent = ESCT_PopulatedBuildingComponent.Cast(buildingsToCheck[i].FindComponent(ESCT_PopulatedBuildingComponent));
			if (!buildingComponent)
				continue;

			//TODO: for some reason there's no multiple spawns on same military buildings from multiple locations, so additional safeguards might not needed
			//let's see and remove them if everything is fine
			//if (buildingComponent.TryClaim(m_Location))
			m_aBuildings.Insert(buildingComponent);
		}
	}
}
