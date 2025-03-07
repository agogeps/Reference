//------------------------------------------------------------------------------------------------
//! Various UI helper functions.
class ESCT_UI
{
	//------------------------------------------------------------------------------------------------
	//! Fills faction toolbox items with faction-related texts and images.
	//! \param[in] toolbox Faction toolbox.
	static void FillFactionToolbox(notnull ESCT_ToolboxComponent toolbox)
	{
		array<SCR_ButtonBaseComponent> items = {};
		toolbox.GetItems(items);
		for (int i=0; i < items.Count(); i++)
		{
			ESCT_Faction faction = ESCT_Faction.Cast(toolbox.GetItemData(i));
			if (!faction)
				continue;

			SCR_ButtonBaseComponent buttonBase = toolbox.GetItem(i);
			if (!buttonBase)
				continue;

			TextWidget buttonText = TextWidget.Cast((buttonBase.GetRootWidget()).FindAnyWidget("Text"));
			if (buttonText)
				buttonText.SetText(faction.GetFactionName());

			ImageWidget image = ImageWidget.Cast((buttonBase.GetRootWidget()).FindAnyWidget("Background"));
			if (image)
			{
				SCR_FactionUIInfo uiInfo = SCR_FactionUIInfo.Cast(faction.GetUIInfo());
				image.LoadImageTexture(0, uiInfo.GetIconPath());
			}
		}
	}
}
