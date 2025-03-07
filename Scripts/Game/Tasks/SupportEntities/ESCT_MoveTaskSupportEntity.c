[EntityEditorProps(category: "Escapists/Tasks/SupportEntities", description: "Support entity for reach extraction point task.", color: "0 0 255 255")]
class ESCT_MoveTaskSupportEntityClass : ESCT_BaseTaskSupportEntityClass
{
}

sealed class ESCT_MoveTaskSupportEntity : ESCT_BaseTaskSupportEntity
{
	override SCR_BaseTask CreateTask(vector position, bool shouldAssign = true)
	{
		return ESCT_MoveTask.Cast(super.CreateTask(position, shouldAssign));
	}
	
	override void FinishTask(notnull SCR_BaseTask task)
	{
		super.FinishTask(task);
		
		ESCT_TaskAssignerComponent taskAssigner = ESCT_TaskAssignerComponent.GetInstance();
		if (!taskAssigner)
			return;
		
		taskAssigner.CreateEscapeTask();
	}
}
