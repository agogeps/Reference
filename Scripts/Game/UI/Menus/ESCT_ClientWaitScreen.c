sealed class ESCT_ClientWaitScreen : ESCT_MenuBase
{	
	override void OnMenuInit()
	{
		super.OnMenuInit();
		
		SCR_RespawnSystemComponent respawnSystem = SCR_RespawnSystemComponent.GetInstance();
		if (respawnSystem)
		{
			ESCT_SpawnLogic spawnLogic = ESCT_SpawnLogic.Cast(respawnSystem.GetSpawnLogic());
			if (spawnLogic)
				spawnLogic.GetOnPlayerSpawned().Insert(ForceCloseOnSpawn);
		}
	}
	
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		SetAboutButton();
	}
	
	private void SetAboutButton()
	{
		Widget testButton = m_wRoot.FindAnyWidget("AboutButton");
		SCR_InputButtonComponent action = SCR_InputButtonComponent.Cast(testButton.FindHandler(SCR_InputButtonComponent));

		if (action)
			action.m_OnActivated.Insert(OpenAboutMenu);
	}
	
	private void OpenAboutMenu()
	{
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.EscapistsAboutMenu);
	}
	
	private void ForceCloseOnSpawn()
	{
		GetGame().GetMenuManager().CloseMenu(this);
	}
}
