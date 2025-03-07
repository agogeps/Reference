[EntityEditorProps("Escapists", description: "Handles support calls on characters.", color: "0 0 255 255")]
class ESCT_SupportCallerComponentClass : ScriptComponentClass
{
}

enum ESCT_ESupportCallState
{
	Ready = 0,
	Called = 4
}

enum ESCT_ESupportCallResult
{
	Ok,
	NoButOk,
	Error
}

sealed class ESCT_SupportCallerComponent : ScriptComponent
{
	[Attribute("30", uiwidget: UIWidgets.Slider, desc: "How many minutes this unit will be unable to call another support.", params: "0 90 1", category: "Escapists")]
	protected float m_fSupportTimeout;
	
	protected SCR_ChimeraAIAgent m_Agent;
	protected SCR_AIUtilityComponent m_Utility;
	protected ESCT_ESupportCallState m_eCallState;

	private WorldTimestamp m_fSupportCallTimestamp;
	ESCT_ESupportCallState GetCallState()
	{
		return m_eCallState;
	}
	
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (SCR_Global.IsEditMode() || !Replication.IsServer()) return;

		SetEventMask(owner, EntityEvent.INIT);
	}

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (SCR_Global.IsEditMode() || !Replication.IsServer()) return;

		AIControlComponent ctrl = AIControlComponent.Cast(owner.FindComponent(AIControlComponent));
		if (ctrl)
		{
			SCR_ChimeraAIAgent agent = SCR_ChimeraAIAgent.Cast(ctrl.GetAIAgent());
			if (agent)
			{
				m_Utility = SCR_AIUtilityComponent.Cast(agent.FindComponent(SCR_AIUtilityComponent));
			}
		}
	}
	
	ESCT_ESupportCallResult CallSupport(vector targetPosition, IEntity target = null)
	{
		bool couldMakeCall = CanMakeSupportCall(targetPosition);
		if (couldMakeCall != ESCT_ESupportCallResult.Ok)
			return couldMakeCall;
		
		ChimeraWorld world = GetOwner().GetWorld();
		m_fSupportCallTimestamp = world.GetServerTimestamp().PlusSeconds(m_fSupportTimeout * 60);
		
		ESCT_SupportSystem supportSystem = ESCT_SupportSystem.GetSystem();
		if (!supportSystem)
			return false;
		
		supportSystem.CreateSupport(target, targetPosition, ESCT_CharacterHelper.GetAIGroup(GetOwner()));
		m_eCallState = ESCT_ESupportCallState.Called;
		GetGame().GetCallqueue().CallLater(ResetSupportState, m_fSupportTimeout * 60 * 1000, false);
		
		return true;
	}
	
	void SendSupportAIMessage(SCR_AIGroup group, vector targetPosition, IEntity target = null)
	{
		if (!m_Utility || !group || m_eCallState != ESCT_ESupportCallState.Ready)
			return;
		
		GetAiAgent();
		if (!m_Agent)
			return;

		SCR_MailboxComponent myMailbox = SCR_MailboxComponent.Cast(m_Agent.FindComponent(SCR_MailboxComponent));
		ESCT_AIMessage_CallSupport msg = ESCT_AIMessage_CallSupport.Create(m_Utility.m_OwnerEntity, target, targetPosition);
		myMailbox.RequestBroadcast(msg, group);
	}
	
	private SCR_ChimeraAIAgent GetAiAgent()
	{
		if (m_Agent)
			return m_Agent;

		AIControlComponent controlComp = AIControlComponent.Cast(GetOwner().FindComponent(AIControlComponent));
		m_Agent = SCR_ChimeraAIAgent.Cast(controlComp.GetAIAgent());

		return m_Agent;
	}
	
	private ESCT_ESupportCallResult CanMakeSupportCall(vector targetPosition)
	{
		if (!Replication.IsServer())
			return ESCT_ESupportCallResult.Error;
		
		if (targetPosition == vector.Zero)
		{
			ESCT_Logger.ErrorFormat("%1 unable to call support due to zero vector position.", GetOwner().ToString());
			return ESCT_ESupportCallResult.Error; 
		}
		
		SCR_CharacterControllerComponent charController = SCR_CharacterControllerComponent.Cast(GetOwner().FindComponent(SCR_CharacterControllerComponent));
		if (charController && charController.GetLifeState() != ECharacterLifeState.ALIVE)
		{
			ESCT_Logger.InfoFormat(
				"%1 unable to call support on %2 position due to being dead/unconscious.", 
				GetOwner().ToString(), 
				targetPosition.ToString(), 
				typename.EnumToString(ESCT_ESupportCallState, m_eCallState)
			);
			return ESCT_ESupportCallResult.NoButOk;
		}
		
		if (m_fSupportCallTimestamp)
		{
			ChimeraWorld world = GetOwner().GetWorld();
			if (world.GetServerTimestamp().Less(m_fSupportCallTimestamp))
			{
				ESCT_Logger.InfoFormat("%1 unable to call support - already did it recently (%2).", GetOwner().ToString(), m_fSupportCallTimestamp.ToString());
				return ESCT_ESupportCallResult.NoButOk;
			}
		}
		
		if (m_eCallState != ESCT_ESupportCallState.Ready)
		{
			ESCT_Logger.InfoFormat(
				"%1 unable to call support on %2 position due to being not ready (%3 state).", 
				GetOwner().ToString(), 
				targetPosition.ToString(), 
				typename.EnumToString(ESCT_ESupportCallState, m_eCallState)
			);

			return ESCT_ESupportCallResult.NoButOk;
		}
		
		return ESCT_ESupportCallResult.Ok;
	}	
	
	private void ResetSupportState()
	{
		m_eCallState = ESCT_ESupportCallState.Ready;
	}
}
