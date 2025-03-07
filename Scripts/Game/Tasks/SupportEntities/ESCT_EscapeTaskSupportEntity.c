[EntityEditorProps(category: "Escapists/Tasks/SupportEntities", description: "Support entity for escape the island task.", color: "0 0 255 255")]
class ESCT_EscapeTaskSupportEntityClass : ESCT_BaseTaskSupportEntityClass
{
}

sealed class ESCT_EscapeTaskSupportEntity : ESCT_BaseTaskSupportEntity
{
	ESCT_EscapeTask CreateEscapeTask(vector position, bool shouldAssign = true)
	{
		if (!GetTaskManager())
			return null;
		
		ESCT_EscapeTask task;
		if (position == vector.Zero)
			task = ESCT_EscapeTask.Cast(GetTaskManager().SpawnTask(GetResourceName()));
		else 
			task = ESCT_EscapeTask.Cast(super.CreateTask(position, shouldAssign));

		if (!task)
			return null;
		
		int taskId = task.GetTaskID();
		m_mTasks.Insert(taskId, task);
		
		if (shouldAssign)
		{			
			array<int> playerIds = {};
			ESCT_Player.GetPlayerIDs(playerIds);
			AssignTaskToPlayers(task, playerIds);
		}
		
		return task;
	}
}
