sealed class ESCT_EnvironmentTab : ESCT_MenuTabBase
{
	private static const string WEATHER_TOOLBOX_ID = "WeatherToolbox";
	private static const string TIME_SLIDER_ID = "TimeAccelerationSlider";
	private static const string START_HOURS_ID = "StartingHoursSlider";
	private static const string TIME_TOOLBOX_ID = "TimeToolbox";
	
	void ESCT_EnvironmentTab(Widget root, map<string, string> tooltipMap, string tooltipId, ESCT_SetupConfig setupConfig)
	{
		m_TooltipMap.Insert(WEATHER_TOOLBOX_ID, "#Escapists-StartMenu_EnvironmentTab_Weather_Tooltip");
		m_TooltipMap.Insert(TIME_TOOLBOX_ID, "#Escapists-StartMenu_EnvironmentTab_TimeOfDay_Tooltip");
		m_TooltipMap.Insert(START_HOURS_ID, "#Escapists-StartMenu_EnvironmentTab_TimeOfDay_Tooltip");
		m_TooltipMap.Insert(TIME_SLIDER_ID, "#Escapists-StartMenu_EnvironmentTab_TimeAcc_Tooltip");
	}
	
	override void Initialize()
	{
		SetWeatherToolbox();
		SetTimeAccSlider();
		SetStartingHoursSlider();
		SetTimeOfDayToolbox();
	}
	
	protected void SetWeatherToolbox()
	{
		Widget of = m_wRoot.FindAnyWidget(WEATHER_TOOLBOX_ID);
		ESCT_ToolboxComponent toolbox = ESCT_ToolboxComponent.Cast(of.FindHandler(ESCT_ToolboxComponent));
		toolbox.ClearAll();
		toolbox.m_OnChanged.Insert(OnCheckWeather);
		toolbox.m_OnFocus.Insert(OnWidgetFocus);

		array<ref ESCT_Weather> weathers =
		{
			new ESCT_Weather("{59A7BD34D284302E}UI/Textures/Editor/Attributes/Weather/Attribute_Weather_Clear.edds", "Clear", 40),
			new ESCT_Weather("{1E1E9B8E2A6ABEDA}UI/Textures/Editor/Attributes/Weather/Attribute_Weather_Cloudy.edds", "Cloudy", 40),
			new ESCT_Weather("{E72405E0209CB3CC}UI/Textures/Editor/Attributes/Weather/Attribute_Weather_Overcast.edds", "Overcast", 40),
			new ESCT_Weather("{786F2BC1E092D7B0}UI/Textures/Editor/Attributes/Weather/Attribute_Weather_Rainy.edds", "Rainy", 30),
			new ESCT_Weather("{945995A91C59DC97}UI/Textures/Editor/Attributes/Weather/Attribute_Weather_Random.edds", "Random", 25)
		};

		for (int i = 0; i < weathers.Count(); i++)
		{
			toolbox.AddItem(string.Empty, i == weathers.Count() - 1, weathers[i]);
		}

		array<SCR_ButtonBaseComponent> items = {};
		toolbox.GetItems(items);

		for (int i=0; i < items.Count(); i++)
		{
			ESCT_Weather weather = ESCT_Weather.Cast(toolbox.GetItemData(i));
			if (!weather)
				continue;

			SCR_ButtonBaseComponent buttonBase = toolbox.GetItem(i);
			Widget button = buttonBase.GetRootWidget();
			SCR_ButtonImageComponent buttonImageComponent = SCR_ButtonImageComponent.Cast(button.FindHandler(SCR_ButtonImageComponent));

			buttonImageComponent.SetImage(weather.m_Icon);
		}

		array<float> weights = {};
		for (int i = 0; i < weathers.Count(); i++)
		{
			weights.Insert(weathers[i].m_fWeight);
		}

		#ifdef WORKBENCH
		const int initialIndex = 0;
		#else
		int initialIndex = SCR_ArrayHelper.GetWeightedIndex(weights, Math.RandomFloat01());
		#endif
		toolbox.SetCurrentItem(initialIndex);
		OnCheckWeather(toolbox, initialIndex);
	}

	protected void SetTimeAccSlider()
	{
		Widget of = m_wRoot.FindAnyWidget(TIME_SLIDER_ID);
		ESCT_SliderComponent slider = ESCT_SliderComponent.Cast(of.FindHandler(ESCT_SliderComponent));
		slider.m_OnChanged.Insert(OnChangeTimeAccSlider);
		slider.m_OnFocus.Insert(OnWidgetFocus);

		slider.SetValue(1);
	}

	protected void SetStartingHoursSlider()
	{
		Widget of = m_wRoot.FindAnyWidget(START_HOURS_ID);
		ESCT_SliderComponent slider = ESCT_SliderComponent.Cast(of.FindHandler(ESCT_SliderComponent));
		slider.GetOnChangedFinal().Insert(OnChangeStartingHoursSlider);
		slider.m_OnFocus.Insert(OnWidgetFocus);
	}

