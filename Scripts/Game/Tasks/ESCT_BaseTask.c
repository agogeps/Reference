[EntityEditorProps(category: "Escapists/Tasks", description: "Move task.", color: "0 0 255 255")]
class ESCT_BaseTaskClass : SCR_BaseTaskClass
{
}

class ESCT_BaseTask : SCR_BaseTask
{
	override void OnStateChanged(SCR_TaskState previousState, SCR_TaskState newState)
	{
		super.OnStateChanged(previousState, newState);

		ESCT_PopupMessageHandlerComponent popupMessageHandler = ESCT_PopupMessageHandlerComponent.GetInstance();
		if (!popupMessageHandler)
			return;

		switch (newState)
		{
			case SCR_TaskState.FINISHED: 
				popupMessageHandler.PopUpMessage(GetTitle(), "#AR-Tasks_StatusFinished-UC", sound: SCR_SoundEvent.TASK_SUCCEED, m_TargetFaction.GetFactionKey());
				break;
			case SCR_TaskState.CANCELLED: 
				popupMessageHandler.PopUpMessage(GetTitle(), "#AR-Tasks_StatusFailed-UC", sound: SCR_SoundEvent.TASK_FAILED, m_TargetFaction.GetFactionKey());
				break;
		}		
	}
	
	protected override void ShowPopUpNotification(string subtitle)
	{
		SCR_PopUpNotification.GetInstance().PopupMsg(GetTitleText(), sound: SCR_SoundEvent.TASK_CREATED, text2: subtitle);
	}
}
