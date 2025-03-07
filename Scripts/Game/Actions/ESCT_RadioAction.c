class ESCT_RadioAction : ScriptedUserAction
{
	private ESCT_Radio m_Radio;

	private static const string WRONG_FREQUENCY_REASON = "#Escapists-FailReason-WrongFrequency";
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);
		
		m_Radio = ESCT_Radio.Cast(pOwnerEntity);
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		m_Radio.SendDistressSignal();
	}

	override bool CanBeShownScript(IEntity user)
	{
		if (ESCT_GameStateManagerComponent.GetInstance().GetGameState() != ESCT_EGameState.RadioStationSearch)
			return false;
		
		if (m_Radio.GetRadioState() > ESCT_RadioState.FrequencySet)
			return false;
		
		return true;
	}

	override bool CanBePerformedScript(IEntity user)
	{
		if (ESCT_GameStateManagerComponent.GetInstance().GetGameState() != ESCT_EGameState.RadioStationSearch)
			return false;
		
		if (m_Radio.GetRadioState() != ESCT_RadioState.FrequencySet)
		{
			SetCannotPerformReason(WRONG_FREQUENCY_REASON);
			return false;
		}

		return true;
	}
}
