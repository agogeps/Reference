class ESCT_Player
{
	static bool GetPlayerIDs(notnull out array<int> playerIds)
	{	
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return false;
	
		playerManager.GetPlayers(playerIds);
		
		return true;
	}
	
	static bool GetPlayers(notnull out array<IEntity> playerEntities)
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();

		array<int> playerIds = {};
		playerManager.GetPlayers(playerIds);

		if (playerIds.IsEmpty())
		{
			return false;
		}

		foreach (int playerId : playerIds)
		{
			IEntity playerEntity = playerManager.GetPlayerControlledEntity(playerId);
			if (!playerEntity)
			{
				continue;
			}

			playerEntities.Insert(playerEntity);
		}

		return true;
	}

	static map<int, IEntity> GetPlayerMap()
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();

		array<int> playerIds = {};
		playerManager.GetPlayers(playerIds);

		if (playerIds.IsEmpty())
		{
			return null;
		}

		map<int, IEntity> playerIdsToEntities = new map<int, IEntity>;
		foreach (int playerId : playerIds)
		{
			IEntity playerEntity = playerManager.GetPlayerControlledEntity(playerId);
			if (!playerEntity)
			{
				continue;
			}

			playerIdsToEntities.Insert(playerId, playerEntity);
		};

		return playerIdsToEntities;
	}

	static void GetAlivePlayers(notnull out array<IEntity> playerEntities, bool includeUnconscious = false)
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();

		array<int> playerIds = {};
		playerManager.GetPlayers(playerIds);

		if (playerIds.IsEmpty())
			return;

		foreach (int playerId : playerIds)
		{
			IEntity playerEntity = playerManager.GetPlayerControlledEntity(playerId);
			if (!playerEntity)
			{
				continue;
			}

			SCR_CharacterControllerComponent charController = SCR_CharacterControllerComponent.Cast(playerEntity.FindComponent(SCR_CharacterControllerComponent));
			if (!charController)
				continue;

			ECharacterLifeState lifeState = charController.GetLifeState();
			
			if (includeUnconscious && lifeState != ECharacterLifeState.DEAD)
			{
				playerEntities.Insert(playerEntity);
			} 
			else if (lifeState == ECharacterLifeState.ALIVE)
			{
				playerEntities.Insert(playerEntity);
			}
		}
	}
}
