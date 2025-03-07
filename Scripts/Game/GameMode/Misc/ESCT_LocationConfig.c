[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ESCT_ELocationType, "m_eLocationType")]
class ESCT_LocationConfig
{
	[Attribute("0", uiwidget: UIWidgets.ComboBox, desc: "Location type.", enums: ParamEnumArray.FromEnum(ESCT_ELocationType), category: "Escapists")]
	protected ESCT_ELocationType m_eLocationType;
	
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Location prefab.", params: "et", category: "Escapists")]
	protected ResourceName m_rLocationPrefab;
	
	[Attribute(defvalue: "0.5", uiwidget: UIWidgets.Slider, params: "0 1 0.05", precision: 2, desc: "Random selection weight")]
	protected float m_fWeight;
	
	ESCT_ELocationType GetLocationType()
	{
		return m_eLocationType;
	}
	
	ResourceName GetLocationPrefab()
	{
		return m_rLocationPrefab;
	}
	
	float GetWeight()
	{
		return m_fWeight;
	}
}
