class ESCT_AmbientEventPicker
{
	protected array<ESCT_AmbientEventBase> m_aActiveEvents;
	protected map<ESCT_EAmbientEventType, ref ESCT_AmbientEventInfo> m_mInfoMap;

	protected ref array<ref ESCT_AmbientEventInfo> m_aPossibleEvents = {};
	protected ref map<ESCT_EAmbientEventType, int> m_mPresentEventsCountMap = new map<ESCT_EAmbientEventType, int>();

	void ESCT_AmbientEventPicker(array<ESCT_AmbientEventBase> activeEvents, map<ESCT_EAmbientEventType, ref ESCT_AmbientEventInfo> infos)
	{
		m_aActiveEvents = activeEvents;
		m_mInfoMap = infos;
	}

	ESCT_AmbientEventInfo PickEvent()
	{
		if (m_mInfoMap.IsEmpty())
			return null;

		m_aPossibleEvents.Clear();
		m_mPresentEventsCountMap.Clear();

		foreach (ESCT_EAmbientEventType type, ESCT_AmbientEventInfo info : m_mInfoMap)
		{
			m_aPossibleEvents.Insert(info);
		}

		CreatePresentEventsCountMap(m_mPresentEventsCountMap);
		RemoveExceedSimultaneousLimitEvents(m_mPresentEventsCountMap);
		if (m_aPossibleEvents.IsEmpty())
			return null;

		array<float> weights = {};
		foreach (ESCT_AmbientEventInfo info : m_aPossibleEvents)
		{
			weights.Insert(info.GetWeight());
		}

		int randomIndex = SCR_ArrayHelper.GetWeightedIndex(weights, Math.RandomFloat01());
		return m_aPossibleEvents[randomIndex];
	}

	private void CreatePresentEventsCountMap(out map<ESCT_EAmbientEventType, int> presentEventsCountMap)
	{
		for (int i = 0; i < m_aActiveEvents.Count(); i++)
		{
			ESCT_AmbientEventBase ambientEvent = m_aActiveEvents[i];
			if (!ambientEvent)
				continue;

			ESCT_EAmbientEventType type = ambientEvent.GetAmbientEventType();
			if (presentEventsCountMap.Contains(type))
			{
				int oldCount = presentEventsCountMap.Get(type);
				presentEventsCountMap.Set(type, ++oldCount);
			}
			else
			{
				presentEventsCountMap.Insert(type, 1);
			}
		}
	}

	private void RemoveExceedSimultaneousLimitEvents(map<ESCT_EAmbientEventType, int> presentEventsCountMap)
	{
		ESCT_AmbientEventInfo info = null;
		for (int i = m_aPossibleEvents.Count() - 1; i >= 0; i--)
		{
			info = m_aPossibleEvents[i];

			int countOfSupportType = presentEventsCountMap.Get(info.GetAmbientEventType());
			if (countOfSupportType)
			{
				int limit = info.GetSimultaneousEventsLimit();
				if (limit != 0 && limit <= countOfSupportType)
				{
					m_aPossibleEvents.Remove(i);
				}
			}
		}
	}
}
