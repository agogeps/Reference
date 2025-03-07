[EntityEditorProps("Prison", description: "Component that allows building to be used as starting prison (spawn point).", color: "0 0 255 255")]
class ESCT_PrisonComponentClass : ESCT_StartAreaComponentClass
{
}

sealed class ESCT_PrisonComponent : ESCT_StartAreaComponent
{
	[Attribute(defvalue: "Door_PrisonCell_E_A_Left_INT_ST_B.xob", uiwidget: UIWidgets.EditBox, desc: "World description", category: "Escapists")]
	protected string m_sDoorObjectName;

	private IEntity m_PrisonDoor;

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		array<IEntity> children = {};
		ESCT_EntityHelper.GetAllChildren(owner, children);

		foreach (IEntity entity : children)
		{
			VObject visualObject = entity.GetVObject();
			if (visualObject)
			{
				string resPath = visualObject.GetResourceName().GetPath();
				if (resPath.IndexOf(m_sDoorObjectName) != -1)
				{
					m_PrisonDoor = entity;
				}
			}
		}

		if (!m_PrisonDoor)
		{
			//this is not good at all, but seems that there is no other way to find custom-placed prison door in building
			GetGame().GetWorld().QueryEntitiesBySphere(owner.GetOrigin(), 30, AddDoor, queryFlags: EQueryEntitiesFlags.ALL);

			if (!m_PrisonDoor)
				ESCT_Logger.ErrorFormat("No door found for %1 prison!", owner.GetPrefabData().GetPrefabName());
		}
	}

	override void Setup()
	{
		super.Setup();

		GetGame().GetCallqueue().CallLater(SetLockpickTask, 2000, false);
	}

	private void SetLockpickTask()
	{
		//small delay so players could have small time window to see where are they now, then task comes in
		GetGame().GetCallqueue().CallLater(ProgressGame, Math.RandomInt(600, 1000), false);
	}

	private void ProgressGame()
	{
		ESCT_Location location = ESCT_Location.Cast(GetOwner());
		if (location)
			location.RemoveSpawnReason(ESCT_ELocationSpawnReason.Start);

		ESCT_TaskAssignerComponent taskAssigner = ESCT_TaskAssignerComponent.GetInstance();
		if (!taskAssigner)
			return;

		taskAssigner.CreateLockpickTask(m_PrisonDoor);
	}

	private bool AddDoor(IEntity entity)
	{
		VObject visualObject = entity.GetVObject();
		if (!visualObject)
			return true;

		string resPath = visualObject.GetResourceName().GetPath();
		if (resPath.IndexOf(m_sDoorObjectName) != -1)
		{
			m_PrisonDoor = entity;
			return false;
		}

		return true;
	}

#ifdef WORKBENCH
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
		const string text = "Start Prison";

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
