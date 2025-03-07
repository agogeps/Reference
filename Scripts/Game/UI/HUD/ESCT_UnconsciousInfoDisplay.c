//------------------------------------------------------------------------------------------------
//! Unconscious/Downed UI.
//! Attached to SCR_HUDManagerComponent which is a component of SCR_PlayerController
class ESCT_UnconsciousInfoDisplay : SCR_InfoDisplayExtended
{
	private const string UNCON_CONTEXT = "EscapistsUnconsciousContext";

	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		super.DisplayUpdate(owner, timeSlice);

		if (!m_bShown)
			return;

		GetGame().GetInputManager().ActivateContext(UNCON_CONTEXT);
	}
}
