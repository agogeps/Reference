[BaseContainerProps(configRoot: true)]
class ESCT_GeneralConfig : ScriptAndConfig
{			
	[Attribute(defvalue: "2", uiwidget: UIWidgets.Slider, desc: "How many entities needs to be taken/destroyed before location will be marked as cleared.", params: "1 30 1", category: "Escapists")]
	int m_iLocationClearThreshold;
	
	[Attribute(defvalue: "400", UIWidgets.Slider, desc: "Distance to reveal true marker instead of question mark.", params: "0 2000 1")]
	float m_fMarkerRevealDistance;
	
	[Attribute(ResourceName.Empty, desc: "Locations that could spawn on random location type markers.", category: "Escapists")]
	ref array<ref ESCT_LocationConfig> m_RandomLocationTypes;
}
