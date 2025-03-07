modded class SCR_OpenVehicleStorageAction : SCR_InventoryAction
{
	protected ESCT_CarAlarmComponent m_CarAlarm;
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);
		
		m_CarAlarm = ESCT_ActionHelper.ObtainCarAlarmComponent(pOwnerEntity);
	}
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (m_CarAlarm && m_CarAlarm.GetAlarmState() != ESCT_CarAlarmState.Disabled)
			m_CarAlarm.EnableAlarm();
			
		super.PerformAction(pOwnerEntity, pUserEntity);
	}
}
