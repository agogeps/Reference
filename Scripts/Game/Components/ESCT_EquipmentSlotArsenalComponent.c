[EntityEditorProps("Escapists", description: "Equipment slot arsenal component.", color: "0 0 255 255")]
class ESCT_EquipmentSlotArsenalComponentClass : ScriptComponentClass
{
}

class ESCT_EquipmentSlotArsenalComponent : ScriptComponent
{
	[Attribute("0", desc: "Type of display data an arsenal item needs in in order to be displayed on this arsenal display", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EArsenalItemDisplayType), category: "Display settings")]
	protected EArsenalItemDisplayType m_eArsenalItemDisplayType;
	
	[Attribute("", desc: "Toggle supported SCR_EArsenalItemType by this arsenal, items are gathered from SCR_Faction or from the overwrite config", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(SCR_EArsenalItemType), category: "Escapists")]
	protected SCR_EArsenalItemType m_eSupportedArsenalItemTypes;

	[Attribute("", desc: "Toggle supported SCR_EArsenalItemMode by this arsenal, items are gathered from SCR_Faction or from the overwrite config", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(SCR_EArsenalItemMode), category: "Escapists")]
	protected SCR_EArsenalItemMode m_eSupportedArsenalItemModes;
	
	[Attribute("0", uiwidget: UIWidgets.CheckBox, desc: "Use player faction to fill this arsenal.", params: "0 1 0.05", category: "Settings")]
	protected bool m_bUsePlayerFaction;
	
	protected ESCT_Faction m_Faction;
	protected bool m_bIsItemAssigned = false;
	
	SCR_EArsenalItemType GetSupportedArsenalItemModes()
	{
		return m_eSupportedArsenalItemModes;
	}
	
	SCR_EArsenalItemMode GetSupportedArsenalItemTypes()
	{
		return m_eSupportedArsenalItemTypes;
	}
	
	Tuple2<SCR_ArsenalItem, SCR_ArsenalItemDisplayData> AssignItem(bool isSpawnGuaranateed, ESCT_Faction faction)
	{		
		if (m_bIsItemAssigned)
			return null;
		
		if (m_bUsePlayerFaction)
			m_Faction = ESCT_FactionManager.GetInstance().GetPlayerFaction();	
		else 
			m_Faction = faction;	
		
		if (!isSpawnGuaranateed && IsSlotInvalid())
			return null;
		
		array<SCR_ArsenalItem> filteredArsenalItems = {};
		if (!GetFiliteredItems(filteredArsenalItems))
		    return null;
		
		int availableItemCount = filteredArsenalItems.Count();
		SCR_ArsenalItem itemToSpawn;
		
		// Use a single loop to create the weights array and find the weighted index
		float totalWeight = 0;
		for (int j = 0; j < availableItemCount; j++)
		{
		    float weight = filteredArsenalItems[j].GetWeight();
		    totalWeight += weight;
		}
		
		float randomValue = Math.RandomFloat01() * totalWeight;
		float cumulativeWeight = 0;
		for (int j = 0; j < availableItemCount; j++)
		{
		    cumulativeWeight += filteredArsenalItems[j].GetWeight() + (Math.RandomFloat01() * 0.01); // add a small random offset;
		    if (randomValue <= cumulativeWeight)
		    {
		        itemToSpawn = filteredArsenalItems[j];
		        break;
		    }
		}
		
		m_bIsItemAssigned = true;
		
		if (!itemToSpawn)
			return null;
	
		return new Tuple2<SCR_ArsenalItem, SCR_ArsenalItemDisplayData>(itemToSpawn, itemToSpawn.GetDisplayDataOfType(m_eArsenalItemDisplayType));
	}
	
	IEntity Spawn(notnull SCR_ArsenalItem arsenalItem, SCR_ArsenalItemDisplayData itemDisplayData = null)
	{
		EntitySpawnParams params = new EntitySpawnParams();
		params.Parent = GetOwner();
		params.TransformMode = ETransformMode.WORLD;
		
		IEntity item = GetGame().SpawnEntityPrefab(arsenalItem.GetItemResource(), GetOwner().GetWorld(), params);
		if (!item)
			return null;
		
		// Since item is in hierarchy in which parent (slotEntity) is not traceable (also has no volume), we'll have
		// to clear its proxy flag so we can trace the item safely. Proxy flag is reset on pickup via item slots
		item.ClearFlags(EntityFlags.PROXY);
		
		vector positionOffset, rotationOffset = vector.Zero;
		bool diffusion = false;
		
		if (itemDisplayData)
		{
			positionOffset = itemDisplayData.GetItemOffset();
			rotationOffset = itemDisplayData.GetItemRotation();
			diffusion = itemDisplayData.ShouldCorrectDiffusion();
		}
		else
		{
			diffusion = true;
			
			//usually mags and weapons should lie on side
			if (!item.FindComponent(BaseLoadoutClothComponent) && !item.FindComponent(SCR_MapGadgetComponent)) {
				rotationOffset = "0 0 90";
			}
		}
		
		// set weapon rotation
		item.SetAngles(rotationOffset);
		
		// offset the item locally
		vector mat[4];
		item.GetLocalTransform(mat);
		mat[3] = mat[3] + positionOffset;
		item.SetLocalTransform(mat);
		
		if (diffusion)
			CorrectDiffusePosition(item);
		
		if (item.FindComponent(MagazineComponent))
			item.SetFlags(EntityFlags.PROXY);
		
		item.ClearFlags(EntityFlags.ACTIVE);
		
		InventoryItemComponent itemComp = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (itemComp)
			itemComp.DisablePhysics();
		
		item.Update();

		return item;
	}
	
	protected void CorrectDiffusePosition(IEntity equipment)
	{
		GenericEntity item = GenericEntity.Cast(equipment);
		BaseWorld world = GetGame().GetWorld();

		TraceBox trace = new TraceBox;
		trace.Flags = TraceFlags.ENTS | TraceFlags.WORLD;
		trace.Start = item.GetOrigin();
		trace.Mins = "-0.001 0 -0.001";
		trace.Maxs = "0.001 0.5 0.001";
		
		float penetrationDepth = world.TracePosition(trace, null);
		if (penetrationDepth < 0)
		{
			penetrationDepth = penetrationDepth / 2;
			vector currentPosition = item.GetOrigin();
			currentPosition[1] = currentPosition[1] - penetrationDepth;
			item.SetOrigin(currentPosition);
		}
	}
	
	protected bool IsSlotInvalid()
	{
		float spawnChance = ESCT_EscapistsConfigComponent.GetInstance().GetItemSpawnChance();
		if (ESCT_EscapistsGameMode.GetGameMode().GetRandomGenerator().RandFloat01() > spawnChance)
			return true;

		return false;
	}
	
	protected bool GetFiliteredItems(out notnull array<SCR_ArsenalItem> filteredArsenalItems)
	{
		if (!m_Faction)
			return false;
		
		//~ Cannot get items if no entity catalog manager
		SCR_EntityCatalogManagerComponent catalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		if (!catalogManager)
			return false;

		//TODO: здеся можем получать вообще все предметы и проверять их арсенал дисплей тайп, если его нет - поворачиваем на бочок, диффузия по дефолту
		
		//~ Get filtered list from faction (if any) or default
		filteredArsenalItems = catalogManager.GetFilteredArsenalItems(m_eSupportedArsenalItemTypes, m_eSupportedArsenalItemModes, -1, m_Faction);
		
		return !filteredArsenalItems.IsEmpty();
	}
}
