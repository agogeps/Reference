class ESCT_AICallSupportToBehaviorParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new ESCT_AICallSupportToBehavior(null, null, vector.Zero, null, 0)).GetPortNames();
	
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	override bool VisibleInPalette() { return true; }
}

class ESCT_AICallSupportToBehavior : SCR_AIBehaviorBase
{
	protected static const string SUPPORT_TO_BEHAVIOUR_TREE = "{9D3CEDA5C166718A}AI/BehaviorTrees/Chimera/Soldier/CallSupportTo.bt";

	//Target Position of the behavior
	ref SCR_BTParam<vector> m_vTargetPosition = new SCR_BTParam<vector>(ESCT_AICallSupportActivity.SUPPORT_POSITION_PORT);
	ref SCR_BTParam<IEntity> m_TargetEntity = new SCR_BTParam<IEntity>(ESCT_AICallSupportActivity.SUPPORT_TARGET_PORT);
	ref SCR_BTParam<float> m_fDelay = new SCR_BTParam<float>(SCR_AIActionTask.DELAY_PORT);

	BaseWorld m_World;
	float m_fStartTime;

	void ESCT_AICallSupportToBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, vector targetPosition, IEntity targetEntity, float delay, float priority = ESCT_AICallSupportActivity.PRIORITY_ACTIVITY_CALL_SUPPORT, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		m_vTargetPosition.Init(this, targetPosition);
		m_TargetEntity.Init(this, targetEntity);
		m_fDelay.Init(this, delay);

		if (utility)
		{
			m_World = utility.m_OwnerEntity.GetWorld();
			m_fStartTime = m_World.GetWorldTime() + delay;
		}

		m_sBehaviorTree = SUPPORT_TO_BEHAVIOUR_TREE;
		SetPriority(priority);
		m_fPriorityLevel.m_Value = priorityLevel;
		m_bAllowLook = false;
	}
	
	//------------------------------------------------------------------------------------------------------------------------------------
	override float CustomEvaluate()
	{
		if (m_World && m_World.GetWorldTime() < m_fStartTime)
			return 0;
		
		return GetPriority();
	}
	
	//----------------------------------------------------------------------------------
	override void OnActionSelected()
	{
		m_Utility.m_AIInfo.SetAIState(EUnitAIState.BUSY);
	}

	//----------------------------------------------------------------------------------
	override void OnActionCompleted()
	{
		super.OnActionCompleted();
		m_Utility.m_AIInfo.SetAIState(EUnitAIState.AVAILABLE);
#ifdef WORKBENCH
		SCR_AIDebugVisualization.VisualizeMessage(m_Utility.m_OwnerEntity, "Support called", EAIDebugCategory.INFO, 5); 
#endif
	}

	//----------------------------------------------------------------------------------
	override void OnActionFailed()
	{
		super.OnActionFailed();
		m_Utility.m_AIInfo.SetAIState(EUnitAIState.AVAILABLE);
#ifdef WORKBENCH
		SCR_AIDebugVisualization.VisualizeMessage(m_Utility.m_OwnerEntity, "Support call failed", EAIDebugCategory.INFO, 5);
#endif
	}

	//----------------------------------------------------------------------------------
	override string GetActionDebugInfo()
	{
		return this.ToString() + " calling support to " + m_vTargetPosition.ToString();
	}
}
