[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ESCT_ESupportType, "m_eSupportType")]
class ESCT_SupportInfo
{
	[Attribute("0", desc: "Support type.", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ESCT_ESupportType), category: "Escapists")]
	protected ESCT_ESupportType m_eSupportType;

	[Attribute(uiwidget: UIWidgets.Auto, desc: "Support prefab that will be used for this support type.", params: "et", category: "Escapists")]
	protected ResourceName m_rSupportPrefab;

	[Attribute(defvalue: "0.5", uiwidget: UIWidgets.Slider, params: "0 1 0.05", precision: 2, desc: "Random selection weight.")]
	float m_fWeight;
	
	[Attribute(defvalue: "360", uiwidget: UIWidgets.Slider, params: "0 3600 1", desc: "For how long support couldn't be used if it was called before (in seconds).")]
	int m_iUsageTimeout;
	
	[Attribute(defvalue: "5", uiwidget: UIWidgets.Slider, params: "0 3600 1", desc: "How many supports of this type could be present simultaneously.")]
	int m_iSimultaneousSupportLimit;

	ESCT_ESupportType GetSupportType()
	{
		return m_eSupportType;
	}

	ResourceName GetSupportPrefab()
	{
		return m_rSupportPrefab;
	}

	float GetWeight()
	{
		return m_fWeight;
	}

	int GetSimultaneousSupportLimit()
	{
		return m_iSimultaneousSupportLimit;
	}
	
	int GetUsageTimeoutInSeconds()
	{
		return m_iUsageTimeout;
	}
}
