class ESCT_ReceiveRadioMessageAction : ScriptedUserAction
{
	private ESCT_Radio m_Radio;

	private static const string AWAITING_RESPONSE_REASON = "#Escapists-FailReason-AwaitingResponse";
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);
		
		m_Radio = ESCT_Radio.Cast(pOwnerEntity);
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		m_Radio.ReceiveHqResponse();
	}

	override bool CanBeShownScript(IEntity user)
	{
		if (ESCT_GameStateManagerComponent.GetInstance().GetGameState() != ESCT_EGameState.RadioStationDefend)
			return false;
		
		if (m_Radio.GetRadioState() == ESCT_RadioState.Done)
			return false;

		return true;
	}

	override bool CanBePerformedScript(IEntity user)
	{
		if (ESCT_GameStateManagerComponent.GetInstance().GetGameState() != ESCT_EGameState.RadioStationDefend)
			return false;
		
		if (m_Radio.GetRadioState() != ESCT_RadioState.ReadyForResponse)
		{
			SetCannotPerformReason(AWAITING_RESPONSE_REASON);
			return false;
		}

		return true;
	}
}
