sealed class ESCT_DifficultyTab : ESCT_MenuTabBase
{
	private static const string SURVIVABILITY_SPIN_ID = "SurvivabilitySpinBox";
	private static const string AI_SKILL_TOOLBOX_ID = "AISkillToolbox";
	private static const string LOCATIONS_QUANTITY_SPIN_ID = "LocationsQuantitySpinBox";
	private static const string EVENTS_FREQUENCY_ID = "EventFrequencySpinbox";
	private static const string WITHSTAND_ID = "WithstandSpinbox";
	
	void ESCT_DifficultyTab(Widget root, map<string, string> tooltipMap, string tooltipId, ESCT_SetupConfig setupConfig)
	{
		m_TooltipMap.Insert(SURVIVABILITY_SPIN_ID, "#Escapists-StartMenu_Difficulty_Survivability_Tooltip");
		m_TooltipMap.Insert(AI_SKILL_TOOLBOX_ID, "#Escapists-StartMenu_Difficulty_AiSkill_Tooltip");
		m_TooltipMap.Insert(LOCATIONS_QUANTITY_SPIN_ID, "#Escapists-StartMenu_Difficulty_LocationsQuantity_Tooltip");
		m_TooltipMap.Insert(EVENTS_FREQUENCY_ID, "#Escapists-StartMenu_Difficulty_EventFrequency_Tooltip");
		m_TooltipMap.Insert(WITHSTAND_ID, "#Escapists-StartMenu_Withstand_Tooltip");
	}

	override void Initialize()
	{
		SetLocationsQuantitySpinbox();
		SetAmbientEventFrequencySpinbox();
		SetSurvivabilitySpinbox();
		SetAiSkillToolbox();
		SetWithstandToolbox();
	}
	
	protected void SetLocationsQuantitySpinbox()
	{
		Widget of = m_wRoot.FindAnyWidget(LOCATIONS_QUANTITY_SPIN_ID);
		ESCT_SpinBoxComponent spin = ESCT_SpinBoxComponent.Cast(of.FindHandler(ESCT_SpinBoxComponent));
		spin.ClearAll();
		spin.m_OnChanged.Insert(OnCheckLocationsQuantity);
		spin.m_OnFocus.Insert(OnWidgetFocus);

		array<ref ESCT_FloatVal> qts =
		{
			new ESCT_FloatVal(0.5, "#Escapists-StartMenu_Difficulty_LocationsQuantity_Low"),
			new ESCT_FloatVal(0.25, "#Escapists-StartMenu_Difficulty_LocationsQuantity_Medium"),
			new ESCT_FloatVal(0.1, "#Escapists-StartMenu_Difficulty_LocationsQuantity_High"),
			new ESCT_FloatVal(0, "#Escapists-StartMenu_Difficulty_LocationsQuantity_All")
		};

		for (int i = 0; i < qts.Count(); i++)
		{
			spin.AddItem(qts[i].m_sTitle, i == qts.Count() - 1, qts[i]);
		}

		spin.SetCurrentItem(1);
		OnCheckLocationsQuantity(spin, 1);
	}

	protected void SetAmbientEventFrequencySpinbox()
	{
		Widget of = m_wRoot.FindAnyWidget(EVENTS_FREQUENCY_ID);
		ESCT_SpinBoxComponent spin = ESCT_SpinBoxComponent.Cast(of.FindHandler(ESCT_SpinBoxComponent));
		spin.ClearAll();
		spin.m_OnChanged.Insert(OnCheckAmbientFrequency);
		spin.m_OnFocus.Insert(OnWidgetFocus);

		array<ref ESCT_IntValue> qts =
		{
			new ESCT_IntValue(-1, "#Escapists-StartMenu_Difficulty_EventFrequency_Off"),
			new ESCT_IntValue(60, "#Escapists-StartMenu_Difficulty_EventFrequency_Low"),
			new ESCT_IntValue(45, "#Escapists-StartMenu_Difficulty_EventFrequency_Normal"),
			new ESCT_IntValue(20, "#Escapists-StartMenu_Difficulty_EventFrequency_Frequent")
		};

		for (int i = 0; i < qts.Count(); i++)
		{
			spin.AddItem(qts[i].m_sTitle, i == qts.Count() - 1, qts[i]);
		}

		spin.SetCurrentItem(1);
		OnCheckAmbientFrequency(spin, 1);
	}

