class ESCT_MenuBase : ChimeraMenuBase
{
	protected Widget m_wRoot;
	
	protected SCR_InputButtonComponent m_BackButton;
	protected SCR_InputButtonComponent m_ChatButton;
	protected SCR_ChatPanel m_ChatPanel;
	
	override void OnMenuInit()
	{
		super.OnMenuInit();
		
		m_wRoot = GetRootWidget();
	}
	
	override void OnMenuOpen()
	{
		super.OnMenuOpen();

		InitializeBasicControls();
	}
	
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		
		if (m_ChatPanel)
			m_ChatPanel.OnUpdateChat(tDelta);
	}
	
	private void InitializeBasicControls()
	{
		Widget chat = GetRootWidget().FindAnyWidget("ChatPanel");
		if (chat)
			m_ChatPanel = SCR_ChatPanel.Cast(chat.FindHandler(SCR_ChatPanel));

		m_ChatButton = SCR_InputButtonComponent.GetInputButtonComponent("ChatButton", m_wRoot);
		if (m_ChatButton)
			m_ChatButton.m_OnActivated.Insert(OnChatToggle);
		
		m_BackButton = SCR_InputButtonComponent.GetInputButtonComponent("BackButton", m_wRoot);
		if (m_BackButton)
			m_BackButton.m_OnActivated.Insert(ReturnToMainMenu);
	}
	
	private void ReturnToMainMenu()
	{
		SCR_ConfigurableDialogUi dlg = SCR_CommonDialogs.CreateDialog("scenario_exit");
		if (!dlg)
			return;
		
		dlg.m_OnConfirm.Insert(BackToMainMenuPopupComfirm);
	}
	
	private void BackToMainMenuPopupComfirm()
	{
		Close();
		GameStateTransitions.RequestGameplayEndTransition();
	}
	
	private void OnChatToggle()
	{
		if (!m_ChatPanel)
		{
			Widget chat = GetRootWidget().FindAnyWidget("ChatPanel");
			if (chat)
				m_ChatPanel = SCR_ChatPanel.Cast(chat.FindHandler(SCR_ChatPanel));
		}
		
		if (!m_ChatPanel || m_ChatPanel.IsOpen())
			return;

		SCR_ChatPanelManager chatPanelManager = SCR_ChatPanelManager.GetInstance();
		
		if (!chatPanelManager)
			return;
		
		SCR_ChatPanelManager.GetInstance().ToggleChatPanel(m_ChatPanel);
	}
}
