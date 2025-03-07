sealed class ESCT_StartAreaSelector
{
	protected ResourceName m_StartAreaLocationPrefab;
	protected array<IEntity> m_aStartAreas;

	void ESCT_StartAreaSelector(notnull array<IEntity> startAreas, ResourceName startAreaLocationPrefab)
	{
		m_aStartAreas = startAreas;
		m_StartAreaLocationPrefab = startAreaLocationPrefab;
	}

	IEntity SelectStartArea()
	{
		if (m_aStartAreas.IsEmpty())
		{
			ESCT_Logger.Error("No start location found, can't setup the game, aborting.");
			return null;
		}

		ESCT_EscapistsConfigComponent gameConfig = ESCT_EscapistsConfigComponent.GetInstance();
		if (!gameConfig)
			return null;

		array<IEntity> startAreas = {};
		FillStartAreasArray(startAreas);

		ESCT_Logger.InfoFormat("Total start locations: %1", startAreas.Count().ToString());

		Math.Randomize(-1);
		IEntity selectedStartArea = startAreas.GetRandomElement();
		if (!selectedStartArea)
		{
			ESCT_Logger.Error("Failed to find start area in start area array!");
			return null;
		}

		//helicrashes doesn't exist on compilation phase so it needed to be spawned before doing any start area actions with them
		if (selectedStartArea.Type() == ESCT_HelicrashLocationHolder)
		{
			ESCT_FactionManager factionManager = ESCT_FactionManager.GetInstance();
			ESCT_Faction targetFaction = factionManager.GetPlayerFaction();
			ESCT_PlayerFactionInfo info = targetFaction.GetPlayerFactionInfo();

			ResourceName helicrashPrefab = info.GetHelicrashPrefab();
			if (!helicrashPrefab)
			{
				ESCT_Logger.ErrorFormat(
					"[ESCT_StartAreaSelector][SelectStartArea] Failed to get helicrash prefab for %1 faction, check player faction info params for it.",
					targetFaction.GetFactionKey());
				return null;
			}

			vector mat[4];
			selectedStartArea.GetTransform(mat);
			selectedStartArea = ESCT_SpawnHelpers.SpawnEntityPrefabMatrix(info.GetHelicrashPrefab(), mat);
		}

		ESCT_StartArea area = ESCT_StartArea.Cast(ESCT_SpawnHelpers.SpawnEntityPrefab(m_StartAreaLocationPrefab, selectedStartArea.GetOrigin(), selectedStartArea.GetAngles()));
		area.SetStaticComposition(selectedStartArea);
		ESCT_StartAreaComponent selectedStartAreaComponent = ESCT_StartAreaComponent.Cast(selectedStartArea.FindComponent(ESCT_StartAreaComponent));
		area.SetStartAreaComponent(selectedStartAreaComponent);
		selectedStartAreaComponent.Setup();
		area.Spawn();

		ESCT_EscapistsGameMode gameMode = ESCT_EscapistsGameMode.GetGameMode();

		switch (true)
		{
			case selectedStartAreaComponent.Type() == ESCT_PrisonComponent:
				gameMode.SetStartType(ESCT_EStartType.Prison);
				break;

			case selectedStartAreaComponent.Type() == ESCT_HideoutStartComponent:
				gameMode.SetStartType(ESCT_EStartType.Hideout);
				break;

			case selectedStartAreaComponent.Type() == ESCT_HelicrashStartComponent:
				gameMode.SetStartType(ESCT_EStartType.Helicrash);
				break;
		}

		RemoveAreas(selectedStartArea);

		return area;
	}

	private void FillStartAreasArray(out array<IEntity> startAreas)
	{
		ESCT_EscapistsConfigComponent gameConfig = ESCT_EscapistsConfigComponent.GetInstance();
		if (!gameConfig)
			return;

		ESCT_EStartType configStartType = gameConfig.GetStartType();

		switch (configStartType)
		{
			case ESCT_EStartType.Prison:
				foreach (IEntity area : m_aStartAreas)
				{
					ESCT_StartAreaComponent areaComponent = ESCT_StartAreaComponent.Cast(area.FindComponent(ESCT_StartAreaComponent));
					if (!areaComponent)
						continue;

					if (areaComponent.Type() == ESCT_PrisonComponent)
					{
						startAreas.Insert(area);
					}
				}
				break;

			case ESCT_EStartType.Hideout:
				foreach (IEntity area : m_aStartAreas)
				{
					ESCT_StartAreaComponent areaComponent = ESCT_StartAreaComponent.Cast(area.FindComponent(ESCT_StartAreaComponent));
					if (!areaComponent)
						continue;

					if (areaComponent.Type() == ESCT_HideoutStartComponent)
					{
						startAreas.Insert(area);
					}
				}
				break;

			case ESCT_EStartType.Helicrash:
				foreach (IEntity area : m_aStartAreas)
				{
					if (area.Type() == ESCT_HelicrashLocationHolder)
					{
						startAreas.Insert(area);
					}
				}
				break;

			case ESCT_EStartType.Random:
			default:
				startAreas = m_aStartAreas;
				break;
		}
	}

	private void RemoveAreas(IEntity selectedStartArea)
	{
		array<IEntity> children = {};
		foreach (IEntity startArea : m_aStartAreas)
		{
			if (selectedStartArea == startArea)
				continue;

			ESCT_StartAreaComponent startAreaComponent = ESCT_StartAreaComponent.Cast(startArea.FindComponent(ESCT_StartAreaComponent));
			if (!startAreaComponent || !startAreaComponent.ShouldDeleteAfterStart())
				continue;

			//for some reason furniture is not part of the building hierarchy :(
			children.Clear();
			ESCT_EntityHelper.GetAllChildren(startArea, children);
			foreach (IEntity deriativeEntity : children)
			{
				SCR_EntityHelper.DeleteEntityAndChildren(deriativeEntity);
			}

			SCR_EntityHelper.DeleteEntityAndChildren(startArea);
		}
	}
}
