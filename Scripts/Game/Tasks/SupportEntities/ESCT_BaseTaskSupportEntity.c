class ESCT_BaseTaskSupportEntityClass : SCR_BaseTaskSupportEntityClass
{
}

class ESCT_BaseTaskSupportEntity : SCR_BaseTaskSupportEntity
{	
	protected ref map<int, SCR_BaseTask> m_mTasks = new map<int, SCR_BaseTask>();

	bool HasTasks()
	{
		if (!m_mTasks)
			return false;
		
		return !m_mTasks.IsEmpty();
	}
		
	void GetTasks(notnull array<SCR_BaseTask> tasks)
	{
		for (int i=0; i < m_mTasks.Count(); i++)
		{
			int taskId = m_mTasks.GetKey(i);
			SCR_BaseTask task = m_mTasks.Get(taskId);
			if (task)
				tasks.Insert(task);
		}
	}
	
	map<int, SCR_BaseTask> GetTasks()
	{
		for (int i=0; i < m_mTasks.Count(); i++)
		{
			int taskId = m_mTasks.GetKey(i);
			SCR_BaseTask task = m_mTasks.Get(taskId);
			PrintFormat("Key: %1, Value: %2", taskId, task);
		}
		
		return m_mTasks;
	}	
	
	SCR_BaseTask GetTaskById(int taskId)
	{
		return m_mTasks.Get(taskId);
	}
	
	SCR_BaseTask GetTaskByType(typename type)
	{
		
		for (int i=0; i < m_mTasks.Count(); i++)
		{
			int taskId = m_mTasks.GetKey(i);
			SCR_BaseTask task = m_mTasks.Get(taskId);
			if (task.Type() == type)
			{
				return task;
			}
		}
		
		return null;
	}
	
	int GetTaskIdByType(typename type)
	{
		for (int i=0; i < m_mTasks.Count(); i++)
		{
			int taskId = m_mTasks.GetKey(i);
			SCR_BaseTask task = m_mTasks.Get(taskId);
			if (task.Type() == type)
			{
				return taskId;
			}
		}
		
		return null;
	}
	
	SCR_BaseTask CreateTask(IEntity entity = null, bool shouldAssign = true)
	{
		SCR_BaseTask baseTask = Create();
		if (!baseTask)
			return null;
		
		if (entity)
		{
			MoveTask(entity.GetOrigin(), baseTask.GetTaskID());
		}
		
		if (shouldAssign)
		{			
			array<int> playerIds = {};
			ESCT_Player.GetPlayerIDs(playerIds);
			AssignTaskToPlayers(baseTask, playerIds);
		}
		
		return baseTask;
	}
	
	SCR_BaseTask CreateTask(vector position, bool shouldAssign = true)
	{
		SCR_BaseTask baseTask = Create();
		if (!baseTask)
			return null;
		
		MoveTask(position, baseTask.GetTaskID());
		
		if (shouldAssign)
		{			
			array<int> playerIds = {};
			ESCT_Player.GetPlayerIDs(playerIds);
			AssignTaskToPlayers(baseTask, playerIds);
		}
		
		return baseTask;
	}
	
	override void FinishTask(notnull SCR_BaseTask task)
	{
		super.FinishTask(task);
		
		int taskId = m_mTasks.GetKeyByValue(task);
		if (taskId)
			m_mTasks.Remove(taskId);
	}
	
	override void FailTask(notnull SCR_BaseTask task)
	{
		super.FailTask(task);
		
		int taskId = m_mTasks.GetKeyByValue(task);
		if (taskId)
			m_mTasks.Remove(taskId);
	}
	
	protected SCR_BaseTask Create()
	{
		if (!GetTaskManager())
			return null;
		
		SCR_BaseTask baseTask = super.CreateTask();
		if (!baseTask)
			return null;
		
		int taskId = baseTask.GetTaskID();
		m_mTasks.Insert(taskId, baseTask);
		
		return baseTask;
	}
	
	protected void AssignTaskToPlayers(SCR_BaseTask task, notnull array<int> playerIds)
	{
		foreach (int playerID : playerIds)
		{
			SCR_BaseTaskExecutor assignee = SCR_BaseTaskExecutor.GetTaskExecutorByID(playerID);
			if (!assignee)
				continue;
	
			SCR_BaseTask assigneeTask = assignee.GetAssignedTask();
			if (assigneeTask)
			{
				UnassignTask(assigneeTask, assignee, SCR_EUnassignReason.ASSIGNEE_ABANDON);
			}
		
			AssignTask(task, assignee, false);
		}
	}
}
