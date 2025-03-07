[BaseContainerProps(category: "Respawn")]
sealed class ESCT_SpawnLogic : SCR_AutoSpawnLogic
{
	protected ESCT_WinLoseHandlerComponent m_WinLoseManager;

	private ref ScriptInvokerVoid m_OnPlayerSpawned;

	ScriptInvokerVoid GetOnPlayerSpawned()
	{
		if (!m_OnPlayerSpawned)
			m_OnPlayerSpawned = new ScriptInvokerVoid();

		return m_OnPlayerSpawned;
	}

	override void OnInit(SCR_RespawnSystemComponent owner)
	{
		super.OnInit(owner);

		m_WinLoseManager = ESCT_WinLoseHandlerComponent.GetInstance();
		if (!m_WinLoseManager)
		{
			ESCT_Logger.Error("[ESCT_SpawnLogic] Failed to obtain ESCT_WinLoseHandlerComponent, check game mode!");
		}
	}

	override void OnPlayerSpawned_S(int playerId, IEntity entity)
	{
		super.OnPlayerSpawned_S(playerId, entity);

		if (!IsEligibleForReconnection(playerId))
		{
			ESCT_CharacterHelper.RandomizeClothes(entity);
			TryPutCharacterIntoHeli(playerId, entity);
		}

		if (ESCT_GameStateManagerComponent.GetInstance().GetGameState() == ESCT_EGameState.Start)
			GetOnPlayerSpawned().Invoke();

		//local player controller and network component
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (pc)
		{
			ESCT_EscapistsNetworkComponent localNetworkComponent =
				ESCT_EscapistsNetworkComponent.Cast(pc.FindComponent(ESCT_EscapistsNetworkComponent));
			if (localNetworkComponent)
			{
				localNetworkComponent.AddUnconsciousDisplay();
				localNetworkComponent.RespawnLocationPopup();
			}
		}

		m_WinLoseManager.AddEscapistToCheck(playerId, entity);
	}

	override void OnPlayerDisconnected_S(int playerId, KickCauseCode cause, int timeout)
	{
		super.OnPlayerDisconnected_S(playerId, cause, timeout);
		m_WinLoseManager.RemoveEscapistFromCheck(playerId);
	}

	override void Spawn(int playerId)
	{
		// Player is disconnecting (and disappearance of controlled entity started this feedback loop).
		// Simply ignore such requests as it would create unwanted entities.
		int indexOf = m_DisconnectingPlayers.Find(playerId);
		if (indexOf != -1)
		{
			m_DisconnectingPlayers.Remove(indexOf);
			return;
		}

		ESCT_EscapistsConfigComponent gameConfig = ESCT_EscapistsConfigComponent.GetInstance();
		if (!gameConfig)
			return;

		ESCT_FactionManager factionManager = ESCT_FactionManager.GetInstance();
		if (!factionManager)
			return;

		ESCT_Faction targetFaction = factionManager.GetPlayerFaction();
		ESCT_PlayerFactionInfo info = targetFaction.GetPlayerFactionInfo();

		// Request both
		if (!GetPlayerFactionComponent_S(playerId).RequestFaction(targetFaction))
		{
			// Try again later
		}

		SCR_BasePlayerLoadout targetLoadout;
		ESCT_PlayerCharacter plrChar = info.GetPlayerCharacterByType(gameConfig.GetHealthType());
		if (!plrChar)
			return;

		string loadoutName = plrChar.GetLoadoutName();
		if (loadoutName != string.Empty)
		{
			targetLoadout = GetGame().GetLoadoutManager().GetLoadoutByName(loadoutName);
		}
		else
		{
			targetLoadout = GetGame().GetLoadoutManager().GetRandomFactionLoadout(targetFaction);
		}

		if (!GetPlayerLoadoutComponent_S(playerId).RequestLoadout(targetLoadout))
		{
			// Try again later
		}

		Faction faction = GetPlayerFactionComponent_S(playerId).GetAffiliatedFaction();
		if (!faction)
		{
			OnPlayerSpawnFailed_S(playerId);
			return;
		}

		SCR_BasePlayerLoadout loadout = GetPlayerLoadoutComponent_S(playerId).GetLoadout();
		if (!loadout)
		{
			OnPlayerSpawnFailed_S(playerId);
			return;
		}

		SCR_SpawnPoint point = SCR_SpawnPoint.GetRandomSpawnPointForFaction(faction.GetFactionKey());
		if (!point)
		{
			OnPlayerSpawnFailed_S(playerId);
			return;
		}

		SCR_SpawnPointSpawnData data = new SCR_SpawnPointSpawnData(loadout.GetLoadoutResource(), point.GetRplId());
		if (GetPlayerRespawnComponent_S(playerId).CanSpawn(data))
			DoSpawn(playerId, data);
		else
			OnPlayerSpawnFailed_S(playerId);
	}
	
	private void TryPutCharacterIntoHeli(int playerId, IEntity character)
	{
		ESCT_EscapistsGameMode gameMode = ESCT_EscapistsGameMode.GetGameMode();
		if (gameMode.GetStartType() != ESCT_EStartType.Helicrash)
			return;
		
		if (ESCT_GameStateManagerComponent.GetInstance().GetGameState() > ESCT_EGameState.Start)
			return;
		
		ESCT_LocationSystem locationSystem = ESCT_LocationSystem.GetSystem();
		ESCT_StartArea helicrash = ESCT_StartArea.Cast(locationSystem.GetStartArea());
		if (!helicrash)
			return;
		
		ESCT_HelicrashStartComponent helicrashStart = ESCT_HelicrashStartComponent.Cast(
			helicrash.GetComposition().FindComponent(ESCT_HelicrashStartComponent));
		if (!helicrashStart)
			return;
		
		IEntity helicopter = helicrashStart.GetHelicopter();
		if (!helicopter)
			return;
		
		ESCT_CharacterSpawnVehicleAssistantComponent spawnAssistant = 
			ESCT_CharacterSpawnVehicleAssistantComponent.Cast(helicopter.FindComponent(ESCT_CharacterSpawnVehicleAssistantComponent));
		if (!spawnAssistant)
			return;
		
		spawnAssistant.TrySchedulePlayerForHelicopter(character, playerId);	
	}
}
