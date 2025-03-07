[EntityEditorProps("Escapists/Location", description: "This component handles character spawn and despawn for locations.", color: "0 0 255 255")]
class ESCT_LocationCharacterManagerComponentClass : ESCT_BaseCharacterManagerComponentClass
{
}

class ESCT_LocationCharacterManagerComponent : ESCT_BaseCharacterManagerComponent
{
	private ref array<SCR_AIGroup> m_aDefenderGroups;

	override void Spawn()
	{
		IEntity composition = m_Location.GetComposition();
		if (!composition)
			return;

		GetCompositionEntities(composition);
		if (!m_Location.IsVisited())
		{
			AssignResources();
		}

		ESCT_FactionManager factionManager = ESCT_FactionManager.GetInstance();
		if (!factionManager)
			return;

		ESCT_Faction faction = m_Location.GetOwnerFaction();
		if (!faction)
			return;

		m_aDefenderGroups = {};

		SpawnCharacters();
	}

	override void Despawn()
	{
		foreach (ESCT_CharacterSpawnPoint spawnPoint : m_aCharacterSpawnPoints)
		{
			IEntity entity = spawnPoint.GetEntity();
			if (!entity)
				continue;

			SCR_EntityHelper.DeleteEntityAndChildren(entity);
		}
		m_aCharacterSpawnPoints.Clear();


		if (!m_aDefenderGroups)
			return;

		foreach (SCR_AIGroup aiGroup : m_aDefenderGroups)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(aiGroup);
		}

		delete m_aDefenderGroups;
	}

	protected void SpawnCharacters()
	{		
		ESCT_EscapistsConfigComponent gameConfig = ESCT_EscapistsConfigComponent.GetInstance();
		if (!gameConfig)
			return;

		RandomGenerator gen = ESCT_EscapistsGameMode.GetGameMode().GetRandomGenerator();
		float spawnChance = gameConfig.GetCharacterSpawnChance();
		if (spawnChance <= 0)
			return;

		ESCT_ELocationType locationType = m_Location.GetLocationType();
		ESCT_WaypointManagerComponent waypointManager = ESCT_WaypointManagerComponent.GetInstance();
		if (!waypointManager)
			return;

		ESCT_FactionManager factionManager = ESCT_FactionManager.GetInstance();
		if (!factionManager)
			return;

		ESCT_Faction faction = m_Location.GetOwnerFaction();
		if (!faction)
			return;

		foreach (ESCT_CharacterSpawnPoint spawnPoint : m_aCharacterSpawnPoints)
		{
			ESCT_CharacterState state = m_CharacterStateMap.Get(spawnPoint.GetEntitySpawnPointId());
			if (!state || state.m_bIsKilled)
				continue;

			spawnPoint.SetCharacterManager(this);

			ResourceName characterPrefab = state.GetCharacterPrefab();
			SCR_AIGroup aiGroup = ESCT_CharacterHelper.CreateGroup(faction.GetFactionKey(), faction.GetBaseFactionGroup());
			if (!aiGroup)
				continue;

			IEntity characterEntity = TrySpawnCharacter(characterPrefab, spawnPoint, gen, spawnChance, aiGroup);
			if (!characterEntity)
			{
				SCR_EntityHelper.DeleteEntityAndChildren(aiGroup);
				continue;
			}
			
			m_aDefenderGroups.Insert(aiGroup);
			waypointManager.DefendPosition(aiGroup, characterEntity.GetOrigin(), 2, priority: 5);
		}
	}

	protected IEntity TrySpawnCharacter(ResourceName characterResource, ESCT_CharacterSpawnPoint spawnPoint, RandomGenerator gen, float spawnChance, SCR_AIGroup group)
	{
		if (spawnPoint.m_IsSpawnGuaranteed)
		{
			spawnPoint.SpawnCharacter(characterResource, group, enableAI: ESCT_GameStateManagerComponent.GetInstance().ShouldEnableAI());
			return null;
		}

		if (gen.RandFloat01() > spawnChance)
			return null;

		IEntity entity = spawnPoint.SpawnCharacter(characterResource, group, enableAI: ESCT_GameStateManagerComponent.GetInstance().ShouldEnableAI());
		if (entity)
			m_Location.ChangeEntitiesToClear(1);

		return entity;
	}

	protected void GetCompositionEntities(IEntity composition)
	{
		array<IEntity> children = {};
		ESCT_EntityHelper.GetAllChildren(composition, children);

		foreach (IEntity entity : children)
		{
			if (!entity || entity.Type() != ESCT_CharacterSpawnPoint)
				continue;
			
			ESCT_CharacterSpawnPoint point = ESCT_CharacterSpawnPoint.Cast(entity);
			m_aCharacterSpawnPoints.Insert(point);
		}	
	}

	protected void AssignResources()
	{	
		ESCT_Faction faction = m_Location.GetOwnerFaction();
		if (!faction)
			return;
		
		ESCT_EMilitaryBranchType branch = GetBranch();
		
		if (branch == ESCT_EMilitaryBranchType.MilitaryPolice)
		{
			Print("MP!");
		}

		foreach (ESCT_CharacterSpawnPoint spawnPoint : m_aCharacterSpawnPoints)
		{
			ResourceName characterPrefab = faction.GetRandomCharacterOfBranch(branch);
			if (!characterPrefab || characterPrefab == ResourceName.Empty)
				continue;

			int id = spawnPoint.GetEntitySpawnPointId();
			ESCT_CharacterState state = new ESCT_CharacterState(id, characterPrefab);
			m_CharacterStateMap.Insert(id, state);
		}
	}
	
	protected ESCT_EMilitaryBranchType GetBranch()
	{
		if (m_Location.GetLocationType() == ESCT_ELocationType.StartArea)
			return ESCT_EMilitaryBranchType.MilitaryPolice;
		else 
			return ESCT_EMilitaryBranchType.Military;
	}
}
