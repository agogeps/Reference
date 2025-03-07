sealed class ESCT_SetupMenu : ESCT_MenuBase
{
	private static const string START_BUTTON_ID = "StartButton";
	private static const string ABOUT_BUTTON_ID = "AboutButton";

	private static const string TAB_VIEW_ID = "ContentTabView";
	private static const string TOOLTIP_ID = "TooltipText";

	private ref ESCT_SetupConfig m_Config;
	private ref map<string, string> m_TooltipMap;

	private ref array<ref ESCT_MenuTabBase> m_aTabs;

	override void OnMenuInit()
	{
		super.OnMenuInit();

		m_Config = ESCT_EscapistsConfigComponent.GetInstance().GetSetupConfig();
		m_TooltipMap = new map<string, string>();
		m_aTabs = {};

		//it's very important to create references (instead of putting it straight into InsertAll) otherwise it will be NULL-ed as reference counter will think that it's an orphan object
		ESCT_MainTab mainTab = new ESCT_MainTab(m_wRoot, m_TooltipMap, TOOLTIP_ID, m_Config);
		ESCT_EnvironmentTab envTab = new ESCT_EnvironmentTab(m_wRoot, m_TooltipMap, TOOLTIP_ID, m_Config);
		ESCT_DifficultyTab difficultyTab = new ESCT_DifficultyTab(m_wRoot, m_TooltipMap, TOOLTIP_ID, m_Config);

		m_aTabs.InsertAll({
			mainTab,
			envTab,
			difficultyTab
		});
	}

	override void OnMenuOpen()
	{
		super.OnMenuOpen();

		Math.Randomize(-1);

		foreach (ESCT_MenuTabBase tab : m_aTabs)
		{
			tab.Initialize();
		}

		SetStartButton();
		SetAboutButton();

		TextWidget tooltip = TextWidget.Cast(m_wRoot.FindAnyWidget(TOOLTIP_ID));
		if (!tooltip)
			return;

		tooltip.SetText("#Escapists-StartMenu_MainTab_EnemyFaction_Tooltip");
	}

	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);

		GetGame().GetInputManager().ActivateContext("EscapistsStartContext");
	}

	protected void SetStartButton()
	{
		Widget startButton = m_wRoot.FindAnyWidget(START_BUTTON_ID);
		SCR_InputButtonComponent action = SCR_InputButtonComponent.Cast(startButton.FindHandler(SCR_InputButtonComponent));

		if (action)
		{
			action.m_OnActivated.Insert(StartGame);
		}
	}

	protected void SetAboutButton()
	{
		Widget testButton = m_wRoot.FindAnyWidget("AboutButton");
		SCR_InputButtonComponent action = SCR_InputButtonComponent.Cast(testButton.FindHandler(SCR_InputButtonComponent));

		if (action)
			action.m_OnActivated.Insert(OpenAboutMenu);
	}

	private void OpenAboutMenu()
	{
		GetGame().GetMenuManager().CloseMenu(this);
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.EscapistsAboutMenu);
	}

	private void StartGame()
	{
		GetGame().GetMenuManager().CloseMenu(this);

		ESCT_EscapistsNetworkComponent networkComponent = ESCT_EscapistsNetworkComponent.GetInstance();
		if (!networkComponent)
			return;

		networkComponent.StartGame(m_Config);
	}
}
