modded class SCR_GetInUserAction : SCR_CompartmentUserAction
{
	protected ESCT_CarAlarmComponent m_CarAlarm;
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);
		
		m_CarAlarm = ESCT_ActionHelper.ObtainCarAlarmComponent(pOwnerEntity);
	}
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (m_CarAlarm)
		{
			if (m_CarAlarm.GetAlarmState() != ESCT_CarAlarmState.Disabled)
				m_CarAlarm.EnableAlarm();
			
			if (m_CarAlarm.IsLocked() && m_CarAlarm.GetAlarmState() != ESCT_CarAlarmState.Disabled)
				return;
		}
			
		super.PerformAction(pOwnerEntity, pUserEntity);
	}
	
	override bool CanBePerformedScript(IEntity user)
	{
		if (m_CarAlarm && m_CarAlarm.IsLocked() && (m_CarAlarm.GetAlarmState() == ESCT_CarAlarmState.Revealed || m_CarAlarm.GetAlarmState() == ESCT_CarAlarmState.Active))
		{
			SetCannotPerformReason("#Escapists-UserAction_SeatCarAlarm");
			return false;
		}
		
		return super.CanBePerformedScript(user);
	}
	
	//FIXME: required by car alarm (otherwise calls to server in car alarm components wouldn't be executed), may have some dangerous implications
	override bool HasLocalEffectOnlyScript()
	{
		return false;
	}
}
