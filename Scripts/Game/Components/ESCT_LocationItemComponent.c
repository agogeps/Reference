[EntityEditorProps("Escapists/Server/Components", description: "Component that allows container to be used by locations.", color: "0 0 255 255")]
class ESCT_LocationItemComponentClass : ScriptComponentClass
{
}

sealed class ESCT_LocationItemComponent : ScriptComponent
{
	private int m_iStorageID;
	private IEntity m_Owner;
	private ESCT_LocationStorageManagerComponent m_StorageManager;
	
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (SCR_Global.IsEditMode()) return;

		SetEventMask(owner, EntityEvent.INIT);
	}
	
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		if (SCR_Global.IsEditMode()) return;
		
		m_Owner = owner;
	}
	
	void AddOnParentSlotChanged(ESCT_LocationStorageManagerComponent storageManager, int storageID)
	{
		m_StorageManager = storageManager;
		m_iStorageID = storageID;
		
		InventoryItemComponent invComp = InventoryItemComponent.Cast(GetOwner().FindComponent(InventoryItemComponent));
		if (invComp)
			invComp.m_OnParentSlotChangedInvoker.Insert(OnInventoryParentChanged);
	}
	
	void RemoveOnParentSlotChanged()
	{
		InventoryItemComponent invComp = InventoryItemComponent.Cast(GetOwner().FindComponent(InventoryItemComponent));
		if (invComp)
			invComp.m_OnParentSlotChangedInvoker.Remove(OnInventoryParentChanged);
	}
	
	private void OnInventoryParentChanged(InventoryStorageSlot oldSlot, InventoryStorageSlot newSlot)
	{
		InventoryItemComponent invComp = InventoryItemComponent.Cast(m_Owner.FindComponent(InventoryItemComponent));
		if (invComp)
			invComp.m_OnParentSlotChangedInvoker.Remove(OnInventoryParentChanged);
		
		if (!m_StorageManager)
			return;
		
		m_StorageManager.UpdateStorageState(m_iStorageID, m_Owner.GetPrefabData().GetPrefabName());
	}
}
