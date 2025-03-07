class ESCT_BarrelDamageManagerComponentClass : SCR_DestructionMultiPhaseComponentClass
{
}

//! It should be destruction multiphase (despite no phases at all) due to collision disappearance bug of a barrel
class ESCT_BarrelDamageManagerComponent : SCR_DestructionMultiPhaseComponent
{
	[Attribute(defvalue: "1.0", desc: "Delay between secondary fire damage\n[s]", params: "0 10000 0.1", category: "Secondary damage")]
	protected float m_fSecondaryFireDamageDelay;
	
	// The vehicle damage manager needs to know about all the burning hitzones that it consists of
	protected ref array<SCR_FlammableHitZone>	m_aFlammableHitZones;
	
	// Vehicle fire
	protected float	m_fVehicleFireDamageTimeout;

	// Audio features
	protected int m_iVehicleFireStateSignalIdx;

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] hitZone
	void RegisterFlammableHitZone(notnull SCR_FlammableHitZone hitZone)
	{
		if (m_aFlammableHitZones)
			m_aFlammableHitZones.Insert(hitZone);
		else
			m_aFlammableHitZones = {hitZone};
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] hitZone
	void UnregisterFlammableHitZone(notnull SCR_FlammableHitZone hitZone)
	{
		if (m_aFlammableHitZones)
			m_aFlammableHitZones.RemoveItem(hitZone);

		if (m_aFlammableHitZones.IsEmpty())
			m_aFlammableHitZones = null;
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateFireDamage(float timeSlice)
	{
		if (!m_aFlammableHitZones)
			return;

		float fireRate;
		UpdateVehicleFireState(fireRate, timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateVehicleFireState(out float fireRate, float timeSlice)
	{
		vector averagePosition = GetSecondaryExplosionPosition(SCR_FlammableHitZone, fireRate);

		// Fire area damage
		m_fVehicleFireDamageTimeout -= timeSlice;
		if (m_fVehicleFireDamageTimeout < 0)
		{
			EntitySpawnParams spawnParams();
			spawnParams.Transform[3] = averagePosition;
			ResourceName fireDamage = GetSecondaryExplosion(fireRate, SCR_ESecondaryExplosionType.FUEL, fire: true);
			SecondaryExplosion(fireDamage, GetInstigator(), spawnParams);

			// Constant intervals for secondary damage
			m_fVehicleFireDamageTimeout = m_fSecondaryFireDamageDelay;
		}

		SCR_ESecondaryExplosionScale vehicleFireState = GetSecondaryExplosionScale(fireRate, SCR_ESecondaryExplosionType.FUEL);
	}
	
	override void SecondaryExplosion(ResourceName prefabName, notnull Instigator instigator, notnull EntitySpawnParams spawnParams)
	{
		if (!spawnParams.Parent)
			spawnParams.Parent = GetOwner();
		
		if (!spawnParams.Parent)
			return;
		
		super.SecondaryExplosion(prefabName, instigator, spawnParams);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Called whenever an instigator is going to be set.
	//! \param[in] currentInstigator: This damage manager's last instigator
	//! \param[in] newInstigator: The new instigator for this damage manager
	//! \return If it returns true, newInstigator will become the new current instigator for the damage manager and it will receive kill credit.
	*/
	protected override bool ShouldOverrideInstigator(notnull Instigator currentInstigator, notnull Instigator newInstigator)
	{
		// if the vehicle is on fire and we have a valid fireInstigator, do not allow any new instigators to override it
		SCR_FlammableHitZone flammableHitZone = SCR_FlammableHitZone.Cast(GetDefaultHitZone());
		Instigator fireInstigator = flammableHitZone.GetFireInstigator();
		if (fireInstigator && flammableHitZone && IsOnFire(flammableHitZone) && flammableHitZone.GetFireInstigator() != newInstigator)
			return false;
		
		// Don't override valid instigators with invalid instigators
		if (newInstigator.GetInstigatorType() == InstigatorType.INSTIGATOR_NONE)
			return false;
		
		return true;
	}
}
