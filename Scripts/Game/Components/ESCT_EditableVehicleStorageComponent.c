[EntityEditorProps("Escapists/Server/Components", description: "Handles vehicle storages.", color: "0 0 255 255")]
class ESCT_EditableVehicleStorageComponentClass : ScriptComponentClass
{
}

sealed class ESCT_EditableVehicleStorageComponent : ScriptComponent
{
	protected SCR_VehicleInventoryStorageManagerComponent m_InventoryStorageManager;
	protected SCR_UniversalInventoryStorageComponent m_InventoryStorage;
	protected ESCT_StorageArsenalComponent m_ArsenalComponent;

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (SCR_Global.IsEditMode() || !Replication.IsServer()) return;

		SetEventMask(owner, EntityEvent.INIT);
	}

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		m_ArsenalComponent = ESCT_StorageArsenalComponent.Cast(owner.FindComponent(ESCT_StorageArsenalComponent));
		if (!m_ArsenalComponent)
			ESCT_Logger.Error("ESCT_StorageArsenalComponent component could not be found on ESCT_LocationStorageComponent entity!");

		m_InventoryStorageManager = SCR_VehicleInventoryStorageManagerComponent.Cast(owner.FindComponent(SCR_VehicleInventoryStorageManagerComponent));
		if (!m_InventoryStorageManager)
			ESCT_Logger.Error("SCR_VehicleInventoryStorageManagerComponent component could not be found on ESCT_LocationStorageComponent entity!");

		m_InventoryStorage = SCR_UniversalInventoryStorageComponent.Cast(owner.FindComponent(SCR_UniversalInventoryStorageComponent));
		if (!m_InventoryStorage)
			ESCT_Logger.Error("SCR_UniversalInventoryStorageComponent component could not be found on ESCT_LocationStorageComponent entity!");

		GetGame().GetCallqueue().CallLater(SpawnEquipment);
	}

	protected void SpawnEquipment()
	{
		if (!m_ArsenalComponent || !m_InventoryStorageManager || !m_InventoryStorage)
			return;

		array<SCR_ArsenalItem> items = {};
		if (!m_ArsenalComponent.AssignItems(items, true))
			return;

		ResourceName resource;
		foreach (SCR_ArsenalItem item : items)
		{
			resource = item.GetItemResourceName();
			if (!resource)
				continue;

			m_InventoryStorageManager.TrySpawnPrefabToStorage(resource, m_InventoryStorage);
		}
	}
}
