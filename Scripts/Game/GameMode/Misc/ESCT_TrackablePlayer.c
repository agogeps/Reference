class ESCT_TrackablePlayer
{
	protected int m_iPlayerId;
	protected IEntity m_PlayerEntity;

	void ESCT_TrackablePlayer(int pid, IEntity playerEntity)
	{
		m_iPlayerId = pid;
		m_PlayerEntity = playerEntity;
	}

	IEntity GetPlayerEntity()
	{
		return m_PlayerEntity;
	}

	int GetPlayerId()
	{
		return m_iPlayerId;
	}
}
