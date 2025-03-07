class ESCT_EquipmentSlotState
{
	bool b_mIsUsed;

	protected int m_Id;
	protected SCR_ArsenalItem m_ArsenalItem;
	protected SCR_ArsenalItemDisplayData m_DisplayData;

	int GetId()
	{
		return m_Id;
	}

	SCR_ArsenalItem GetArsenalItem()
	{
		return m_ArsenalItem;
	}
	
	SCR_ArsenalItemDisplayData GetDisplayData()
	{
		return m_DisplayData;
	}

	void ESCT_EquipmentSlotState(int id, SCR_ArsenalItem arsenalItem, SCR_ArsenalItemDisplayData displayData, bool isUsed = false)
	{
		m_Id = id;
		m_ArsenalItem = arsenalItem;
		m_DisplayData = displayData;
		b_mIsUsed = isUsed;
	}
}
