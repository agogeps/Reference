[EntityEditorProps(category: "Escapists/PlayerController", description: "Handles notifications, music playback and other local feedback things.", color: "0 0 255 255")]
class ESCT_EscapistsFeedbackComponentClass : ScriptComponentClass
{
}

class ESCT_EscapistsFeedbackComponent : ScriptComponent
{
	[Attribute("{E97099EEB6A3E0E3}Configs/Hints/EscapistsHints.conf", params: "conf class=ESCT_EscapistsHintStorage")]
	protected ResourceName m_sHintsConfig;

	protected ref array<int> m_aShownHints = {};
	protected ref array<int> m_aHintQueue = {};
	
	protected ref ESCT_EscapistsHintStorage m_HintsConfig;	
	protected SCR_PlayerController m_PlayerController;
	protected ESCT_EscapistsGameMode m_Escapists;
	
	protected float m_fNextAllowedHintTimestamp;	
	protected MenuBase m_SetupMenu;

	protected static const int AFTER_RESPAWN_HINT_DELAY_MS = 4000;
	protected static const int DELAY_BETWEEN_HINTS_MS = 1000;
	protected static const int FEATURE_HINT_DELAY = 120000;

	//------------------------------------------------------------------------------------------------
	//! \return
	static ESCT_EscapistsFeedbackComponent GetInstance()
	{
		PlayerController pc = GetGame().GetPlayerController();

		if (!pc)
			return null;

		return ESCT_EscapistsFeedbackComponent.Cast(pc.FindComponent(ESCT_EscapistsFeedbackComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (!GetGame().InPlayMode())
			return;

		SetEventMask(owner, EntityEvent.INIT);

		m_PlayerController = SCR_PlayerController.Cast(owner);

		if (RplSession.Mode() == RplMode.Dedicated)
			return;

		//Parse & register hints list
		Resource container = BaseContainerTools.LoadContainer(m_sHintsConfig);
		m_HintsConfig = ESCT_EscapistsHintStorage.Cast(BaseContainerTools.CreateInstanceFromContainer(container.GetResource().ToBaseContainer()));
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (!m_PlayerController)
			return;

		m_Escapists = ESCT_EscapistsGameMode.GetGameMode();
		m_PlayerController.GetOnOwnershipChangedInvoker().Insert(OnOwnershipChanged);

		RplComponent rpl = RplComponent.Cast(m_PlayerController.FindComponent(RplComponent));
		if (rpl && rpl.IsOwner())
			ProcessEvents(true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] isVisible
	void SetSetupMenuVisibility(bool isVisible)
	{
		if (!SCR_Global.IsAdmin())
			return;

		m_SetupMenu = GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.EscapistsSetupMenu);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] changing
	//! \param[in] becameOwner
	void OnOwnershipChanged(bool changing, bool becameOwner)
	{
		if (changing)
			return;

		ProcessEvents(becameOwner);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Hints are displayed with a delay after respawn so player has time to find their bearings
	void OnRespawn()
	{
		m_fNextAllowedHintTimestamp = 0;
		ProcessHintQueue();
		
		ESCT_EscapistsGameMode gameMode = ESCT_EscapistsGameMode.GetGameMode();
		if (!gameMode)
			return;
		
		ESCT_EStartType startType = gameMode.GetStartType();
		EHint welcomeHint;
		
		switch (startType)
		{
			case ESCT_EStartType.Prison:
				welcomeHint = EHint.ESCAPISTS_WELCOME_PRISON;
				break;
			case ESCT_EStartType.Hideout:
				welcomeHint = EHint.ESCAPISTS_WELCOME_HIDEOUT;
				break;
			case ESCT_EStartType.Helicrash:
				welcomeHint = EHint.ESCAPISTS_WELCOME_HELICRASH;
				break;
		}

		if (!m_aShownHints.Contains(welcomeHint))
			GetGame().GetCallqueue().CallLater(ShowHint, AFTER_RESPAWN_HINT_DELAY_MS, repeat: false, welcomeHint, true, false);
	}
	
	protected void ProcessEvents(bool activate)
	{
		if (RplSession.Mode() == RplMode.Dedicated)
			return;

		if (activate)
		{
			if (m_Escapists)
			{
				m_Escapists.GetEscapistsManager().GetOnDefendRadioStart().Insert(OnDefendRadio);
				m_Escapists.GetEscapistsManager().GetOnApproachHelipad().Insert(OnApproachHelipad);
				m_Escapists.GetEscapistsManager().GetOnHelicopterEntered().Insert(OnHelicopterEntered);
				m_Escapists.GetEscapistsManager().GetOnRunStart().Insert(OnRunStart);
			}
		}
		else
		{
			if (m_Escapists)
			{
				m_Escapists.GetEscapistsManager().GetOnDefendRadioStart().Remove(OnDefendRadio);
				m_Escapists.GetEscapistsManager().GetOnApproachHelipad().Remove(OnApproachHelipad);
				m_Escapists.GetEscapistsManager().GetOnHelicopterEntered().Remove(OnHelicopterEntered);
				m_Escapists.GetEscapistsManager().GetOnRunStart().Remove(OnRunStart);
			}
		}
	}
	
	protected void ShowHint(EHint hintID, bool showImmediately = false, bool showMultipleTimes = false)
	{
		if (RplSession.Mode() == RplMode.Dedicated)
			return;

		SCR_HintUIInfo info = m_HintsConfig.GetHintByEnum(hintID);

		if (!info)
			return;

		// Currently we want no limit on the amount of times these hints can be displayed (in multiple matches)
		bool showAlways = true;

#ifdef WORKBENCH
		showAlways = true;
#endif

		if (!showMultipleTimes && (m_aShownHints.Contains(hintID) || m_aHintQueue.Contains(hintID)))
			return;

		float currentTime = GetGame().GetWorld().GetWorldTime();

		if (currentTime < m_fNextAllowedHintTimestamp && !showImmediately)
		{
			m_aHintQueue.Insert(hintID);
		}
		else
		{
			m_aShownHints.Insert(hintID);
			bool isShown = SCR_HintManagerComponent.ShowHint(info, ignoreShown: showAlways);
			
			// Show the next hint in queue after this hint's duration expires
			float durationMs = 1000 * info.GetDuration();
			m_fNextAllowedHintTimestamp = currentTime + durationMs;
			GetGame().GetCallqueue().Remove(ProcessHintQueue);
			GetGame().GetCallqueue().CallLater(ProcessHintQueue, durationMs + DELAY_BETWEEN_HINTS_MS);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ProcessHintQueue()
	{
		if (m_aHintQueue.IsEmpty())
			return;

		int hintId = m_aHintQueue[0];
		m_aHintQueue.RemoveOrdered(0);

		ShowHint(hintId, showMultipleTimes: true);
	}

	protected void PauseHintQueue()
	{
		GetGame().GetCallqueue().Remove(ProcessHintQueue);
	}
	
	private void OnApproachHelipad()
	{
		GetGame().GetCallqueue().CallLater(ShowHint, AFTER_RESPAWN_HINT_DELAY_MS, repeat: false, EHint.ESCAPISTS_HELIPAD_REACH, true, false);
	}
	
	private void OnHelicopterEntered()
	{
		GetGame().GetCallqueue().CallLater(ShowHint, AFTER_RESPAWN_HINT_DELAY_MS, repeat: false, EHint.ESCAPISTS_FLY_AWAY, true, false);
	}
	
	private void OnDefendRadio()
	{
		GetGame().GetCallqueue().CallLater(ShowHint, AFTER_RESPAWN_HINT_DELAY_MS, repeat: false, EHint.ESCAPISTS_DEFEND_RADIO, true, false);
	}
	
	private void OnRunStart()
	{
		GetGame().GetCallqueue().CallLater(ShowHint, AFTER_RESPAWN_HINT_DELAY_MS, repeat: false, EHint.ESCAPISTS_GOALS, true, false);
	}
}
