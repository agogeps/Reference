[EntityEditorProps(category: "Escapists/Entities/Locations", description: "Starting prison location type.")]
class ESCT_StartAreaClass : ESCT_LocationClass
{
}

sealed class ESCT_StartArea : ESCT_Location
{
	private ESCT_StartAreaComponent m_startAreaComponent;

	void SetStartAreaComponent(notnull ESCT_StartAreaComponent startAreaComponent)
	{
		m_startAreaComponent = startAreaComponent;
	}

	override bool Despawn()
	{
		if (!Replication.IsServer() || m_SpawnState == ESCT_ESpawnState.Idle)
			return false;

		bool despawn = super.Despawn();

		SCR_SpawnPoint spawnPoint = m_startAreaComponent.GetStartingSpawnPoint();
		if (spawnPoint && spawnPoint.IsSpawnPointEnabled())
			spawnPoint.SetSpawnPointEnabled_S(false);

		return true;
	}
}
