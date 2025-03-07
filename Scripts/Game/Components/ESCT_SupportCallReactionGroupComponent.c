[EntityEditorProps("Escapists", description: "Handles support call reactions for characters in group.", color: "0 0 255 255")]
class ESCT_SupportCallReactionGroupComponentClass : ScriptComponentClass
{
}

sealed class ESCT_SupportCallReactionGroupComponent : ScriptComponent
{
	protected ref array<ESCT_SupportCallerComponent> m_aSupportCallers;
	protected SCR_AIGroup m_Group;
	
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
		
		m_Group = SCR_AIGroup.Cast(owner);
		if (m_Group)
		{
			m_aSupportCallers = {};
			m_Group.GetOnAgentAdded().Insert(OnAgentAdded);
		}
	}
	
	protected void OnAgentAdded(AIAgent aiAgent)
	{
		if (!aiAgent)
			return;
		
		IEntity charEntity = aiAgent.GetControlledEntity();
		if (!charEntity)
			return;
			
		ESCT_SupportCallerComponent caller = ESCT_SupportCallerComponent.Cast(charEntity.FindComponent(ESCT_SupportCallerComponent));
		if (caller)
		{
			m_aSupportCallers.Insert(caller);
			ESCT_Logger.InfoFormat("%1 added as group support caller at %2.", ESCT_Misc.GetPrefabName(charEntity), charEntity.GetOrigin().ToString(true));
		}
		
		SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.Cast(charEntity.FindComponent(SCR_DamageManagerComponent));
		if (damageManager)
		{
			damageManager.GetOnDamage().Insert(Event_OnDamage);
			ESCT_Logger.InfoFormat("%1 attached support call on damage event handler at %2.", ESCT_Misc.GetPrefabName(charEntity), charEntity.GetOrigin().ToString());
		}
	}
	
	protected void Event_OnDamage(BaseDamageContext damageContext)
	{
		if (damageContext.damageType == EDamageType.COLLISION || damageContext.damageType == EDamageType.FIRE || damageContext.damageType == EDamageType.REGENERATION)
			return;
		
		if (!damageContext.hitEntity)
			return;
		
		IEntity damagedEntity = damageContext.hitEntity;
		
		//if there are no support callers - then there's no point to proceed
		if (!m_aSupportCallers || m_aSupportCallers.IsEmpty())
		{
			SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.Cast(damagedEntity.FindComponent(SCR_DamageManagerComponent));
			if (damageManager)
				damageManager.GetOnDamage().Remove(Event_OnDamage);
			return;
		}
		
		Instigator instigator = damageContext.instigator;
		if (!instigator)
			return;

		IEntity instigatorEntity = instigator.GetInstigatorEntity();
		if (!instigatorEntity)
			return;
		
		//same factions shouldn't count
		FactionAffiliationComponent instigatorFactionAffiliation = FactionAffiliationComponent.Cast(instigatorEntity.FindComponent(FactionAffiliationComponent));
		FactionAffiliationComponent ownerFactionAffiliation = FactionAffiliationComponent.Cast(damagedEntity.FindComponent(FactionAffiliationComponent));
		if (instigatorFactionAffiliation && ownerFactionAffiliation)
		{
			if (instigatorFactionAffiliation.GetAffiliatedFactionKey() == ownerFactionAffiliation.GetAffiliatedFactionKey())
				return;
		}
		
		IEntity caller = null;
		ESCT_SupportCallerComponent supportCallerComponent = null;
		for (int i = 0; i < m_aSupportCallers.Count(); i++)
		{
			supportCallerComponent = m_aSupportCallers[i];
			
			caller = supportCallerComponent.GetOwner();
			if (!ESCT_CharacterHelper.CanFight(caller))
				continue;
			
			if (supportCallerComponent.GetCallState() == ESCT_ESupportCallState.Ready)
				break;
			
			supportCallerComponent = null;
			caller = null;
		}
		
		if (!supportCallerComponent)
			return;

		SCR_AIGroup group = ESCT_CharacterHelper.GetAIGroup(caller);
		if(!group)
			return;
		
		supportCallerComponent.SendSupportAIMessage(group, instigatorEntity.GetOrigin(), instigatorEntity);
		//m_DamageManager.GetOnDamageEffectAdded().Remove(Event_CallForSupportOnDamage);
	}
}
