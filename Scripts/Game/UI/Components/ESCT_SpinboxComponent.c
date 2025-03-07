class ESCT_SpinBoxComponent : SCR_SpinBoxComponent
{
	ref WidgetFocusInvoker m_OnFocus = new WidgetFocusInvoker();
	ref WidgetFocusInvoker m_OnFocusLost = new WidgetFocusInvoker();
	
	override bool OnFocus(Widget w, int x, int y)
	{
		super.OnFocus(w, x, y);
		
		m_OnFocus.Invoke(w);
		
		return false;
	}
	
	override bool OnFocusLost(Widget w, int x, int y)
	{
		super.OnFocusLost(w, x, y);
		
		m_OnFocusLost.Invoke(w);
		
		return false;
	}
}
