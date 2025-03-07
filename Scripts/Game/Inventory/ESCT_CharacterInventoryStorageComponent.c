modded class SCR_CharacterInventoryStorageComponent : CharacterInventoryStorageComponent
{
protected static const ref array<ref array<int>>	DEFAULT_QUICK_SLOTS = {	{ EWeaponType.WT_RIFLE, EWeaponType.WT_SNIPERRIFLE, EWeaponType.WT_MACHINEGUN },
		{ EWeaponType.WT_RIFLE, EWeaponType.WT_ROCKETLAUNCHER, EWeaponType.WT_GRENADELAUNCHER, EWeaponType.WT_SNIPERRIFLE, EWeaponType.WT_MACHINEGUN },
		{ EWeaponType.WT_HANDGUN },
		{ EWeaponType.WT_FRAGGRENADE },
		{ EWeaponType.WT_SMOKEGRENADE },
		{ EGadgetType.CONSUMABLE + GADGET_OFFSET + SCR_EConsumableType.BANDAGE },
		{ EGadgetType.CONSUMABLE + GADGET_OFFSET + SCR_EConsumableType.TOURNIQUET, EGadgetType.CONSUMABLE + GADGET_OFFSET + SCR_EConsumableType.EPINEPHRINE },
		{ EGadgetType.CONSUMABLE + GADGET_OFFSET + SCR_EConsumableType.MORPHINE, EGadgetType.CONSUMABLE + GADGET_OFFSET + SCR_EConsumableType.SALINE },
		{ EGadgetType.RADIO + GADGET_OFFSET }, // Preferably as GadgetRadio action, then it can be saline
		{ EGadgetType.BUILDING_TOOL + GADGET_OFFSET } // To be replaced with engineering tool
	};
}

modded class SCR_ApplicableMedicalItemPredicate : InventorySearchPredicate
{
	//------------------------------------------------------------------------------------------------
	override protected bool IsMatch(BaseInventoryStorageComponent storage, IEntity item, array<GenericComponent> queriedComponents, array<BaseItemAttributeData> queriedAttributes)
	{
		SCR_EConsumableType type = SCR_ConsumableItemComponent.Cast(queriedComponents[0]).GetConsumableType();
		bool isMatch = (type == SCR_EConsumableType.BANDAGE)
			|| (type == SCR_EConsumableType.HEALTH)
			|| (type == SCR_EConsumableType.TOURNIQUET)
			|| (type == SCR_EConsumableType.SALINE)
			|| (type == SCR_EConsumableType.MORPHINE)
			|| (type == SCR_EConsumableType.EPINEPHRINE);

		if (!isMatch)
			return false;

		SCR_ConsumableItemComponent medicalItem = SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));
		SCR_ConsumableEffectHealthItems effect = SCR_ConsumableEffectHealthItems.Cast(medicalItem.GetConsumableEffect());
		if (!effect)
			return false;

		return effect.CanApplyEffectToHZ(characterEntity, characterEntity, hitZoneGroup);
	}
}
