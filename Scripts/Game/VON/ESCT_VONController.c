//------------------------------------------------------------------------------------------------
//! Scripted VON input and control, attached to SCR_PlayerController
modded class SCR_VONController : ScriptComponent
{
	//------------------------------------------------------------------------------------------------
	//! VON activation
	//! \param entry is the subject VON entry
 	protected override void ActivateVON(EVONTransmitType transmitType)
	{				
		if (!m_VONComp)
			return;
		
		m_eVONType = transmitType;
		SCR_VONEntry entry = GetEntryByTransmitType(transmitType);
					
		SetActiveTransmit(entry);
		m_VONComp.SetCapture(true);
		m_bIsActive = true;
	}
}
