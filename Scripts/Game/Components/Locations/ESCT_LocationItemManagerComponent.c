[EntityEditorProps("Escapists/Location", description: "Component that allows building to be used as starting prison (spawn point).", color: "0 0 255 255")]
class ESCT_LocationItemManagerComponentClass : ESCT_LocationComponentClass
{
}

class ESCT_LocationItemManagerComponent : ESCT_LocationComponent
{
	private ref array<ESCT_EquipmentSlotEntity> m_aEquipmentSlots = {};
	private ref map<int, ref ESCT_EquipmentSlotState> m_SlotStateMap = new map<int, ref ESCT_EquipmentSlotState>();
	
	map<int, ref ESCT_EquipmentSlotState> GetSlotStates()
	{
		return m_SlotStateMap;
	}

	override void Spawn()
	{
		IEntity composition = m_Location.GetComposition();
		if (!composition)
			return;
		
		GetCompositionSlots(composition);
		GetBuildingsEquipmentSlots();
		AssignSlots();

		ESCT_EquipmentSlotEntity currentSlot;
		for (int i = 0 ; i < m_aEquipmentSlots.Count(); i++)
		{
			currentSlot = m_aEquipmentSlots[i];
			IEntity entity = currentSlot.Spawn();
			if (entity)
				m_Location.ChangeEntitiesToClear(1);
		}
	}

	override void Despawn()
	{
		IEntity entity;
		for (int i = 0 ; i < m_aEquipmentSlots.Count(); i++)
		{
			entity = m_aEquipmentSlots[i].GetEntity();
			if (entity)
				SCR_EntityHelper.DeleteEntityAndChildren(entity);
		}
		
		m_aEquipmentSlots.Clear();
	}

	bool AddToSlotStateMap(int slotId, notnull ESCT_EquipmentSlotState state)
	{
		return m_SlotStateMap.Insert(slotId, state);
	}
	
	void UpdateSlotState(ESCT_EquipmentSlotState state)
	{
		int id = state.GetId();

		ESCT_EquipmentSlotState stateToUpdate = m_SlotStateMap[id];
		if (!stateToUpdate)
			return;

		stateToUpdate.b_mIsUsed = state.b_mIsUsed;
		m_SlotStateMap[id] = stateToUpdate;
		
		m_Location.GetOnEntityChange().Invoke(-1);
	}
	
	private void GetCompositionSlots(IEntity composition)
	{
		array<IEntity> children = {};
		ESCT_EntityHelper.GetAllChildren(composition, children);

		IEntity entity;
		for (int i = 0; i < children.Count(); i++)
		{
			entity = children[i];
			if (entity.Type() != ESCT_EquipmentSlotEntity)
				continue;
			
			ESCT_EquipmentSlotEntity slot = ESCT_EquipmentSlotEntity.Cast(entity);
			m_aEquipmentSlots.Insert(slot);
		}
	}

	private void GetBuildingsEquipmentSlots()
	{
		ESCT_MilitaryBuildingStorageComponent buildingStorage = ESCT_LocationFuncs<ESCT_MilitaryBuildingStorageComponent>.GetLocationComponent(m_Location);
		if (!buildingStorage)
			return;
		
		array<ESCT_PopulatedBuildingComponent> buildings = {};
		buildingStorage.GetMilitaryBuildings(buildings);
		
		ESCT_PopulatedBuildingComponent building;
		for (int i = 0; i < buildings.Count(); i++)
		{	
			building = buildings[i];
			if (!building)
				continue;
			
			array<ESCT_EquipmentSlotEntity> slots = {};
			building.GetEquipmentSlots(slots);
			m_aEquipmentSlots.InsertAll(slots);
		}
	}

	private void AssignSlots()
	{
		if (!m_Location.IsVisited())
		{
			ESCT_EquipmentSlotEntity currentSlot;
			for (int i = 0 ; i < m_aEquipmentSlots.Count(); i++)
			{
				currentSlot = m_aEquipmentSlots[i];
				currentSlot.SetItemManager(this);
				currentSlot.FirstSetupSlot(m_Location.GetOwnerFaction());
			}
			
			return;
		}

		//if we're revisiting location then it means that the state for them (respawned points with the same ids) 
		//is already there so we need to just pass the manager
		ESCT_EquipmentSlotEntity currentSlot;
		for (int i = 0 ; i < m_aEquipmentSlots.Count(); i++)
		{
			currentSlot = m_aEquipmentSlots[i];
			currentSlot.SetItemManager(this);
		}
	}
}
