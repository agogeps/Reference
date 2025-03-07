[EntityEditorProps("Escapists", description: "Weapon rack arsenal component.", color: "0 0 255 255")]
class ESCT_WeaponRackArsenalComponentClass : SCR_ArsenalComponentClass
{
}

class ESCT_WeaponRackArsenalComponent : SCR_ArsenalComponent
{
	[Attribute("0", desc: "Type of display data an arsenal item needs in in order to be displayed on this arsenal display", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EArsenalItemDisplayType), category: "Display settings")]
	protected EArsenalItemDisplayType m_eArsenalItemDisplayType;

	[Attribute("0.3", uiwidget: UIWidgets.Slider, desc: "Chance that slot will be filled with equipment.", params: "0 1 0.05", category: "Settings")]
	protected float m_fSlotFillChance;
	
	[Attribute("0", uiwidget: UIWidgets.CheckBox, desc: "Use player faction to fill this arsenal.", params: "0 1 0.05", category: "Settings")]
	protected bool m_bUsePlayerFaction;

	protected ref array<ESCT_WeaponRackSlotEntity> m_aSlots = {};

	private bool m_bAreResourcesAssigned = false;

	override void EOnInit(IEntity owner)
	{
		IEntity child = owner.GetChildren();
		while (child)
		{
			ESCT_WeaponRackSlotEntity slot = ESCT_WeaponRackSlotEntity.Cast(child);
			if (slot)
				RegisterSlot(slot);

			child = child.GetSibling();
		}

		m_FactionManager = GetGame().GetFactionManager();
	}

	void Spawn()
	{
		if (!m_bAreResourcesAssigned)
		{
			InitFaction();
			AssignResources();
		}

		SpawnItems();
	}
	
	override void ClearArsenal()
	{
		for (int i = 0; i < m_aSlots.Count(); i++)
		{
			m_aSlots[i].RemoveItem();
		}
	}

	protected void RegisterSlot(ESCT_WeaponRackSlotEntity slot)
	{
		if (!slot || m_aSlots.Find(slot) != -1)
		{
			return;
		}

		m_aSlots.Insert(slot);
	}

	protected void AssignResources()
	{			
		array<SCR_ArsenalItem> filteredArsenalItems = {};
		if (!GetFiliteredItems(filteredArsenalItems))
		    return;
		
		int availableItemCount = filteredArsenalItems.Count();
		int availableSlotCount = m_aSlots.Count();
		
		SCR_ArsenalItem itemToSpawn;
		ESCT_WeaponRackSlotEntity currentSlot;
		int index;
		
		array<float> weights = {};
		for (int j = 0; j < availableItemCount; j++)
		{
			float weight = filteredArsenalItems[j].GetWeight() + (Math.RandomFloat01() * 0.01);
			weights.Insert(weight); //small noise for more variety
		}
		
		for (int i = 0; i < availableSlotCount; i++)
		{
			currentSlot = m_aSlots[i];
			
			if (IsSlotInvalid(currentSlot))
				continue;
			
			int iteration = 30;
			
			while (iteration > 0)
			{
				index = SCR_ArrayHelper.GetWeightedIndex(weights, Math.RandomFloat01());
				itemToSpawn = filteredArsenalItems[index];
				if (GetItemValidForSlot(itemToSpawn.GetItemType(), itemToSpawn.GetItemMode(), currentSlot.GetSlotSupportedItemTypes(), currentSlot.GetSlotSupportedItemModes()))
				{
					break;
				}
				
				iteration--;
				itemToSpawn = null;
			}

			if (!itemToSpawn)
				continue;

			currentSlot.AssignItem(itemToSpawn.GetItemResourceName(), itemToSpawn, itemToSpawn.GetDisplayDataOfType(m_eArsenalItemDisplayType));			
		}	

		m_bAreResourcesAssigned = true;
	}

	protected void SpawnItems()
	{
		for (int i = 0; i < m_aSlots.Count(); i++)
		{
			m_aSlots[i].SpawnItem();
		}
	}

	protected bool IsSlotInvalid(ESCT_WeaponRackSlotEntity slotEntity)
	{
		if (!slotEntity)
			return false;

		//not every slot should be filled
		if (Math.RandomFloat(0, 1) > m_fSlotFillChance)
			return true;

		return false;
	}

	protected bool GetItemValidForSlot(SCR_EArsenalItemType itemType, SCR_EArsenalItemMode itemMode, SCR_EArsenalItemType supportedSlotTypes, SCR_EArsenalItemMode supportedSlotModes)
	{
		return GetItemValid(itemType, itemMode) && SCR_Enum.HasPartialFlag(supportedSlotTypes, itemType) && SCR_Enum.HasPartialFlag(supportedSlotModes, itemMode);
	}

	protected void InitFaction()
	{	
		SCR_FactionAffiliationComponent factionComponent = SCR_FactionAffiliationComponent.Cast(GetOwner().FindComponent(SCR_FactionAffiliationComponent));
		if (!factionComponent)
			return;
		
		if (factionComponent.GetAffiliatedFactionKey() != string.Empty)
			return;
		
		if (m_bUsePlayerFaction)
		{
			string factionKey = ESCT_FactionManager.GetInstance().GetPlayerFactionKey();
			factionComponent.SetAffiliatedFactionByKey(factionKey);
		}
		else 
		{
			string factionKey = ESCT_FactionManager.GetInstance().GetEnemyFactionKey();
			factionComponent.SetAffiliatedFactionByKey(factionKey);
		}

		FactionInit(GetOwner());
	}
	
	protected bool GetFiliteredItems(out notnull array<SCR_ArsenalItem> filteredArsenalItems)
	{
		return GetFilteredArsenalItems(filteredArsenalItems, m_eArsenalItemDisplayType);
	}
}
