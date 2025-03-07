class ESCT_Weather
{
	string m_sWeather;
	ResourceName m_Icon;
	float m_fWeight;
	
	void ESCT_Weather(ResourceName icon, string weather, float weight)
	{
		m_sWeather = weather;
		m_Icon = icon;
		m_fWeight = weight;
	}
}
