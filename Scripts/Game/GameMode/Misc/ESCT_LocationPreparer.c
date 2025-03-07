sealed class ESCT_LocationPreparer
{
	private array<ESCT_Location> m_Locations;
	private array<IEntity> m_aStaticLocations;
	private array<ESCT_RandomLocationHolder> m_aRandomLocationHolders;
	private IEntity m_StartingPrison;

	private static const float NO_IMPORTANT_LOCATIONS_AROUND_START = 800.0;
	private static const string RADIO_STATION_PREFAB = "{10315A25A03D4558}Prefabs/LocationTypes/RadioStation.et";
	private static const string HOSPITAL_PREFAB = "{5EBBC576CBBA7751}Prefabs/LocationTypes/Hospital.et";

	void ESCT_LocationPreparer(notnull array<ESCT_Location> locations, array<IEntity> staticLocations, array<ESCT_RandomLocationHolder> randomLocationHolders, IEntity prison)
	{
		m_Locations = locations;
		m_aStaticLocations = staticLocations;
		m_aRandomLocationHolders = randomLocationHolders;
		m_StartingPrison = prison;
	}

	void InititalizeLocations()
	{
		AddRandomLocations();

		int removeQuantity = m_Locations.Count() * ESCT_EscapistsConfigComponent.GetInstance().GetLocationsQuantityMultiplier();

		Math.Randomize(-1);
		ShuffleLocations(m_Locations);

		Tuple2<int, int> importantLocations = CountNecessaryLocations();
		int helipadCount = importantLocations.param1;
		int radioStationCount = importantLocations.param2;

		vector startingPosition = m_StartingPrison.GetOrigin();
		ESCT_Location location = null;

		bool areInsurgentsPresent = ESCT_EscapistsConfigComponent.GetInstance().IsInsurgencyEnabled();

		for (int i = m_Locations.Count() - 1; i >= 0; i--)
		{
			location = m_Locations[i];
			if (!location)
				continue;

			ESCT_ELocationType locationType = location.GetLocationType();
			if (locationType == ESCT_ELocationType.RadioStation)
			{
				//do not remove last radio station
				if (location.IsSpawnGuaranteed() || radioStationCount == 1)
					continue;

				if (ESCT_Math.IsPointInRadiusXZ(location.GetOrigin(), startingPosition, NO_IMPORTANT_LOCATIONS_AROUND_START))
				{
					SCR_EntityHelper.DeleteEntityAndChildren(location);
					m_Locations.RemoveItem(location);
					radioStationCount--;
				}
			}
			else if (locationType == ESCT_ELocationType.Helipad)
			{
				//do not remove last radio helipad
				if (location.IsSpawnGuaranteed() || helipadCount == 1)
					continue;

				if (ESCT_Math.IsPointInRadiusXZ(location.GetOrigin(), startingPosition, NO_IMPORTANT_LOCATIONS_AROUND_START))
				{
					SCR_EntityHelper.DeleteEntityAndChildren(location);
					m_Locations.RemoveItem(location);
					helipadCount--;
				}
			}
			else if (locationType == ESCT_ELocationType.Hideout && !areInsurgentsPresent)
			{
				m_Locations.RemoveItem(location);
			}
		}

		while (removeQuantity > 0)
		{
			ESCT_Location removableLocation = m_Locations.GetRandomElement();
			ESCT_ELocationType type = removableLocation.GetLocationType();

			if (removableLocation.IsSpawnGuaranteed() || type == ESCT_ELocationType.City || type == ESCT_ELocationType.StartArea || type == ESCT_ELocationType.Patrol)
				continue;

			bool isRemovalAllowed = true;

			if (type == ESCT_ELocationType.Helipad)
			{
				helipadCount++;
				if (helipadCount == 1)
					isRemovalAllowed = false;
			}
			else if (type == ESCT_ELocationType.RadioStation)
			{
				radioStationCount++;
				if (radioStationCount == 1)
					isRemovalAllowed = false;
			}

			if (!isRemovalAllowed)
			{
				removeQuantity--; //to prevent deadlocks
				continue;
			}

			SCR_EntityHelper.DeleteEntityAndChildren(removableLocation);
			m_Locations.RemoveItem(removableLocation);
			removeQuantity--;
		}

		AddStaticLocations();
		EnableAtLeastOneRadioStation();
		InitLocations();
	}

	private void InitLocations()
	{
		for (int i = 0; i < m_Locations.Count(); i++)
		{
			ESCT_Location location = m_Locations[i];
			if (!location)
				continue;

			location.Initialize();
		}
	}

	private Tuple2<int, int> CountNecessaryLocations()
	{
		int helipadCount = 0;
		int radioStationCount = 0;

		for (int i = 0; i < m_Locations.Count(); i++)
		{
			ESCT_ELocationType type = m_Locations[i].GetLocationType();
			if (type == ESCT_ELocationType.Helipad)
			{
				helipadCount++;
			}
			else if (type == ESCT_ELocationType.RadioStation)
			{
				radioStationCount++;
			}
		}

		return new Tuple2<int, int>(helipadCount, radioStationCount);
	}

	private void AddRandomLocations()
	{
		ESCT_EscapistsConfigComponent gameConfig = ESCT_EscapistsConfigComponent.GetInstance();
		if (!gameConfig)
			return;

		Math.Randomize(-1);

		array<ref ESCT_LocationConfig> randomLocationTypes = gameConfig.GetRandomLocationTypes();
		int locationTypeCount = randomLocationTypes.Count();

		// Use a single loop to create the weights array and find the weighted index
		float totalWeight = 0;
		for (int j = 0; j < locationTypeCount; j++)
		{
			float noisyWeight = randomLocationTypes[j].GetWeight() + (Math.RandomFloat01() * 0.01);
			totalWeight += noisyWeight;
		}

		ESCT_RandomLocationHolder holder = null;
		ESCT_LocationConfig locationConfig = null;

		for (int i = m_aRandomLocationHolders.Count() - 1; i >= 0; i--)
		{
			holder = m_aRandomLocationHolders[i];
			if (!holder)
				return;

			float randomValue = Math.RandomFloat01() * totalWeight;
			float cumulativeWeight = 0;
			for (int j = 0; j < locationTypeCount; j++)
			{
				cumulativeWeight += randomLocationTypes[j].GetWeight();
				if (randomValue <= cumulativeWeight)
				{
					locationConfig = randomLocationTypes[j];
					break;
				}
			}

			if (!locationConfig || !locationConfig.GetLocationPrefab())
				continue;

			vector transform[4];
			holder.GetTransform(transform);

			IEntity entity =ESCT_SpawnHelpers.SpawnEntityPrefabMatrix(locationConfig.GetLocationPrefab(), transform);
			ESCT_Location location = ESCT_Location.Cast(entity);
			if (!location)
			{
				SCR_EntityHelper.DeleteEntityAndChildren(entity);
				continue;
			}

			if (holder.IsRotationPreserved())
			{
				location.SetOrigin(holder.GetOrigin());
				location.SetUseLocationRotator(true);
			}

			if (holder.IsSpawnGuaranteed())
				location.SetSpawnGuaranteed(true);

			ESCT_LocationPatrolManagerComponent patrolManager = ESCT_LocationPatrolManagerComponent.Cast(location.FindComponent(ESCT_LocationPatrolManagerComponent));
			if (patrolManager)
			{
				if (holder.ShouldSpawnAdditionalGroupsForPoints())
					patrolManager.SetSpawnAdditionalGroup(true);
				
				array<ref ESCT_LocationPatrolPoint> points = holder.GetLocationPatrolPoints();
				if (!points.IsEmpty())
					patrolManager.SetLocationPatrolPoints(points);

				ESCT_LocationPatrolPoint defendPoint = holder.GetDefendPoint();
				if (defendPoint)
					patrolManager.SetDefendPoint(defendPoint);
			}

			//mission completed, no longer needed
			SCR_EntityHelper.DeleteEntityAndChildren(holder);
		}
	}

	private void AddStaticLocations()
	{
		ESCT_EscapistsConfigComponent gameConfig = ESCT_EscapistsConfigComponent.GetInstance();
		if (!gameConfig)
			return;

		float chance = gameConfig.GetStaticLocationChance();
		RandomGenerator rnd = ESCT_EscapistsGameMode.GetGameMode().GetRandomGenerator();

		foreach (IEntity location : m_aStaticLocations)
		{
			if (!location || rnd.RandFloat01() > chance)
				continue;

			ESCT_StaticLocationComponent staticLocation = ESCT_StaticLocationComponent.Cast(location.FindComponent(ESCT_StaticLocationComponent));
			if (!staticLocation)
				continue;

			//it should add itself to locations array via EOnInit as any ordinary location
			ESCT_ELocationType type = staticLocation.GetLocationType();
			if (type == ESCT_ELocationType.Hospital)
			{
				IEntity entity = ESCT_SpawnHelpers.SpawnEntityPrefab(HOSPITAL_PREFAB, location.GetOrigin());
				if (!entity)
					continue;

				ESCT_Location hospitalEntity = ESCT_Location.Cast(entity);
				if (!hospitalEntity)
					continue;

				hospitalEntity.SetStaticComposition(location);
			}
			else if (type == ESCT_ELocationType.RadioStation)
			{
				ESCT_StaticRadioStationComponent staticRadioStation = ESCT_StaticRadioStationComponent.Cast(location.FindComponent(ESCT_StaticRadioStationComponent));
				if (staticRadioStation)
					staticRadioStation.SpawnRadio();

				IEntity entity = ESCT_SpawnHelpers.SpawnEntityPrefab(RADIO_STATION_PREFAB, location.GetOrigin());
				if (!entity)
					return;

				ESCT_RadioStation radioStation = ESCT_RadioStation.Cast(entity);
				if (!radioStation)
					return;

				radioStation.SetStaticComposition(location);
			}
		}
	}

	private void EnableAtLeastOneRadioStation()
	{
		Tuple2<int, int> importantLocations = CountNecessaryLocations();
		int radioStationCount = importantLocations.param2;

		if (radioStationCount > 0)
			return;

		array<IEntity> radioStations = {};
		foreach (IEntity location : m_aStaticLocations)
		{
			ESCT_StaticLocationComponent staticLocation = ESCT_StaticLocationComponent.Cast(location.FindComponent(ESCT_StaticLocationComponent));
			if (!staticLocation)
				continue;

			ESCT_ELocationType type = staticLocation.GetLocationType();
			if (type != ESCT_ELocationType.RadioStation)
				continue;

			radioStations.Insert(location);
		}

		IEntity necessaryRadioStation = radioStations.GetRandomElement();
		if (!necessaryRadioStation)
			return;

		ESCT_StaticRadioStationComponent staticRadioStation = ESCT_StaticRadioStationComponent.Cast(necessaryRadioStation.FindComponent(ESCT_StaticRadioStationComponent));
		if (staticRadioStation)
			staticRadioStation.SpawnRadio();

		IEntity entity = ESCT_SpawnHelpers.SpawnEntityPrefab(RADIO_STATION_PREFAB, necessaryRadioStation.GetOrigin());
		if (!entity)
			return;

		ESCT_RadioStation radioStation = ESCT_RadioStation.Cast(entity);
		if (!radioStation)
			return;

		radioStation.SetStaticComposition(necessaryRadioStation);
	}

	//FIXME: I really don't get how to use generic helper SCR_ArrayHelperT.Shuffle() for generic entities, so this could be likely deleted later
	private void ShuffleLocations(notnull inout array<ESCT_Location> items, int shuffles = 50)
	{
		if (items.Count() < 2)
			return;

		// two checks are faster than Math.ClampInt
		if (shuffles < 1)
			shuffles = 1;

		if (shuffles > 50)
			shuffles = 50;

		while (shuffles > 0)
		{
			for (int i = 0, count = items.Count(); i < count; i++)
			{
				int index1 = Math.RandomInt(0, count);
				int index2 = Math.RandomInt(0, count);
				if (index1 != index2)
					items.SwapItems(index1, index2);
			}

			shuffles--;
		}
	}
}
