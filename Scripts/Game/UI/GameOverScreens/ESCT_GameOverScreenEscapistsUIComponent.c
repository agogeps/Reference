class ESCT_GameOverScreenEscapistsUIComponent : SCR_GameOverScreenContentUIComponent
{
	[Attribute("GameOver_Time")]
	protected string m_sTime;

	[Attribute("GameOver_Escapees")]
	protected string m_sEscapees;

	[Attribute("GameOver_LeftBehinders")]
	protected string m_sLeftBehinders;

	override void InitContent(SCR_GameOverScreenUIContentData endScreenUIContent)
	{
		super.InitContent(endScreenUIContent);
		
		ChimeraWorld world = GetGame().GetWorld();
		if (world)
		{
			MusicManager musicManager = world.GetMusicManager();
			if (musicManager)
			{
				musicManager.MuteCategory(MusicCategory.Ambient, true, true);
			}
		}
		
		SetElapsedTime();
		SetPlayersAndTitle();
	}

	private void SetElapsedTime()
	{
		RichTextWidget timeWidget = RichTextWidget.Cast(m_wRoot.FindAnyWidget(m_sTime));

		SCR_BaseGameMode baseGameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!baseGameMode)
			return;

		int days, hours, minutes, seconds;
		SCR_DateTimeHelper.GetDayHourMinuteSecondFromSeconds(baseGameMode.GetElapsedTime(), days, hours, minutes, seconds);

		string timeElapsed = string.Format(WidgetManager.Translate("#AR-CareerProfile_TimePlayed_TotalValue", ""+days, ""+hours, ""+minutes, ""+seconds));

		if (timeWidget)
			timeWidget.SetText(timeElapsed);
	}

	private void SetPlayersAndTitle()
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return;

		ref array<int> escapees = {};
		ref array<int> leftBehinders = {};
		bool noMoreVehicles = false;

		ESCT_WinLoseHandlerComponent winLoseHandler = ESCT_WinLoseHandlerComponent.GetInstance();
		if (winLoseHandler)
			winLoseHandler.GetEndStates(escapees, leftBehinders, noMoreVehicles);
		
		if (noMoreVehicles)
		{	
			HidePlayerWidgets();
			return;
		}
		
		RichTextWidget escapeesWidget = RichTextWidget.Cast(m_wRoot.FindAnyWidget(m_sEscapees));

		string eText = GetPlayerString(playerManager, escapees);
		string lbText = GetPlayerString(playerManager, leftBehinders);

		if (escapeesWidget)
			escapeesWidget.SetText(eText);

		if (!leftBehinders.IsEmpty())
		{
			RichTextWidget leftBehindersWidget = RichTextWidget.Cast(m_wRoot.FindAnyWidget(m_sLeftBehinders));
			if (leftBehindersWidget) 
				leftBehindersWidget.SetText(lbText);
		}
		else
		{
			RichTextWidget leftBehindersHeader = RichTextWidget.Cast(m_wRoot.FindAnyWidget("GameOver_LeftBehindersHeader"));
			leftBehindersHeader.SetVisible(false);

			RichTextWidget leftBehindersWidget = RichTextWidget.Cast(m_wRoot.FindAnyWidget(m_sLeftBehinders));
			leftBehindersWidget.SetVisible(false);
		}
		
		SCR_BaseGameMode baseGameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		SCR_GameModeEndData endData = baseGameMode.GetEndGameData();
		
		if (endData.GetEndReason() == EGameOverTypes.ESCAPISTS_DEFEAT)
		{
			RichTextWidget escapeesHeader = RichTextWidget.Cast(m_wRoot.FindAnyWidget("GameOver_EscapeesHeader"));
			escapeesHeader.SetVisible(false);
		}
	}

	private string GetPlayerString(PlayerManager playerManager, array<int> playerIds)
	{
		if (!playerIds)
			return string.Empty;
		
		string result;
		for (int i = 0; i < playerIds.Count(); i++)
		{
			int playerId = playerIds[i];
			string nickname = playerManager.GetPlayerName(playerId);

			if (i != (playerIds.Count() - 1))
			{
				result += nickname + ", ";
			}
			else
			{
				result += nickname;
			}
		}

		return result;
	}
	
	private void HidePlayerWidgets()
	{
		RichTextWidget leftBehindersHeader = RichTextWidget.Cast(m_wRoot.FindAnyWidget("GameOver_LeftBehindersHeader"));
		leftBehindersHeader.SetVisible(false);
		
		RichTextWidget leftBehindersWidget = RichTextWidget.Cast(m_wRoot.FindAnyWidget(m_sLeftBehinders));
		leftBehindersWidget.SetVisible(false);
			
		RichTextWidget escapeesHeader = RichTextWidget.Cast(m_wRoot.FindAnyWidget("GameOver_EscapeesHeader"));
		escapeesHeader.SetVisible(false);
			
		RichTextWidget escapeesWidget = RichTextWidget.Cast(m_wRoot.FindAnyWidget(m_sEscapees));
		escapeesWidget.SetVisible(false);
	}
}
