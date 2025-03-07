[EntityEditorProps(category: "Escapists/Entities", description: "Network-friendly HUD with counter used by defend radio task.")]
class ESCT_DefendRadioTaskPresentationClass : GenericEntityClass
{
}

sealed class ESCT_DefendRadioTaskPresentation : GenericEntity
{
	[Attribute(defvalue: "{47864BB47AB0B1F4}UI/layouts/HUD/CampaignMP/CampaignMainHUD.layout", category: "Defend params")]
	protected ResourceName m_sCountdownHUD;

	[Attribute(uiwidget: UIWidgets.LocaleEditBox, desc: "Text that will be displayed above the countdown number", category: "Defend params")]
	protected LocalizedString m_sCountdownTitleText;

	[Attribute(uiwidget: UIWidgets.LocaleEditBox, desc: "Text that will be displayed to inform players that they have to eliminate all attacker units", category: "Defend params")]
	protected LocalizedString m_sFinishText;

	[Attribute(defvalue: "300", UIWidgets.Slider, desc: "For how long you have to Defend the objective of the task. Value -1 is for indefinitely.", params: "-1 86400 1", category: "Defend params")]
	protected float m_fDefendTime;

	protected string m_sFormattedCountdownTitle = string.Format(WidgetManager.Translate("<color rgba=\"255, 255, 255, 255\">%1</color>", m_sCountdownTitleText));
	protected string m_sFormattedFinishTitle = string.Format(WidgetManager.Translate("<color rgba=\"226, 168, 80, 255\">%1</color>", m_sFinishText));

#ifdef WORKBENCH
	protected float m_fTempCountdown = 15;
#else
	protected float m_fTempCountdown = m_fDefendTime;
#endif

	protected float m_fDefaultAlpha;
	protected float m_fTempTimeSlice;
	protected Widget m_wRoot;
	protected Widget m_wCountdownOverlay;
	protected RichTextWidget m_wCountdown;
	protected RichTextWidget m_wFlavour;
	protected ProgressBarWidget m_wProgressBar;

	private ESCT_DefendRadioTask m_Task;
	private ESCT_DefendRadioTaskSupportEntity m_SupportEntity;

	private static const float FADE_DURATION = 0.5;

