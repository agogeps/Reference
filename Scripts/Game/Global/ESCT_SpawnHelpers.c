class ESCT_SpawnHelpers
{
	static IEntity SpawnEntityPrefab(ResourceName prefab, vector origin, vector rotator = "0 0 0", bool global = true)
	{
		EntitySpawnParams spawnParams = EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		spawnParams.Transform[3] = origin;

		Math3D.AnglesToMatrix(rotator, spawnParams.Transform);

		if (!global)
			return GetGame().SpawnEntityPrefabLocal(Resource.Load(prefab), params: spawnParams);

		return GetGame().SpawnEntityPrefab(Resource.Load(prefab), GetGame().GetWorld(), spawnParams);
	}

	static IEntity SpawnEntityPrefabMatrix(ResourceName prefab, vector mat[4], bool global = true)
	{
		EntitySpawnParams spawnParams = EntitySpawnParams();

		spawnParams.TransformMode = ETransformMode.WORLD;
		spawnParams.Transform = mat;

		if (!global)
			return GetGame().SpawnEntityPrefabLocal(Resource.Load(prefab), params: spawnParams);

		return GetGame().SpawnEntityPrefab(Resource.Load(prefab), GetGame().GetWorld(), spawnParams);
	}

	static IEntity SpawnVehiclePrefab(ResourceName prefab, vector position, vector directionVector, bool startEngine = true, bool handbrake = false)
	{
		vector transform[4];
		transform[3] = position + "0 1 0";
		Math3D.DirectionAndUpMatrix(directionVector, vector.Up, transform);

		EntitySpawnParams spawnParams = EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		spawnParams.Transform = transform;

		IEntity vehicle = GetGame().SpawnEntityPrefab(Resource.Load(prefab), GetGame().GetWorld(), spawnParams);
		if (!vehicle)
			return null;

		//make vehicle align terrain properly (some vehicles freeze in air on spawn without light physics "push")
		Physics physicsComponent = vehicle.GetPhysics();
		if (physicsComponent)
			physicsComponent.SetVelocity("0 -0.1 0");

		CarControllerComponent carController = CarControllerComponent.Cast(vehicle.FindComponent(CarControllerComponent));
		if (carController)
		{
			if (startEngine)
				carController.StartEngine();

			if (handbrake)
				carController.SetPersistentHandBrake(true)
		}

		return vehicle;
	}

	static IEntity SpawnEditablePrefab(ResourceName prefab, vector origin, vector orientation = "0 0 0")
	{
		vector transform[4];
		transform[3] = origin;
		Math3D.AnglesToMatrix(orientation, transform);

		Resource prefabResource = Resource.Load(prefab);
		if (!prefabResource || !prefabResource.IsValid())
		{
			Print(string.Format("Cannot create composition, error when loading prefab '%1'!", prefab), LogLevel.ERROR);
			return null;
		}

		SCR_EditorPreviewParams params = CreateParams(transform, prefabResource);
		if (!params)
			return null;

		SCR_EditableEntityComponent editableEntity = SCR_PlacingEditorComponent.SpawnEntityResource(params, prefabResource);
		if (!editableEntity)
			return null;

		return editableEntity.GetOwner();
	}

	private static SCR_EditorPreviewParams CreateParams(vector transform[4], Resource res)
	{
		IEntityComponentSource editableEntitySource = SCR_EditableEntityComponentClass.GetEditableEntitySource(res);
		if (!editableEntitySource)
			return null;

		bool hasHorizontalAlignment =
			SCR_Enum.HasPartialFlag(SCR_EditableEntityComponentClass.GetEntityFlags(editableEntitySource), EEditableEntityFlag.HORIZONTAL);

		SCR_EditorPreviewParams params = new SCR_EditorPreviewParams();
		Math3D.MatrixCopy(transform, params.m_vTransform);
		if (hasHorizontalAlignment)
		{
			//for buildings that shouldn't be aligned on terrain
			params.m_VerticalMode = EEditorTransformVertical.SEA;
		}
		else
		{
			params.m_VerticalMode = EEditorTransformVertical.TERRAIN;
		}
		params.m_bIsUnderwater = false;
		params.m_TargetInteraction = EEditableEntityInteraction.NONE;
		params.SetTarget(null);
		params.m_ParentID = Replication.INVALID_ID;
		params.m_bParentChanged = 1;
		return params;
	}
}
