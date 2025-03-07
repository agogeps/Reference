[ComponentEditorProps(category: "Escapists/GameMode/Components", description: "Components that checks win-lose conditions applied to Escapists game mode.")]
class ESCT_WinLoseHandlerComponentClass : ESCT_GameModeBaseComponentClass
{
}

sealed class ESCT_WinLoseHandlerComponent : ESCT_GameModeBaseComponent
{
	[Attribute(defvalue: "0 0 0", uiwidget: UIWidgets.Coords, category: "Escapists")]
	protected vector m_vEscapeTaskPosition;

	[Attribute(UIWidgets.Coords, desc: "Border polygon that players need to exit to win the game.", category: "Escapists")]
	protected ref array<vector> m_aWinBorderPolygon;

	[Attribute(UIWidgets.Coords, desc: "Border polygon that players need to enter to win the game.", category: "Escapists")]
	protected ref array<vector> m_aWinCrossPolygon;

	[RplProp()]
	protected ref array<int> m_aEscapees = {};

	[RplProp()]
	protected ref array<int> m_aLeftBehinders = {};

	protected ChimeraWorld m_World;
	protected PlayerManager m_PlayerManager;
	protected ESCT_GameStateManagerComponent m_GameState;

	protected ref array<IEntity> m_aEscapeVehicles;

	private ref array<ref ESCT_TrackablePlayer> m_aTrackedPlayers;
	private bool m_bNoMoreFleeVehicles = false;
	private bool m_bIsLosing = false;
	private float m_fLastExecutionTime = 0;

	private static const float CHECK_INTERVAL = 1000.0;

	private static ESCT_WinLoseHandlerComponent s_Instance = null;
	static ESCT_WinLoseHandlerComponent GetInstance()
	{
		if (!s_Instance)
		{
			BaseGameMode gameMode = GetGame().GetGameMode();
			if (gameMode)
			{
				s_Instance = ESCT_WinLoseHandlerComponent.Cast(gameMode.FindComponent(ESCT_WinLoseHandlerComponent));
			}
		}

		return s_Instance;
	}

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (SCR_Global.IsEditMode() || !Replication.IsServer()) return;

