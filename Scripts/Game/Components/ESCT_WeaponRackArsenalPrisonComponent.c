[EntityEditorProps("Escapists", description: "Weapon rack arsenal component.", color: "0 0 255 255")]
class ESCT_WeaponRackArsenalPrisonComponentClass : ESCT_WeaponRackArsenalComponentClass
{
}

class ESCT_WeaponRackArsenalPrisonComponent : ESCT_WeaponRackArsenalComponent
{
	protected override bool GetFiliteredItems(out notnull array<SCR_ArsenalItem> filteredArsenalItems)
	{	
		if (!GetFilteredArsenalItems(filteredArsenalItems, m_eArsenalItemDisplayType))
			return false;
		
		for (int i = filteredArsenalItems.Count() - 1; i >= 0; i--)
		{
		    if (!filteredArsenalItems[i].IsEligibleForPrison())
		    {
		        filteredArsenalItems.Remove(i);
		    }
		}
		
		return true;
	}
}
