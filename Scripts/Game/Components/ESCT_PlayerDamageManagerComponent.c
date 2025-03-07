[EntityEditorProps("Escapists/Components", description: "Component for player characters to handle damage.", color: "0 0 255 255")]
class ESCT_PlayerDamageManagerComponentClass : SCR_CharacterDamageManagerComponentClass
{
}

class ESCT_PlayerDamageManagerComponent : SCR_CharacterDamageManagerComponent
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Can character lose consciousness if the damage is too high?", category: "Escapists")]
	protected bool m_bCauseUnconscious;

	[Attribute("12", uiwidget: UIWidgets.Slider, desc: "Minimal unconscious time before character wakes up.", params: "1 1000 1", category: "Escapists")]
	protected int m_iUnconsciousMinTime;

	[Attribute("24", uiwidget: UIWidgets.Slider, desc: "Maximum unconscious time before character wakes up.", params: "1 1000 1", category: "Escapists")]
	protected int m_iUnconsciousMaxTime;

	[Attribute("30", uiwidget: UIWidgets.Slider, desc: "Damage threshold to lose conscousness.", params: "1 100 0.5", category: "Escapists")]
	protected float m_fDamageThreshold;

	protected bool m_bIsEligibleForLose = false;

	bool IsEligibleForLose()
	{
		return m_bIsEligibleForLose;
	}
	
	void UpdateLoseConditions()
	{
		Rpc(UpdateLoseConditions_SA);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void UpdateLoseConditions_SA()
	{
		HitZone bloodHZ = GetBloodHitZone();
		if (!bloodHZ)
			return;

		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return;

		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return;

		ECharacterBloodState bloodState = bloodHZ.GetDamageState();
		bool isBloodCritical = bloodHZ.GetDamageStateThreshold(bloodState) <= bloodHZ.GetDamageStateThreshold(ECharacterBloodState.UNCONSCIOUS);

		if (isBloodCritical && controller.IsUnconscious())
		{
			m_bIsEligibleForLose = !HasEpinephrine(character);
		}
		else
		{
			m_bIsEligibleForLose = false;
		}
	}

	protected override void OnDamage(notnull BaseDamageContext damageContext)
	{
		//modify damage so player character wouldn't die from wounds (hitzone reaches zero => death) and pass OnDamage on
		ApplyGeneralDamage();
		ApplyFallDamage(damageContext.damageType, damageContext.damageValue);

		super.OnDamage(damageContext);
	}

	private void ApplyGeneralDamage()
	{
		if (GetDefaultHitZone().GetHealthScaled() >= 0.01)
			return;

		GetDefaultHitZone().SetHealthScaled(0.01);
	}

	private void ApplyFallDamage(EDamageType damageType, float damage)
	{
		if (damageType != EDamageType.COLLISION)
			return;

		array<HitZone> hitZones = {};
		GetAllHitZones(hitZones);

		if (hitZones.IsEmpty())
			return;

		for (int i = 0; i < hitZones.Count(); i++)
		{
			HitZone hitZone = hitZones[i];
			float damageToApply = 0;

			//it's all legs
			switch (hitZone.GetName())
			{
				case "RFoot":
				case "LFoot":
				case "RCalf":
				case "LCalf":
				case "RThigh":
				case "LThigh":
					damageToApply = damage * 0.1;
					break;
			}

			if (damageToApply == 0)
				continue;

			float healthToApply = hitZone.GetHealth() - damageToApply;
			hitZone.SetHealthScaled(Math.Max(healthToApply, 0.01));
		}

		ApplyUnconsciousState(damage);
	}

	private void ApplyUnconsciousState(float damage)
	{
		if (!m_bCauseUnconscious || damage < m_fDamageThreshold)
			return;

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return;

		CharacterControllerComponent charController = character.GetCharacterController();
		if (!charController)
			return;

		charController.SetUnconscious(true);

		int wakeupTime = Math.RandomInt(m_iUnconsciousMinTime, m_iUnconsciousMaxTime);
		GetGame().GetCallqueue().CallLater(charController.SetUnconscious, wakeupTime * 1000, false, false);
	}
	
	private bool HasEpinephrine(ChimeraCharacter character)
	{
		if (!ESCT_EscapistsConfigComponent.GetInstance().IsWithstandEnabled())
			return false;
			
		bool hasEpi = false;
		
		SCR_InventoryStorageManagerComponent inventoryStorage = 
			SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (inventoryStorage)
		{
			array<IEntity> items = {};
			inventoryStorage.GetItems(items);
	
			IEntity item = null;
			for (int i = 0; i < items.Count(); i++)
			{
				item = items[i];
				if (!item)
					continue;
	
				InventoryItemComponent inventoryItem = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
				if (!inventoryItem)
					continue;
	
				if (inventoryItem.GetAttributes().GetCommonType() == ECommonItemType.EPINEPHRINE)
				{
					hasEpi = true; 
					break;
				}
			}
		}
		
		return hasEpi;
	}
}