		SetEventMask(owner, EntityEvent.INIT);
	}

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (SCR_Global.IsEditMode() || !Replication.IsServer()) return;

		ESCT_Logger.Info("[ESCT_WinLoseHandlerComponent] Initializing Win/Lose Handler.");

		if (m_aWinBorderPolygon.IsEmpty() && m_aWinCrossPolygon.IsEmpty())
			ESCT_Logger.Error("[ESCT_WinLoseHandlerComponent] Win border and cross polygons are not specified, check win/lose handler in game mode.");

		m_World = GetOwner().GetWorld();
		m_PlayerManager = GetGame().GetPlayerManager();
		m_GameState = ESCT_GameStateManagerComponent.GetInstance();

		m_aTrackedPlayers = {};
		m_aEscapeVehicles = {};

		SCR_RespawnSystemComponent respawnSystem = SCR_RespawnSystemComponent.GetInstance();
		if (respawnSystem)
		{
			ESCT_SpawnLogic spawnLogic = ESCT_SpawnLogic.Cast(respawnSystem.GetSpawnLogic());
			if (spawnLogic)
				spawnLogic.GetOnPlayerSpawned().Insert(ActivateWinLoseOnPlayerSpawn);
		}

		ESCT_Logger.Info("[ESCT_WinLoseHandlerComponent] Win/Lose Handler initialized.");
	}

	override void EOnFixedFrame(IEntity owner, float timeSlice)
	{
		super.EOnFixedFrame(owner, timeSlice);

		float currentTime = GetGame().GetWorld().GetWorldTime();
		if (currentTime - m_fLastExecutionTime < 1)
			return;

		m_fLastExecutionTime = currentTime + CHECK_INTERVAL;

		if (m_aTrackedPlayers.IsEmpty())
			return;

		CheckLoseConditions();
		CheckWinConditions();
	}

	void AddEscapistToCheck(int pid, notnull IEntity escapistEntity)
	{
		bool alreadyThere = false;
		foreach (ESCT_TrackablePlayer player : m_aTrackedPlayers)
		{
			if (!player)
				continue;

			if (player.GetPlayerId() == pid)
			{
				alreadyThere = true;
				break;
			}
		}

		if (alreadyThere)
		{
			ESCT_Logger.WarningFormat("[ESCT_WinLoseHandlerComponent] Spawn system tried to add %1 PID for player that already exists in tracked array!", pid.ToString());
			return;
		}

		ESCT_TrackablePlayer newPlayer = new ESCT_TrackablePlayer(pid, escapistEntity);
		m_aTrackedPlayers.Insert(newPlayer);
	}

	void RemoveEscapistFromCheck(int pid)
	{
		ESCT_TrackablePlayer player = null;

		for (int i = m_aTrackedPlayers.Count() - 1; i >= 0; i--)
		{
			player = m_aTrackedPlayers[i];
			if (!player)
				continue;

			if (pid == player.GetPlayerId())
			{
				m_aTrackedPlayers.Remove(i);
				break;
			}
		}

		if (!player)
		{
			ESCT_Logger.WarningFormat("[ESCT_WinLoseHandlerComponent] Spawn system tried to remove %1 PID for player that's already doesn't exist in tracked array.", pid.ToString());
			return;
		}
	}

	void AddEscapeVehicle(notnull IEntity escapeVehicle)
	{
		if (!m_aEscapeVehicles)
			m_aEscapeVehicles = {};

		m_aEscapeVehicles.Insert(escapeVehicle);
	}

	void RemoveEscapeVehicle(notnull IEntity escapeVehicle)
	{
		if (!m_aEscapeVehicles)
			return;

		m_aEscapeVehicles.RemoveItem(escapeVehicle);
	}

	vector GetEscapeTaskPosition()
	{
		return m_vEscapeTaskPosition;
	}

	void EnableWinLoseChecks()
	{
		SetEventMask(GetOwner(), EntityEvent.FIXEDFRAME);
	}

	void SetNoMoreVehicles()
	{
		m_bNoMoreFleeVehicles = true;
	}

	void ForceEndGame()
	{
		ESCT_Logger.InfoFormat("Forcing game end!");
		switch (true)
		{
			case (!m_aLeftBehinders.IsEmpty()):
			{
				ESCT_Logger.InfoFormat("Left behind ending.");
				GetEscapistsGameMode().EndGameMode(SCR_GameModeEndData.CreateSimple(EGameOverTypes.ESCAPISTS_PARTIAL_VICTORY));
				break;
			}
			case (!m_aEscapees.IsEmpty()):
			{
				ESCT_Logger.InfoFormat("Victory ending.");
				GetEscapistsGameMode().EndGameMode(SCR_GameModeEndData.CreateSimple(EGameOverTypes.ESCAPISTS_VICTORY));
				break;
			}
			case (m_bNoMoreFleeVehicles):
			{
				ESCT_Logger.InfoFormat("No more vehicles ending.");
				GetEscapistsGameMode().EndGameMode(SCR_GameModeEndData.CreateSimple(EGameOverTypes.ESCAPISTS_DEFEAT_NO_MORE_VEHICLES));
				break;
			}
			default:
			{
				ESCT_Logger.InfoFormat("Defeat ending.");
				GetEscapistsGameMode().EndGameMode(SCR_GameModeEndData.CreateSimple(EGameOverTypes.ESCAPISTS_DEFEAT));
			}
		}
	}

	void GetEndStates(notnull out array<int> escapees, notnull out array<int> leftBehinders, out bool noMoreVehicles)
	{
		escapees.InsertAll(m_aEscapees);
		leftBehinders.InsertAll(m_aLeftBehinders);
		noMoreVehicles = m_bNoMoreFleeVehicles;
	}

	private void Win(array<int> escapees, array<int> leftBehinders)
	{
		ESCT_Logger.Info("Win conditions met, winning the game.");

		//broadcast playerids to show correct info about players on endgame screen
		m_aEscapees.InsertAll(escapees);
		m_aLeftBehinders.InsertAll(leftBehinders);
		Replication.BumpMe();

		ESCT_ScreenFadeComponent screenFade = ESCT_ScreenFadeComponent.GetInstance();
		if (!screenFade)
			return;

		screenFade.FadeToBlackToEnd();
	}

	private void Lose()
	{
		ESCT_Logger.Info("Lose conditions met, losing the game.");

		ESCT_TaskAssignerComponent taskAssigner = ESCT_TaskAssignerComponent.GetInstance();
		if (taskAssigner)
			taskAssigner.FailEscapeTask();

		ESCT_ScreenFadeComponent screenFade = ESCT_ScreenFadeComponent.GetInstance();
		if (screenFade)
			screenFade.FadeToBlackToEnd();
	}

	void ActivateWinLoseOnPlayerSpawn()
	{
		if (SCR_Enum.HasPartialFlag(GetEventMask(), EntityEvent.FIXEDFRAME))
		{
			RemoveSpawnEvent();
			return;
		}

		SetEventMask(GetOwner(), EntityEvent.FIXEDFRAME);
		RemoveSpawnEvent();
	}

	private void RemoveSpawnEvent()
	{
		SCR_RespawnSystemComponent respawnSystem = SCR_RespawnSystemComponent.GetInstance();
		if (!respawnSystem)
			return;

		ESCT_SpawnLogic spawnLogic = ESCT_SpawnLogic.Cast(respawnSystem.GetSpawnLogic());
		if (!spawnLogic)
			return;

		spawnLogic.GetOnPlayerSpawned().Remove(ActivateWinLoseOnPlayerSpawn);
	}

	private void CheckLoseConditions()
	{
		bool isSomeoneAlive = false;
		IEntity playerEntity = null;

		foreach (ESCT_TrackablePlayer player : m_aTrackedPlayers)
		{
			if (!player)
				continue;

			playerEntity = player.GetPlayerEntity();
			if (!playerEntity)
				continue;

			SCR_ChimeraCharacter playerCharacter = SCR_ChimeraCharacter.Cast(playerEntity);
			if (!playerCharacter)
				continue;

			ESCT_PlayerDamageManagerComponent damageManager = ESCT_PlayerDamageManagerComponent.Cast(playerCharacter.GetDamageManager());
			if (!damageManager)
				continue;

			if (!damageManager.IsEligibleForLose())
			{
				isSomeoneAlive = true;
				break;
			}
		}

		if (!isSomeoneAlive)
		{
			Lose();
			ClearEventMask(GetOwner(), EntityEvent.FIXEDFRAME);
			m_aTrackedPlayers.Clear();
			return;
		}
	}

	private void CheckWinConditions()
	{
		if (m_GameState.GetGameState() != ESCT_EGameState.Transport || m_aEscapeVehicles.IsEmpty())
			return;

		array<int> escapees = {};
		array<int> leftBehinders = {};

		IEntity escapeVehicle = null;
		array<IEntity> occupants = {};

		for (int i = m_aEscapeVehicles.Count() - 1; i >= 0; --i)
		{
			escapeVehicle = m_aEscapeVehicles[i];
			if (!escapeVehicle)
			{
				m_aEscapeVehicles.Remove(i);
				continue;
			}

			//vehicles that's still inside playable zone are not interesting for us
			if (!m_aWinBorderPolygon.IsEmpty() && Math2D.IsPointInPolygonXZ(m_aWinBorderPolygon, escapeVehicle.GetOrigin()))
				continue;

			//vehicles that are not inside the win border are not interesting for us
			if (!m_aWinCrossPolygon.IsEmpty() && !Math2D.IsPointInPolygonXZ(m_aWinCrossPolygon, escapeVehicle.GetOrigin()))
				continue;

			SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(escapeVehicle.FindComponent(SCR_BaseCompartmentManagerComponent));
			if (!compartmentManager)
				continue;

			occupants.Clear();
			compartmentManager.GetOccupants(occupants);

			foreach (IEntity occupant : occupants)
			{
				if (!EntityUtils.IsPlayer(occupant))
					continue;

				int pid = m_PlayerManager.GetPlayerIdFromControlledEntity(occupant);
				escapees.Insert(pid);
			}
		}

		foreach (ESCT_TrackablePlayer player : m_aTrackedPlayers)
		{
			if (!player)
				continue;
			
			int pid = player.GetPlayerId();
			
			
			if (escapees.Find(pid) != -1)
				continue;

			leftBehinders.Insert(pid);
		}

		if (!escapees.IsEmpty())
		{
			Win(escapees, leftBehinders);
			ClearEventMask(GetOwner(), EntityEvent.FIXEDFRAME);
			m_aTrackedPlayers.Clear();
			return;
		}
	}

