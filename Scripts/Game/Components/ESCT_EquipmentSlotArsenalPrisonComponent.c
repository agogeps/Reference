[EntityEditorProps("Escapists", description: "Equipment slot arsenal component.", color: "0 0 255 255")]
class ESCT_EquipmentSlotArsenalPrisonComponentClass : ESCT_EquipmentSlotArsenalComponentClass
{
}

class ESCT_EquipmentSlotArsenalPrisonComponent : ESCT_EquipmentSlotArsenalComponent
{
	protected override bool GetFiliteredItems(out notnull array<SCR_ArsenalItem> filteredArsenalItems)
	{
		if (!m_Faction)
			return false;
		
		//~ Cannot get items if no entity catalog manager
		SCR_EntityCatalogManagerComponent catalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		if (!catalogManager)
			return false;

		//~ Get filtered list from faction (if any) or default
		filteredArsenalItems = catalogManager.GetFilteredArsenalItems(m_eSupportedArsenalItemTypes, m_eSupportedArsenalItemModes, -1, m_Faction);
		
		for (int i = filteredArsenalItems.Count() - 1; i >= 0; i--)
		{
		    if (!filteredArsenalItems[i].IsEligibleForPrison())
		    {
		        filteredArsenalItems.Remove(i);
		    }
		}
		
		return !filteredArsenalItems.IsEmpty();
	}
}
