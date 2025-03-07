[EntityEditorProps("Prison", description: "Component that allows building to be used as starting prison (spawn point).", color: "0 0 255 255")]
class ESCT_HideoutStartComponentClass : ESCT_StartAreaComponentClass
{
}

sealed class ESCT_HideoutStartComponent : ESCT_StartAreaComponent
{
	override void Setup()
	{
		super.Setup();
		
		GetGame().GetCallqueue().CallLater(SetRadioTask, Math.RandomInt(8000, 16000), false);
	}
		
	private void SetRadioTask()
	{
		ESCT_GameStateManagerComponent.GetInstance().SetGameState(ESCT_EGameState.RadioStationSearch);
		GetGame().GetCallqueue().CallLater(ProvideFeedback, Math.RandomInt(30000, 60000), false);
		
		ESCT_Location location = ESCT_Location.Cast(GetOwner());
		if (location)
			location.RemoveSpawnReason(ESCT_ELocationSpawnReason.Start);
	}
	
	private void ProvideFeedback()
	{
		ESCT_EscapistsGameMode gameMode = ESCT_EscapistsGameMode.GetGameMode();
		if (!gameMode)
			return;
		
		ESCT_EscapistsManager manager = gameMode.GetEscapistsManager();
		if (!manager)
			return;
		
		manager.GetOnRunStart().Invoke();
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
		const string text = "Start Hideout";

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
