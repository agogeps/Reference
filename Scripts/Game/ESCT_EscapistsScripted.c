//------------------------------------------------------------------------------------------------
//! Main game instance.
//! Created when the game starts (not when a world starts!) and persists until the game is closed.
//! Thanks veteran29 for showing this workaround.
modded class ArmaReforgerScripted : ChimeraGame
{
	protected static ref ESCT_MissionHeader m_Escapists_MissionHeader;

	//------------------------------------------------------------------------------------------------
	//! Load settings when starting the game for the first time
	override bool OnGameStart()
	{
		if ((!Replication.IsRunning() || Replication.IsServer()) && !m_Escapists_MissionHeader)
		{
			//store mission header between the runs
			m_Escapists_MissionHeader = ESCT_MissionHeader.Cast(GetMissionHeader());
		}
		
		return super.OnGameStart();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns stored mission
	ESCT_MissionHeader GetEscapistsMissionHeader()
	{
		if (m_Escapists_MissionHeader)
			return m_Escapists_MissionHeader;
		
		return ESCT_MissionHeader.Cast(GetMissionHeader());
	}
	
	void ~ArmaReforgerScripted()
	{
		if (!m_Escapists_MissionHeader)
			return;
		
		//game will crash on exit unless this object would be explicitly removed
		delete m_Escapists_MissionHeader;
	}
}
