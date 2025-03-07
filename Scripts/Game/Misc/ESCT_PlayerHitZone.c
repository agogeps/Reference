class ESCT_PlayerHitZone : SCR_CharacterHitZone
{
	override void OnDamage(notnull BaseDamageContext damageContext)
	{
        if(GetHealthScaled() < 0.01)
        {	
            SetHealthScaled(0.01);
        }
		
		super.OnDamage(damageContext);
	}
}

class ESCT_PlayerHeadHitZone : SCR_CharacterHeadHitZone
{
	override void OnDamage(notnull BaseDamageContext damageContext)
	{
        if(GetHealthScaled() < 0.01)
        {	
            SetHealthScaled(0.01);
        }
		
		super.OnDamage(damageContext);
	}
}

class ESCT_PlayerBloodHitZone : SCR_CharacterBloodHitZone
{
	override void OnDamage(notnull BaseDamageContext damageContext)
	{
        if(GetHealthScaled() < 0.01)
        {	
            SetHealthScaled(0.01);
        }
		
		super.OnDamage(damageContext);
	}
	
	override void OnDamageStateChanged(ECharacterBloodState newState, ECharacterBloodState previousDamageState, bool isJIP)
	{
		super.OnDamageStateChanged(newState, previousDamageState, isJIP);
		
		UpdateLoseConditions();
	}
	
	protected void UpdateLoseConditions()
	{
		ESCT_PlayerDamageManagerComponent damageManager = ESCT_PlayerDamageManagerComponent.Cast(GetHitZoneContainer());
		if (damageManager)
			damageManager.UpdateLoseConditions();
	}
}

class ESCT_PlayerResilienceHitZone : SCR_CharacterResilienceHitZone
{
	override void OnDamage(notnull BaseDamageContext damageContext)
	{
        if(GetHealthScaled() < 0.01)
        {	
            SetHealthScaled(0.01);
        }
		
		super.OnDamage(damageContext);
	}
}
