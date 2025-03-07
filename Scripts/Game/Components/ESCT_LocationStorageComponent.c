[EntityEditorProps("Escapists/Server/Components", description: "Component that allows container to be used by locations.", color: "0 0 255 255")]
class ESCT_LocationStorageComponentClass : ScriptComponentClass
{
}

sealed class ESCT_LocationStorageComponent : ScriptComponent
{
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Probability of spawn will be not checked and storage will be always filled with items.", category: "Escapists")]
	protected bool m_IsSpawnGuaranteed;

	protected int m_iStorageId;
	protected bool m_IsStatic = true;
	protected ESCT_LocationStorageManagerComponent m_StorageManager;
	protected SCR_InventoryStorageManagerComponent m_InventoryStorageManager;
	protected SCR_UniversalInventoryStorageComponent m_InventoryStorage;
	protected ESCT_StorageArsenalComponent m_ArsenalComponent;

	int GetStorageId()
	{
		return m_iStorageId;
	}

	bool IsStatic()
	{
		return m_IsStatic;
	}

	void SetIsStatic(bool isStatic)
	{
		m_IsStatic = isStatic;
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

		m_ArsenalComponent = ESCT_StorageArsenalComponent.Cast(owner.FindComponent(ESCT_StorageArsenalComponent));
		if (!m_ArsenalComponent)
			ESCT_Logger.Error("ESCT_StorageArsenalComponent component could not be found on ESCT_LocationStorageComponent entity!");

		m_InventoryStorageManager = SCR_InventoryStorageManagerComponent.Cast(owner.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!m_InventoryStorageManager)
			ESCT_Logger.Error("SCR_InventoryStorageManagerComponent component could not be found on ESCT_LocationStorageComponent entity!");

		m_InventoryStorage = SCR_UniversalInventoryStorageComponent.Cast(owner.FindComponent(SCR_UniversalInventoryStorageComponent));
		if (!m_InventoryStorage)
			ESCT_Logger.Error("SCR_UniversalInventoryStorageComponent component could not be found on ESCT_LocationStorageComponent entity!");

		m_iStorageId = ESCT_IdentifierGenerator.GenerateIntId();
	}

	void SetStorageManager(notnull ESCT_LocationStorageManagerComponent storageManager)
	{
		m_StorageManager = storageManager;
	}

	void ClearStorage()
	{
		if (!m_InventoryStorageManager)
			return;

		array<IEntity> items = {};
		m_InventoryStorageManager.GetItems(items);

		foreach (IEntity entity : items)
		{
			if (!entity)
				continue;

			ESCT_LocationItemComponent locationItem = ESCT_LocationItemComponent.Cast(entity.FindComponent(ESCT_LocationItemComponent));
			if (locationItem)
				locationItem.RemoveOnParentSlotChanged();
			SCR_EntityHelper.DeleteEntityAndChildren(entity);
		}
	}

	void FirstSetup()
	{
		if (!m_StorageManager || !m_ArsenalComponent)
			return;

		array<SCR_ArsenalItem> items = {};
		if (!m_ArsenalComponent.AssignItems(items, m_IsSpawnGuaranteed))
			return;

		array<ResourceName> resources = {};
		foreach (SCR_ArsenalItem item : items)
		{
			resources.Insert(item.GetItemResourceName());
		}

		ESCT_StorageState state = new ESCT_StorageState(m_iStorageId, resources, GetOwner());
		m_StorageManager.AddToSlotStateMap(m_iStorageId, state);

		//ESCT_Logger.InfoFormat("Saving StorageState: %1, %2", param1: m_iStorageId.ToString(), param2: state.GetStoredItems().Count().ToString());
	}

	void Spawn()
	{
		if (!m_StorageManager || !m_InventoryStorageManager || !m_InventoryStorage)
			return;

		//ESCT_Logger.InfoFormat("ID: %1", param1: m_iStorageId.ToString());

		map<int, ref ESCT_StorageState> stateMap = m_StorageManager.GetStorageStates();
		ESCT_StorageState state = stateMap.Get(m_iStorageId);
		if (!state)
			return;

		//ESCT_Logger.InfoFormat("Loading StorageState: %1, %2", param1: m_iStorageId.ToString(), param2: state.GetStoredItems().Count().ToString());

		foreach (ResourceName prefab : state.GetStoredItems())
		{
			m_InventoryStorageManager.TrySpawnPrefabToStorage(prefab, m_InventoryStorage);
		}
		
		array<IEntity> spawnedItems = {};
		m_InventoryStorage.GetAll(spawnedItems);
		
		foreach (IEntity spawnedEntity : spawnedItems)
		{
			ESCT_LocationItemComponent locationItem = ESCT_LocationItemComponent.Cast(spawnedEntity.FindComponent(ESCT_LocationItemComponent));
			if (locationItem)
				locationItem.AddOnParentSlotChanged(m_StorageManager, m_iStorageId);
		
			spawnedEntity.ClearFlags(EntityFlags.ACTIVE);

			InventoryItemComponent itemComp = InventoryItemComponent.Cast(spawnedEntity.FindComponent(InventoryItemComponent));
			if (itemComp)
				itemComp.DisablePhysics();

			spawnedEntity.Update();
		}
	}
}
