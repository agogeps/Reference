class ESCT_FlammableHitZone : SCR_FlammableHitZone
{
	private ESCT_BarrelDamageManagerComponent m_BarrelDamageManager;
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.OnInit(pOwnerEntity, pManagerComponent);

		if (SCR_Global.IsEditMode())
			return;

		InitFireRates();

		m_BarrelDamageManager = GetBarrelDamageManager();
		if (m_BarrelDamageManager)
			m_BarrelDamageManager.RegisterFlammableHitZone(this);

		m_SignalsManager = SignalsManagerComponent.Cast(pOwnerEntity.FindComponent(SignalsManagerComponent));
		if (m_SignalsManager && !m_sFireStateSignal.IsEmpty())
			m_iFireStateSignalIdx = m_SignalsManager.AddOrFindSignal(m_sFireStateSignal);
	}

	protected override void ApplyFireDamage(float fireRate, float deltaTime)
	{
		if (fireRate < m_fFireDamageRateMin)
		{
			SetDamageOverTime(EDamageType.FIRE, 0);
			return;
		}

		SetDamageOverTime(EDamageType.FIRE, fireRate);

		Instigator instigator = m_FireInstigator;
		if (!instigator)
			instigator = Instigator.CreateInstigator(null);

		ESCT_Logger.InfoFormat("Barrel hitzone health: %1", GetHealth().ToString());

		if (GetHealth() <= 0 && m_BarrelDamageManager)
		{
			m_BarrelDamageManager.DeleteParentWithEffects();
		}
	}
	
	private ESCT_BarrelDamageManagerComponent GetBarrelDamageManager()
	{
		ESCT_BarrelDamageManagerComponent damageManager = ESCT_BarrelDamageManagerComponent.Cast(m_RootDamageManager);
		if (!damageManager)
		{
			IEntity owner = GetOwner();
			if (!owner)
				return null;
			
			damageManager = ESCT_BarrelDamageManagerComponent.Cast(owner.FindComponent(ESCT_BarrelDamageManagerComponent));
			if (!damageManager)
			{
				IEntity root = owner.GetRootParent();
				if (!root)
					return null;
				
				damageManager = ESCT_BarrelDamageManagerComponent.Cast(root.FindComponent(ESCT_BarrelDamageManagerComponent));
			}
		}
		
		return damageManager;
	}
}
