[EntityEditorProps("Prison", description: "Component for static radio stations.", color: "0 0 255 255")]
class ESCT_StaticRadioStationComponentClass : ESCT_StaticLocationComponentClass
{
}

class ESCT_StaticRadioStationComponent : ESCT_StaticLocationComponent
{
	override ESCT_ELocationType GetLocationType()
	{
		return ESCT_ELocationType.RadioStation;
	}
	
	void SpawnRadio()
	{
		array<ESCT_RadioSlotEntity> radioSlots = {};
		ESCT_EntityHelperT<ESCT_RadioSlotEntity>.GetChildrenOfType(GetOwner(), radioSlots);
		
		ESCT_Faction faction = GetOwnerFaction();
		if (!faction)
		{
			ESCT_Logger.ErrorFormat("[ESCT_StaticRadioStationComponent] Failed to obtain faction for %1 static radio", ToString());
			return;
		}
		
		foreach (ESCT_RadioSlotEntity slot : radioSlots)
		{
			slot.SpawnRadio(faction);
		}
	}
}
