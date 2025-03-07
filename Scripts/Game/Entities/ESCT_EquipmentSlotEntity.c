[EntityEditorProps(category: "Escapists/Entities", description: "Location equipment slot (Escapists game mode).", color: "0 0 255 255")]
class ESCT_EquipmentSlotEntityClass : GenericEntityClass
{
}

sealed class ESCT_EquipmentSlotEntity : GenericEntity
{
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Probability of spawn will be not checked and item will be always present on this point.", category: "Escapists")]
	bool m_IsSpawnGuaranteed;

	protected IEntity m_Entity;
	protected int m_iPersistentSlotId;
	protected ESCT_LocationItemManagerComponent m_LocationItemManager;
	protected ESCT_EquipmentSlotArsenalComponent m_ArsenalComponent;

	IEntity GetEntity()
	{
		return m_Entity;
	}

	void ESCT_EquipmentSlotEntity(IEntitySource src, IEntity parent)
	{
		if (SCR_Global.IsEditMode() || !Replication.IsServer()) return;

		SetEventMask(EntityEvent.INIT);
	}

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		m_ArsenalComponent = ESCT_EquipmentSlotArsenalComponent.Cast(owner.FindComponent(ESCT_EquipmentSlotArsenalComponent));
		if (!m_ArsenalComponent)
			ESCT_Logger.Error("Equipment slot arsenal component could not be found on ESCT_EquipmentSlotEntity entity");

		m_iPersistentSlotId = ESCT_IdentifierGenerator.GenerateIntId();
	}
	
	void SetItemManager(notnull ESCT_LocationItemManagerComponent itemManager)
	{
		m_LocationItemManager = itemManager;
	}

	void FirstSetupSlot(ESCT_Faction faction)
	{
		if(!m_LocationItemManager)
			return;

		Tuple2<SCR_ArsenalItem, SCR_ArsenalItemDisplayData> arsenalItem = m_ArsenalComponent.AssignItem(m_IsSpawnGuaranteed, faction);
		if (!arsenalItem)
			return;

		ESCT_EquipmentSlotState state = new ESCT_EquipmentSlotState(m_iPersistentSlotId, arsenalItem.param1, arsenalItem.param2);

		//ESCT_Logging.LogFormat("Saving SlotState id: %1, %2", param1: m_iPersistentSlotId.ToString(),  param2: arsenalItem.param1.GetItemResourceName());

		m_LocationItemManager.AddToSlotStateMap(m_iPersistentSlotId, state);
	}

	IEntity Spawn()
	{
		if (!m_LocationItemManager)
			return null;

		map<int, ref ESCT_EquipmentSlotState> stateMap = m_LocationItemManager.GetSlotStates();
		ESCT_EquipmentSlotState state = stateMap.Get(m_iPersistentSlotId);
		if (!state || state.b_mIsUsed)
			return null;

		//ESCT_Logging.LogFormat("Loading SlotState id: %1, %2", param1: m_iPersistentSlotId.ToString(), param2: state.GetArsenalItem().GetItemResourceName());

		IEntity item = m_ArsenalComponent.Spawn(state.GetArsenalItem(), state.GetDisplayData());
		if (!item)
			return null;

		m_Entity = item;

		InventoryItemComponent invComp = InventoryItemComponent.Cast(m_Entity.FindComponent(InventoryItemComponent));
		if (invComp)
			invComp.m_OnParentSlotChangedInvoker.Insert(OnInventoryParentChanged);

		return item;
	}

	private void OnInventoryParentChanged(InventoryStorageSlot oldSlot, InventoryStorageSlot newSlot)
	{
		InventoryItemComponent invComp = InventoryItemComponent.Cast(m_Entity.FindComponent(InventoryItemComponent));
		if (invComp)
			invComp.m_OnParentSlotChangedInvoker.Remove(OnInventoryParentChanged);

		//picking up item frees it from entity spawn position
		m_Entity = null;

		if (m_LocationItemManager)
			m_LocationItemManager.UpdateSlotState(new ESCT_EquipmentSlotState(m_iPersistentSlotId, null, null, true));
	}

#ifdef WORKBENCH
	private const float _DRAW_DISTANCE = 100 * 100;

	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		WorldEditorAPI api = this._WB_GetEditorAPI();
		if (!api)
			return;

		IEntitySource src = api.EntityToSource(this);
		if (!src)
			return;

		if (!api.IsEntityVisible(src) || !api.IsEntityLayerVisible(src.GetSubScene(), src.GetLayerID()))
			return;

		vector outMat[4];
		GetWorldTransform(outMat);

		vector cameraTransform[4];
		GetWorld().GetCurrentCamera(cameraTransform);

		int colorBackground = 0x00000000;
		if (api.IsEntitySelected(src))
		{
			colorBackground = ARGBF(1, 0, 0, 0);
		}
		else
		{
			//--- Draw only if near when not selected
			if (vector.DistanceSq(GetOrigin(), cameraTransform[3]) > _DRAW_DISTANCE)
				return;
		}

		const float radius = 0.1;

		SCR_EArsenalItemMode itemModes = m_ArsenalComponent.GetSupportedArsenalItemModes();
		SCR_EArsenalItemType itemTypes = m_ArsenalComponent.GetSupportedArsenalItemTypes();

		int color;
		string text;

		//TODO: mix colors and texts for slots with more types
		if (SCR_Enum.HasPartialFlag(itemModes, SCR_EArsenalItemMode.WEAPON | SCR_EArsenalItemMode.WEAPON_VARIANTS))
		{
			text = "Weapon";
			color = Color.Cyan.PackToInt();
		}
		else if (SCR_Enum.HasPartialFlag(itemTypes, SCR_EArsenalItemType.HEAL))
		{
			text = "Medicine";
			color = COLOR_BLUE;
		}
		else if (SCR_Enum.HasPartialFlag(itemModes, SCR_EArsenalItemMode.AMMUNITION | SCR_EArsenalItemMode.CONSUMABLE | SCR_EArsenalItemMode.ATTACHMENT))
		{
			text = "Equipment";
			color = COLOR_YELLOW;
		}

		vector position = outMat[3];
		ref Shape pointShape = Shape.CreateSphere(color, ShapeFlags.ONCE | ShapeFlags.NOOUTLINE, position, radius);
		ref Shape arrowShape = Shape.CreateArrow(position, position + outMat[2], 0.2, color, ShapeFlags.ONCE);
		ref DebugTextWorldSpace textShape = DebugTextWorldSpace.Create(
			GetWorld(),
			text,
			DebugTextFlags.ONCE | DebugTextFlags.CENTER | DebugTextFlags.FACE_CAMERA,
			position[0],
			position[1] + 0.2,
			position[2],
			14.0,
			color,
			colorBackground
		);

		if (m_IsSpawnGuaranteed)
		{
			ref Shape sphere = Shape.CreateSphere(0, ShapeFlags.ONCE | ShapeFlags.NOOUTLINE, position + "0 0.1 0", radius / 2);
		}

		super._WB_AfterWorldUpdate(timeSlice);
	}
#endif
}
