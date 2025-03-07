[ComponentEditorProps(category: "Escapists/GameMode/Components", description: "Components that handles game mode initialization so it would be ready to be played.")]
class ESCT_GameStartComponentClass : ESCT_GameModeBaseComponentClass
{
}

sealed class ESCT_GameStartComponent: ESCT_GameModeBaseComponent
{
	private static ESCT_GameStartComponent s_Instance = null;
	static ESCT_GameStartComponent GetInstance()
	{
		if (!s_Instance)
		{
			BaseGameMode gameMode = GetGame().GetGameMode();
			if (gameMode)
			{
				s_Instance = ESCT_GameStartComponent.Cast(gameMode.FindComponent(ESCT_GameStartComponent));
			}
		}

		return s_Instance;
	}

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (SCR_Global.IsEditMode() || !Replication.IsServer())
			return;

		SetEventMask(owner, EntityEvent.INIT);
	}

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (SCR_Global.IsEditMode() || !Replication.IsServer()) return;

		ESCT_GameStateManagerComponent.GetInstance().GetOnGameStateChanged().Insert(HandleGameStateChanged);
	}

	private void HandleGameStateChanged(ESCT_EGameState gameState)
	{
		if (gameState != ESCT_EGameState.Start)
			return;

		SetWeather();
		SetTimeAcceleration();
		SetStartTime();

		ESCT_LocationSystem locationSystem = ESCT_LocationSystem.GetSystem();
		if (!locationSystem)
		{
			ESCT_Logger.Error("Failed to change game state in game start component, location system wasn't found!");
			return;
		}

		locationSystem.SelectStartArea();
		locationSystem.PrepareLocations();

		ESCT_FactionManager.GetInstance().SetupFactionGroups();

		ESCT_GameStateManagerComponent.GetInstance().GetOnGameStateChanged().Remove(HandleGameStateChanged);
	}

	private void SetWeather()
	{
		ChimeraWorld world = GetGame().GetWorld();
		TimeAndWeatherManagerEntity time = world.GetTimeAndWeatherManager();
		time.ForceWeatherTo(false, ESCT_EscapistsConfigComponent.GetInstance().GetStartingWeather());
	}

	private void SetTimeAcceleration()
	{
		float timeAcc = ESCT_EscapistsConfigComponent.GetInstance().GetTimeAcceleration();

		if (timeAcc == 1)
		{
			return;
		}

		ChimeraWorld world = ChimeraWorld.CastFrom(GetOwner().GetWorld());
		if (!world)
			return;

		TimeAndWeatherManagerEntity manager = world.GetTimeAndWeatherManager();
		if (!manager)
			return;

		const int DAY_DURATION = 24 * 60 * 60;
		manager.SetDayDuration(DAY_DURATION / timeAcc);
	}

	private void SetStartTime()
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(GetOwner().GetWorld());
		if (!world)
			return;

		TimeAndWeatherManagerEntity manager = world.GetTimeAndWeatherManager();
		if (!manager)
			return;

		ESCT_EscapistsConfigComponent gameConfig = ESCT_EscapistsConfigComponent.GetInstance();
		if (!gameConfig)
			return;

		if (!gameConfig.IsStartingTimeRandomized())
		{
			int hours = gameConfig.GetStartingHours();
			manager.SetTimeOfTheDay(hours, true);
			return;
		}

		//delicious copypasta from SCR_TimeAndWeatherHandlerComponent
		float sunrise, morning, evening, sunset;
		int hours, minutes, seconds = 0;

		// Use world-calculated sunrise and sunset values if possible, otherwise use defaults
		if (manager.GetSunriseHour(sunrise))
		{
			manager.GetSunsetHour(sunset);
		}
		else
		{
			sunrise = SCR_TimeAndWeatherHandlerComponent.DEFAULT_DAYTIME_START;
			sunset = SCR_TimeAndWeatherHandlerComponent.DEFAULT_DAYTIME_END;
		}

		// Compile a list of presets based on the sunrise and sunset times of current world if we're randomizing
		morning = sunrise + 0.25;	// Just so it's not still completely dark at the start
		float noon = (sunrise + sunset) * 0.5;
		float afternoon = (noon + sunset) * 0.5;
		evening = sunset - 0.5;
		float night = noon + 12;

		if (night >= 24)
			night -= 24;

		array<float> startingTimes = {morning, noon, afternoon, evening, night};

		// Add weights so evening / night is a bit more rare
		Math.Randomize(-1);
		int index = SCR_ArrayHelper.GetWeightedIndex({25, 25, 25, 15, 5}, Math.RandomFloat01());
		float startingTime;

		if (startingTimes.IsIndexValid(index))
			startingTime = startingTimes[index];
		else
			startingTime = startingTimes.GetRandomElement();

		manager.TimeToHoursMinutesSeconds(startingTime, hours, minutes, seconds);
	}
}
