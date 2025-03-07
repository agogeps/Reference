class ESCT_CharacterLookup : ESCT_FactionLookup
{
	protected ref map<ESCT_EMilitaryBranchType, ref array<ref ESCT_FactionLookupValue>> m_mFactionCharacters;
	protected ref map<ESCT_EUnitRole, ref array<ref ESCT_FactionLookupValue>> m_mRoleCharacters;

	ResourceName GetRandomCharacterOfBranch(ESCT_EMilitaryBranchType branch)
	{
		if (!m_mFactionCharacters)
			return ResourceName.Empty;

		array<ref ESCT_FactionLookupValue> characters = m_mFactionCharacters.Get(branch);
		if (characters.IsEmpty())
			return ResourceName.Empty;

		int charCount = characters.Count();

		// Use a single loop to create the weights array and find the weighted index
		float totalWeight = 0;
		for (int j = 0; j < charCount; j++)
		{
			float weight = characters[j].GetWeight();
			totalWeight += weight;
		}

		float randomValue;
		float cumulativeWeight;
		ResourceName charPrefab = ResourceName.Empty;

		randomValue = Math.RandomFloat01() * totalWeight;
		cumulativeWeight = 0;
		for (int j = 0; j < charCount; j++)
		{
			cumulativeWeight += characters[j].GetWeight() + (Math.RandomFloat01() * 0.01); // add a small random offset
			if (randomValue <= cumulativeWeight)
			{
				charPrefab = characters[j].GetPrefab();
				break;
			}
		}

		return charPrefab;
	}

	ResourceName GetRandomCharacterOfRole(ESCT_EUnitRole role)
	{
		if (!m_mRoleCharacters)
			return ResourceName.Empty;

		array<ref ESCT_FactionLookupValue> characters = m_mRoleCharacters.Get(role);
		if (characters.IsEmpty())
			return ResourceName.Empty;

		int charCount = characters.Count();

		// Use a single loop to create the weights array and find the weighted index
		float totalWeight = 0;
		for (int j = 0; j < charCount; j++)
		{
			float weight = characters[j].GetWeight();
			totalWeight += weight;
		}

		float randomValue;
		float cumulativeWeight;
		ResourceName charPrefab = ResourceName.Empty;

		randomValue = Math.RandomFloat01() * totalWeight;
		cumulativeWeight = 0;
		for (int j = 0; j < charCount; j++)
		{
			cumulativeWeight += characters[j].GetWeight() + (Math.RandomFloat01() * 0.01); // add a small random offset
			if (randomValue <= cumulativeWeight)
			{
				charPrefab = characters[j].GetPrefab();
				break;
			}
		}

		return charPrefab;
	}

	override void Build()
	{
		if (m_mFactionCharacters && !m_mFactionCharacters.IsEmpty())
			return;

		ESCT_Logger.InfoFormat("[ESCT_CharacterLookup] Starting to build character maps for %1 faction.", m_Faction.GetFactionKey());

		m_mFactionCharacters = new map<ESCT_EMilitaryBranchType, ref array<ref ESCT_FactionLookupValue>>();
		m_mRoleCharacters = new map<ESCT_EUnitRole, ref array<ref ESCT_FactionLookupValue>>();

		array<ESCT_EMilitaryBranchType> branches = {};
		SCR_Enum.GetEnumValues(ESCT_EMilitaryBranchType, branches);
		foreach (ESCT_EMilitaryBranchType branchToAdd : branches)
		{
			m_mFactionCharacters.Insert(branchToAdd, {});
		}

		array<ESCT_EUnitRole> roles = {ESCT_EUnitRole.HeliCrew, ESCT_EUnitRole.HeliPilot};
		foreach (ESCT_EUnitRole roleToAdd : roles)
		{
			m_mRoleCharacters.Insert(roleToAdd, {});
		}

		SCR_EntityCatalog catalog = m_Faction.GetFactionEntityCatalogOfType(EEntityCatalogType.CHARACTER);
		if (!catalog)
		{
			ESCT_Logger.ErrorFormat(
				"[ESCT_CharacterLookup] Failed to obtain faction characters catalog for %1 faction during the lookup character map initialization, aborting!",
				m_Faction.GetFactionKey()
			);
			return;
		}

		array<SCR_EntityCatalogEntry> entries = {};
		catalog.GetEntityList(entries);

		ESCT_Logger.InfoFormat("[ESCT_CharacterLookup] Total characters found in entity catalog: %1.", entries.Count().ToString());

		array<SCR_BaseEntityCatalogData> entityDataList = {};

		foreach (SCR_EntityCatalogEntry entry : entries)
		{
			if (!entry || !entry.GetPrefab())
				continue;

			//GetEntityDataList will clear the array
			entry.GetEntityDataList(entityDataList);

			ESCT_EMilitaryBranchType branch = GetUnitBranch(entityDataList);
			ESCT_EUnitRole role = GetUnitRole(entityDataList);
			
			if (role != 0)
			{
				ESCT_FactionLookupValue lookupValue = new ESCT_FactionLookupValue(entry.GetPrefab(), entry.GetWeight());
				array<ref ESCT_FactionLookupValue> roleCharacters = m_mRoleCharacters.Get(role);
				roleCharacters.Insert(lookupValue);
			}
			else 
			{
				ESCT_Logger.WarningFormat("[ESCT_CharacterLookup] Skipping %1 character because it has no role specified.", ESCT_Misc.GetPrefabName(entry.GetPrefab()));
			}
			
			if (branch != 0)
			{
				ESCT_FactionLookupValue lookupValue = new ESCT_FactionLookupValue(entry.GetPrefab(), entry.GetWeight());
				array<ref ESCT_FactionLookupValue> characters = m_mFactionCharacters.Get(branch);
				characters.Insert(lookupValue);
			}
			else 
			{
				ESCT_Logger.WarningFormat("[ESCT_CharacterLookup] Skipping %1 character because it has no branch specified.", ESCT_Misc.GetPrefabName(entry.GetPrefab()));
			}
		}

		foreach (ESCT_EMilitaryBranchType logBranch : branches)
		{
			string total = m_mFactionCharacters.Get(logBranch).Count().ToString();
			ESCT_Logger.InfoFormat(
				"[ESCT_CharacterLookup] Faction character map has been built for %1 faction, %2. Total: %1.",
				m_Faction.GetFactionKey(),
				typename.EnumToString(ESCT_EMilitaryBranchType, logBranch),
				total
			);
		}

//		foreach (int key, array<ref ESCT_FactionLookupValue> val : m_mFactionCharacters)
//		{
//			Print("-------------");
//			PrintFormat("Lookup map key: %1", key);
//			foreach (ref ESCT_FactionLookupValue flv : val)
//			{
//				PrintFormat("Lookup prefab: %1", ESCT_Misc.GetPrefabName(flv.GetPrefab()));
//			}
//		}
	}

	private ESCT_EUnitRole GetUnitRole(array<SCR_BaseEntityCatalogData> entityDataList)
	{
		ESCT_EUnitRole role;

		foreach (SCR_BaseEntityCatalogData data : entityDataList)
		{
			if (data.Type() == ESCT_ExcludeFromCharacterLookupData)
			{
				ESCT_ExcludeFromCharacterLookupData lookupData = ESCT_ExcludeFromCharacterLookupData.Cast(data);
				if (lookupData && lookupData.IsExcludedFromRoleLookup())
					return null;
			}
			else if (data.Type() == ESCT_UnitRoleData)
			{
				ESCT_UnitRoleData roleData = ESCT_UnitRoleData.Cast(data);
				role = roleData.GetUnitRole();
			}
		}

		return role;
	}

	private ESCT_EMilitaryBranchType GetUnitBranch(array<SCR_BaseEntityCatalogData> entityDataList)
	{
		ESCT_EMilitaryBranchType branch;

		foreach (SCR_BaseEntityCatalogData data : entityDataList)
		{
			if (data.Type() == ESCT_ExcludeFromCharacterLookupData)
			{
				ESCT_ExcludeFromCharacterLookupData lookupData = ESCT_ExcludeFromCharacterLookupData.Cast(data);
				if (lookupData && lookupData.IsExcludedFromBranchLookup())
					return null;
			}
			else if (data.Type() == ESCT_MilitaryBranchData)
			{
				ESCT_MilitaryBranchData branchData = ESCT_MilitaryBranchData.Cast(data);
				branch = branchData.GetBranch();
			}
		}

		if (branch == 0)
		{
			branch = ESCT_EMilitaryBranchType.Military;
		}

		return branch;
	}
}
