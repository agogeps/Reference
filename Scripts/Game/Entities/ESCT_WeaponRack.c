[EntityEditorProps(category: "Escapists/Entities", description: "Weapon rack.")]
class ESCT_WeaponRackClass : GenericEntityClass
{
}

sealed class ESCT_WeaponRack : GenericEntity
{
	protected ESCT_WeaponRackArsenalComponent m_WeaponRackArsenalComponent;
	
	ESCT_WeaponRackArsenalComponent GetWeaponRackArsenal()
	{
		return m_WeaponRackArsenalComponent;
	}
	
	void ESCT_WeaponRack(IEntitySource src, IEntity parent)
	{
		if (!GetGame().InPlayMode())
			return;
		
		SetEventMask(EntityEvent.INIT);
	}

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		m_WeaponRackArsenalComponent = ESCT_WeaponRackArsenalComponent.Cast(FindComponent(ESCT_WeaponRackArsenalComponent));
		if (!m_WeaponRackArsenalComponent)
			ESCT_Logger.Error("Weapon rack arsenal component could not be found on ESCT_WeaponRack entity");
	}
	
	void Spawn()
	{
		m_WeaponRackArsenalComponent.Spawn();
	}
	
	void Despawn()
	{
		m_WeaponRackArsenalComponent.ClearArsenal();
	}
}
