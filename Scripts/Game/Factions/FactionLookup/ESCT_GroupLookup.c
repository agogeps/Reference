class ESCT_GroupLookup : ESCT_FactionLookup
{
	protected ref map<int, ref array<ref ESCT_FactionLookupValue>> m_mFactionGroups;

	ResourceName GetRandomGroupOfBranchAndSize(ESCT_EMilitaryBranchType branch, ESCT_EGroupSize size)
	{
		if (!m_mFactionGroups)
			return ResourceName.Empty;

		array<ref ESCT_FactionLookupValue> groups = m_mFactionGroups.Get(size | branch);

		if (groups.IsEmpty())
			return ResourceName.Empty;

		int groupCount = groups.Count();

		// Use a single loop to create the weights array and find the weighted index
		float totalWeight = 0;
		for (int j = 0; j < groupCount; j++)
		{
			float weight = groups[j].GetWeight();
			totalWeight += weight;
		}

		float randomValue;
		float cumulativeWeight;
		ResourceName groupPrefab = ResourceName.Empty;

		randomValue = Math.RandomFloat01() * totalWeight;
		cumulativeWeight = 0;
		for (int j = 0; j < groupCount; j++)
		{
			cumulativeWeight += groups[j].GetWeight() + (Math.RandomFloat01() * 0.01); // add a small random offset
			if (randomValue <= cumulativeWeight)
			{
				groupPrefab = groups[j].GetPrefab();
				break;
			}
		}

		return groupPrefab;
	}

	override void Build()
	{
		if (m_mFactionGroups && !m_mFactionGroups.IsEmpty())
			return;

		ESCT_Logger.InfoFormat("[ESCT_GroupLookup] Starting to build group maps for %1 faction.", m_Faction.GetFactionKey());

		m_mFactionGroups = new map<int, ref array<ref ESCT_FactionLookupValue>>();

		array<ESCT_EGroupSize> sizes = { ESCT_EGroupSize.Sentry, ESCT_EGroupSize.Fireteam, ESCT_EGroupSize.Squad };
		array<ESCT_EMilitaryBranchType> branches = {};
		SCR_Enum.GetEnumValues(ESCT_EMilitaryBranchType, branches);

		//using composite keys and storing them in int value as it's much more efficient than nested maps (make sure that keys doesn't overlap in values!)
		foreach (ESCT_EMilitaryBranchType sizeToAdd : sizes)
		{
			foreach (ESCT_EMilitaryBranchType branchToAdd : branches)
			{
				m_mFactionGroups.Insert(sizeToAdd | branchToAdd, {});
			}
		}

		SCR_EntityCatalog catalog = m_Faction.GetFactionEntityCatalogOfType(EEntityCatalogType.GROUP);
		if (!catalog)
		{
			ESCT_Logger.ErrorFormat(
				"[ESCT_GroupLookup] Failed to obtain faction groups catalog for %1 faction during the lookup group map initialization, aborting!",
				m_Faction.GetFactionKey()
			);
			return;
		}

		array<SCR_EntityCatalogEntry> groupEntries = {};
		catalog.GetEntityList(groupEntries);

		ESCT_Logger.InfoFormat("[ESCT_GroupLookup] Total groups found in entity catalog: %1.", groupEntries.Count().ToString());

		array<ResourceName> groupPrefabs = {};
		array<vector> groupOffsets = {}; //not needed, by GetMembers API requires it

		foreach (SCR_EntityCatalogEntry entry : groupEntries)
		{
			if (!entry || !entry.GetPrefab())
				continue;

			Resource resource = Resource.Load(entry.GetPrefab());
			if (!resource.IsValid())
				continue;

			IEntitySource entitySource = SCR_BaseContainerTools.FindEntitySource(resource);
			if (!entitySource)
				continue;

			groupPrefabs.Clear();
			groupOffsets.Clear();
			SCR_AIGroupClass.GetMembers(entitySource, groupPrefabs, groupOffsets);

			int soldiersCount = groupPrefabs.Count();
			if (soldiersCount == 0)
				continue;

			ESCT_EGroupSize size = GetGroupSize(soldiersCount);
			if (size == 0)
			{
				ESCT_Logger.WarningFormat("[ESCT_GroupLookup] Skipping %1 group because it has no soldiers specified (empty unit prefab slots array in prefab settings?).", ESCT_Misc.GetPrefabName(entry.GetPrefab()));
				continue;
			}

			ESCT_EMilitaryBranchType branch = GetGroupBranch(entry);
			if (branch == 0)
			{
				ESCT_Logger.WarningFormat("[ESCT_GroupLookup] Skipping %1 group because it has no branch specified (this is ok for some groups, usually not spawned ones).", ESCT_Misc.GetPrefabName(entry.GetPrefab()));
				continue;
			}

			//ESCT_Logger.InfoFormat("%1 prefab: %2 %3", ESCT_Misc.GetPrefabName(entry.GetPrefab()), typename.EnumToString(ESCT_EGroupSize, size), typename.EnumToString(ESCT_EMilitaryBranchType, branch));

			ESCT_FactionLookupValue lookupValue = new ESCT_FactionLookupValue(entry.GetPrefab(), entry.GetWeight());
			array<ref ESCT_FactionLookupValue> groups = m_mFactionGroups.Get(size | branch);
			groups.Insert(lookupValue);
		}

		foreach (ESCT_EGroupSize groupSize : sizes)
		{
			string milPoliceCount = m_mFactionGroups.Get(groupSize | ESCT_EMilitaryBranchType.MilitaryPolice).Count().ToString();
			string militaryCount = m_mFactionGroups.Get(groupSize | ESCT_EMilitaryBranchType.Military).Count().ToString();
			string sfCount = m_mFactionGroups.Get(groupSize | ESCT_EMilitaryBranchType.SF).Count().ToString();

			ESCT_Logger.InfoFormat(
				"[ESCT_GroupLookup] Faction group map has been built for %1 faction %2. MilPolice: %3, military: %4, SF: %5.",
				m_Faction.GetFactionKey(),
				typename.EnumToString(ESCT_EGroupSize, groupSize),
				milPoliceCount,
				militaryCount,
				sfCount
			);
		}
		
//		foreach (int key, array<ref ESCT_FactionLookupValue> val : m_mFactionGroups)
//		{
//			Print("-------------");
//			PrintFormat("Lookup map key: %1", key);
//			foreach (ref ESCT_FactionLookupValue flv : val)
//			{
//				PrintFormat("Lookup prefab: %1", ESCT_Misc.GetPrefabName(flv.GetPrefab()));
//			}
//		}
	}

	private ESCT_EMilitaryBranchType GetGroupBranch(SCR_EntityCatalogEntry entry)
	{
		array<SCR_BaseEntityCatalogData> entityDataList = {};

		//assuming that everything is milbranch by default as some catalog entries might missing this information
		ESCT_EMilitaryBranchType branch;

		entry.GetEntityDataList(entityDataList);
		foreach (SCR_BaseEntityCatalogData data : entityDataList)
		{
			if (data.Type() == ESCT_ExcludeFromGroupLookupData)
			{
				return 0;
			}
			else if (data.Type() == ESCT_MilitaryBranchData)
			{
				ESCT_MilitaryBranchData branchData = ESCT_MilitaryBranchData.Cast(data);
				branch = branchData.GetBranch();
				break;
			}
		}

		if (branch == 0)
		{
			branch = ESCT_EMilitaryBranchType.Military;
		}

		return branch;
	}


	private ESCT_EGroupSize GetGroupSize(int soldiersCount)
	{
		if (soldiersCount == 0)
		{
			return 0;
		}
		else if (soldiersCount < 4)
		{
			return ESCT_EGroupSize.Sentry;
		}
		else if (soldiersCount >= 4 && soldiersCount <= 5)
		{
			return ESCT_EGroupSize.Fireteam;
		}

		return ESCT_EGroupSize.Squad;
	}
}
