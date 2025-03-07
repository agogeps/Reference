[EntityEditorProps(category: "Escapists/Tasks", description: "Defend radio task.", color: "0 0 255 255")]
class ESCT_DefendRadioTaskClass : ESCT_BaseTaskClass
{
}

class ESCT_DefendRadioTask : ESCT_BaseTask
{
	void ESCT_DefendRadioTask(IEntitySource src, IEntity parent)
	{
		if (!GetGame().InPlayMode())
			return;

		SetEventMask(EntityEvent.INIT);
	}

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (!GetGame().InPlayMode())
			return;

		TriggerFeedback();
	}

	private void TriggerFeedback()
	{
		ESCT_EscapistsGameMode gameMode = ESCT_EscapistsGameMode.GetGameMode();
		if (!gameMode || RplSession.Mode() == RplMode.Dedicated)
			return;

		gameMode.GetEscapistsManager().GetOnDefendRadioStart().Invoke();
	}
}
