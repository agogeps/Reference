class ESCT_EpinephrineDamageEffect: SCR_InstantDamageEffect
{
	override void OnEffectAdded(SCR_ExtendedDamageManagerComponent dmgManager)			
	{
		super.OnEffectAdded(dmgManager);
	
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(dmgManager);
		if (!damageMgr)
			return;
		
		array<EHitZoneGroup> hzGroups =
		{ 
			ECharacterHitZoneGroup.HEAD, 
			ECharacterHitZoneGroup.LEFTLEG, ECharacterHitZoneGroup.LEFTARM,
			ECharacterHitZoneGroup.RIGHTARM, ECharacterHitZoneGroup.RIGHTLEG,
			ECharacterHitZoneGroup.LOWERTORSO, ECharacterHitZoneGroup.UPPERTORSO,
		};
		
		foreach (EHitZoneGroup group: hzGroups)
		{
			damageMgr.RemoveGroupBleeding(group);
		}			
	}

	//------------------------------------------------------------------------------------------------
	override EDamageType GetDefaultDamageType()
	{
		return EDamageType.HEALING;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void HandleConsequences(SCR_ExtendedDamageManagerComponent dmgManager, DamageEffectEvaluator evaluator)
	{
		super.HandleConsequences(dmgManager, evaluator);
		
		evaluator.HandleEffectConsequences(this, dmgManager);
	}
}