	void ESCT_DefendRadioTaskPresentation(IEntitySource src, IEntity parent)
	{
		if (!GetGame().InPlayMode())
			return;

		if (Replication.IsServer())
		{
			SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);
		}
		else
		{
			SetEventMask(EntityEvent.INIT);
		}
	}

	void ~ESCT_DefendRadioTaskPresentation()
	{
		if (!m_wRoot)
			return;

		if (m_wRoot.IsVisible())
			m_wRoot.SetVisible(false);

		array<Widget> widgets = {m_wCountdownOverlay, m_wCountdown, m_wFlavour, m_wProgressBar, m_wRoot};
		foreach (Widget widget : widgets)
		{
			widget.RemoveFromHierarchy();
		}
	}

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (!GetGame().InPlayMode())
			return;

		m_SupportEntity = ESCT_DefendRadioTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(ESCT_DefendRadioTaskSupportEntity));
		m_Task = ESCT_DefendRadioTask.Cast(m_SupportEntity.GetTaskByType(ESCT_DefendRadioTask));
	}

	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);

		m_fTempTimeSlice += timeSlice;
		if (m_fTempTimeSlice >= 1 && m_fTempCountdown >= 0)
		{
			UpdateHUD();
			return;
		}

		EvaluateTaskConditions();
	}

	void Setup()
	{
		if (!Replication.IsServer())
			return;

		InitHUD();
	}

	void Clear()
	{
		Rpc_DoClear();
		Rpc(Rpc_DoClear);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	private void Rpc_DoClear()
	{
		if (!m_wRoot)
			return;

		//m_wRoot.SetVisible(false);
		FadeWidget(m_wRoot, true);
		m_wRoot.RemoveFromHierarchy();
		m_wRoot = null;
	}

	private void InitHUD()
	{
		SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();
		if (!hudManager)
			return;

		m_wRoot = hudManager.CreateLayout(m_sCountdownHUD, EHudLayers.MEDIUM, 0);
		if (!m_wRoot)
			return;
		
		if (!m_Task)
			m_Task = ESCT_DefendRadioTask.Cast(m_SupportEntity.GetTaskByType(ESCT_DefendRadioTask));

		m_wCountdownOverlay = m_wRoot.FindAnyWidget("Countdown");
		m_wProgressBar = ProgressBarWidget.Cast(m_wRoot.FindAnyWidget("Progress"));
		m_wCountdown = RichTextWidget.Cast(m_wRoot.FindAnyWidget("CountdownWin"));
		m_wFlavour = RichTextWidget.Cast(m_wRoot.FindAnyWidget("FlavourText"));
		ImageWidget winScoreSideLeft = ImageWidget.Cast(m_wRoot.FindAnyWidget("ObjectiveLeft"));
		ImageWidget winScoreSideRight = ImageWidget.Cast(m_wRoot.FindAnyWidget("ObjectiveRight"));

		string shownTime = SCR_FormatHelper.GetTimeFormatting(m_fTempCountdown, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS | ETimeFormatParam.MINUTES);
		m_wCountdown.SetText(shownTime);
		m_wProgressBar.SetCurrent(0);

		m_wFlavour.SetText(m_sFormattedCountdownTitle);
		m_wRoot.SetVisible(true);
		m_fDefaultAlpha = m_wRoot.GetColor().A();
		FadeWidget(m_wRoot);
	}

	private void UpdateHUD()
	{
		m_fTempTimeSlice = 0;
		m_fTempCountdown--;

		Rpc(RpcDo_UpdateHUD, m_fTempCountdown);

		if (!m_wRoot)
			return;

		if (m_fTempCountdown < 0)
		{
			m_wFlavour.SetText(m_sFinishText);
			m_wRoot.SetVisible(true);
			m_wCountdownOverlay.SetVisible(false);
			return;
		}

		if (m_Task && (m_Task.GetTaskState() == SCR_TaskState.CANCELLED || m_Task.GetTaskState() == SCR_TaskState.FINISHED))
		{
			m_wRoot.SetVisible(false);
			return;
		}

		string shownTime = SCR_FormatHelper.GetTimeFormatting(m_fTempCountdown, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS | ETimeFormatParam.MINUTES);
		m_wCountdown.SetText(shownTime);
		m_wProgressBar.SetCurrent(1 - m_fTempCountdown / m_fDefendTime);

		m_wFlavour.SetText(m_sFormattedCountdownTitle);
		m_wRoot.SetVisible(true);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_UpdateHUD(float countdown)
	{
		m_fTempCountdown = countdown;

		if (!m_wRoot)
		{
			InitHUD();
			if (!m_wRoot)
				return;
		}

		if (m_fTempCountdown < 0)
		{
			m_wFlavour.SetText(m_sFinishText);
			m_wRoot.SetVisible(true);
			m_wCountdownOverlay.SetVisible(false);
			return;
		}

		if (m_Task && (m_Task.GetTaskState() == SCR_TaskState.CANCELLED || m_Task.GetTaskState() == SCR_TaskState.FINISHED))
		{
			m_wRoot.SetVisible(false);
			return;
		}

		string shownTime = SCR_FormatHelper.GetTimeFormatting(m_fTempCountdown, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS | ETimeFormatParam.MINUTES);
		m_wCountdown.SetText(shownTime);
		m_wProgressBar.SetCurrent(1 - m_fTempCountdown / m_fDefendTime);

		m_wFlavour.SetText(m_sFormattedCountdownTitle);
		m_wRoot.SetVisible(true);
	}

	private void EvaluateTaskConditions()
	{
		if (m_fTempCountdown > 0)
			return;

		ClearEventMask(EntityEvent.FRAME);
		ClearFlags(EntityFlags.ACTIVE);
		UpdateHUD();

		ESCT_Radio radio = m_SupportEntity.GetRadio();
		if (!radio)
		{
			ESCT_Logger.ErrorFormat("Defend Radio Task entity has no reference to radio!", this.ToString());
			return;
		}

		radio.SetRadioState(ESCT_RadioState.ReadyForResponse);
	}

	private void FadeWidget(notnull Widget widget, bool fadeOut = false)
	{
		float alpha, targetAlpha;

		if (fadeOut)
		{
			alpha = m_fDefaultAlpha;
			targetAlpha = 0;
		}
		else
		{
			alpha = 0;
			targetAlpha = m_fDefaultAlpha;
		}

		widget.SetOpacity(alpha);
		AnimateWidget.Opacity(widget, targetAlpha, FADE_DURATION, !fadeOut || widget.IsVisible());
	}
}
