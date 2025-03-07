[BaseContainerProps()]
class ESCT_AIGoalReaction_CallSupportTo : SCR_AIGoalReaction
{	
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		ESCT_AIMessage_CallSupportTo msg = ESCT_AIMessage_CallSupportTo.Cast(message);
		if (!msg)
			return;

		ESCT_AICallSupportToBehavior behavior = new ESCT_AICallSupportToBehavior(utility, msg.m_RelatedGroupActivity, msg.m_vTargetPosition, msg.m_TargetEntity, msg.m_fDelay, msg.m_fPriorityLevel);
		utility.AddAction(behavior);
	}
}

[BaseContainerProps()]
class ESCT_AIGoalReaction_CallSupport : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = ESCT_AIMessage_CallSupport.Cast(message);
		if (!msg)
			return;

		SCR_AIActionBase currentAction = SCR_AIActionBase.Cast(utility.GetCurrentAction());
		if (!currentAction)
			return;

		float priorityLevelClamped = currentAction.GetRestrictedPriorityLevel();

		IEntity target = msg.m_TargetEntity;
		vector targetPosition = msg.m_vTargetPosition;
		IEntity caller = msg.m_CallerEntity;
		if (!caller)
			return;
		
		ESCT_SupportSystem supportSystem = ESCT_SupportSystem.GetSystem();
		if (!supportSystem)
			return;
		
		SCR_AIGroup group = ESCT_CharacterHelper.GetAIGroup(caller);
		if (group && supportSystem.HasSimilarActiveSupport(group, targetPosition))
			return;

		// Ignore message if we already have an activity to call support (so no multiple calls)
		array<ref AIActionBase> actions = {};
		utility.GetActions(actions);
		foreach (AIActionBase action : actions)
		{
			ESCT_AICallSupportActivity supportActivity = ESCT_AICallSupportActivity.Cast(action);
			if (supportActivity)
			{
				supportActivity.SetPriorityLevel(priorityLevelClamped);
				return;
			}
		}

		auto activity = new ESCT_AICallSupportActivity(utility, null, caller, target, targetPosition, priorityLevel: priorityLevelClamped);
		utility.AddAction(activity);
	}
}
