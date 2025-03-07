class ESCT_AIActivityCallSupportFeature : SCR_AIActivityFeatureBase
{
	bool Execute(
		notnull SCR_AIGroupUtilityComponent groupUtility,
		IEntity supportCallerEntity,
		IEntity targetEntity,
		vector targetPosition,
		AIAgent supportCallerAgent,
		SCR_AIActivityBase contextActivity = null
	) {
		if (!supportCallerEntity || targetPosition == vector.Zero || !supportCallerAgent)
			return false;

		RandomGenerator rnd = ESCT_EscapistsGameMode.GetGameMode().GetRandomGenerator();
		vector impreciseTargetPosition = rnd.GenerateRandomPointInRadius(5, 10, targetPosition, false);
		impreciseTargetPosition[1] = SCR_TerrainHelper.GetTerrainY(impreciseTargetPosition);

		AICommunicationComponent comms = groupUtility.m_Owner.GetCommunicationComponent();
		if (!comms)
			return false;

		float delay = Math.RandomFloat(0, 5);
		SendMessage(comms, contextActivity, targetPosition, targetEntity, supportCallerAgent, delay);

		return true;
	}

	bool ExecuteForActivity(SCR_AIActivityBase activity)
	{
		SCR_AIGroupUtilityComponent groupUtility = activity.m_Utility;
		if (!groupUtility)
			return false;

		return Execute(
			groupUtility,
			GetSupportCaller(activity),
			GetTarget(activity),
			GetActivityTargetPosition(activity),
			GetSupportCallerAgent(activity),
			activity
		);
	}

	protected void SendMessage(AICommunicationComponent comms, SCR_AIActivityBase activity, vector targetPosition, IEntity targetEntity, AIAgent agent, float delay)
	{
		ESCT_AIMessage_CallSupportTo msg = ESCT_AIMessage_CallSupportTo.Create(targetEntity, targetPosition, delay);
		msg.m_RelatedGroupActivity = activity;
		msg.m_fPriorityLevel = ESCT_AICallSupportActivity.PRIORITY_ACTIVITY_CALL_SUPPORT;
		msg.SetReceiver(agent);
		comms.RequestBroadcast(msg, agent);
	}

	protected vector GetActivityTargetPosition(SCR_AIActivityBase activity)
	{
		ESCT_AICallSupportActivity callSupportActivity = ESCT_AICallSupportActivity.Cast(activity);

		if (callSupportActivity.m_vTargetPosition.m_Value)
			return callSupportActivity.m_vTargetPosition.m_Value;

		return vector.Zero;
	}

	protected IEntity GetSupportCaller(SCR_AIActivityBase activity)
	{
		ESCT_AICallSupportActivity callSupportActivity = ESCT_AICallSupportActivity.Cast(activity);

		IEntity entity = callSupportActivity.m_CallerEntity.m_Value;
		if (!entity)
			return null;

		return entity;
	}

	protected IEntity GetTarget(SCR_AIActivityBase activity)
	{
		ESCT_AICallSupportActivity callSupportActivity = ESCT_AICallSupportActivity.Cast(activity);

		IEntity entity = callSupportActivity.m_TargetEntity.m_Value;
		if (!entity)
			return null;

		return entity;
	}

	protected AIAgent GetSupportCallerAgent(SCR_AIActivityBase activity)
	{
		ESCT_AICallSupportActivity callSupportActivity = ESCT_AICallSupportActivity.Cast(activity);

		ChimeraCharacter character = ChimeraCharacter.Cast(callSupportActivity.m_CallerEntity.m_Value);
		if (!character)
			return null;

		CharacterControllerComponent charCtrl = character.GetCharacterController();
		if (!charCtrl)
			return null;

		AIControlComponent aiCtrl = charCtrl.GetAIControlComponent();
		if (!aiCtrl)
			return null;

		return aiCtrl.GetAIAgent();
	}
}
