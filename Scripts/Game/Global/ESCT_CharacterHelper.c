class ESCT_CharacterHelper
{
	static IEntity SpawnCharacter(ResourceName prefab, vector origin = vector.Zero, vector orientation = "0 0 0", bool enableAI = true, ResourceName groupPrefab = "{000CD338713F2B5A}Prefabs/AI/Groups/Group_Base.et")
	{
		EntitySpawnParams spawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;

		Math3D.AnglesToMatrix(orientation, spawnParams.Transform);
		spawnParams.Transform[3] = origin;

		IEntity character = GetGame().SpawnEntityPrefab(Resource.Load(prefab), GetGame().GetWorld(), spawnParams);
		if (!character)
		{
			Print(string.Format("[Escapists] Failed to spawn character. Check if ResourceName is correct! Path: '%1'", prefab), LogLevel.ERROR);
			return null;
		}

		if (enableAI)
			EnableAI(character, groupPrefab);

		return character;
	}

	static IEntity SpawnCharacterForGroup(ResourceName prefab, notnull SCR_AIGroup group, vector origin, vector orientation = "0 0 0", bool enableAI = true)
	{
		EntitySpawnParams spawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;

		Math3D.AnglesToMatrix(orientation, spawnParams.Transform);
		spawnParams.Transform[3] = origin;

		IEntity character = GetGame().SpawnEntityPrefab(Resource.Load(prefab), GetGame().GetWorld(), spawnParams);
		if (!character)
		{
			Print(string.Format("[Escapists] Failed to spawn character. Check if ResourceName is correct! Path: '%1'", prefab), LogLevel.ERROR);
			return null;
		}

		AIControlComponent agentControlComponent = AIControlComponent.Cast(character.FindComponent(AIControlComponent));
		if (!agentControlComponent)
		{
			Print("[Escapists] Could not get AIControlComponent from spawned character!", LogLevel.ERROR);
			return character;
		}

		if (enableAI)
		{
			agentControlComponent.ActivateAI();
		}

		FactionAffiliationComponent factionComponent = FactionAffiliationComponent.Cast(character.FindComponent(FactionAffiliationComponent));
		if (factionComponent)
		{
			Faction faction = factionComponent.GetAffiliatedFaction();
			if (faction)
				group.InitFactionKey(faction.GetFactionKey());
		}

		//~ Add the entity to the group
		group.AddAgent(agentControlComponent.GetControlAIAgent());

		return character;
	}

	static bool EnableAI(notnull IEntity character, ResourceName groupPrefab = "{000CD338713F2B5A}Prefabs/AI/Groups/Group_Base.et")
	{
		AIControlComponent agentControlComponent = AIControlComponent.Cast(character.FindComponent(AIControlComponent));
		if (!agentControlComponent)
		{
			Print("[Escapists] Could not get AIControlComponent from spawned character!", LogLevel.ERROR);
			return false;
		}

		agentControlComponent.ActivateAI();

		IEntity groupEntity = GetGame().SpawnEntityPrefab(Resource.Load(groupPrefab));
		if (!groupEntity)
		{
			Print("[Escapists] Could not create group for spawned character. Most likly incorrect group prefab given!", LogLevel.ERROR);
			return character;
		}

		AIGroup group = AIGroup.Cast(groupEntity);
		if (!group)
		{
			Print("[Escapists] Could not create group for spawned character as spawned group prefab is missing AIGroup component!", LogLevel.ERROR);
			return false;
		}

		//~ Set group's faction
		SCR_AIGroup groupScripted = SCR_AIGroup.Cast(group);
		if (groupScripted)
		{
			FactionAffiliationComponent factionComponent = FactionAffiliationComponent.Cast(character.FindComponent(FactionAffiliationComponent));
			if (factionComponent)
			{
				Faction faction = factionComponent.GetAffiliatedFaction();
				if (faction)
					groupScripted.InitFactionKey(faction.GetFactionKey());
			}
		}

		//~ Add the entity to the group
		group.AddAgent(agentControlComponent.GetControlAIAgent());

		return true;
	}

	static SCR_AIGroup CreateGroup(string factionKey, ResourceName groupPrefab = "{000CD338713F2B5A}Prefabs/AI/Groups/Group_Base.et")
	{
		IEntity groupEntity = GetGame().SpawnEntityPrefab(Resource.Load(groupPrefab));
		if (!groupEntity)
		{
			Print("[Escapists] Could not create group. Most likly incorrect group prefab given!", LogLevel.ERROR);
			return null;
		}

		AIGroup group = AIGroup.Cast(groupEntity);
		if (!group)
		{
			Print("[Escapists] Could not create group as spawned group prefab is missing AIGroup component!", LogLevel.ERROR);
			return null;
		}

		SCR_AIGroup groupScripted = SCR_AIGroup.Cast(group);
		if (!groupScripted)
		{
			Print("[Escapists] Could not create group as spawned group prefab is missing AIGroup component!", LogLevel.ERROR);
			return null;
		}

		groupScripted.InitFactionKey(factionKey);

		return groupScripted;
	}

	static void AddToGroup(notnull IEntity character, notnull SCR_AIGroup group)
	{
		AIControlComponent agentControlComponent = AIControlComponent.Cast(character.FindComponent(AIControlComponent));
		if (!agentControlComponent)
		{
			ESCT_Logger.Error("Could not get AIControlComponent from spawned character!");
			return;
		}

		group.AddAgent(agentControlComponent.GetControlAIAgent());
	}
	
	static void RandomizeClothes(IEntity character)
	{
		InventoryStorageManagerComponent inventoryStorage = InventoryStorageManagerComponent.Cast(character.FindComponent(InventoryStorageManagerComponent));
		if (!inventoryStorage)
			return;
		
		ESCT_Faction faction = ESCT_FactionManager.GetInstance().GetPlayerFaction();
		if (!faction)
			return;
		
		ESCT_PlayerFactionInfo info = faction.GetPlayerFactionInfo();
		if (!info)
			return;
		
		array<ref ESCT_EquipmentInfo> equipmentPairs = info.GetEquipmentPairs();
		if (equipmentPairs.IsEmpty())
			return;

		array<float> weights = {};
		foreach (ESCT_EquipmentInfo epair : equipmentPairs)
		{
			weights.Insert(epair.m_fWeight);
		}
		
		ESCT_EquipmentInfo equipInfo = equipmentPairs[SCR_ArrayHelper.GetWeightedIndex(weights, Math.RandomFloat01())];
		if (equipInfo.m_Pants)
		{
			inventoryStorage.TrySpawnPrefabToStorage(equipInfo.m_Pants, null, -1, EStoragePurpose.PURPOSE_DEPOSIT);
		}

		if (equipInfo.m_Jacket)
		{
			inventoryStorage.TrySpawnPrefabToStorage(equipInfo.m_Jacket, null, -1, EStoragePurpose.PURPOSE_DEPOSIT);
		}
		
		if (equipInfo.m_Footwear)
		{
			inventoryStorage.TrySpawnPrefabToStorage(equipInfo.m_Footwear, null, -1, EStoragePurpose.PURPOSE_DEPOSIT);
		}
		
		if (equipInfo.m_Headgear)
		{
			inventoryStorage.TrySpawnPrefabToStorage(equipInfo.m_Headgear, null, -1, EStoragePurpose.PURPOSE_DEPOSIT);
		}
		
		if (equipInfo.m_Vest)
		{
			inventoryStorage.TrySpawnPrefabToStorage(equipInfo.m_Vest, null, -1, EStoragePurpose.PURPOSE_DEPOSIT);
		}

		ESCT_EscapistsGameMode gameMode = ESCT_EscapistsGameMode.GetGameMode();
		if (gameMode.GetStartType() == ESCT_EStartType.Hideout || gameMode.GetStartType() == ESCT_EStartType.Helicrash)
		{
			ResourceName flashlightPrefab = info.GetFlashlightPrefab();
			if (!flashlightPrefab)
				return;
			
			TimeAndWeatherManagerEntity timeAndWeatherManager;
			ChimeraWorld world = ChimeraWorld.CastFrom(character.GetWorld());
			if (world)
				timeAndWeatherManager = world.GetTimeAndWeatherManager();
			
			if (!timeAndWeatherManager || !timeAndWeatherManager.IsSunSet())
				return;
			
			inventoryStorage.TrySpawnPrefabToStorage(flashlightPrefab, null, -1, EStoragePurpose.PURPOSE_EQUIPMENT_ATTACHMENT);
		}
	}
	
	static SCR_AIGroup GetAIGroup(IEntity entity)
	{
		if (!entity)
			return null;

		SCR_AIGroup aiGroup = SCR_AIGroup.Cast(entity);
		if (aiGroup)
			return aiGroup;

		AIAgent agent = AIAgent.Cast(entity);
		if (agent)
			return SCR_AIGroup.Cast(agent.GetParentGroup());

		if (!SCR_ChimeraCharacter.Cast(entity))
			return null;

		AIControlComponent control = AIControlComponent.Cast(entity.FindComponent(AIControlComponent));
		if (!control)
			return null;

		agent = control.GetControlAIAgent();
		if (!agent)
			return null;

		return SCR_AIGroup.Cast(agent.GetParentGroup());
	}
	
	static bool CanFight(IEntity characterEntity)
	{
		if (!characterEntity)
			return false;
		
		SCR_CharacterControllerComponent charController = SCR_CharacterControllerComponent.Cast(characterEntity.FindComponent(SCR_CharacterControllerComponent));
		if (!charController)
			return false;
		
		return charController.GetLifeState() == ECharacterLifeState.ALIVE;
	}
	
	static SCR_ChimeraAIAgent GetAIAgent(IEntity character)
	{
		if (!character)
			return null;
		
		AIControlComponent controlComp = AIControlComponent.Cast(character.FindComponent(AIControlComponent));
		if (!controlComp)
			return null;
		
		SCR_ChimeraAIAgent agent = SCR_ChimeraAIAgent.Cast(controlComp.GetAIAgent());
		return agent;
	}
	
	static void SpawnGunner(notnull IEntity vehicle, notnull ESCT_Faction faction, ResourceName gunnerPrefab, AIGroup group)
	{
		SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(vehicle.FindComponent(SCR_BaseCompartmentManagerComponent));
		if (!compartmentManager)
			return;

		array<BaseCompartmentSlot> compartments = {};
		compartmentManager.GetFreeCompartmentsOfType(compartments, ECompartmentType.TURRET);
		
		foreach (BaseCompartmentSlot slot : compartments)
		{
			slot.SpawnCharacterInCompartment(gunnerPrefab, group, faction.GetBaseFactionGroup());
		}
	}
}
