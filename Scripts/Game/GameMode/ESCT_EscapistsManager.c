class ESCT_EscapistsManager
{
	protected ESCT_EscapistsGameMode m_Escapists;
	
	protected ref ScriptInvokerVoid m_OnRunStartInvoker;
	protected ref ScriptInvokerVoid m_OnDefendRadioStartInvoker;
	protected ref ScriptInvokerVoid m_OnApproachHelipadInvoker;
	protected ref ScriptInvokerVoid m_OnHeicopterEnteredInvoker;
	
	ScriptInvokerVoid GetOnRunStart()
	{
		if (!m_OnRunStartInvoker)
			m_OnRunStartInvoker = new ScriptInvokerVoid();
		
		return m_OnRunStartInvoker;
	}
	
	ScriptInvokerVoid GetOnDefendRadioStart()
	{
		if (!m_OnDefendRadioStartInvoker)
			m_OnDefendRadioStartInvoker = new ScriptInvokerVoid();
		
		return m_OnDefendRadioStartInvoker;
	}
	
	ScriptInvokerVoid GetOnApproachHelipad()
	{
		if (!m_OnApproachHelipadInvoker)
			m_OnApproachHelipadInvoker = new ScriptInvokerVoid();
		
		return m_OnApproachHelipadInvoker;
	}
	
	ScriptInvokerVoid GetOnHelicopterEntered()
	{
		if (!m_OnHeicopterEnteredInvoker)
			m_OnHeicopterEnteredInvoker = new ScriptInvokerVoid();
		
		return m_OnHeicopterEnteredInvoker;
	}
	
	void ESCT_EscapistsManager(notnull ESCT_EscapistsGameMode escapists)
	{
		m_Escapists = escapists;
	}
}
