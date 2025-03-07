modded class SCR_RegenerationScreenEffect : SCR_BaseScreenEffect
{
	[Attribute(defvalue: "6", uiwidget: UIWidgets.EditBox, desc: "Duration of the regeneration effect in seconds")]
	protected float m_fEpinephrineRegenEffectDuration;

	[Attribute("0 0 1 1", UIWidgets.GraphDialog, desc: "Trajectory of the intensity of the regenerationEffect")]
	protected ref Curve m_EpinephrineCurve;

	private bool m_bIsEpinephrine = false;

	//------------------------------------------------------------------------------------------------
	protected override void RegenerationEffect(float timeSlice)
	{
		m_fRegenEffectTimeRemaining -= timeSlice;

		float chromAberProgress;
		Curve curve;
		
		if (m_bIsEpinephrine)
		{
			chromAberProgress = Math.InverseLerp(m_fEpinephrineRegenEffectDuration, 0, m_fRegenEffectTimeRemaining);
			curve = m_EpinephrineCurve;
		}
		else
		{
			chromAberProgress = Math.InverseLerp(m_fRegenEffectDuration, 0, m_fRegenEffectTimeRemaining);
			curve = m_Curve;
		}

		vector chromAberPowerScaled = Math3D.Curve(ECurveType.CatmullRom, chromAberProgress, curve);

		// The maximum value of the EMAT is 0.05. The chromatic aberiation cannot go over this value
		s_fChromAberPower = Math.Lerp(0, 0.05, chromAberPowerScaled[1]);

		if (m_fRegenEffectTimeRemaining <= 0)
		{
			m_bRegenerationEffect = false;
			m_fRegenEffectTimeRemaining = 0;
			s_fChromAberPower = 0;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnDamageEffectAdded(notnull SCR_DamageEffect dmgEffect)
	{
		if (dmgEffect.GetDamageType() != EDamageType.HEALING)
			return;
		
		if (dmgEffect.Type() == ESCT_EpinephrineDamageEffect)
		{
			m_fRegenEffectTimeRemaining = m_fEpinephrineRegenEffectDuration;
			m_bIsEpinephrine = true;
		}	
		else
		{
			m_fRegenEffectTimeRemaining = m_fRegenEffectDuration;
			m_bIsEpinephrine = false;
		}
		
		m_bRegenerationEffect = true;
	}

//FIXME: enable if  ESCT_EpinephrineScreenEffect starts working and remove other overrides from this class	
//	//------------------------------------------------------------------------------------------------
//	protected override void OnDamageEffectAdded(notnull SCR_DamageEffect dmgEffect)
//	{
//		if (dmgEffect.Type() == ESCT_EpinephrineDamageEffect)
//			return;
//
//		super.OnDamageEffectAdded(dmgEffect);
//	}
}


//FIXME: as for 11.08.2024 post-effect is not working despite it being full copy
//! Separate screen effect for epinephrine injections.
//class ESCT_EpinephrineScreenEffect : SCR_BaseScreenEffect
//{
//	// PP constants
//	// Variables connected to a material, need to be static
//	static const int CHROM_ABER_PRIORITY								= 9;
//
//	[Attribute(defvalue: "6", uiwidget: UIWidgets.EditBox, desc: "Duration of the regeneration effect in seconds")]
//	protected float m_fRegenEffectDuration;
//
//	[Attribute("0 0 1 1", UIWidgets.GraphDialog, desc: "Trajectory of the intensity of the regenerationEffect")]
//	protected ref Curve m_Curve;
//
//	//Character
//	protected ChimeraCharacter m_pCharacterEntity;
//	protected SCR_CharacterDamageManagerComponent m_pDamageManager;
//	protected const string CHROMATIC_ABERIATION_EMAT = "{A78A424C3179C706}UI/Materials/ScreenEffects_ChromAberrPP.emat";
//	protected bool m_bRegenerationEffect;
//	protected float m_fRegenEffectTimeRemaining;
//
//	// Variables connected to a material, need to be static
//	private static bool s_bEnabled;
//	private static float s_fChromAberPower;
//
//	//------------------------------------------------------------------------------------------------
//	override void DisplayStartDraw(IEntity owner)
//	{
//		m_pCharacterEntity = ChimeraCharacter.Cast(owner);
//	}
//
//	//------------------------------------------------------------------------------------------------
//	override void DisplayControlledEntityChanged(IEntity from, IEntity to)
//	{
//		ClearEffects();
//
//		m_pCharacterEntity = ChimeraCharacter.Cast(to);
//		if (!m_pCharacterEntity)
//			return;
//
//		m_pDamageManager = SCR_CharacterDamageManagerComponent.Cast(m_pCharacterEntity.GetDamageManager());
//		if (!m_pDamageManager)
//			return;
//
//		m_pDamageManager.GetOnDamageEffectAdded().Insert(OnDamageEffectAdded);
//
//		m_pCharacterEntity.GetWorld().SetCameraPostProcessEffect(m_pCharacterEntity.GetWorld().GetCurrentCameraId(), CHROM_ABER_PRIORITY, PostProcessEffectType.ChromAber, CHROMATIC_ABERIATION_EMAT);
//	}
//
//	//------------------------------------------------------------------------------------------------
//	override protected void DisplayOnSuspended()
//	{
//		s_bEnabled = false;
//	}
//
//	//------------------------------------------------------------------------------------------------
//	override protected void DisplayOnResumed()
//	{
//		s_bEnabled = true;
//	}
//
//	//------------------------------------------------------------------------------------------------
//	override void UpdateEffect(float timeSlice)
//	{
//		if (m_bRegenerationEffect)
//			RegenerationEffect(timeSlice);
//	}
//
//	//------------------------------------------------------------------------------------------------
//	protected void RegenerationEffect(float timeSlice)
//	{
//		m_fRegenEffectTimeRemaining -= timeSlice;
//
//		float chromAberProgress = Math.InverseLerp(m_fRegenEffectDuration, 0, m_fRegenEffectTimeRemaining);
//
//		vector chromAberPowerScaled = Math3D.Curve(ECurveType.CatmullRom, chromAberProgress, m_Curve);
//
//		// The maximum value of the EMAT is 0.05. The chromatic aberiation cannot go over this value
//		s_fChromAberPower = Math.Lerp(0, 0.05, chromAberPowerScaled[1]);
//
//		if (m_fRegenEffectTimeRemaining <= 0)
//		{
//			m_bRegenerationEffect = false;
//			m_fRegenEffectTimeRemaining = 0;
//			s_fChromAberPower = 0;
//		}
//	}
//
//	//------------------------------------------------------------------------------------------------
//	protected void OnDamageEffectAdded(notnull SCR_DamageEffect dmgEffect)
//	{
//		if (dmgEffect.Type() != ESCT_EpinephrineDamageEffect && dmgEffect.GetDamageType() != EDamageType.HEALING)
//			return;
//
//		m_fRegenEffectTimeRemaining = m_fRegenEffectDuration;
//		m_bRegenerationEffect = true;
//	}
//
//	//------------------------------------------------------------------------------------------------
//	protected override void ClearEffects()
//	{
//		m_fRegenEffectTimeRemaining = 0;
//		s_fChromAberPower = 0;
//
//		if (m_pDamageManager)
//			m_pDamageManager.GetOnDamageEffectAdded().Remove(OnDamageEffectAdded);
//	}
//}
