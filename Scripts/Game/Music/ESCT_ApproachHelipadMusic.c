class ESCT_ApproachHelipadMusic : ScriptedMusic
{
	MusicManager m_MusicManager;

	protected void OnApproachHelipad()
	{	
		if (!m_MusicManager)
			return;
		
		m_MusicManager.Play(SCR_SoundEvent.SOUND_ONENTERINGENEMYBASE);
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
		
		manager.GetOnApproachHelipad().Insert(OnApproachHelipad);
	}
	
	override void OnDelete()
	{
		ESCT_EscapistsGameMode gameMode = ESCT_EscapistsGameMode.GetGameMode();
		if (!gameMode)
			return;
		
		ESCT_EscapistsManager manager = gameMode.GetEscapistsManager();
		if (!manager)
			return;
		
		manager.GetOnApproachHelipad().Remove(OnApproachHelipad);
	}
}
