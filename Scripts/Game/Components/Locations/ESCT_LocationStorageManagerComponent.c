[EntityEditorProps("Escapists/Location", description: "Manages inventory storages for location.", color: "0 0 255 255")]
class ESCT_LocationStorageManagerComponentClass : ESCT_LocationComponentClass
{
}

class ESCT_LocationStorageManagerComponent : ESCT_LocationComponent
{
	private ref array<ESCT_StorageSlotEntity> m_aStorageSlots = {};
	private ref array<ESCT_LocationStorageComponent> m_aStorages = {};
	private ref map<int, ref ESCT_StorageState> m_StorageStateMap = new map<int, ref ESCT_StorageState>();
	
	map<int, ref ESCT_StorageState> GetStorageStates()
	{
		return m_StorageStateMap;
	}

	override void Spawn()
	{
		IEntity composition = m_Location.GetComposition();
		if (!composition)
			return;
		
		//spawn before grab all storage slots as there may be both dynamic and static storages
		GetStorageSlots(composition);
		SpawnStorageSlots();
		
		GetStorages(composition);
		AssignSlots();

		ESCT_LocationStorageComponent currentStorage;
		for (int i = 0 ; i < m_aStorages.Count(); i++)
		{
			currentStorage = m_aStorages[i];
			currentStorage.Spawn();
		}
	}

	override void Despawn()
	{
		foreach (ESCT_LocationStorageComponent storage : m_aStorages)
		{
			if (storage.IsStatic())
			{
				storage.ClearStorage();
			}
			else 
			{
				IEntity entity = storage.GetOwner();
				if (entity)
					SCR_EntityHelper.DeleteEntityAndChildren(entity);
			}
		}
		
		m_aStorages.Clear();
		m_aStorageSlots.Clear();
	}

	bool AddToSlotStateMap(int slotId, notnull ESCT_StorageState state)
	{
		return m_StorageStateMap.Insert(slotId, state);
	}
	
	void UpdateStorageState(int id, ResourceName prefab)
	{
		ESCT_StorageState storage = m_StorageStateMap.Get(id);
		if (!storage)
			return;
		
		storage.UpdateStorage(prefab);
	}
	
	private void GetStorages(IEntity composition)
	{
		array<IEntity> children = {};
		ESCT_EntityHelper.GetAllChildren(composition, children);

		IEntity entity;
		for (int i = 0; i < children.Count(); i++)
		{
			entity = children[i];			
			ESCT_LocationStorageComponent storage = ESCT_LocationStorageComponent.Cast(entity.FindComponent(ESCT_LocationStorageComponent));
			if (storage && m_aStorages.Find(storage) == -1)
				m_aStorages.Insert(storage);
		}
	}
	
	private void GetStorageSlots(IEntity composition)
	{
		array<IEntity> children = {};
		ESCT_EntityHelper.GetAllChildren(composition, children);
		
		IEntity entity;
		for (int i = 0; i < children.Count(); i++)
		{
			entity = children[i];
			if (entity.Type() != ESCT_StorageSlotEntity)
				continue;
			
			m_aStorageSlots.Insert(ESCT_StorageSlotEntity.Cast(entity));
		}
	}	
	
	private void SpawnStorageSlots()
	{
		foreach (ESCT_StorageSlotEntity slot : m_aStorageSlots)
		{
			IEntity storage = slot.SpawnStorage();
			if (!storage)
				return;
			
			ESCT_LocationStorageComponent locationStorage = ESCT_LocationStorageComponent.Cast(storage.FindComponent(ESCT_LocationStorageComponent));
			if (locationStorage)
			{
				locationStorage.SetIsStatic(false);
				m_aStorages.Insert(locationStorage);
			}
		}
	}

	private void AssignSlots()
	{
		if (!m_Location.IsVisited())
		{
			ESCT_LocationStorageComponent currentStorage;
			for (int i = 0 ; i < m_aStorages.Count(); i++)
			{
				currentStorage = m_aStorages[i];
				currentStorage.SetStorageManager(this);
				currentStorage.FirstSetup();
			}
			
			return;
		}

		//if we're revisiting location then it means that the state for them (respawned points with the same ids) 
		//is already there so we need to just pass the manager
		ESCT_LocationStorageComponent currentStorage;
		for (int i = 0 ; i < m_aStorages.Count(); i++)
		{
			currentStorage = m_aStorages[i];
			currentStorage.SetStorageManager(this);
		}
	}
}
