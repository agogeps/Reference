class ESCT_UiContext : ScriptAndConfig
{
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Layout to show", params: "layout")]
	ResourceName m_Layout;

	protected IEntity m_Owner;
	protected Widget m_wRoot;

	void Init(IEntity owner)
	{
		m_Owner = owner;
	}

	Widget GetRootWidget()
	{
		return m_wRoot;
	}

	void ShowLayout()
	{
		if (!m_Layout)
		{
			Print("No Layout!", LogLevel.ERROR);
			return;
		}

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		m_wRoot = workspace.CreateWidgets(m_Layout);
	}

	void HideLayout()
	{
		if (!m_wRoot)
			return;

		m_wRoot.RemoveFromHierarchy();
	}
}
