class ESCT_FlyAwayMusic : ScriptedMusic
{
	MusicManager m_MusicManager;

	private static const string SOUND_FLY_AWAY = "SOUND_FLY_AWAY";
	
	protected void OnFlyAway()
	{	
		if (!m_MusicManager)
			return;
		
		m_MusicManager.Play(SOUND_FLY_AWAY);
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
		
		manager.GetOnHelicopterEntered().Insert(OnFlyAway);
	}
	
	override void OnDelete()
	{
		ESCT_EscapistsGameMode gameMode = ESCT_EscapistsGameMode.GetGameMode();
		if (!gameMode)
			return;
		
		ESCT_EscapistsManager manager = gameMode.GetEscapistsManager();
		if (!manager)
			return;
		
		manager.GetOnHelicopterEntered().Remove(OnFlyAway);
	}
}
