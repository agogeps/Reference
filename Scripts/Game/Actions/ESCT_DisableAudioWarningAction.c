class ESCT_DisableAudioWarningAction : ScriptedUserAction
{
	[Attribute("", UIWidgets.Auto)]
	protected ref SCR_AudioSourceConfiguration m_AudioSourceConfiguration;

	[Attribute("", UIWidgets.Coords)]
	private vector m_vSoundOffset;

	protected AudioHandle m_AudioHandle = AudioHandle.Invalid;

	private ESCT_HelicrashHelicopterComponent m_Helicopter;

	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);

		m_Helicopter = ESCT_HelicrashHelicopterComponent.Cast(pOwnerEntity.FindComponent(ESCT_HelicrashHelicopterComponent));
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!m_Helicopter)
			return;
		
		m_Helicopter.SetIsEnabled(false);
		m_Helicopter.DisableWarningSound();

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

	override bool CanBeShownScript(IEntity user)
	{
		if (!m_Helicopter.IsEnabled())
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	void ~ESCT_DisableAudioWarningAction()
	{
		AudioSystem.TerminateSound(m_AudioHandle);
	}
}
