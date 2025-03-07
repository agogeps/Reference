[EntityEditorProps("Escapists", description: "Container component for building character spawn points.", color: "0 0 255 255")]
class ESCT_PopulatedBuildingComponentClass : ScriptComponentClass
{
}

sealed class ESCT_PopulatedBuildingComponent : ScriptComponent
{
	protected ESCT_Location m_Location = null;
	
	void GetCharacterSpawnPoints(out array<ESCT_CharacterSpawnPoint> spawnPoints)
	{		
		ESCT_EntityHelperT<ESCT_CharacterSpawnPoint>.GetChildrenOfType(GetOwner(), spawnPoints);
	}
	
	void GetEquipmentSlots(out array<ESCT_EquipmentSlotEntity> slots)
	{
		ESCT_EntityHelperT<ESCT_EquipmentSlotEntity>.GetChildrenOfType(GetOwner(), slots);
	}
	
	bool TryClaim(ESCT_Location location)
	{
		if(!m_Location)
		{
			m_Location = location;
			return true;
		}
		
		return m_Location == location;
	}
}
