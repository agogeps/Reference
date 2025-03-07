class ESCT_CharacterState
{
	bool m_bIsKilled;

	protected int m_Id;
	protected ResourceName m_CharacterPrefab;

	int GetId()
	{
		return m_Id;
	}

	ResourceName GetCharacterPrefab()
	{
		return m_CharacterPrefab;
	}

	void ESCT_CharacterState(int id, ResourceName characterPrefab, bool isKilled = false)
	{
		m_Id = id;
		m_CharacterPrefab = characterPrefab;
		m_bIsKilled = isKilled;
	}
}
