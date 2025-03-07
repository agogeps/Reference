[EntityEditorProps(category: "Escapists/Entities", description: "Should be placed nearby some static location (static radio station or prison) building to disable it for good.", color: "0 255 255 255")]
class ESCT_DisableLocationEntityClass : GenericEntityClass
{
}

sealed class ESCT_DisableLocationEntity : GenericEntity
{
	void ESCT_DisableLocationEntity(IEntitySource src, IEntity parent)
	{
		if (!GetGame().InPlayMode())
			return;

		SetFlags(EntityFlags.ACTIVE, true);
	}
	
#ifdef WORKBENCH
	private const float _DRAW_DISTANCE = 100 * 100;
	
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

		const float radius = 15;

		int color = Color.Red.PackToInt();
		const string text = "Disabled Location";
		
		vector outMat[4];
		GetWorldTransform(outMat);
		
		vector position = outMat[3];
		vector rotator = outMat[2];
		
		ref DebugTextWorldSpace textShape = DebugTextWorldSpace.Create(
			GetWorld(),
			text,
			DebugTextFlags.ONCE | DebugTextFlags.CENTER | DebugTextFlags.FACE_CAMERA,
			position[0],
			position[1] + 0.6,
			position[2],
			24.0,
			color,
			colorBackground
		);

		Shape distanceShape = Shape.CreateSphere(
			ARGB(80, 0xF2, 0x0, 0x0),
			ShapeFlags.ONCE | ShapeFlags.NOOUTLINE | ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOZWRITE,
			GetOrigin(),
			radius
		);
	}
#endif
}
