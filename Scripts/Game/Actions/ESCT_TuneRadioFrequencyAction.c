class ESCT_TuneRadioFrequencyAction : ESCT_HoldAction
{
	private ESCT_Radio m_Radio;
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);
		
		if (SCR_Global.IsEditMode()) return;
		
		m_Radio = ESCT_Radio.Cast(pOwnerEntity);
	}
	
	override void OnActionStart(IEntity pUserEntity)
	{
		super.OnActionStart(pUserEntity);
		
		if (!m_Radio || m_Radio.GetRadioState() != ESCT_RadioState.Idle)
			return;
		
		//player shouldn't hear "base" frequency, only static noise 
		m_Radio.NoFrequency();
	}
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!pOwnerEntity || !pUserEntity || !m_Radio)
			return;
		
		m_Radio.TuneFrequency();
		
		super.PerformAction(pOwnerEntity, pUserEntity);
	}

	override bool CanBePerformedScript(IEntity user)
	{
		if (!super.CanBePerformedScript(user))
			return false;
		
		if (m_Radio.GetRadioState() != ESCT_RadioState.Idle && m_Radio.GetRadioState() != ESCT_RadioState.NoFrequency)
			return false;

		return true;
	}

	override bool CanBeShownScript(IEntity user)
	{
		if (ESCT_GameStateManagerComponent.GetInstance().GetGameState() != ESCT_EGameState.RadioStationSearch)
			return false;
		
		if (m_Radio.GetRadioState() != ESCT_RadioState.Idle && m_Radio.GetRadioState() != ESCT_RadioState.NoFrequency)
			return false;
		
		return true;
	}
}
