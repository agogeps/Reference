[EntityEditorProps("Escapists/Components/StaticLocations", description: "Component for static (no dynamic spawnable composition) locations.", color: "0 0 255 255")]
class ESCT_StaticLocationComponentClass : ScriptComponentClass
{
}

class ESCT_StaticLocationComponent : ScriptComponent
{
	protected IEntity m_Owner;
	
	ESCT_ELocationType GetLocationType()
	{
		return ESCT_ELocationType.Undefined;
	}
	
	ESCT_Faction GetOwnerFaction()
	{
		ESCT_FactionManager factionManager = ESCT_FactionManager.GetInstance();
		if (!factionManager)
			return null;
		
		return factionManager.GetEnemyFaction();
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

		m_Owner = owner;

		TryAddToStaticPool();
	}

	private void TryAddToStaticPool()
	{
		ESCT_LocationSystem locationSystem = ESCT_LocationSystem.GetSystem();	
		if (!locationSystem)
			return;
		
		ESCT_EntitySearch entitySearch = new ESCT_EntitySearch();
		array<IEntity> disablePrisonEntities = entitySearch.FindEntitiesByPrefabInRadius(m_Owner.GetOrigin(), 4, ESCT_Constants.DISABLE_LOCATION);
		if (!disablePrisonEntities.IsEmpty())
			return;
		
		locationSystem.AddStaticLocation(m_Owner);
	}
	
#ifdef WORKBENCH
	private const float _DRAW_DISTANCE = 100 * 100;

	override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
	{
		super._WB_AfterWorldUpdate(owner, timeSlice);

		GenericEntity genericOwner = GenericEntity.Cast(owner);
		if (!genericOwner)
			return;

		WorldEditorAPI api = genericOwner._WB_GetEditorAPI();
		if (!api)
			return;

		IEntitySource src = api.EntityToSource(owner);
		if (!src)
			return;

		if (!api.IsEntityVisible(src) || !api.IsEntityLayerVisible(src.GetSubScene(), src.GetLayerID()))
			return;

		vector cameraTransform[4];
		owner.GetWorld().GetCurrentCamera(cameraTransform);

		int colorBackground = 0x00000000;
		if (api.IsEntitySelected(src))
		{
			colorBackground = ARGBF(1, 0, 0, 0);
		}
		else
		{
			//--- Draw only if near when not selected
			if (vector.DistanceSq(owner.GetOrigin(), cameraTransform[3]) > _DRAW_DISTANCE)
				return;
		}

		int color;
		string text;

		switch (GetLocationType())
		{
			case ESCT_ELocationType.RadioStation:
			{
				color = Color.Yellow.PackToInt();
				text = "Static Radio Station";
				break;
			}
			case ESCT_ELocationType.Hospital:
			{
				color = Color.Red.PackToInt();
				text = "Static Hospital";
				break;
			}
			default:
			{
				color = Color.White.PackToInt();
				break;
			}
		}

		vector outMat[4];
		genericOwner.GetWorldTransform(outMat);

		vector position = outMat[3];

		ref DebugTextWorldSpace textShape = DebugTextWorldSpace.Create(
			genericOwner.GetWorld(),
			text,
			DebugTextFlags.ONCE | DebugTextFlags.CENTER | DebugTextFlags.FACE_CAMERA,
			position[0],
			position[1] + 16,
			position[2],
			18.0,
			color,
			colorBackground
		);
	}
#endif
}
