[BaseContainerProps(configRoot: true)]
class ESCT_AmbientSystemConfig : ScriptAndConfig
{
	[Attribute("", UIWidgets.Auto, category: "Escapists")]
	protected ref array<ref ESCT_AmbientEventInfo> m_AmbientEvents;
	
	void GetAmbientEvents(out array<ref ESCT_AmbientEventInfo> ambientEvents)
	{
		foreach (ESCT_AmbientEventInfo info : m_AmbientEvents) 
		{
			if (!info.IsEnabled())
				continue;
			
			ambientEvents.Insert(info);
		}
	}
}
