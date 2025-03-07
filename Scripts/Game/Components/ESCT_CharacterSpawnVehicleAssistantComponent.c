[EntityEditorProps("Escapists", description: "Handles character spawns inside vehicle.", color: "0 0 255 255")]
class ESCT_CharacterSpawnVehicleAssistantComponentClass : ScriptComponentClass
{
}

sealed class ESCT_CharacterSpawnVehicleAssistantComponent : ScriptComponent
{
	protected PlayerManager m_PlayerManager;
	protected SCR_BaseCompartmentManagerComponent m_CompartmentManager;
	protected IEntity m_Helicopter;

	private WorldTimestamp m_fLastAddTimestamp;
	private ref array<BaseCompartmentSlot> m_aFreeCompartmentsStack;
	private ref map<int, BaseCompartmentSlot> m_mPlayerSlotMap;
	private bool m_bIsPuttingIntoHeli = false;
	private bool m_bIsDone = false;

	private static const float WAIT_FOR_CONNECTIONS_SECONDS = 1;

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (SCR_Global.IsEditMode() || !Replication.IsServer()) return;

		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);
	}

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (SCR_Global.IsEditMode() || !Replication.IsServer()) return;

		m_Helicopter = owner;
		m_mPlayerSlotMap = new map<int, BaseCompartmentSlot>();
		m_CompartmentManager = SCR_BaseCompartmentManagerComponent.Cast(m_Helicopter.FindComponent(SCR_BaseCompartmentManagerComponent));
		m_PlayerManager = GetGame().GetPlayerManager();

		m_aFreeCompartmentsStack = {};
		m_CompartmentManager.GetCompartments(m_aFreeCompartmentsStack);
	}

	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);

		if (!m_fLastAddTimestamp)
			return;

		ChimeraWorld world = GetGame().GetWorld();
		if (!world || world.GetServerTimestamp().Less(m_fLastAddTimestamp.PlusSeconds(WAIT_FOR_CONNECTIONS_SECONDS)))
			return;

		PutIntoCompartments();
		ClearEventMask(m_Helicopter, EntityEvent.FRAME);
	}

	bool TrySchedulePlayerForHelicopter(notnull IEntity player, int playerId)
	{
		if (!Replication.IsServer() || m_bIsDone || !m_Helicopter || !m_CompartmentManager || m_aFreeCompartmentsStack.IsEmpty())
			return false;

		BaseCompartmentSlot slot = m_aFreeCompartmentsStack.Get(0);
		if (!slot)
			return false;

		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return false;

		if (m_mPlayerSlotMap.Insert(playerId, slot))
		{
			m_aFreeCompartmentsStack.Remove(0);
			m_fLastAddTimestamp = world.GetServerTimestamp();
		}

		//if everything goes ok, we may spawn units from the get go,
		//otherwise (if someone drops out or crashes,for example) onframe will spawn them after waiting for additional connections
		if (m_PlayerManager.GetPlayerCount() == m_mPlayerSlotMap.Count())
		{
			PutIntoCompartments();
			ClearEventMask(m_Helicopter, EntityEvent.FRAME);
		}

		return true;
	}

	protected void PutIntoCompartments()
	{
		if (m_bIsPuttingIntoHeli || m_mPlayerSlotMap.IsEmpty() || !m_Helicopter)
			return;

		m_bIsPuttingIntoHeli = true;

		ChimeraWorld world = GetGame().GetWorld();

		int index = 0;
		foreach (int pid, BaseCompartmentSlot slot : m_mPlayerSlotMap)
		{
			IEntity playerCharacter = m_PlayerManager.GetPlayerControlledEntity(pid);
			if (!playerCharacter)
				continue;

			SCR_CompartmentAccessComponent compAccess = SCR_CompartmentAccessComponent.Cast(playerCharacter.FindComponent(SCR_CompartmentAccessComponent));
			if (!compAccess)
				continue;

			if (!compAccess.MoveInVehicle(m_Helicopter, ECompartmentType.CARGO, world.IsGameTimePaused()))
			{
				if (compAccess.IsInCompartment())
					compAccess.GetOutVehicle(EGetOutType.TELEPORT, -1, ECloseDoorAfterActions.INVALID, false);

				//works without this thing, but in case of problems probably could be turned on
				//UpdateCompartment(m_Helicopter);
			}

			//first slot is always pilot slot
			if (index < 2)
			{
				SCR_InventoryStorageManagerComponent invManager = SCR_InventoryStorageManagerComponent.Cast(playerCharacter.FindComponent(SCR_InventoryStorageManagerComponent));
				if (invManager)
				{
					StripCharacter(playerCharacter, invManager);
					EquipPilotOutfit(invManager);
				}
			}

			index++;
		}

		//no longer needed, mission accomplished
		m_mPlayerSlotMap.Clear();
		m_aFreeCompartmentsStack.Clear();

		m_bIsDone = true;
	}

	private void EquipPilotOutfit(SCR_InventoryStorageManagerComponent invStorageManager)
	{
		ESCT_Faction faction = ESCT_FactionManager.GetInstance().GetPlayerFaction();
		if (!faction)
			return;

		ESCT_PlayerFactionInfo info = faction.GetPlayerFactionInfo();
		if (!info)
			return;

		ESCT_EquipmentInfo equipInfo = info.GetPilotEquipment();
		if (!equipInfo)
			return;

		if (equipInfo.m_Pants)
		{
			invStorageManager.TrySpawnPrefabToStorage(equipInfo.m_Pants, null, -1, EStoragePurpose.PURPOSE_DEPOSIT);
		}

		if (equipInfo.m_Jacket)
		{
			invStorageManager.TrySpawnPrefabToStorage(equipInfo.m_Jacket, null, -1, EStoragePurpose.PURPOSE_DEPOSIT);
		}

		if (equipInfo.m_Footwear)
		{
			invStorageManager.TrySpawnPrefabToStorage(equipInfo.m_Footwear, null, -1, EStoragePurpose.PURPOSE_DEPOSIT);
		}

		if (equipInfo.m_Headgear)
		{
			invStorageManager.TrySpawnPrefabToStorage(equipInfo.m_Headgear, null, -1, EStoragePurpose.PURPOSE_DEPOSIT);
		}

		if (equipInfo.m_Vest)
		{
			invStorageManager.TrySpawnPrefabToStorage(equipInfo.m_Vest, null, -1, EStoragePurpose.PURPOSE_DEPOSIT);
		}
	}

	private void StripCharacter(IEntity character, SCR_InventoryStorageManagerComponent invStorageManager)
	{
		//Remove old items
		array<IEntity> rootItems = {};
		invStorageManager.GetAllRootItems(rootItems);

		IEntity itemToRemove;
		for (int i = 0; i < rootItems.Count(); i++)
		{
			itemToRemove = rootItems[i];
			if (!itemToRemove)
				continue;

			invStorageManager.TryRemoveItemFromInventory(itemToRemove); //remove first so we dont get registry errors for gadgets
			SCR_EntityHelper.DeleteEntityAndChildren(itemToRemove); //then delete old items
		}
	}

	//------------------------------------------------------------------------------------------------
	//--- #HACK - when the game is paused, unpause it to ensure that the character is moved in/out of a compartment, which is an operation that needs simulation to be running
	protected void UpdateCompartment(IEntity newParent)
	{
		ChimeraWorld world = GetGame().GetWorld();
		if (world.IsGameTimePaused())
		{
			world.PauseGameTime(false);
			GetGame().GetCallqueue().CallLater(WaitForCompartmentUpdate, 1, true, newParent);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void WaitForCompartmentUpdate(IEntity newParent)
	{
		ChimeraWorld world = GetGame().GetWorld();
		if (world.IsGameTimePaused() || !GetOwner() || CompartmentAccessComponent.GetVehicleIn(GetOwner()) == newParent)
		{
			world.PauseGameTime(true);
			GetGame().GetCallqueue().Remove(WaitForCompartmentUpdate);
		}
	}
}
