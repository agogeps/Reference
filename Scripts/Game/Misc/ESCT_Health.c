class ESCT_Health
{
	LocalizedString m_sTitle;
	ESCT_EHealthType m_eHealthType;
	
	void ESCT_Health(ESCT_EHealthType healthType, LocalizedString title)
	{
		m_sTitle = title;
		m_eHealthType = healthType;
	}
}
