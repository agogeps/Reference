[EntityEditorProps(category: "Escapists/Entities/Locations", description: "City location type.")]
class ESCT_CityClass : ESCT_LocationClass
{
}

sealed class ESCT_City : ESCT_Location
{
	[Attribute("5", uiwidget: UIWidgets.Slider, desc: "How many pedestrians", params: "0 20 1", category: "Escapists")]
	protected int m_iMaxPedestrians;
	
	[Attribute("2", uiwidget: UIWidgets.Slider, desc: "How many cars will be spawned in this city.", params: "1 20 1", category: "Escapists")]
	protected int m_iMaxCars;
	
	int GetMaxPedestrians()
	{
		return m_iMaxPedestrians;
	}
	
	int GetMaxCars()
	{
		return m_iMaxCars;
	}
}
