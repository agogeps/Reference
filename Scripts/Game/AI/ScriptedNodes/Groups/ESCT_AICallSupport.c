class ESCT_AICallSupport : AITaskScripted
{	
	SCR_AIGroupUtilityComponent m_GroupUtilityComponent;

	//------------------------------------------------------------------------------------------------
	override bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		AIGroup group = AIGroup.Cast(owner);
		if (group)
			m_GroupUtilityComponent = SCR_AIGroupUtilityComponent.Cast(group.FindComponent(SCR_AIGroupUtilityComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_GroupUtilityComponent || !m_GroupUtilityComponent.m_aInfoComponents)
			return ENodeResult.FAIL;
		
		SCR_AIActivityBase currentActivity = SCR_AIActivityBase.Cast(m_GroupUtilityComponent.GetCurrentAction());
		if (!currentActivity)
			return ENodeResult.FAIL;
		
		// Find smoke cover feature in current activity
		ESCT_AIActivityCallSupportFeature supportFeature = ESCT_AIActivityCallSupportFeature.Cast(
			currentActivity.FindActivityFeature(ESCT_AIActivityCallSupportFeature));
		
		// Exit if smoke cover feature is not supported in current activity
		if (!supportFeature)
			return ENodeResult.FAIL;
		
		if (supportFeature.ExecuteForActivity(currentActivity))
			return ENodeResult.SUCCESS;
		
		return ENodeResult.FAIL;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool CanReturnRunning() { return false; }
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {};
	override TStringArray GetVariablesOut() { return s_aVarsOut; }

	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	//------------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription()
	{
		return "Getter returns group member available for specific role";
	}	
}
