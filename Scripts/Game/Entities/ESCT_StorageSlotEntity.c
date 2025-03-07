[EntityEditorProps(category: "Escapists/Entities", description: "A weapon rack slot to be filled with a weapon (Escapists game mode).", color: "0 0 255 255")]
class ESCT_StorageSlotEntityClass : GenericEntityClass
{
}

sealed class ESCT_StorageSlotEntity : GenericEntity
{
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Use player faction storages.", category: "Escapists")]
	bool m_bUsePlayerFaction;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Probability of spawn will be not checked and item will be always present on this point.", category: "Escapists")]
	bool m_IsSpawnGuaranteed;
	
	IEntity SpawnStorage()
	{
		ESCT_Faction faction = null;
		if (m_bUsePlayerFaction)
		{
			faction = ESCT_FactionManager.GetInstance().GetPlayerFaction();
		}
		else 
		{
			faction = ESCT_FactionManager.GetInstance().GetEnemyFaction();
		}

		if (!faction)
			return null;
		
		SCR_EntityCatalog catalog = faction.GetFactionEntityCatalogOfType(EEntityCatalogType.STORAGE);
		if (!catalog)
			return null;
		
		array<SCR_EntityCatalogEntry> storages = {};
		catalog.GetEntityList(storages);
		
		if (storages.IsEmpty())
			return null;
		
		SCR_EntityCatalogEntry storageToSpawn = storages.GetRandomElement();
		ResourceName prefab = storageToSpawn.GetPrefab();
		if (!prefab)
			return null;
		
		vector transform[4];
		GetTransform(transform);
		
		return ESCT_SpawnHelpers.SpawnEntityPrefabMatrix(prefab, transform);
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

		const float radius = 0.1;

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

		int color = COLOR_GREEN;
		const string text = "Storage";

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
