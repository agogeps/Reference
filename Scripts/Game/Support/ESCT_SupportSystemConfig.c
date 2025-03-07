[BaseContainerProps(configRoot: true)]
class ESCT_SupportSystemConfig : ScriptAndConfig
{
	[Attribute("", UIWidgets.Auto, category: "Escapists")]
	protected ref array<ref ESCT_SupportInfo> m_Supports;
	
	array<ref ESCT_SupportInfo> GetSupportInfos()
	{
		return m_Supports;
	}
}
