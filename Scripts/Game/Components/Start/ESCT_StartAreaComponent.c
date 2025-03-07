[EntityEditorProps("Escapists/Start", description: "Component that determines starting location.", color: "0 0 255 255")]
class ESCT_StartAreaComponentClass : ScriptComponentClass
{
}

class ESCT_StartAreaComponent : ScriptComponent
{
	[Attribute("0", UIWidgets.Auto, desc: "Should unused unstances of this composition be deleted after game start?")]
	protected bool m_bDeleteAfterStart;

	protected IEntity m_Owner;
	protected ESCT_SpawnPoint m_StartingSpawnPoint;

	private static const string AMBIENT_ENEMY_POINT = "{EB1DF9D81A55F5D9}Prefabs/Systems/AmbientVehicles/Escapists_AmbientVehicleSpawnpoint_Enemy.et";
	private static const string AMBIENT_CIV_POINT = "{4F5481F0B4E6D462}Prefabs/Systems/AmbientVehicles/Escapists_AmbientVehicleSpawnpoint_CIV.et";

	ESCT_SpawnPoint GetStartingSpawnPoint()
	{
		if (!m_StartingSpawnPoint)
			FindSpawnPointInHierarchy();
		
		return m_StartingSpawnPoint;
	}

	bool ShouldDeleteAfterStart()
	{
		return m_bDeleteAfterStart;
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

		if (!TryAddToStartAreaPool())
			return;

		FindSpawnPointInHierarchy();
	}

	void Setup()
	{
		EnableSpawnPoint();
		EnsureNearbyVehicleSpawn();

		ESCT_Location location = ESCT_Location.Cast(GetOwner());
		if (location)
			location.AddSpawnReason(ESCT_ELocationSpawnReason.Start);

		ESCT_GameStateManagerComponent.GetInstance().GetOnGameStateChanged().Insert(HandleGameStateChanged);
	}

	protected bool TryAddToStartAreaPool()
	{
		ESCT_LocationSystem locationSystem = ESCT_LocationSystem.GetSystem();
		if (!locationSystem)
			return false;

		ESCT_EntitySearch entitySearch = new ESCT_EntitySearch();
		array<IEntity> disableEntities = entitySearch.FindEntitiesByPrefabInRadius(m_Owner.GetOrigin(), 4, ESCT_Constants.DISABLE_LOCATION);
		if (!disableEntities.IsEmpty())
			return false;

		locationSystem.AddStartArea(m_Owner);

		return true;
	}
	
	private void FindSpawnPointInHierarchy()
	{
		if (m_StartingSpawnPoint)
			return;
		
		array<IEntity> children = {};
		ESCT_EntityHelper.GetAllChildren(GetOwner(), children);
		foreach (IEntity entity : children)
		{
			if (entity.Type() == ESCT_SpawnPoint)
			{
				m_StartingSpawnPoint = ESCT_SpawnPoint.Cast(entity);
				break;
			}
		}
	}
	

	private void EnableSpawnPoint()
	{
		string factionKey = ESCT_FactionManager.GetInstance().GetPlayerFactionKey();
		m_StartingSpawnPoint.SetFactionKey(factionKey);
		m_StartingSpawnPoint.SetSpawnPointEnabled_S(true);
	}

	private void EnsureNearbyVehicleSpawn()
	{
		if (!m_Owner)
			return;

		ESCT_EntitySearch entitySearch = new ESCT_EntitySearch();
		array<IEntity> ambientVehiclePoints = {};
		ambientVehiclePoints.InsertAll(entitySearch.FindEntitiesByPrefabInRadius(m_Owner.GetOrigin(), 300, AMBIENT_CIV_POINT));
		ambientVehiclePoints.InsertAll(entitySearch.FindEntitiesByPrefabInRadius(m_Owner.GetOrigin(), 300, AMBIENT_ENEMY_POINT));

		if (ambientVehiclePoints.IsEmpty())
			return;

		for (int i = 0; i < ambientVehiclePoints.Count(); i++)
		{
			ESCT_AmbientVehicleSpawnPointComponent spawnPointComponent =
				ESCT_AmbientVehicleSpawnPointComponent.Cast(ambientVehiclePoints[i].FindComponent(ESCT_AmbientVehicleSpawnPointComponent));

			if (!spawnPointComponent)
				continue;

			spawnPointComponent.Register();
		}
	}

	private void HandleGameStateChanged(ESCT_EGameState gameState)
	{
		if (gameState != ESCT_EGameState.RadioStationSearch)
			return;

		ESCT_TaskAssignerComponent taskAssigner = ESCT_TaskAssignerComponent.GetInstance();
		if (!taskAssigner)
			return;

		taskAssigner.SetupRadioTaskAssignmentChecker(m_Owner);
		ESCT_GameStateManagerComponent.GetInstance().GetOnGameStateChanged().Remove(HandleGameStateChanged);
	}


#ifdef WORKBENCH
	protected const float _DRAW_DISTANCE = 100 * 100;

	override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
	{
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

		int color = Color.White.PackToInt();
		const string text = "Start Area";

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
