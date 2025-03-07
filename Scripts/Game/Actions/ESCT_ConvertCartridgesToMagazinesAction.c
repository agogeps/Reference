class ESCT_ConvertCartridgesToMagazinesAction : ScriptedUserAction
{
	[Attribute("#Escapists-UserAction_ConvertCartridges", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sActionName;
	
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Magazine that will be used as end result of ammobox conversion.", params: "et", category: "Escapists")]
	protected ResourceName m_ResultMagazine;
	
	[Attribute(defvalue: "2", UIWidgets.Slider, category: "Escapists", params: "1 10 1")]
	protected int m_iMagazinesQuantityPerUse;
	
	[Attribute("", UIWidgets.Auto)]
	protected ref SCR_AudioSourceConfiguration m_AudioSourceConfiguration;

	[Attribute("", UIWidgets.Coords)]
	private vector m_vSoundOffset;

	protected AudioHandle m_AudioHandle = AudioHandle.Invalid;
	protected ESCT_CartridgeAmmoboxComponent m_CartridgeAmmoboxComponent;
	protected IEntity m_ActionPerformer;
	
	override void OnActionStart(IEntity pUserEntity)
	{
		super.OnActionStart(pUserEntity);
		
		//take "ownership" of hold action so other players can't do it too
		m_ActionPerformer = pUserEntity;
	}
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);
		
		m_CartridgeAmmoboxComponent = ESCT_CartridgeAmmoboxComponent.Cast(pOwnerEntity.FindComponent(ESCT_CartridgeAmmoboxComponent));
	}
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		super.PerformAction(pOwnerEntity, pUserEntity);

		if (!m_CartridgeAmmoboxComponent)
			return;
		
		PlaySound();
		
		m_CartridgeAmmoboxComponent.CreateMagazines(m_ResultMagazine, m_iMagazinesQuantityPerUse);
		
		m_ActionPerformer = null;
	}
	
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		m_ActionPerformer = null;
		
		StopSound(pOwnerEntity);
		
		super.OnActionCanceled(pOwnerEntity, pUserEntity);
	}
	
	override bool CanBePerformedScript(IEntity user)
	{
		if (m_CartridgeAmmoboxComponent && m_CartridgeAmmoboxComponent.GetRemainingUses() == 0)
		{
			SetCannotPerformReason("#Escapists-UserAction_ConvertCartridges_NoAmmo");
			return false;
		}
		
		if (!m_ActionPerformer)
			return true;
		
		if (m_ActionPerformer != user)
			return false;
		
		return super.CanBePerformedScript(user);
	}
	
	override bool GetActionNameScript(out string outName)
	{
		if (m_CartridgeAmmoboxComponent && m_CartridgeAmmoboxComponent.GetRemainingUses() > 0)
		{
			int remainingUses = m_CartridgeAmmoboxComponent.GetRemainingUses();
			int totalUses = m_CartridgeAmmoboxComponent.GetTotalUses();
		
			outName = m_sActionName + " (" + remainingUses.ToString() + "/" + totalUses.ToString() + ")";
		}
		else
		{
			outName = m_sActionName;
		}

		return true;
	}
	
	private void PlaySound()
	{
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
	
	private void StopSound(IEntity owner)
	{
		if (!owner)
			return;
		
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

		SCR_AudioSource audioSource = soundManagerEntity.CreateAudioSource(owner, audioSourceConfiguration);
		if (!audioSource)
			return;

		vector mat[4];
		mat[3] = owner.CoordToParent(m_vSoundOffset);

		soundManagerEntity.PlayAudioSource(audioSource, mat);
		m_AudioHandle = audioSource.m_AudioHandle;
	}
	
	void ~ESCT_ConvertCartridgesToMagazinesAction()
	{
		AudioSystem.TerminateSound(m_AudioHandle);
	}
}
