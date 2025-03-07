//---------------------------------------------------------------------------------------
// Generated from class: ESCT_AIMessage_CallSupport
class ESCT_AISendGoalMessage_CallSupport : SCR_AISendMessageGenerated
{
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"SupportCallerEntity",
		"SupportTargetEntity",
		"SupportTargetPosition"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		ESCT_AIMessage_CallSupport msg = new ESCT_AIMessage_CallSupport();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		GetVariableIn("SupportCallerEntity", msg.m_CallerEntity);
		
		GetVariableIn("SupportTargetEntity", msg.m_TargetEntity);
		
		GetVariableIn("SupportTargetPosition", msg.m_vTargetPosition);
		
		if (msg.m_bIsWaypointRelated)
			msg.m_RelatedWaypoint = GetRelatedWaypoint(owner);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: ESCT_AIMessage_CallSupportTo
class ESCT_AISendGoalMessage_CallSupportTo : SCR_AISendMessageGenerated
{
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"SupportTargetEntity",
		"TargetPosition",
		"Delay"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		ESCT_AIMessage_CallSupportTo msg = new ESCT_AIMessage_CallSupportTo();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		GetVariableIn("SupportTargetEntity", msg.m_TargetEntity);
		
		GetVariableIn("TargetPosition", msg.m_vTargetPosition);
		
		GetVariableIn("Delay", msg.m_fDelay);
		
		if (msg.m_bIsWaypointRelated)
			msg.m_RelatedWaypoint = GetRelatedWaypoint(owner);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override bool VisibleInPalette() { return true; }
}

