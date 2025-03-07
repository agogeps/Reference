modded enum SCR_EConsumableType
{
	EPINEPHRINE
}

modded enum SCR_EConsumableFailReason
{
	CONSCIOUS
}

modded enum ECommonItemType
{
	EPINEPHRINE = 102
}

//! Epinephrine effect
[BaseContainerProps()]
class ESCT_ConsumableEpinephrine : SCR_ConsumableEffectHealthItems
{
	override void ApplyEffect(notnull IEntity target, notnull IEntity user, IEntity item, ItemUseParameters animParams)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(target);
		if (!char)
			return;

		ApplyHeal(char);

		super.ApplyEffect(target, user, item, animParams);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanApplyEffect(notnull IEntity target, notnull IEntity user, out SCR_EConsumableFailReason failReason)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(target);
		if (!char)
			return false;

		CharacterControllerComponent charController = char.GetCharacterController();
		if (!charController)
			return false;

		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!damageMgr)
			return false;

		array<ref PersistentDamageEffect> effects = damageMgr.GetAllPersistentEffectsOfType(ESCT_EpinephrineDamageEffect);
		if (!effects.IsEmpty())
		{
			failReason = SCR_EConsumableFailReason.ALREADY_APPLIED;
			return false;
		}

		//if character is unconscious (it's primary use to wake up the people)
		ECharacterLifeState lifeState = charController.GetLifeState();
		if (lifeState == ECharacterLifeState.INCAPACITATED)
			return true;

		//if character has bleedings (epinephrine can *casually* close bleedings)
		array<EDamageType> damageTypes = {};
		SCR_Enum.GetEnumValues(EDamageType, damageTypes);
		foreach (EDamageType type : damageTypes)
		{
			// Prevent removing of healing DOTs so morphine and saline effects aren't stopped by healing action
			if (type == EDamageType.REGENERATION || type == EDamageType.HEALING || type == EDamageType.INCENDIARY)
				continue;

			if (damageMgr.IsDamagedOverTime(type))
				return true;
		}

		SCR_CharacterBloodHitZone blood = damageMgr.GetBloodHitZone();
		if (blood)
		{
			ECharacterBloodState bloodState = blood.GetDamageState();
			if (blood.GetDamageStateThreshold(bloodState) <= blood.GetDamageStateThreshold(ECharacterBloodState.UNCONSCIOUS))
				return true;
		}

		SCR_CharacterResilienceHitZone resilience = damageMgr.GetResilienceHitZone();
		if (resilience)
		{
			ECharacterResilienceState resilienceState = resilience.GetDamageState();
			if (resilience.GetDamageStateThreshold(resilienceState) <= resilience.GetDamageStateThreshold(ECharacterResilienceState.WEAKENED))
				return true;
		}

		// Fix other hit zones a bit
		array<HitZone> hitZones = {};
		damageMgr.GetAllHitZones(hitZones);
		foreach (HitZone hitZone : hitZones)
		{
			if (hitZone.Type() == SCR_CharacterResilienceHitZone || hitZone.Type() == SCR_CharacterBloodHitZone)
				continue;

			if (hitZone && hitZone.GetDamageState() != EDamageState.UNDAMAGED)
				return true;
		}

		failReason = SCR_EConsumableFailReason.UNDAMAGED;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanApplyEffectToHZ(notnull IEntity target, notnull IEntity user, ECharacterHitZoneGroup group, out SCR_EConsumableFailReason failReason = SCR_EConsumableFailReason.NONE)
	{
		return CanApplyEffect(target, user, failReason);
	}

	//------------------------------------------------------------------------------------------------
	EDamageType GetDefaultDamageType()
	{
		return EDamageType.HEALING;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	void ESCT_ConsumableEpinephrine()
	{
		m_eConsumableType = SCR_EConsumableType.EPINEPHRINE;
	}

	private void ApplyHeal(ChimeraCharacter char)
	{
		if (!char)
			return;

		CharacterControllerComponent charController = char.GetCharacterController();
		if (!charController)
			return;

		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!damageMgr)
			return;

		// Heal remaining damage over time
		array<EDamageType> damageTypes = {};
		SCR_Enum.GetEnumValues(EDamageType, damageTypes);
		foreach (EDamageType type : damageTypes)
		{
			// Prevent removing of healing DOTs so morphine and saline effects aren't stopped by healing action
			if (type == EDamageType.REGENERATION || type == EDamageType.HEALING)
				continue;

			if (damageMgr.IsDamagedOverTime(type))
				damageMgr.RemoveDamageOverTime(type);
		}

		SCR_CharacterBloodHitZone blood = damageMgr.GetBloodHitZone();
		if (blood)
		{
			ECharacterBloodState bloodState = blood.GetDamageState();
			if (blood.GetDamageStateThreshold(bloodState) <= blood.GetDamageStateThreshold(ECharacterBloodState.UNCONSCIOUS))
			{
				float healValue = Math.Min(blood.GetHealthScaled() + 0.25, 1.0);
				blood.SetHealthScaled(healValue);
			}
		}

		SCR_CharacterResilienceHitZone resilience = damageMgr.GetResilienceHitZone();
		if (resilience)
		{
			ECharacterResilienceState resilienceState = resilience.GetDamageState();
			if (resilience.GetDamageStateThreshold(resilienceState) <= resilience.GetDamageStateThreshold(ECharacterResilienceState.WEAKENED))
			{
				float healValue = Math.Min(resilience.GetHealthScaled() + 0.3, 1.0);
				resilience.SetHealthScaled(healValue);
			}
		}

		// Fix other hit zones a bit
		array<HitZone> hitZones = {};
		damageMgr.GetAllHitZones(hitZones);
		foreach (HitZone hitZone : hitZones)
		{
			if (hitZone.Type() == SCR_CharacterResilienceHitZone || hitZone.Type() == SCR_CharacterBloodHitZone)
				continue;

			if (hitZone && hitZone.GetDamageState() != EDamageState.UNDAMAGED)
			{
				float health = Math.Min(hitZone.GetHealthScaled() + 0.15, 1.0);
				hitZone.SetHealthScaled(health);
			}
		}

		if (charController.IsUnconscious())
			charController.SetUnconscious(false);
	}
}
