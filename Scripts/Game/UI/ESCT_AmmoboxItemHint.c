[BaseContainerProps(configRoot: true)]
class ESCT_AmmoboxItemHint : SCR_InventoryItemHintUIInfo
{
	override string GetItemHintName(InventoryItemComponent item)
	{
		if (!item)
			return super.GetItemHintName(item);

		ESCT_CartridgeAmmoboxComponent ammobox = ESCT_CartridgeAmmoboxComponent.Cast(item.GetOwner().FindComponent(ESCT_CartridgeAmmoboxComponent));
		if (!ammobox)
			return super.GetItemHintName(item);

		return WidgetManager.Translate(GetName(), ammobox.GetRemainingUses().ToString());
	}
}
