sealed class ESCT_MainTab : ESCT_MenuTabBase
{
	private static const string ENEMY_FACTION_TOOLBOX_ID = "EnemyFactionToolbox";
	private static const string PLAYER_FACTION_TOOLBOX_ID = "PlayerFactionToolbox";
	private static const string INSURGENCY_SPINBOX_ID = "InsurgencySpinbox";
	private static const string CIVILIAN_PRESENCE_ID = "CivilianPresenceSpinbox";
	private static const string START_TYPE_ID = "StartTypeToolbox";

	void ESCT_MainTab(Widget root, map<string, string> tooltipMap, string tooltipId, ESCT_SetupConfig setupConfig)
	{
		m_TooltipMap.Insert(ENEMY_FACTION_TOOLBOX_ID, "#Escapists-StartMenu_MainTab_EnemyFaction_Tooltip");
		m_TooltipMap.Insert(PLAYER_FACTION_TOOLBOX_ID, "#Escapists-StartMenu_MainTab_PlayerFaction_Tooltip");
		m_TooltipMap.Insert(INSURGENCY_SPINBOX_ID, "#Escapists-StartMenu_MainTab_Insurgency_Tooltip");
		m_TooltipMap.Insert(CIVILIAN_PRESENCE_ID, "#Escapists-StartMenu_MainTab_CivilianPresence_Tooltip");
		m_TooltipMap.Insert(START_TYPE_ID, "#Escapists-StartMenu_MainTab_CivilianPresence_Tooltip");
	}

	override void Initialize()
	{
		SetPlayerFactionToolbox();
		SetEnemyFactionToolbox();
		SetCivilianPresenceSpinbox();
		SetInsurgencyToolbox();
		SetStartTypeToolbox();
	}

	protected void SetEnemyFactionToolbox()
	{
		Widget of = m_wRoot.FindAnyWidget(ENEMY_FACTION_TOOLBOX_ID);
		ESCT_ToolboxComponent toolbox = ESCT_ToolboxComponent.Cast(of.FindHandler(ESCT_ToolboxComponent));
		toolbox.ClearAll();
		toolbox.m_OnChanged.Insert(OnCheckEnemyFaction);
		toolbox.m_OnFocus.Insert(OnWidgetFocus);

		string tooltipText = m_TooltipMap.Get(ENEMY_FACTION_TOOLBOX_ID);
		if (tooltipText)
		{
			TextWidget tooltip = TextWidget.Cast(m_wRoot.FindAnyWidget(m_sTooltipId));
			if (tooltip)
				tooltip.SetText(tooltipText);
		}

		FactionManager factionManager = GetGame().GetFactionManager();

		if (!factionManager)
			return;

		array<Faction> factions = {};
		factionManager.GetFactionsList(factions);
		for (int i = 0; i < factions.Count(); i++)
		{
			ESCT_Faction faction = ESCT_Faction.Cast(factions[i]);
			if (!faction || !faction.IsPlayable())
				factions.Remove(i);
		}

		for (int i=0; i < factions.Count(); i++)
		{
			Faction faction = factions[i];
			if (!faction)
				continue;

			toolbox.AddItem(faction.GetFactionName(), i == factions.Count() - 1, faction);
		}

		array<SCR_ButtonBaseComponent> items = {};
		toolbox.GetItems(items);

		ESCT_UI.FillFactionToolbox(toolbox);

		toolbox.SetCurrentItem(0);
		OnCheckEnemyFaction(toolbox, 0);
	}

	protected void SetPlayerFactionToolbox()
	{
		Widget of = m_wRoot.FindAnyWidget(PLAYER_FACTION_TOOLBOX_ID);
		ESCT_ToolboxComponent toolbox = ESCT_ToolboxComponent.Cast(of.FindHandler(ESCT_ToolboxComponent));
		toolbox.ClearAll();
		toolbox.m_OnChanged.Insert(OnCheckPlayerFaction);
		toolbox.m_OnFocus.Insert(OnWidgetFocus);

		string tooltipText = m_TooltipMap.Get(PLAYER_FACTION_TOOLBOX_ID);
		if (tooltipText)
		{
			TextWidget tooltip = TextWidget.Cast(m_wRoot.FindAnyWidget(m_sTooltipId));
			if (tooltip)
				tooltip.SetText(tooltipText);
		}

		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;

		array<Faction> factions = {};
		factionManager.GetFactionsList(factions);
		for (int i=0; i < factions.Count(); i++)
		{
			ESCT_Faction faction = ESCT_Faction.Cast(factions[i]);
			if (!faction || !faction.IsPlayable())
				continue;

			toolbox.AddItem(faction.GetFactionName(), i == factions.Count() - 1, faction);
		}

		array<SCR_ButtonBaseComponent> items = {};
		toolbox.GetItems(items);

		ESCT_UI.FillFactionToolbox(toolbox);

		toolbox.SetCurrentItem(1);
		OnCheckPlayerFaction(toolbox, 1);
	}
	
	protected void SetCivilianPresenceSpinbox()
	{
		Widget of = m_wRoot.FindAnyWidget(CIVILIAN_PRESENCE_ID);
		ESCT_SpinBoxComponent spin = ESCT_SpinBoxComponent.Cast(of.FindHandler(ESCT_SpinBoxComponent));
		spin.ClearAll();
		spin.m_OnChanged.Insert(OnCivilianPresenceChange);
		spin.m_OnFocus.Insert(OnWidgetFocus);

		array<ref ESCT_BoolValue> qts =
		{
			new ESCT_BoolValue(false, "#Escapists-StartMenu_MainTab_Off"),
			new ESCT_BoolValue(true, "#Escapists-StartMenu_MainTab_On")
		};

		for (int i = 0; i < qts.Count(); i++)
		{
			spin.AddItem(qts[i].m_sTitle, i == qts.Count() - 1, qts[i]);
		}

		spin.SetCurrentItem(1);
		OnCivilianPresenceChange(spin, 1);
	}

