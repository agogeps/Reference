class ESCT_BreakIntoVehicleAction : ESCT_CompartmentHoldAction
{
	private ESCT_CarAlarmComponent m_CarAlarm;
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);
		
		if (SCR_Global.IsEditMode()) return;
		
		m_CarAlarm = ESCT_ActionHelper.ObtainCarAlarmComponent(pOwnerEntity);
	}
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{		
		super.PerformAction(pOwnerEntity, pUserEntity);
		
		if (!m_CarAlarm || !m_CarAlarm.IsLocked())
			return;
		
		if (m_CarAlarm.IsLocked())
		{
			m_CarAlarm.EnableAlarm();
			m_CarAlarm.UnlockVehicle();
		}
	}

	override bool CanBePerformedScript(IEntity user)
	{
		if (!super.CanBePerformedScript(user))
			return false;
		
		if (!m_CarAlarm)
			return false;

		return true;
	}

	override bool CanBeShownScript(IEntity user)
	{
		return m_CarAlarm 
			&& m_CarAlarm.IsLocked() 
			&& (m_CarAlarm.GetAlarmState() == ESCT_CarAlarmState.Revealed || m_CarAlarm.GetAlarmState() == ESCT_CarAlarmState.Active);	
	}
	
	override bool HasLocalEffectOnlyScript() 
	{ 
		return false; 
	}
}
