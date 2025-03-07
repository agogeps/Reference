[EntityEditorProps(category: "Escapists/Entities", description: "A weapon rack slot to be filled with a weapon (Escapists game mode).", color: "0 0 255 255")]
class ESCT_WeaponRackSlotEntityClass: GenericEntityClass
{
}

//------------------------------------------------------------------------------------------------
sealed class ESCT_WeaponRackSlotEntity : GenericEntity
{
	[Attribute("0", UIWidgets.Flags, "", enums: ParamEnumArray.FromEnum(SCR_EArsenalItemType))]
	protected SCR_EArsenalItemType m_eSupportedItemTypes;
	
	[Attribute("0", UIWidgets.Flags, "", enums: ParamEnumArray.FromEnum(SCR_EArsenalItemMode))]
	protected SCR_EArsenalItemMode m_eSupportedItemModes;
	
	protected ResourceName m_Resource;
	protected ref SCR_ArsenalItem m_ArsenalItem;
	protected ref SCR_ArsenalItemDisplayData m_ItemDisplayData;
	protected IEntity m_Item;
	protected bool m_bIsPickedUp = false;

	bool HasResource()
	{
		return m_Resource;
	}
	
	bool IsPickedUp()
	{
		return m_bIsPickedUp; 
	}

	SCR_EArsenalItemType GetSlotSupportedItemTypes()
	{
		return m_eSupportedItemTypes;
	}
	
	SCR_EArsenalItemMode GetSlotSupportedItemModes()
	{
		return m_eSupportedItemModes;
	}
		
	void AssignItem(ResourceName prefab, SCR_ArsenalItem arsenalData, SCR_ArsenalItemDisplayData itemDisplayData = null)
	{
		m_Resource = prefab;
		m_ArsenalItem = arsenalData;
		m_ItemDisplayData = itemDisplayData;
	}
	
	void RemoveItem()
	{
		IEntity child = GetChildren();
		if (child)
		{
			RplComponent.DeleteRplEntity(child, false);
		}
	}
	
	void SpawnItem()
	{
		if (!m_Resource || m_bIsPickedUp)
			return;
		
		EntitySpawnParams params = new EntitySpawnParams();
		params.Parent = this;
		params.TransformMode = ETransformMode.WORLD;
		
		Resource itemResource = Resource.Load(m_Resource);
		
		IEntity item = GetGame().SpawnEntityPrefab(itemResource, GetWorld(), params);
		if (!item)
			return;
		
		m_Item = item;
		
		// Since item is in hierarchy in which parent (slotEntity) is not traceable (also has no volume), we'll have
		// to clear its proxy flag so we can trace the item safely. Proxy flag is reset on pickup via item slots
		item.ClearFlags(EntityFlags.PROXY);
		
		vector positionOffset, rotationOffset = vector.Zero;
		if (m_ItemDisplayData)
		{
			positionOffset = m_ItemDisplayData.GetItemOffset();
			rotationOffset = m_ItemDisplayData.GetItemRotation();
		}
		
		// set weapon rotation
		item.SetAngles(rotationOffset);
		
		// offset the item locally
		vector mat[4];
		item.GetLocalTransform(mat);
		mat[3] = mat[3] + positionOffset;
		item.SetLocalTransform(mat);
		
		if (item.FindComponent(MagazineComponent))
			item.SetFlags(EntityFlags.PROXY);
		
		item.ClearFlags(EntityFlags.ACTIVE);
		
		InventoryItemComponent itemComp = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (itemComp)
		{
			itemComp.m_OnParentSlotChangedInvoker.Insert(OnInventoryParentChanged);
			itemComp.DisablePhysics();
		}
		
		item.Update();
	}
	
	private void OnInventoryParentChanged(InventoryStorageSlot oldSlot, InventoryStorageSlot newSlot)
	{
		InventoryItemComponent invComp = InventoryItemComponent.Cast(m_Item.FindComponent(InventoryItemComponent));
		if (invComp)
			invComp.m_OnParentSlotChangedInvoker.Remove(OnInventoryParentChanged);

		//picking up item frees it from persistency trackers
		m_bIsPickedUp = true;
	}
}
