class ESCT_EquipmentInfo : ScriptAndConfig
{
	[Attribute(ResourceName.Empty, UIWidgets.ResourcePickerThumbnail, desc: "Headgear.", params: "et")]
	ResourceName m_Headgear;
	
	[Attribute(ResourceName.Empty, UIWidgets.ResourcePickerThumbnail, desc: "Headgear.", params: "et")]
	ResourceName m_Vest;
	
	[Attribute(ResourceName.Empty, UIWidgets.ResourcePickerThumbnail, desc: "Jacket that fits pants.", params: "et")]
	ResourceName m_Jacket;
	
	[Attribute(ResourceName.Empty, UIWidgets.ResourcePickerThumbnail, desc: "Pants that fit jacket.", params: "et")]
	ResourceName m_Pants;
	
	[Attribute(ResourceName.Empty, UIWidgets.ResourcePickerThumbnail, desc: "Footwear. If not defined default character prefab one will be used instead.", params: "et")]
	ResourceName m_Footwear;
	
	[Attribute(defvalue: "1.0", uiwidget: UIWidgets.Slider, params: "0 100 1", precision: 2, desc: "Random selection weight")]
	float m_fWeight;
}