	protected void SetSurvivabilitySpinbox()
	{
		Widget of = m_wRoot.FindAnyWidget(SURVIVABILITY_SPIN_ID);
		ESCT_SpinBoxComponent spin = ESCT_SpinBoxComponent.Cast(of.FindHandler(ESCT_SpinBoxComponent));
		spin.ClearAll();
		spin.m_OnChanged.Insert(OnCheckHealth);
		spin.m_OnFocus.Insert(OnWidgetFocus);

		array<ref ESCT_Health> healths =
		{
			new ESCT_Health(ESCT_EHealthType.Default, "#Escapists-StartMenu_Difficulty_Survivability_Option_Standard"),
			new ESCT_Health(ESCT_EHealthType.SlightlyIncreased, "#Escapists-StartMenu_Difficulty_Survivability_Option_Slightly"),
			new ESCT_Health(ESCT_EHealthType.ModeratelyIncreased, "#Escapists-StartMenu_Difficulty_Survivability_Option_Moderately")
		};

		for (int i = 0; i < healths.Count(); i++)
		{
			spin.AddItem(healths[i].m_sTitle, i == healths.Count() - 1, healths[i]);
		}

		spin.SetCurrentItem(0);
		OnCheckHealth(spin, 0);
	}

	protected void SetAiSkillToolbox()
	{
		Widget of = m_wRoot.FindAnyWidget(AI_SKILL_TOOLBOX_ID);
		ESCT_ToolboxComponent toolbox = ESCT_ToolboxComponent.Cast(of.FindHandler(ESCT_ToolboxComponent));
		toolbox.ClearAll();
		toolbox.m_OnChanged.Insert(OnCheckAiSkill);
		toolbox.m_OnFocus.Insert(OnWidgetFocus);

		array<ref ESCT_Skill> skills =
		{
			new ESCT_Skill(EAISkill.ROOKIE, "#Escapists-StartMenu_Difficulty_AiSkill_Rookie"),
			new ESCT_Skill(EAISkill.REGULAR, "#Escapists-StartMenu_Difficulty_AiSkill_Regular"),
			new ESCT_Skill(EAISkill.VETERAN, "#Escapists-StartMenu_Difficulty_AiSkill_Veteran"),
			new ESCT_Skill(EAISkill.EXPERT, "#Escapists-StartMenu_Difficulty_AiSkill_Expert")
		};

		for (int i = 0; i < skills.Count(); i++)
		{
			toolbox.AddItem(skills[i].m_sTitle, i == skills.Count() - 1, skills[i]);
		}

		toolbox.SetCurrentItem(1);
		OnCheckAiSkill(toolbox, 1);
	}
	
	protected void SetWithstandToolbox()
	{
		Widget of = m_wRoot.FindAnyWidget(WITHSTAND_ID);
		ESCT_SpinBoxComponent spin = ESCT_SpinBoxComponent.Cast(of.FindHandler(ESCT_SpinBoxComponent));
		spin.ClearAll();
		spin.m_OnChanged.Insert(OnWithstandChange);
		spin.m_OnFocus.Insert(OnWidgetFocus);

		array<ref ESCT_BoolValue> vals =
		{
			new ESCT_BoolValue(false, "#Escapists-StartMenu_MainTab_Off"),
			new ESCT_BoolValue(true, "#Escapists-StartMenu_MainTab_On")
		};

		for (int i = 0; i < vals.Count(); i++)
		{
			spin.AddItem(vals[i].m_sTitle, i == vals.Count() - 1, vals[i]);
		}

		spin.SetCurrentItem(0);
		OnWithstandChange(spin, 0);
	}
	
	private void OnCheckLocationsQuantity(ESCT_SpinBoxComponent spin, int index)
	{
		ESCT_FloatVal data = ESCT_FloatVal.Cast(spin.GetItemData(index));
		if (!data)
			return;

		m_Config.m_fDisabledLocationsRatio = data.m_fValue;
	}
	
	private void OnCheckAmbientFrequency(ESCT_SpinBoxComponent spin, int index)
	{
		ESCT_IntValue data = ESCT_IntValue.Cast(spin.GetItemData(index));
		if (!data)
			return;

		m_Config.m_iAmbientEventsFrequency = data.m_iValue;
	}

	private void OnCheckHealth(ESCT_SpinBoxComponent spin, int index)
	{
		ESCT_Health data = ESCT_Health.Cast(spin.GetItemData(index));
		if (!data)
			return;

		m_Config.m_eHealthType = data.m_eHealthType;
	}

	private void OnCheckAiSkill(ESCT_ToolboxComponent toolbox, int index)
	{
		ESCT_Skill data = ESCT_Skill.Cast(toolbox.GetItemData(index));
		if (!data)
			return;

		m_Config.m_eSkill = data.m_eSkill;
	}
	
	private void OnWithstandChange(ESCT_SpinBoxComponent spin, int index)
	{
		ESCT_BoolValue data = ESCT_BoolValue.Cast(spin.GetItemData(index));
		if (!data)
			return;

		m_Config.m_bUseWithstand = data.m_bValue;
	}
}
