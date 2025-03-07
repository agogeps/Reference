class ESCT_BoolValue
{
	LocalizedString m_sTitle;
	bool m_bValue;
	
	void ESCT_BoolValue(bool value, LocalizedString title)
	{
		m_sTitle = title;
		m_bValue = value;
	}
}
