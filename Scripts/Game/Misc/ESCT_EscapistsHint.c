modded enum EHint
{
	ESCAPISTS_WELCOME_PRISON = 2000,
	ESCAPISTS_WELCOME_HIDEOUT = 2001,
	ESCAPISTS_WELCOME_HELICRASH = 2002,
	ESCAPISTS_GOALS = 2003,
	ESCAPISTS_HELIPAD_REACH = 2004,
	ESCAPISTS_DEFEND_RADIO = 2005,
	ESCAPISTS_FLY_AWAY = 2006
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot: true)]
class ESCT_EscapistsHintStorage
{
	[Attribute()]
	protected ref array<ref ESCT_EscapistsHintEntry> m_aHintEntries;

	//------------------------------------------------------------------------------------------------
	SCR_HintUIInfo GetHintByEnum(EHint id)
	{
		foreach (ESCT_EscapistsHintEntry hintEntry : m_aHintEntries)
		{
			if (hintEntry.GetHintId() == id)
				return hintEntry.GetHintInfo();
		}
		
		return null;
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EHint, "m_eHintId")]
class ESCT_EscapistsHintEntry
{
	[Attribute(uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EHint))]
	protected EHint m_eHintId;

	[Attribute()]
	protected ref SCR_HintUIInfo m_HintInfo;
	
	//------------------------------------------------------------------------------------------------
	EHint GetHintId()
	{
		return m_eHintId;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_HintUIInfo GetHintInfo()
	{
		return m_HintInfo;
	}
}