#ifdef WORKBENCH
	override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
	{
		if (!m_aWinBorderPolygon.IsEmpty())
		{
			int pointCount = m_aWinBorderPolygon.Count();

			if (!pointCount)
				return;

			vector poly[100];

			for (int i = 0; i < pointCount; i++)
			{
				vector pos = m_aWinBorderPolygon[i];
				pos[1] = Math.Max(SCR_TerrainHelper.GetTerrainY(pos), 0) + 5;
				poly[i] = pos;
			};

			poly[pointCount] = poly[0];

			Shape.CreateLines(
				Color.DARK_GREEN,
				ShapeFlags.ONCE | ShapeFlags.NOZWRITE,
				poly,
				Math.Min(pointCount + 1, 100)
			);
		}

		if (!m_aWinCrossPolygon.IsEmpty())
		{
			int pointCount = m_aWinCrossPolygon.Count();

			if (!pointCount)
				return;

			vector poly[100];

			for (int i = 0; i < pointCount; i++)
			{
				vector pos = m_aWinCrossPolygon[i];
				pos[1] = Math.Max(SCR_TerrainHelper.GetTerrainY(pos), 0) + 5;
				poly[i] = pos;
			};

			poly[pointCount] = poly[0];

			Shape.CreateLines(
				Color.DARK_RED,
				ShapeFlags.ONCE | ShapeFlags.NOZWRITE,
				poly,
				Math.Min(pointCount + 1, 100)
			);
		}
	}
#endif
}
