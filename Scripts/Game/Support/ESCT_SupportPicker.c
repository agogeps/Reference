class ESCT_SupportPicker
{
	protected array<ESCT_SupportBase> m_aActiveSupports;
	protected map<ESCT_ESupportType, ref ESCT_SupportInfo> m_mSupportInfoMap;

	protected ref array<ESCT_SupportInfo> m_aPossibleSupports = {};
	protected ref map<ESCT_ESupportType, int> m_mPresentSupportCountMap = new map<ESCT_ESupportType, int>();

	void ESCT_SupportPicker(array<ESCT_SupportBase> activeSupports, map<ESCT_ESupportType, ref ESCT_SupportInfo> supportInfos)
	{
		m_aActiveSupports = activeSupports;
		m_mSupportInfoMap = supportInfos;
	}

	ESCT_SupportInfo PickSupportType()
	{
		if (m_mSupportInfoMap.IsEmpty())
			return null;
		
		m_aPossibleSupports.Clear();
		m_mPresentSupportCountMap.Clear();
		
		foreach (ESCT_ESupportType type, ESCT_SupportInfo info : m_mSupportInfoMap)
		{
			m_aPossibleSupports.Insert(info);
		}

		CreatePresentSupportCountMap(m_mPresentSupportCountMap);
		RemoveExceedSimultaneousLimitSupports(m_mPresentSupportCountMap);
		RemoveUsageTimeoutSupports();

		if (m_aPossibleSupports.IsEmpty())
			return null;

		array<float> weights = {};
		foreach (ESCT_SupportInfo info : m_aPossibleSupports)
		{
			weights.Insert(info.GetWeight());
		}

		int randomIndex = SCR_ArrayHelper.GetWeightedIndex(weights, Math.RandomFloat01());

		return m_aPossibleSupports.Get(randomIndex);
	}

	private void CreatePresentSupportCountMap(out map<ESCT_ESupportType, int> presentSupportCountMap)
	{
		for (int i = 0; i < m_aActiveSupports.Count(); i++)
		{
			ESCT_SupportBase support = m_aActiveSupports[i];
			if (!support)
				continue;

			ESCT_ESupportType type = support.GetSupportType();
			if (presentSupportCountMap.Contains(type))
			{
				int oldCount = presentSupportCountMap.Get(type);
				presentSupportCountMap.Set(type, ++oldCount);
			}
			else
			{
				presentSupportCountMap.Insert(support.GetSupportType(), 1);
			}
		}
	}

	private void RemoveExceedSimultaneousLimitSupports(map<ESCT_ESupportType, int> presentSupportCountMap)
	{
		ESCT_SupportInfo supportInfo = null;
		for (int i = m_aPossibleSupports.Count() - 1; i >= 0; i--)
		{
			supportInfo = m_aPossibleSupports[i];

			int countOfSupportType = presentSupportCountMap.Get(supportInfo.GetSupportType());
			if (countOfSupportType)
			{
				int limit = supportInfo.GetSimultaneousSupportLimit();
				if (limit != 0 && limit <= countOfSupportType)
				{
					m_aPossibleSupports.Remove(i);
				}
			}
		}
	}

	private void RemoveUsageTimeoutSupports()
	{
		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return;

		ESCT_SupportInfo supportInfo = null;
		for (int i = m_aPossibleSupports.Count() - 1; i >= 0; i--)
		{
			supportInfo = m_aPossibleSupports[i];
			if (!supportInfo)
				continue;

			int usageTimeout = supportInfo.GetUsageTimeoutInSeconds();
			bool isUsageExeeded = false;

			if (usageTimeout <= 0)
				continue;

			WorldTimestamp supportTimestamp = world.GetServerTimestamp();
			foreach (ESCT_SupportBase support : m_aActiveSupports)
			{
				if (!support)
					continue;

				WorldTimestamp launchTimestamp = support.GetLaunchTimestamp();
				if (!launchTimestamp)
					continue;

				if (launchTimestamp.PlusSeconds(supportInfo.GetUsageTimeoutInSeconds()).GreaterEqual(supportTimestamp))
				{
					m_aPossibleSupports.Remove(i);
					break;
				}
			}
		}
	}
}
