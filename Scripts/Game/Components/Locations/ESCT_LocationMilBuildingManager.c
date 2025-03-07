[EntityEditorProps("Escapists/Location", description: "This component handles character spawn and despawn for locations.", color: "0 0 255 255")]
class ESCT_LocationMilBuildingManagerComponentClass : ESCT_BaseCharacterManagerComponentClass
{
}

class ESCT_LocationMilBuildingManagerComponent : ESCT_BaseCharacterManagerComponent
{
	private ref array<SCR_AIGroup> m_aDefenderGroups = {};

	override void Spawn()
	{
		if (!m_Location.IsVisited())
		{
			GetSpawnPoints();
			AssignResourcesToSpawnPoints();
		}

		SpawnCharacters();
	}

	override void Despawn()
	{
		foreach (ESCT_CharacterSpawnPoint spawnPoint : m_aCharacterSpawnPoints)
		{
			IEntity entity = spawnPoint.GetEntity();
			if (entity)
			{
				SCR_EntityHelper.DeleteEntityAndChildren(entity);
			}
		}
		
		for (int i = 0; i < m_aDefenderGroups.Count(); i++)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(m_aDefenderGroups[i]);
		}
	}

	private void SpawnCharacters()
	{
		ESCT_EscapistsConfigComponent gameConfig = ESCT_EscapistsConfigComponent.GetInstance();
		if (!gameConfig)
			return;
		
		RandomGenerator gen = ESCT_EscapistsGameMode.GetGameMode().GetRandomGenerator();
		float spawnChance = gameConfig.GetCharacterSpawnChance();
		if (spawnChance <= 0)
			return;
		
		ESCT_Faction faction = m_Location.GetOwnerFaction();
		if (!faction)
			return;
		
		SCR_AIGroup defenderGroup = ESCT_CharacterHelper.CreateGroup(faction.GetFactionKey(), faction.GetBaseFactionGroup());
		
		m_aDefenderGroups.Insert(defenderGroup);

		foreach (ESCT_CharacterSpawnPoint spawnPoint : m_aCharacterSpawnPoints)
		{
			ESCT_CharacterState state = m_CharacterStateMap.Get(spawnPoint.GetEntitySpawnPointId());
			if (!state || state.m_bIsKilled)
				continue;

			spawnPoint.SetCharacterManager(this);

			ResourceName characterPrefab = state.GetCharacterPrefab();
			TrySpawnCharacter(characterPrefab, spawnPoint, gen, spawnChance, defenderGroup);	
					
			//ESCT_Global.GetWaypointManager().DefendPosition(defenderGroup, spawnPoint.GetOrigin(), radius: 2.0);
		}	
	}

	private void TrySpawnCharacter(ResourceName character, ESCT_CharacterSpawnPoint spawnPoint, RandomGenerator gen, float spawnChance, SCR_AIGroup group)
	{
		if (spawnPoint.m_IsSpawnGuaranteed)
		{
			spawnPoint.SpawnCharacter(character, group, enableAI: ESCT_GameStateManagerComponent.GetInstance().ShouldEnableAI());
			return;
		}

		if (gen.RandFloat01() > spawnChance)
			return;

		spawnPoint.SpawnCharacter(character, group, enableAI: ESCT_GameStateManagerComponent.GetInstance().ShouldEnableAI());
	}
	
	private void GetSpawnPoints()
	{
		ESCT_MilitaryBuildingStorageComponent buildingStorage = ESCT_LocationFuncs<ESCT_MilitaryBuildingStorageComponent>.GetLocationComponent(m_Location);
		if (!buildingStorage)
			return;
		
		array<ESCT_PopulatedBuildingComponent> buildings = {};
		buildingStorage.GetMilitaryBuildings(buildings);
		
		ESCT_Logger.InfoFormat("Buildings for location %1: %2", typename.EnumToString(ESCT_ELocationType, m_Location.GetLocationType()), buildings.Count().ToString());
	
		ESCT_PopulatedBuildingComponent building;
		for (int i = 0; i < buildings.Count(); i++)
		{	
			building = buildings[i];
			if (!building)
				continue;
			
			array<ESCT_CharacterSpawnPoint> spawnPoints = {};
			building.GetCharacterSpawnPoints(spawnPoints);
			m_aCharacterSpawnPoints.InsertAll(spawnPoints);
		}
	}

	private void AssignResourcesToSpawnPoints()
	{
		ESCT_Faction faction = m_Location.GetOwnerFaction();
		if (!faction)
			return;
		
		foreach (ESCT_CharacterSpawnPoint spawnPoint : m_aCharacterSpawnPoints)
		{
			ResourceName characterPrefab = faction.GetRandomCharacterOfBranch(ESCT_EMilitaryBranchType.Military);
			if (!characterPrefab || characterPrefab == ResourceName.Empty)
				continue;
			
			int id = spawnPoint.GetEntitySpawnPointId();
			ESCT_CharacterState state = new ESCT_CharacterState(id, characterPrefab);
			m_CharacterStateMap.Insert(id, state);
		}
	}
}
