[EntityEditorProps(category: "Escapists/Tasks", description: "Move task.", color: "0 0 255 255")]
class ESCT_MoveTaskClass: ESCT_BaseTaskClass
{
}

class ESCT_MoveTask : ESCT_BaseTask
{
	[Attribute("5")]
	protected float m_fMaxDistance;
	
	[Attribute("150")]
	protected float m_fFeedbackDistance;
	
	void PeriodicalCheck()
	{		
		if (!GetTaskManager())
			return;
		
		ESCT_MoveTaskSupportEntity supportEntity = ESCT_MoveTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(ESCT_MoveTaskSupportEntity));
		if (!supportEntity)
			return;
		
		array<IEntity> players = {};
		ESCT_Player.GetAlivePlayers(players, true);
		
		foreach (IEntity player : players)
		{
			float distance = vector.Distance(player.GetOrigin(), GetOrigin());
			if (distance < m_fMaxDistance)
			{
				supportEntity.FinishTask(this);
				SCR_BaseTaskManager.s_OnPeriodicalCheck2Second.Remove(PeriodicalCheck);
				return;
			}
		}
	}
	
	void FeedbackCheck()
	{	
		ESCT_EscapistsGameMode gameMode = ESCT_EscapistsGameMode.GetGameMode();
		if (!gameMode)
			return;
		
		array<IEntity> players = {};
		ESCT_Player.GetAlivePlayers(players, true);
		
		foreach (IEntity player : players)
		{
			float distance = vector.Distance(player.GetOrigin(), GetOrigin());
			if (distance < m_fFeedbackDistance)
			{
				gameMode.GetEscapistsManager().GetOnApproachHelipad().Invoke();
				SCR_BaseTaskManager.s_OnPeriodicalCheck5Second.Remove(FeedbackCheck);
				return;
			}
		}
	}
	
	void ESCT_MoveTask(IEntitySource src, IEntity parent)
	{
		if (SCR_Global.IsEditMode(this))
			return;
		
		SCR_BaseTaskManager.s_OnPeriodicalCheck2Second.Insert(PeriodicalCheck);
		SCR_BaseTaskManager.s_OnPeriodicalCheck5Second.Insert(FeedbackCheck);
	}
	
	void ~ESCT_MoveTask()
	{
		if (SCR_Global.IsEditMode(this) || !GetGame().GetGameMode())
			return;
		
		SCR_BaseTaskManager.s_OnPeriodicalCheck2Second.Remove(PeriodicalCheck);
		SCR_BaseTaskManager.s_OnPeriodicalCheck5Second.Remove(FeedbackCheck);
	}
}
