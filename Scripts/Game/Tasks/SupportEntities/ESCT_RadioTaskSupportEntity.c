[EntityEditorProps(category: "Escapists/Tasks", description: "Task about sending distress signal via radio.", color: "0 0 255 255")]
class ESCT_RadioTaskSupportEntityClass : ESCT_BaseTaskSupportEntityClass
{
}

sealed class ESCT_RadioTaskSupportEntity : ESCT_BaseTaskSupportEntity
{
	private bool m_bIsCreated = false;
	
	override SCR_BaseTask CreateTask(IEntity entity, bool shouldAssign = true)
	{
		if (m_bIsCreated)
			return null;
		
		ESCT_RadioTask radioTask = ESCT_RadioTask.Cast(super.CreateTask(entity, shouldAssign));
		
		if (!radioTask)
		{
			ESCT_Logger.Error("Failed to create radio station search task!");
			return null;
		}
		
		SubscribeEarlyRadioStationSpawns(radioTask);
		
		m_bIsCreated = true;
		
		return radioTask;
	}
	
	private void SubscribeEarlyRadioStationSpawns(ESCT_RadioTask radioTask)
	{
		ESCT_LocationSystem locationSystem = ESCT_LocationSystem.GetSystem();	
		if (!locationSystem)
			return;
		
		array<ESCT_Location> locations = {};
		locationSystem.GetLocations(locations);
		
		ESCT_Location location;
		ESCT_RadioStation radioStation;
		
		for (int i = 0; i < locations.Count(); i++)
		{
			location = locations[i];
			
			if (location.GetLocationType() == ESCT_ELocationType.RadioStation && location.GetSpawnState() == ESCT_ESpawnState.Spawned)
			{
				radioStation = ESCT_RadioStation.Cast(location);
				if (!radioStation)
					continue;
				
				ESCT_Radio radio = radioStation.GetRadio();
				if (!radio)
					continue;
				
				radio.GetOnSignalSent().Insert(radioTask.OnSignalSent);
				radio.SetIsSubscribedToTask(true);
			}	
		}
	}
}
