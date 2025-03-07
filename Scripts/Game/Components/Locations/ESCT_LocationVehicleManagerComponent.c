[EntityEditorProps("Escapists/Location", description: "Component that manages state of vehicles for this particular location.", color: "0 0 255 255")]
class ESCT_LocationVehicleManagerComponentClass : ESCT_LocationComponentClass
{
}

class ESCT_LocationVehicleManagerComponent : ESCT_LocationComponent
{
	private ref array<ESCT_VehicleSpawnPoint> m_aVehiclePoints = {};
	private ref map<int, ref ESCT_VehicleSpawnPointState> m_SlotStateMap = new map<int, ref ESCT_VehicleSpawnPointState>();
	
	map<int, ref ESCT_VehicleSpawnPointState> GetSpawnPointStates()
	{
		return m_SlotStateMap;
	}
	
	override void Spawn()
	{
		IEntity composition = m_Location.GetComposition();
		if (!composition)
			return;
		
		GetCompositionSlots(composition);
		AssignSlots();
		
		ESCT_VehicleSpawnPoint currentPoint;
		for (int i = 0 ; i < m_aVehiclePoints.Count(); i++)
		{
			currentPoint = m_aVehiclePoints[i];
			currentPoint.Spawn();
		}
	}

	override void Despawn()
	{
		IEntity entity;
		for (int i = 0 ; i < m_aVehiclePoints.Count(); i++)
		{
			entity = m_aVehiclePoints[i].GetVehicle();
			if (entity)
				SCR_EntityHelper.DeleteEntityAndChildren(entity);
		}
		
		m_aVehiclePoints.Clear();
	}
	
	bool AddToSlotStateMap(int slotId, notnull ESCT_VehicleSpawnPointState state)
	{
		return m_SlotStateMap.Insert(slotId, state);
	}
	
	void UpdateState(ESCT_VehicleSpawnPointState newState)
	{
		int id = newState.GetId();

		ESCT_VehicleSpawnPointState stateToUpdate = m_SlotStateMap[id];
		if (!stateToUpdate)
			return;
		
		stateToUpdate.m_bIsGunnerAlive = newState.m_bIsGunnerAlive;
		stateToUpdate.b_mIsUsed = newState.b_mIsUsed;
		m_SlotStateMap[id] = stateToUpdate;
	}
	
	private void GetCompositionSlots(IEntity composition)
	{
		array<IEntity> children = {};
		ESCT_EntityHelper.GetAllChildren(composition, children);

		IEntity entity;
		for (int i = 0; i < children.Count(); i++)
		{
			entity = children[i];
			if (entity.Type() != ESCT_VehicleSpawnPoint)
				continue;
			
			ESCT_VehicleSpawnPoint slot = ESCT_VehicleSpawnPoint.Cast(entity);
			m_aVehiclePoints.Insert(slot);
		}
	}

	private void AssignSlots()
	{
		if (!m_Location.IsVisited())
		{
			ESCT_VehicleSpawnPoint point;
			for (int i = 0 ; i < m_aVehiclePoints.Count(); i++)
			{
				point = m_aVehiclePoints[i];
				point.SetVehicleManager(this);
				point.FirstSetup(m_Location.GetOwnerFaction());
			}
			
			return;
		}
		
		//if we're revisiting location then it means that the state for them (respawned points with the same ids) 
		//is already there so we need to just pass the manager
		ESCT_VehicleSpawnPoint point;
		for (int i = 0 ; i < m_aVehiclePoints.Count(); i++)
		{
			point = m_aVehiclePoints[i];
			point.SetVehicleManager(this);
		}
	}
}
