[BaseContainerProps(configRoot: true), BaseContainerCustomCheckIntTitleField("m_bEnabled", "Ambient Event", "DISABLED - Ambient Event", 1)]
class ESCT_AmbientEventInfo
{
	[Attribute("1", desc: "Allows to disable the Entity info. SCR_EntityCatalog will ignore the info as if it is null. Used for specific gamemodes and modding.")]
	protected bool m_bEnabled;
	
	[Attribute("0", desc: "Ambient event type.", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ESCT_EAmbientEventType), category: "Escapists")]
	protected ESCT_EAmbientEventType m_eAmbientEventType;
	
	[Attribute(uiwidget: UIWidgets.Auto, desc: "Event prefab that will be used for this ambient event type.", params: "et", category: "Escapists")]
	protected ResourceName m_rEventPrefab;
	
	[Attribute(defvalue: "0.5", uiwidget: UIWidgets.Slider, params: "0 1 0.05", precision: 2, desc: "Random selection weight.")]
	float m_fWeight;
	
	[Attribute(defvalue: "3", uiwidget: UIWidgets.Slider, params: "0 10 1", desc: "How many events of this type could be present simultaneously.")]
	int m_iSimultaneousEventsLimit;
	
	bool IsEnabled()
	{
		return m_bEnabled;
	}
	
	ESCT_EAmbientEventType GetAmbientEventType()
	{
		return m_eAmbientEventType;
	}
	
	ResourceName GetEventPrefab()
	{
		return m_rEventPrefab;
	}
	
	float GetWeight()
	{
		return m_fWeight;
	}
	
	int GetSimultaneousEventsLimit()
	{
		return m_iSimultaneousEventsLimit;
	}
}
