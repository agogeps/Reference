[EntityEditorProps(category: "Escapists/Entities", description: "Dynamic radio slot holder (for static locations).", color: "0 0 255 255")]
class ESCT_RadioSlotEntityClass : GenericEntityClass
{
}

sealed class ESCT_RadioSlotEntity : GenericEntity
{
	void SpawnRadio(notnull ESCT_Faction faction)
	{
		IEntity child = GetChildren();
		if (child)
			RplComponent.DeleteRplEntity(child, false);
		
		EntitySpawnParams params = new EntitySpawnParams();
		params.Parent = this;
		params.TransformMode = ETransformMode.WORLD;
		
		ResourceName radioPrefab = faction.GetRadio();
		if (!radioPrefab)
			return;
		
		IEntity radio = GetGame().SpawnEntityPrefab(Resource.Load(radioPrefab), GetWorld(), params);
		if (!radio)
			return;
		
		radio.SetFlags(EntityFlags.ACTIVE);
		radio.Update();
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

		int color = Color.DarkGreen.PackToInt();
		const string text = "Radio";

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
		
		super._WB_AfterWorldUpdate(timeSlice);
	}
#endif
}
