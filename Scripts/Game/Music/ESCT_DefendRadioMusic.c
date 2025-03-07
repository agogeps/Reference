class ESCT_DefendRadioMusic : ScriptedMusic
{
	MusicManager m_MusicManager;
	
	private static const string SOUND_DEFEND_RADIO = "SOUND_DEFEND_RADIO";
	
	protected void OnDefendRadio()
	{	
		if (!m_MusicManager)
			return;
		
		m_MusicManager.Play(SOUND_DEFEND_RADIO);
	}
	
	override void Init() 
	{
		ChimeraWorld world = GetGame().GetWorld();

		if (!world)
			return;
		
		m_MusicManager = world.GetMusicManager();
		
		if (!m_MusicManager)
			return;
		
		ESCT_EscapistsGameMode gameMode = ESCT_EscapistsGameMode.GetGameMode();
		if (!gameMode)
			return;
		
		ESCT_EscapistsManager manager = gameMode.GetEscapistsManager();
		if (!manager)
			return;
		
		manager.GetOnDefendRadioStart().Insert(OnDefendRadio);
	}
	
	override void OnDelete()
	{
		ESCT_EscapistsGameMode gameMode = ESCT_EscapistsGameMode.GetGameMode();
		if (!gameMode)
			return;
		
		ESCT_EscapistsManager manager = gameMode.GetEscapistsManager();
		if (!manager)
			return;
		
		manager.GetOnDefendRadioStart().Remove(OnDefendRadio);
	}
}
