sealed class ESCT_AboutMenu : ChimeraMenuBase
{
	private static const string BACK_BUTTON_ID = "BackButton";
	private static const string VERSION_TEXT_ID = "VersionText";
	
	protected Widget m_wRoot;
	
	override void OnMenuInit()
	{
		m_wRoot = GetRootWidget();
	}
	
	override void OnMenuOpen()
	{
		super.OnMenuOpen();

		TextWidget version = TextWidget.Cast(m_wRoot.FindAnyWidget(VERSION_TEXT_ID));
		if (version)
			version.SetText(ESCT_Constants.ESCAPISTS_VERSION);
		
		Widget backButton = m_wRoot.FindAnyWidget(BACK_BUTTON_ID);
		SCR_InputButtonComponent action = SCR_InputButtonComponent.Cast(backButton.FindHandler(SCR_InputButtonComponent));

		if (action)
			action.m_OnActivated.Insert(BackToStartMenu);
	}
	
	private void BackToStartMenu()
	{
		GetGame().GetMenuManager().CloseMenu(this);
		
		int playerID = GetGame().GetPlayerController().GetPlayerId();
		if (SCR_Global.IsAdmin(playerID))
			GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.EscapistsSetupMenu);
	}
}
