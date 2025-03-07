[EntityEditorProps(category: "Escapists/Tasks", description: "Use radio task.", color: "0 0 255 255")]
class ESCT_RadioTaskClass : ESCT_BaseTaskClass
{
}

class ESCT_RadioTask : ESCT_BaseTask
{
	void OnSignalSent(ESCT_Radio radio)
	{
		ESCT_RadioTaskSupportEntity supportEntity = ESCT_RadioTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(ESCT_RadioTaskSupportEntity));
		if (!supportEntity)
			return;

		supportEntity.FinishTask(this);
		
		if (!radio)
		{
			ESCT_Logger.Error("Failed to create defend task - missing radio reference!");
			return;
		}
		
		ESCT_TaskAssignerComponent taskAssigner = ESCT_TaskAssignerComponent.GetInstance();
		if (taskAssigner)
			taskAssigner.CreateDefendRadioTask(radio, radio.GetOrigin());
	}
}
