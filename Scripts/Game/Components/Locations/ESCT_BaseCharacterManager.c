[EntityEditorProps("Escapists", description: "This component handles character spawn and despawn for locations.", color: "0 0 255 255", visible: false)]
class ESCT_BaseCharacterManagerComponentClass : ESCT_LocationComponentClass
{
}

class ESCT_BaseCharacterManagerComponent : ESCT_LocationComponent
{
	protected ref array<ESCT_CharacterSpawnPoint> m_aCharacterSpawnPoints = {};
	protected ref map<int, ref ESCT_CharacterState> m_CharacterStateMap = new map<int, ref ESCT_CharacterState>();
	
	void UpdateState(ESCT_CharacterState state)
	{
		int id = state.GetId();

		ESCT_CharacterState stateToUpdate = m_CharacterStateMap[id];
		if (!stateToUpdate)
			return;

		stateToUpdate.m_bIsKilled = state.m_bIsKilled;
		m_CharacterStateMap[id] = stateToUpdate;
		
		m_Location.GetOnEntityChange().Invoke(-1);
	}
}
