[EntityEditorProps(category: "GameScripted/GameMode", description: "Spawn point entity", visible: false)]
class ESCT_SpawnPointClass : SCR_SpawnPointClass
{
}

class ESCT_SpawnPoint : SCR_SpawnPoint
{
	protected override bool GetEmptyPositionAndRotationInRange(out vector pos, out vector rot)
	{
		SCR_SpawnPositionComponentManager spawnPosManagerComponent = SCR_SpawnPositionComponentManager.GetInstance();
		if (!spawnPosManagerComponent)
			return false;

		array<SCR_SpawnPositionComponent> positions = {};
		int count = spawnPosManagerComponent.GetSpawnPositionsInRange(GetOrigin(), m_fSpawnPositionUsageRange, positions);
		if (count < 0)
			return false;

		//I don't want to override SpawnPositionComponentManager so let's just cast-copy
		array<ESCT_SpawnPositionComponent> ePositions = {};
		foreach (SCR_SpawnPositionComponent spawnPosition : positions)
		{
			ePositions.Insert(ESCT_SpawnPositionComponent.Cast(spawnPosition));
		}

		ESCT_SpawnPositionComponent position;

		while (!ePositions.IsEmpty())
		{
			position = ePositions.GetRandomElement();

			if (!position.IsClaimed())
			{
				position.Claim();
				pos = position.GetOwner().GetOrigin();
				rot = position.GetOwner().GetAngles();
				return true;
			}
			else
			{
				ePositions.RemoveItem(position);
			}
		}

		return false;
	}
}
