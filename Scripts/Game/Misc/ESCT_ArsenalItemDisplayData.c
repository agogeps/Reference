[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EArsenalItemDisplayType, "m_eArsenalItemDisplayType")]
modded class SCR_ArsenalItemDisplayData
{
	[Attribute("1", desc: "Tracing algorhitm to correct clipping issues (used by EQUIPMENT_SLOT only).", uiwidget: UIWidgets.CheckBox, category: "Display settings")]
	protected bool m_bUseDiffusionCorrection;
	
	bool ShouldCorrectDiffusion()
	{
		return m_bUseDiffusionCorrection;	
	}
}

modded enum EArsenalItemDisplayType
{
	EQUIPMENT_SLOT
}
