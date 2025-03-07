class ESCT_ActionHelper
{
	static ESCT_CarAlarmComponent ObtainCarAlarmComponent(IEntity owner)
	{				
		ESCT_CarAlarmComponent carAlarm = ESCT_CarAlarmComponent.Cast(owner.FindComponent(ESCT_CarAlarmComponent));
		
		//if it's still not found it means that owner is not actual car but some of his part so we need to find root entity (car)
		if (!carAlarm)
		{
			Vehicle vehicle = Vehicle.Cast(owner.GetRootParent());
			if (vehicle)
				carAlarm = ESCT_CarAlarmComponent.Cast(vehicle.FindComponent(ESCT_CarAlarmComponent));
		}
		
		return carAlarm;
	}
}
