[EntityEditorProps(category: "Escapists/Tasks", description: "Lockpick task.", color: "0 0 255 255")]
class ESCT_LockpickTaskClass : ESCT_BaseTaskClass
{
}

class ESCT_LockpickTask : ESCT_BaseTask
{
	void OnLockpickPrisonDoor()
	{
		ESCT_LockpickTaskSupportEntity supportEntity = ESCT_LockpickTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(ESCT_LockpickTaskSupportEntity));
		if (!supportEntity)
			return;

		supportEntity.FinishTask(this);
	}
}
