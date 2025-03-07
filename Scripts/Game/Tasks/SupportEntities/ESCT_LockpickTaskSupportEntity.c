[EntityEditorProps(category: "Escapists/Tasks/SupportEntities", description: "Support entity for lockpick the door task.", color: "0 0 255 255")]
class ESCT_LockpickTaskSupportEntityClass : ESCT_BaseTaskSupportEntityClass
{
}

sealed class ESCT_LockpickTaskSupportEntity : ESCT_BaseTaskSupportEntity
{
	private ESCT_PrisonDoorComponent m_Door;
	
	override SCR_BaseTask CreateTask(IEntity entity, bool shouldAssign = true)
	{
		ESCT_LockpickTask task = ESCT_LockpickTask.Cast(super.CreateTask(entity, shouldAssign));
		
		m_Door = ESCT_PrisonDoorComponent.Cast(entity.FindComponent(ESCT_PrisonDoorComponent));
		if (m_Door)
		{
			m_Door.GetOnDoorLockpick().Insert(task.OnLockpickPrisonDoor);
		}
		
		return task;
	}
	
	override void FinishTask(notnull SCR_BaseTask task)
	{
		ESCT_LockpickTask lockpickTask = ESCT_LockpickTask.Cast(task);
		if (lockpickTask && m_Door)
			m_Door.GetOnDoorLockpick().Remove(lockpickTask.OnLockpickPrisonDoor);
		
		super.FinishTask(task);
		
		m_Door = null;
	}
}
