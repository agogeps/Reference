[EntityEditorProps(category: "Escapists/Entities", description: "Holds position for helicrash as possible start area.")]
class ESCT_HelicrashLocationHolderClass : GenericEntityClass
{
}

sealed class ESCT_HelicrashLocationHolder : GenericEntity
{		
	void ESCT_HelicrashLocationHolder(IEntitySource src, IEntity parent)
	{
		if (!GetGame().InPlayMode())
			return;

		SetEventMask(EntityEvent.INIT);
	}

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (!GetGame().InPlayMode() || !Replication.IsServer())
			return;
		
		ESCT_LocationSystem locationSystem = ESCT_LocationSystem.GetSystem();
		if (!locationSystem)
			return;

		locationSystem.AddStartArea(this);
	}
	
#ifdef WORKBENCH
	protected const float _DRAW_DISTANCE = 2000 * 2000;
	
	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		super._WB_AfterWorldUpdate(timeSlice);

		WorldEditorAPI api = this._WB_GetEditorAPI();
		if (!api)
			return;

		IEntitySource src = api.EntityToSource(this);
		if (!src)
			return;

		if (!api.IsEntityVisible(src) || !api.IsEntityLayerVisible(src.GetSubScene(), src.GetLayerID()))
			return;

		const float radius = 0.1;

		Color color = Color.Pink;
		const string text = "Helicrash";

		vector outMat[4];
		GetWorldTransform(outMat);

		vector position = outMat[3];
		vector rotator = outMat[2];
		
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

		ref DebugTextWorldSpace textShape = DebugTextWorldSpace.Create(
			GetWorld(),
			text,
			DebugTextFlags.ONCE | DebugTextFlags.CENTER | DebugTextFlags.FACE_CAMERA,
			position[0],
			position[1] + 0.6,
			position[2],
			18.0,
			color.PackToInt(),
			colorBackground
		);

		ref Shape pointShape = Shape.CreateSphere(color.PackToInt(), ShapeFlags.ONCE | ShapeFlags.NOOUTLINE, position, 0.2);
		ref Shape arrowShape = Shape.CreateArrow(position, position + rotator, 0.2, color.PackToInt(), ShapeFlags.ONCE);	
	}
#endif
}
