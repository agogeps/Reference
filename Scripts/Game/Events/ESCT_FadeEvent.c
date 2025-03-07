class ESCT_FadeEvent
{
	protected ref ScriptInvokerVoid m_OnScreenFade;

	ScriptInvokerVoid GetOnScreenFade()
	{
		if (!m_OnScreenFade)
		{
			m_OnScreenFade = new ScriptInvokerVoid();
		}
		
		return m_OnScreenFade;
	}
	
	void OnScreenFade()
	{
		if (!m_OnScreenFade) return;
		
		m_OnScreenFade.Invoke();
	}
}
