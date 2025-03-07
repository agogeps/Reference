class ESCT_FadeEffect
{
#ifdef WORKBENCH
	private static const float FADE_DURATION = 0.01;
	private static const float BLACKSCREEN_DURATION = 0.01;
	private static const float FADE_TO_SCENE_DURATION = 0.01;
#else
	private static const float FADE_DURATION = 1.0;
	private static const float BLACKSCREEN_DURATION = 1.5;
	private static const float FADE_TO_SCENE_DURATION = 4.0;
#endif	

	ref ESCT_FadeEvent m_eFadeEvent;
	ref SCR_FadeInOutEffect m_Fade;
		
	void ESCT_FadeEffect(Widget rootWidget)
	{
		//this is an extremely clumsy way do this and I'm pretty sure that there's better way, but it's good for now
		m_Fade = new SCR_FadeInOutEffect();
		m_Fade.SetRootWidget(rootWidget);
		m_Fade.DisplayStartDraw(null); //it assigns actual fadeout image widget (dive into source code)
		
		m_eFadeEvent = new ESCT_FadeEvent();
	}
	
	void ~ESCT_FadeEffect()
	{
		delete m_Fade;
		delete m_eFadeEvent;
	}
	
	void FadeToBlack()
	{
		if (!m_Fade) 
		{
			Print("[Escapists] Fade effect doesn't exist, abort", LogLevel.ERROR);
			return;
		}
		m_Fade.FadeOutEffect(true, FADE_DURATION);
	
		
		// Wait for fade to black
		GetGame().GetCallqueue().CallLater(OnFade, (FADE_DURATION + BLACKSCREEN_DURATION) * 1000);	
	}
	
	private void OnFade()
	{
		if (m_eFadeEvent)
			m_eFadeEvent.OnScreenFade();
		
		GetGame().GetCallqueue().CallLater(ApplyFadeToScene, FADE_TO_SCENE_DURATION * 1000);
	}
	
	private void ApplyFadeToScene()
	{
		m_Fade.FadeOutEffect(false, FADE_DURATION);
	}
}
