modded class LightUserAction : BaseLightUserAction
{
	protected ESCT_CarAlarmComponent m_CarAlarm;
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);
		
		m_CarAlarm = ESCT_ActionHelper.ObtainCarAlarmComponent(pOwnerEntity);
	}
	
	override bool CanBePerformedScript(IEntity user)
	{
		if (m_CarAlarm && (m_CarAlarm.GetAlarmState() == ESCT_CarAlarmState.Revealed || m_CarAlarm.GetAlarmState() == ESCT_CarAlarmState.Active))
		{
			SetCannotPerformReason("#Escapists-UserAction_SeatCarAlarm");
			return false;
		}
		
		return super.CanBePerformedScript(user);
	}
}
