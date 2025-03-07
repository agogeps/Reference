[ComponentEditorProps(category: "Escapists/GameMode/Components", description: "Component that assigns various tasks for players.")]
class ESCT_TaskAssignerComponentClass: ESCT_GameModeBaseComponentClass
{
}

sealed class ESCT_TaskAssignerComponent: ESCT_GameModeBaseComponent
{
	private WorldTimestamp m_fTaskAssignWaitTimestamp;
	
	private static const float CHECK_INTERVAL = 1000.0; // 30 secs
	
	private static ESCT_TaskAssignerComponent s_Instance = null;
	static ESCT_TaskAssignerComponent GetInstance()
	{
		if (!s_Instance)
		{
			BaseGameMode gameMode = GetGame().GetGameMode();
			if (gameMode)
			{
				s_Instance = ESCT_TaskAssignerComponent.Cast(gameMode.FindComponent(ESCT_TaskAssignerComponent));
			}
		}

		return s_Instance;
	}
	
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		if (SCR_Global.IsEditMode() || !Replication.IsServer())
			return;

		SetEventMask(owner, EntityEvent.INIT);
	}

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		if (SCR_Global.IsEditMode() || !Replication.IsServer()) return;

		ESCT_GameStateManagerComponent.GetInstance().GetOnGameStateChanged().Insert(HandleGameStateChanged);
	}
	
	void SetupRadioTaskAssignmentChecker(notnull IEntity startArea)
	{
		ChimeraWorld world = GetOwner().GetWorld();
		if (world)
		{
			m_fTaskAssignWaitTimestamp = world.GetServerTimestamp().PlusSeconds(Math.RandomInt(45, 90));
		}
			
		GetGame().GetCallqueue().CallLater(RadioTaskAssignmentChecker, CHECK_INTERVAL, true, startArea);
	}
	
	void CreateLockpickTask(notnull IEntity door)
	{	
		ESCT_LockpickTaskSupportEntity supportEntity = ESCT_LockpickTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(ESCT_LockpickTaskSupportEntity));
		if (!supportEntity || supportEntity.HasTasks())
			return;

		ESCT_LockpickTask task = ESCT_LockpickTask.Cast(supportEntity.CreateTask(door));
		if (!task)
			return;
		
		supportEntity.SetTargetFaction(task, ESCT_FactionManager.GetInstance().GetPlayerFaction());
	}
	
	void CreateRadioTask()
	{
		if(ESCT_GameStateManagerComponent.GetInstance().GetGameState() != ESCT_EGameState.RadioStationSearch)
			return;
		
		ESCT_RadioTaskSupportEntity supportEntity = ESCT_RadioTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(ESCT_RadioTaskSupportEntity));
		if (!supportEntity || supportEntity.HasTasks())
			return;
		
		ESCT_RadioTask task = ESCT_RadioTask.Cast(supportEntity.CreateTask(null));
		if (!task)
			return;
		
		supportEntity.SetTargetFaction(task, ESCT_FactionManager.GetInstance().GetPlayerFaction());
	}
	
	void CreateExtractionPointTask(vector position)
	{
		if(ESCT_GameStateManagerComponent.GetInstance().GetGameState() != ESCT_EGameState.Transport)
			return;
		
		ESCT_MoveTaskSupportEntity supportEntity = ESCT_MoveTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(ESCT_MoveTaskSupportEntity));
		if (!supportEntity || supportEntity.HasTasks())
			return;
		
		ESCT_MoveTask task = ESCT_MoveTask.Cast(supportEntity.CreateTask(position));
		if (!task)
			return;
		
		supportEntity.SetTargetFaction(task, ESCT_FactionManager.GetInstance().GetPlayerFaction());
	}
	
	void CreateDefendRadioTask(ESCT_Radio radio, vector position)
	{
		ESCT_DefendRadioTaskSupportEntity supportEntity = ESCT_DefendRadioTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(ESCT_DefendRadioTaskSupportEntity));
		if (!supportEntity || supportEntity.HasTasks())
			return;
		
		SCR_BaseTask task = supportEntity.CreateDefendTask(radio, position);
		if (!task)
			return;
		
		supportEntity.SetTargetFaction(task, ESCT_FactionManager.GetInstance().GetPlayerFaction());
	}
	
	void CreateEscapeTask()
	{
		if(ESCT_GameStateManagerComponent.GetInstance().GetGameState() != ESCT_EGameState.Transport)
			return;
		
		ESCT_EscapeTaskSupportEntity supportEntity = ESCT_EscapeTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(ESCT_EscapeTaskSupportEntity));
		if (!supportEntity || supportEntity.HasTasks())
			return;
		
		if (!supportEntity.GetTasks().IsEmpty())
			return;
		
		ESCT_WinLoseHandlerComponent wlHandler = ESCT_WinLoseHandlerComponent.GetInstance();
		ESCT_EscapeTask task = supportEntity.CreateEscapeTask(wlHandler.GetEscapeTaskPosition());
		if (!task)
			return;
		
		supportEntity.SetTargetFaction(task, ESCT_FactionManager.GetInstance().GetPlayerFaction());
	}
	
	void FailEscapeTask()
	{
		ESCT_EscapeTaskSupportEntity supportEntity = ESCT_EscapeTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(ESCT_EscapeTaskSupportEntity));
		if (!supportEntity)
			return;
		
		if (supportEntity.GetTasks().IsEmpty())
			return;
		
		ESCT_EscapeTask escapeTask = ESCT_EscapeTask.Cast(GetTaskManager().GetTask(supportEntity.GetTaskIdByType(ESCT_EscapeTask)));
		if (!escapeTask)
			return;
		
		supportEntity.FailTask(escapeTask);
	}
	
	void FinishDefendRadioTask()
	{
		ESCT_DefendRadioTaskSupportEntity supportEntity = ESCT_DefendRadioTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(ESCT_DefendRadioTaskSupportEntity));
		if (!supportEntity)
			return;
		
		ref array<SCR_BaseTask> tasks = {};
		supportEntity.GetTasks(tasks);
		
		foreach (SCR_BaseTask task : tasks)
		{
			if (!task)
				continue;
			
			ESCT_DefendRadioTask defendRadioTask = ESCT_DefendRadioTask.Cast(task);
			if (!defendRadioTask)
				continue;
			
			supportEntity.FinishTask(defendRadioTask);
		}
	}
		
	private void RadioTaskAssignmentChecker(IEntity startArea)
	{
		if (m_fTaskAssignWaitTimestamp)
		{
			ChimeraWorld world = GetOwner().GetWorld();
			if (world && world.GetServerTimestamp().GreaterEqual(m_fTaskAssignWaitTimestamp))
			{
				CreateRadioTask();
				GetGame().GetCallqueue().Remove(RadioTaskAssignmentChecker);
				return;
			}
		}
		
		array<IEntity> players = {};
		ESCT_Player.GetAlivePlayers(players, true);
		
		//prison already despawned shortcut
		if (!startArea)
		{
			GetGame().GetCallqueue().CallLater(CreateRadioTask, Math.RandomInt(1000, 4000), false);
			GetGame().GetCallqueue().Remove(RadioTaskAssignmentChecker);	
			return;
		}
		
		if (ESCT_Misc.HasNearEntities(startArea, players, 20))
			return;

		GetGame().GetCallqueue().CallLater(CreateRadioTask, Math.RandomInt(1000, 4000), false);
		GetGame().GetCallqueue().Remove(RadioTaskAssignmentChecker);	
	}
	
	private void HandleGameStateChanged(ESCT_EGameState state)
	{
		if (state != ESCT_EGameState.Transport)
			return;
		
		FinishDefendRadioTask();
		
		ESCT_GameStateManagerComponent.GetInstance().GetOnGameStateChanged().Remove(HandleGameStateChanged);
	}
}
