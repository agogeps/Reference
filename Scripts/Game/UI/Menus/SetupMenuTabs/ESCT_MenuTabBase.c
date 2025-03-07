class ESCT_MenuTabBase
{
	protected string m_sTooltipId;
	protected Widget m_wRoot;
	protected map<string, string> m_TooltipMap;
	protected ESCT_SetupConfig m_Config;
	
	void ESCT_MenuTabBase(Widget root, map<string, string> tooltipMap, string tooltipId, ESCT_SetupConfig setupConfig)
	{
		m_wRoot = root;
		m_TooltipMap = tooltipMap;
		m_sTooltipId = tooltipId;
		m_Config = setupConfig;
	}
	
	void Initialize();
	
	protected void OnWidgetFocus(Widget widget)
	{
		if (!widget)
			return;

		string name = widget.GetName();

		TextWidget tooltip = TextWidget.Cast(m_wRoot.FindAnyWidget(m_sTooltipId));
		if (!tooltip)
			return;

		string tooltipText = m_TooltipMap.Get(name);
		if (!tooltipText)
			return;

		tooltip.SetText(tooltipText);
	}
}
