modded enum EMessageType_Info
{
	ESCAPISTS_CALL_SUPPORT_FAILED
}

modded enum EMessageType_Goal
{
	ESCAPISTS_CALL_SUPPORT,
	ESCAPISTS_CALL_SUPPORT_TO
}

class ESCT_AIMessage_CallSupport : SCR_AIMessageGoal // MESSAGE_CLASS(GenerateSendGoalMessage, ESCT_AISendGoalMessage_CallSupport)
{
	IEntity m_CallerEntity; // VARIABLE(NodePort, SupportCallerEntity)
	IEntity m_TargetEntity; // VARIABLE(NodePort, SupportTargetEntity)
	vector m_vTargetPosition; // VARIABLE(NodePort, SupportTargetPosition)

	void ESCT_AIMessage_CallSupport()
	{
		m_MessageType = EMessageType_Goal.ESCAPISTS_CALL_SUPPORT;
	}
	
	static ESCT_AIMessage_CallSupport Create(IEntity callerEntity, IEntity targetEntity, vector targetPosition)
	{
		ESCT_AIMessage_CallSupport message = new ESCT_AIMessage_CallSupport();
		message.m_CallerEntity = callerEntity;
		message.m_TargetEntity = targetEntity;
		message.m_vTargetPosition = targetPosition;
		
		return message;
	}
}

class ESCT_AIMessage_CallSupportTo : SCR_AIMessageGoal // MESSAGE_CLASS(GenerateSendGoalMessage, ESCT_AISendGoalMessage_CallSupportTo)
{
	IEntity m_TargetEntity; // VARIABLE(NodePort, SupportTargetEntity)
	vector m_vTargetPosition; // VARIABLE(NodePort, TargetPosition)
	float m_fDelay; // VARIABLE(NodePort, Delay)
	
	void ESCT_AIMessage_CallSupportTo()
	{
		m_MessageType = EMessageType_Goal.ESCAPISTS_CALL_SUPPORT_TO;
	}

	static ESCT_AIMessage_CallSupportTo Create(IEntity targetEntity, vector targetPosition, float delay)
	{
		ESCT_AIMessage_CallSupportTo message = new ESCT_AIMessage_CallSupportTo();
		message.m_TargetEntity = targetEntity;
		message.m_vTargetPosition = targetPosition;
		message.m_fDelay = delay;

		return message;
	}
};

