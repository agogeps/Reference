class ESCT_TimeOfDay
{
	int m_iHours;
	ResourceName m_Icon;
	float m_fWeight = weight;
	
	void ESCT_TimeOfDay(ResourceName icon, int hours, float weight)
	{
		m_iHours = hours;
		m_Icon = icon;
		m_fWeight = weight;
	}
}