	protected void SetInsurgencyToolbox()
	{
		Widget of = m_wRoot.FindAnyWidget(INSURGENCY_SPINBOX_ID);
		ESCT_SpinBoxComponent toolbox = ESCT_SpinBoxComponent.Cast(of.FindHandler(ESCT_SpinBoxComponent));
		toolbox.ClearAll();
		toolbox.m_OnChanged.Insert(OnInsurgencyChange);
		toolbox.m_OnFocus.Insert(OnWidgetFocus);

		array<ref ESCT_BoolValue> qts =
		{
			new ESCT_BoolValue(false, "#Escapists-StartMenu_MainTab_Off"),
			new ESCT_BoolValue(true, "#Escapists-StartMenu_MainTab_On")
		};

		for (int i = 0; i < qts.Count(); i++)
		{
			toolbox.AddItem(qts[i].m_sTitle, i == qts.Count() - 1, qts[i]);
		}

		toolbox.SetCurrentItem(1);
		OnInsurgencyChange(toolbox, 1);
	}
	
	protected void SetStartTypeToolbox()
	{
		Widget of = m_wRoot.FindAnyWidget(START_TYPE_ID);
		ESCT_ToolboxComponent toolbox = ESCT_ToolboxComponent.Cast(of.FindHandler(ESCT_ToolboxComponent));
		toolbox.ClearAll();
		toolbox.m_OnChanged.Insert(OnStartTypeChange);
		toolbox.m_OnFocus.Insert(OnWidgetFocus);

		array<ref ESCT_IntValue> qts =
		{
			new ESCT_IntValue(ESCT_EStartType.Prison, "#Escapists-StartMenu_MainTab_StartType_Prison"),
			new ESCT_IntValue(ESCT_EStartType.Hideout, "#Escapists-StartMenu_MainTab_StartType_Hideout"),
			new ESCT_IntValue(ESCT_EStartType.Helicrash, "#Escapists-StartMenu_MainTab_StartType_Helicrash"),
			new ESCT_IntValue(ESCT_EStartType.Random, "#Escapists-StartMenu_MainTab_StartType_Random")
		};

		for (int i = 0; i < qts.Count(); i++)
		{
			toolbox.AddItem(qts[i].m_sTitle, i == qts.Count() - 1, qts[i]);
		}

		int index = qts.Count() - 1;
		toolbox.SetCurrentItem(index);
		OnStartTypeChange(toolbox, index);
	}
	
	private void OnStartTypeChange(ESCT_ToolboxComponent toolbox, int index)
	{
		ESCT_IntValue data = ESCT_IntValue.Cast(toolbox.GetItemData(index));
		if (!data)
			return;

		m_Config.m_eStartType = data.m_iValue;
	}

	private void OnInsurgencyChange(ESCT_SpinBoxComponent toolbox, int index)
	{
		ESCT_BoolValue data = ESCT_BoolValue.Cast(toolbox.GetItemData(index));
		if (!data)
			return;

		m_Config.m_bInsurgency = data.m_bValue;
	}

	private void OnCivilianPresenceChange(ESCT_SpinBoxComponent spin, int index)
	{
		ESCT_BoolValue data = ESCT_BoolValue.Cast(spin.GetItemData(index));
		if (!data)
			return;

		m_Config.m_bCivilianPresence = data.m_bValue;
	}

	private void OnCheckEnemyFaction(ESCT_ToolboxComponent toolbox, int index)
	{
		ESCT_Faction data = ESCT_Faction.Cast(toolbox.GetItemData(index));
		if (!data)
			return;

		m_Config.m_sEnemyFactionKey = data.GetFactionKey();

		Widget of = m_wRoot.FindAnyWidget(PLAYER_FACTION_TOOLBOX_ID);
		ESCT_ToolboxComponent playerToolbox = ESCT_ToolboxComponent.Cast(of.FindHandler(ESCT_ToolboxComponent));
		if (!playerToolbox)
			return;

		array<SCR_ButtonBaseComponent> items = {};
		playerToolbox.GetItems(items);

		map<string, string> forbiddenFactions = new map<string, string>();
		forbiddenFactions.Insert(m_Config.m_sEnemyFactionKey, string.Empty);
		foreach (string factionKey : data.GetExcludedPlayerFactionIds())
		{
			forbiddenFactions.Insert(factionKey, string.Empty);
		}

		array<int> skipIndexes = {};

		for (int i=0; i < items.Count(); i++)
		{
			Faction playerData = Faction.Cast(playerToolbox.GetItemData(i));
			if (!playerData)
				continue;

			SCR_ButtonBaseComponent item = items[i];

			if (forbiddenFactions.Contains(playerData.GetFactionKey()))
			{
				skipIndexes.Insert(i);
				item.SetEnabled(false);
			}
		}

		for (int i=0; i < items.Count(); i++)
		{
			if (skipIndexes.Contains(i))
				continue;

			SCR_ButtonBaseComponent item = items[i];
			if (!item.IsEnabled())
			{
				item.SetEnabled(true);
			}

			playerToolbox.SetCurrentItem(i);
			OnCheckPlayerFaction(playerToolbox, i);
		}
	}

	private void OnCheckPlayerFaction(ESCT_ToolboxComponent toolbox, int index)
	{
		Faction data = Faction.Cast(toolbox.GetItemData(index));
		if (!data)
			return;

		m_Config.m_sPlayerFactionKey = data.GetFactionKey();
	}
	
}
