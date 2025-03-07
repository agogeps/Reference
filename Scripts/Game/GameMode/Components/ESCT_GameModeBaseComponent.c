[ComponentEditorProps(description: "Base for gamemode scripted component.", visible: false)]
class ESCT_GameModeBaseComponentClass: SCR_BaseGameModeComponentClass
{
}

class ESCT_GameModeBaseComponent: SCR_BaseGameModeComponent
{
	private ESCT_EscapistsGameMode m_EscapistsGameMode;

	protected ESCT_EscapistsGameMode GetEscapistsGameMode()
	{
		if (!m_EscapistsGameMode)
		{
			IEntity owner = GetOwner();
			m_EscapistsGameMode = ESCT_EscapistsGameMode.Cast(owner);
			if (!m_EscapistsGameMode)
			{
				string message = string.Format("%1 is attached to entity '%2' type=%3, required type=%4! This is not allowed!", Type().ToString(), owner.GetName(), owner.ClassName(), "A4A_GameModeBaseComponent");
				Debug.Error(message);
				ESCT_Logger.Error(message);
				return null;
			}
		}

		return m_EscapistsGameMode;
	}
}
