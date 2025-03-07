[EntityEditorProps("Escapists/Location", description: "Weapon rack manager in locations.", color: "0 0 255 255")]
class ESCT_LocationWeaponRackManagerComponentClass : ESCT_LocationComponentClass
{
}

class ESCT_LocationWeaponRackManagerComponent : ESCT_LocationComponent
{
	protected ref array<ESCT_WeaponRack> m_aWeaponRacks;
	
	override void Spawn()
	{
		IEntity composition = m_Location.GetComposition();
		if (!composition)
			return;
		
		TryGetWeaponRacks(composition);
		if (!m_aWeaponRacks || m_aWeaponRacks.IsEmpty())
			return;
		
		
		for (int i = 0; i < m_aWeaponRacks.Count(); i++)
		{
			m_aWeaponRacks[i].Spawn();
		}
	}
	
	override void Despawn()
	{
		if(!m_aWeaponRacks || m_aWeaponRacks.IsEmpty())
			return;
		
		for (int i = 0; i < m_aWeaponRacks.Count(); i++)
		{
			m_aWeaponRacks[i].Despawn();
		}
		m_aWeaponRacks.Clear();
	}
	
	protected void TryGetWeaponRacks(IEntity composition)
	{
		array<IEntity> children = {};
		ESCT_EntityHelper.GetAllChildren(composition, children);
		
		if (children.IsEmpty())
			return;
		
		m_aWeaponRacks = {};
		foreach (IEntity entity : children)
		{
			if (entity.Type() != ESCT_WeaponRack)
				continue;

			m_aWeaponRacks.Insert(ESCT_WeaponRack.Cast(entity));
		}
	}
}
