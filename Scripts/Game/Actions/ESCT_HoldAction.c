//------------------------------------------------------------------------------------------------
class ESCT_HoldAction : ScriptedUserAction
{
	[Attribute("", UIWidgets.Auto)]
	protected ref SCR_AudioSourceConfiguration m_AudioSourceConfiguration;

	[Attribute("", UIWidgets.Coords)]
	private vector m_vSoundOffset;

	protected AudioHandle m_AudioHandle = AudioHandle.Invalid;
	protected IEntity m_ActionPerformer;

	//------------------------------------------------------------------------------------------------
	override void OnActionStart(IEntity pUserEntity)
	{
		super.OnActionStart(pUserEntity);
		
		//take "ownership" of hold action so other players can't do it too
		m_ActionPerformer = pUserEntity;
		
		SCR_SoundManagerEntity soundManagerEntity = GetGame().GetSoundManagerEntity();
		if (!soundManagerEntity)
			return;

		if (!m_AudioSourceConfiguration || !m_AudioSourceConfiguration.IsValid())
			return;

		SCR_AudioSource audioSource = soundManagerEntity.CreateAudioSource(GetOwner(), m_AudioSourceConfiguration);
		if (!audioSource)
			return;

		vector mat[4];
		mat[3] = GetOwner().CoordToParent(m_vSoundOffset);

		AudioSystem.TerminateSound(m_AudioHandle);
		soundManagerEntity.PlayAudioSource(audioSource, mat);
		m_AudioHandle = audioSource.m_AudioHandle;
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		super.PerformAction(pOwnerEntity, pUserEntity);
		
		m_ActionPerformer = null;
		
		//in case if action is faster than sound
		AudioSystem.TerminateSound(m_AudioHandle);
	}

	//------------------------------------------------------------------------------------------------
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		m_ActionPerformer = null;
		
		AudioSystem.TerminateSound(m_AudioHandle);

		SCR_SoundManagerEntity soundManagerEntity = GetGame().GetSoundManagerEntity();
		if (!soundManagerEntity)
			return;

		if (!m_AudioSourceConfiguration || m_AudioSourceConfiguration.m_sSoundProject == string.Empty)
			return;

		SCR_AudioSourceConfiguration audioSourceConfiguration = new SCR_AudioSourceConfiguration;
		audioSourceConfiguration.m_sSoundProject = m_AudioSourceConfiguration.m_sSoundProject;
		audioSourceConfiguration.m_eFlags = m_AudioSourceConfiguration.m_eFlags;
		audioSourceConfiguration.m_sSoundEventName = SCR_SoundEvent.SOUND_STOP_PLAYING;

		SCR_AudioSource audioSource = soundManagerEntity.CreateAudioSource(pOwnerEntity, audioSourceConfiguration);
		if (!audioSource)
			return;

		vector mat[4];
		mat[3] = pOwnerEntity.CoordToParent(m_vSoundOffset);

		soundManagerEntity.PlayAudioSource(audioSource, mat);
		m_AudioHandle = audioSource.m_AudioHandle;
	}
	
	override bool CanBePerformedScript(IEntity user)
	{
		if (!m_ActionPerformer)
			return true;
		
		if (m_ActionPerformer != user)
			return false;
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void ~ESCT_HoldAction()
	{
		AudioSystem.TerminateSound(m_AudioHandle);
	}
}
