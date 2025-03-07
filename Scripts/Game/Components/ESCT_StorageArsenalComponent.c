[EntityEditorProps("Escapists", description: "Inventory storage arsenal component.", color: "0 0 255 255")]
class ESCT_StorageArsenalComponentClass : ScriptComponentClass
{
}

class ESCT_StorageArsenalComponent : ScriptComponent
{
	[Attribute("0", uiwidget: UIWidgets.Slider, desc: "How many items will be stored in the. If zero then player count will be used as base modifier for owner-container.", params: "0 50 1", category: "Escapists")]
	protected int m_iItemQuantity;

	[Attribute("", desc: "Toggle supported SCR_EArsenalItemType by this arsenal, items are gathered from SCR_Faction or from the overwrite config", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(SCR_EArsenalItemType), category: "Escapists")]
	protected SCR_EArsenalItemType m_eSupportedArsenalItemTypes;

	[Attribute("", desc: "Toggle supported SCR_EArsenalItemMode by this arsenal, items are gathered from SCR_Faction or from the overwrite config", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(SCR_EArsenalItemMode), category: "Escapists")]
	protected SCR_EArsenalItemMode m_eSupportedArsenalItemModes;

	[Attribute("0", uiwidget: UIWidgets.CheckBox, desc: "Use player faction to fill this arsenal.", params: "0 1 0.05", category: "Settings")]
	protected bool m_bUsePlayerFaction;

	protected bool m_bAreItemsAssigned = false;
	protected ESCT_Faction m_Faction;

	bool AssignItems(out array<SCR_ArsenalItem> items, bool isSpawnGuaranateed)
	{
		if (m_bAreItemsAssigned)
			return false;

		AssignFaction();
		
		if (!m_Faction)
			return false;

		if (!isSpawnGuaranateed && IsSlotInvalid())
			return false;
		
		//ESCT_Logger.InfoFormat("[ESCT_StorageArsenalComponent] Selected %1 faction for location on %2 position.", m_Faction.GetFactionKey(), GetOwner().GetOrigin().ToString()); 

		array<SCR_ArsenalItem> filteredArsenalItems = {};
		if (!GetFiliteredItems(filteredArsenalItems))
			return false;
		
		//ESCT_Logger.InfoFormat("[ESCT_StorageArsenalComponent] Storage on %1 position - %2 available item count.", GetOwner().GetOrigin().ToString(), filteredArsenalItems.Count().ToString()); 

		int availableItemCount = filteredArsenalItems.Count();
		int itemsToFill = GetItemCount();
		
		//ESCT_Logger.InfoFormat("[ESCT_StorageArsenalComponent] About to fill storage on %1 position with %2 items.", GetOwner().GetOrigin().ToString(), itemsToFill.ToString()); 
		
		if (itemsToFill == 0)
			return false;

		// Use a single loop to create the weights array and find the weighted index
		float totalWeight = 0;
		for (int j = 0; j < availableItemCount; j++)
		{
			totalWeight += filteredArsenalItems[j].GetWeight();
		}

		for (int i = 0; i < itemsToFill; i++)
		{
			float randomValue = Math.RandomFloat01() * totalWeight;
			float cumulativeWeight = 0;

			for (int j = 0; j < availableItemCount; j++)
			{
				cumulativeWeight += filteredArsenalItems[j].GetWeight() + (Math.RandomFloat01() * 0.01); // add a small random offset;
				if (randomValue <= cumulativeWeight)
				{
					if (SCR_Enum.HasPartialFlag(filteredArsenalItems[j].GetItemMode(), SCR_EArsenalItemMode.AMMUNITION) 
						&& !SCR_Enum.HasPartialFlag(filteredArsenalItems[j].GetItemType(), SCR_EArsenalItemType.EQUIPMENT))
					{
						int insertCount = Math.RandomInt(2, 5);
						for (int q = 1; q <= insertCount; q++)
						{
							items.Insert(filteredArsenalItems[j]);
						}
					}
					else 
					{
						items.Insert(filteredArsenalItems[j]);
					}
					
					break;
				}
			}
		}

		m_bAreItemsAssigned = true;

		return !items.IsEmpty();
	}

	protected bool IsSlotInvalid()
	{
		float spawnChance = ESCT_EscapistsConfigComponent.GetInstance().GetItemSpawnChance();
		if (ESCT_EscapistsGameMode.GetGameMode().GetRandomGenerator().RandFloat01() > spawnChance)
		{
			return true;
		}

		return false;
	}

	protected void AssignFaction()
	{
		if (m_bUsePlayerFaction)
		{
			m_Faction = ESCT_FactionManager.GetInstance().GetPlayerFaction();
			return;
		}

		SCR_FactionAffiliationComponent factionComponent = SCR_FactionAffiliationComponent.Cast(GetOwner().FindComponent(SCR_FactionAffiliationComponent));
		if (factionComponent)
		{
			m_Faction = ESCT_Faction.Cast(factionComponent.GetAffiliatedFaction());
			return;
		}

		ESCT_FactionManager factionManager = ESCT_FactionManager.GetInstance();
		if (!factionManager)
			return;

		m_Faction = factionManager.GetEnemyFaction();
	}

	protected bool GetFiliteredItems(out notnull array<SCR_ArsenalItem> filteredArsenalItems)
	{
		if (!m_Faction)
			return false;

		//~ Cannot get items if no entity catalog manager
		SCR_EntityCatalogManagerComponent catalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		if (!catalogManager)
			return false;

		//~ Get filtered list from faction (if any) or default
		filteredArsenalItems = catalogManager.GetFilteredArsenalItems(m_eSupportedArsenalItemTypes, m_eSupportedArsenalItemModes, -1, m_Faction);

		return !filteredArsenalItems.IsEmpty();
	}

	protected int GetItemCount()
	{
		if (m_iItemQuantity != 0)
			return m_iItemQuantity;

		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return 0;

		return playerManager.GetPlayerCount() * Math.RandomInt(2, 5);
	}
}
