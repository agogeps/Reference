class ESCT_AIGetCallSupportActivityParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new ESCT_AICallSupportActivity(null, null, null, null, vector.Zero)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	override bool VisibleInPalette() { return true; }
}

class ESCT_AISetCallSupportActivityParameters : SCR_AISetActionParameters
{
	protected static ref TStringArray s_aVarsIn = (new ESCT_AICallSupportActivity(null, null, null, null, vector.Zero)).GetPortNames();
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	override bool VisibleInPalette() { return true; }
}

class ESCT_AICallSupportActivity : SCR_AIActivityBase
{
	static const float PRIORITY_ACTIVITY_CALL_SUPPORT = 160;
	protected static const int TIMEOUT_FAILED = 5 * 60 * 1000;
	protected static const string SUPPORT_BEHAVIOUR_TREE = "AI/BehaviorTrees/Chimera/Group/ActivityCallSupport.bt";

	const string SUPPORT_CALLER_PORT = "SupportCallerEntity";
	const string SUPPORT_POSITION_PORT = "TargetPosition";
	const string SUPPORT_TARGET_PORT = "TargetEntity";

	// Support's target. Assigned from behavior tree.
	ref SCR_BTParam<IEntity> m_TargetEntity = new SCR_BTParam<IEntity>(SUPPORT_TARGET_PORT);
	
	// Who will call for support. Assigned from behavior tree.
	ref SCR_BTParam<IEntity> m_CallerEntity = new SCR_BTParam<IEntity>(SUPPORT_CALLER_PORT);

	//Where support should be at (usually some player position).
	ref SCR_BTParam<vector> m_vTargetPosition = new SCR_BTParam<vector>(SUPPORT_POSITION_PORT);

	protected float m_fTimeCreated_world; // World time when this was created
	protected float m_fTimeCheckConditions_world;

	// Features
	static ref array<ref SCR_AIActivityFeatureBase> s_ActivityFeatures = {new ESCT_AIActivityCallSupportFeature()};
	override array<ref SCR_AIActivityFeatureBase> GetActivityFeatures() { return s_ActivityFeatures; }

	override float CustomEvaluate()
	{
		float worldTime = GetGame().GetWorld().GetWorldTime();

		if (worldTime - m_fTimeCheckConditions_world > 2500)
		{
			// Complete if support is already called on that position
			ESCT_SupportSystem supportSystem = ESCT_SupportSystem.GetSystem();
			if (!supportSystem)
			{
				ESCT_Logger.Error("Can't call for support on car alarm - Support System wasn't found! Check if call is made by the server!");
				Fail();
				return 0;
			}

			SCR_AIGroup group = ESCT_CharacterHelper.GetAIGroup(m_CallerEntity.m_Value);
			
			//if there is a support already - don't call it
			if (group && supportSystem.HasSimilarActiveSupport(group, m_vTargetPosition.m_Value))
			{
				Complete();
				return 0;
			}

			// Replan if support caller is null or destroyed
			if (!SCR_AIDamageHandling.IsAlive(m_CallerEntity.m_Value))
			{
				SetActionIsSuspended(false);
			}

			m_fTimeCheckConditions_world = worldTime;
		}

		// Fail on timeout
		if (worldTime - m_fTimeCreated_world > TIMEOUT_FAILED)
		{
			#ifdef AI_DEBUG
			AddDebugMessage("Timeout, action failed.");
			#endif

			Fail();
			return 0;
		}

		return GetPriority();
	}

	void InitParameters(IEntity callerEntity, IEntity targetEntity, vector supportPosition, float priorityLevel)
	{
		IEntity caller = callerEntity;
		IEntity target = targetEntity;
		m_vTargetPosition.Init(this, supportPosition);
		m_fPriorityLevel.Init(this, priorityLevel);
		
		m_TargetEntity.Init(this, target); // Must use a variable, otherwise null directly doesn't work with templates.
		m_CallerEntity.Init(this, caller); // Must use a variable, otherwise null directly doesn't work with templates.
	}

	//------------------------------------------------------------------------------------------------
	void ESCT_AICallSupportActivity(SCR_AIGroupUtilityComponent utility, AIWaypoint relatedWaypoint, IEntity caller, IEntity target, vector targetPosition, float priority = PRIORITY_ACTIVITY_CALL_SUPPORT, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		m_sBehaviorTree = SUPPORT_BEHAVIOUR_TREE;
		SetPriority(priority);
		InitParameters(caller, target, targetPosition, priorityLevel);
		SetIsUniqueInActionQueue(false);

		auto game = GetGame();
		if (game)
		{
			m_fTimeCreated_world = game.GetWorld().GetWorldTime();
		}
	}

	//------------------------------------------------------------------------------------------------
	override string GetActionDebugInfo()
	{
		return this.ToString() + " calls for support at " + m_vTargetPosition.ToString();
	}
}
