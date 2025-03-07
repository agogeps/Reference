class ESCT_Skill
{
	LocalizedString m_sTitle;
	EAISkill m_eSkill;
	
	void ESCT_Skill(EAISkill skill, LocalizedString title)
	{
		m_sTitle = title;
		m_eSkill = skill;
	}
}
