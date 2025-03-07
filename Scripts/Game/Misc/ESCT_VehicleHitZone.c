void ScriptInvoker_DamageContextOnHitMethod(IEntity vehicle, BaseDamageContext damageContext);
typedef func ScriptInvoker_DamageContextOnHitMethod;
typedef ScriptInvokerBase<ScriptInvoker_DamageContextOnHitMethod> ScriptInvokerDamageContext;

modded class SCR_VehicleHitZone : SCR_DestructibleHitzone
{
	protected bool m_bIsSubscribed = false;
	protected ref ScriptInvokerDamageContext m_OnHit;
	protected IEntity m_Owner;
	
	bool IsOnHitSubscribed()
	{
		return m_bIsSubscribed;
	}
	
	void SetSubscribe(bool value)
	{
		m_bIsSubscribed = value;
	}
	
	override void OnInit(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.OnInit(pOwnerEntity, pManagerComponent);
		
		m_Owner = GetOwner();
	}
	
	ScriptInvokerDamageContext GetOnHit()
	{
		if (!m_OnHit)
			m_OnHit = new ScriptInvokerDamageContext();
		
		return m_OnHit;
	}
	
	override float ComputeEffectiveDamage(notnull BaseDamageContext damageContext, bool isDOT)
	{
		if (m_bIsSubscribed)
			m_OnHit.Invoke(m_Owner, damageContext);
		
		return super.ComputeEffectiveDamage(damageContext, isDOT);
	}
}