	protected void SetTimeOfDayToolbox()
	{
		Widget of = m_wRoot.FindAnyWidget(TIME_TOOLBOX_ID);
		ESCT_ToolboxComponent toolbox = ESCT_ToolboxComponent.Cast(of.FindHandler(ESCT_ToolboxComponent));
		toolbox.ClearAll();
		toolbox.m_OnChanged.Insert(OnCheckTime);
		toolbox.m_OnFocus.Insert(OnWidgetFocus);

		array<ref ESCT_TimeOfDay> times =
		{
			new ESCT_TimeOfDay("{94B936AA2CE13242}UI/Textures/Editor/Attributes/Time/Attribute_Time_EarlyMorning.edds", 8, 40),
			new ESCT_TimeOfDay("{FF96EE66370C0CBB}UI/Textures/Editor/Attributes/Time/Attribute_Time_Morning.edds", 10, 40),
			new ESCT_TimeOfDay("{1F170BA4B611B7D6}UI/Textures/Editor/Attributes/Time/Attribute_Time_Midday.edds", 12, 40),
			new ESCT_TimeOfDay("{F63FE753B10DD0F0}UI/Textures/Editor/Attributes/Time/Attribute_Time_Afternoon.edds", 15, 40),
			new ESCT_TimeOfDay("{59ABC4E5F1CC0755}UI/Textures/Editor/Attributes/Time/Attribute_Time_Evening.edds", 18, 25),
			new ESCT_TimeOfDay("{5E793A7B8DA84847}UI/Textures/Editor/Attributes/Time/Attribute_Time_Midnight.edds", 20, 15),
			new ESCT_TimeOfDay("{945995A91C59DC97}UI/Textures/Editor/Attributes/Weather/Attribute_Weather_Random.edds", -1, 10)
		};

		for (int i = 0; i < times.Count(); i++)
		{
			toolbox.AddItem(string.Empty, i == times.Count() - 1, times[i]);
		}

		array<SCR_ButtonBaseComponent> items = {};
		toolbox.GetItems(items);

		for (int i=0; i < items.Count(); i++)
		{
			ESCT_TimeOfDay timeOfDay = ESCT_TimeOfDay.Cast(toolbox.GetItemData(i));
			if (!timeOfDay)
				continue;

			SCR_ButtonBaseComponent buttonBase = toolbox.GetItem(i);
			Widget button = buttonBase.GetRootWidget();
			SCR_ButtonImageComponent buttonImageComponent = SCR_ButtonImageComponent.Cast(button.FindHandler(SCR_ButtonImageComponent));

			buttonImageComponent.SetImage(timeOfDay.m_Icon);
		}

		array<float> weights = {};
		for (int i = 0; i < times.Count(); i++)
		{
			weights.Insert(times[i].m_fWeight);
		}

		#ifdef WORKBENCH
		const int initialIndex = 0;
		#else
		int initialIndex = SCR_ArrayHelper.GetWeightedIndex(weights, Math.RandomFloat01());
		#endif
		toolbox.SetCurrentItem(initialIndex);
		OnCheckTime(toolbox, initialIndex);
	}
	
	private void OnCheckWeather(ESCT_ToolboxComponent toolbox, int index)
	{
		ESCT_Weather data = ESCT_Weather.Cast(toolbox.GetItemData(index));

		if (data.m_sWeather == "Random")
		{
			array<string> weathers = { "Clear", "Cloudy", "Overcast", "Rainy" };
			m_Config.m_sStartingWeather = weathers.GetRandomElement();
		}
		else
		{
			m_Config.m_sStartingWeather = data.m_sWeather;
		}
	}

	private void OnChangeStartingHoursSlider(ESCT_SliderComponent slider, int index)
	{
		int hours = slider.GetValue();
		m_Config.m_iStartingHours = hours;

		Widget of = m_wRoot.FindAnyWidget(TIME_TOOLBOX_ID);
		ESCT_ToolboxComponent toolbox = ESCT_ToolboxComponent.Cast(of.FindHandler(ESCT_ToolboxComponent));
		SCR_ButtonBaseComponent buttonBase = toolbox.GetItem(toolbox.GetCurrentIndex());
		buttonBase.SetToggled(false);
	}

	private void OnChangeTimeAccSlider(ESCT_SliderComponent slider, int index)
	{
		float timeAcc = slider.GetValue();
		m_Config.m_fTimeAcceleration = timeAcc;
	}

	private void OnCheckTime(ESCT_ToolboxComponent toolbox, int index)
	{
		ESCT_TimeOfDay data = ESCT_TimeOfDay.Cast(toolbox.GetItemData(index));

		if (data.m_iHours == -1)
		{
			m_Config.m_bRandomStartingTime = true;
		}
		else
		{
			m_Config.m_bRandomStartingTime = false;
			m_Config.m_iStartingHours = data.m_iHours;
		}

		UpdateStartingHoursSlider(data.m_iHours);
	}

	private void UpdateStartingHoursSlider(int hours)
	{
		Widget of = m_wRoot.FindAnyWidget(START_HOURS_ID);
		ESCT_SliderComponent slider = ESCT_SliderComponent.Cast(of.FindHandler(ESCT_SliderComponent));

		if (hours != -1)
		{
			if (!slider.IsEnabled())
			{
				slider.SetEnabled(true);
			}
			slider.SetValue(hours);
		}
		else
		{
			slider.SetEnabled(false);
		}
	}
}
