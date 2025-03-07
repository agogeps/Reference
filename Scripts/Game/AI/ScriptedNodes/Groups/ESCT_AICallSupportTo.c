class ESCT_AICallSupportTo : AITaskScripted
{	
	// Inputs
	static const string PORT_TARGET_POSITION = "TargetPosition";
	static const string PORT_TARGET_ENTITY = "TargetEntity";
	
	//------------------------------------------------------------------------------------------------
	override bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		IEntity characterEntity = owner.GetControlledEntity();
		if (!characterEntity)
			return ENodeResult.FAIL;
		
		vector targetPosition = vector.Zero;
		GetVariableIn(PORT_TARGET_POSITION, targetPosition);
		if (targetPosition == vector.Zero)
			return ENodeResult.FAIL;
		
		IEntity targetEntity;
		GetVariableIn(PORT_TARGET_ENTITY, targetEntity);
		
		ESCT_SupportCallerComponent supportCaller = ESCT_SupportCallerComponent.Cast(characterEntity.FindComponent(ESCT_SupportCallerComponent));
		if (!supportCaller)
			return ENodeResult.FAIL;
		
		supportCaller.CallSupport(targetPosition, targetEntity);
		
		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool CanReturnRunning() { return false; }
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {};
	override TStringArray GetVariablesOut() { return s_aVarsOut; }

	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = { PORT_TARGET_POSITION, PORT_TARGET_ENTITY };
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	//------------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription()
	{
		return "Soldier will call for support.";
	}	
}
