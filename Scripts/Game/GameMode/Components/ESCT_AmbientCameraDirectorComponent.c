[ComponentEditorProps(category: "Escapists/GameMode/Components", description: "Ambient camera, mainly used as nice scenery (background) for various screens and menus.")]
class ESCT_AmbientCameraDirectorComponentClass : ESCT_GameModeBaseComponentClass
{
}

sealed class ESCT_AmbientCameraDirectorComponent: ESCT_GameModeBaseComponent
{
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Camera prefab.", params: "et")]
	protected ResourceName m_rCameraPrefab;

	[Attribute("", desc: "Ambient camera positions.", UIWidgets.Object)]
	protected ref array<ref ESCT_CameraPosition> m_aCameraPositions;

	private CameraBase m_Camera;
	private static ESCT_AmbientCameraDirectorComponent s_Instance = null;

	//------------------------------------------------------------------------------------------------
	//! Gets instance of ambient camera component.
	//! \return ESCT_AmbientCameraDirectorComponent instance.
	static ESCT_AmbientCameraDirectorComponent GetInstance()
	{
		if (!s_Instance)
		{
			BaseGameMode gameMode = GetGame().GetGameMode();
			if (gameMode)
			{
				s_Instance = ESCT_AmbientCameraDirectorComponent.Cast(gameMode.FindComponent(ESCT_AmbientCameraDirectorComponent));
			}
		}

		return s_Instance;
	}

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (SCR_Global.IsEditMode()) return;

		SetEventMask(owner, EntityEvent.INIT);
	}

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (SCR_Global.IsEditMode()) return;

		ESCT_Logger.Info("Initializing Ambient Camera Director.");

		if (RplSession.Mode() != RplMode.Dedicated)
		{
			ESCT_EscapistsGameMode gameMode = GetEscapistsGameMode();
			if (gameMode)
			{
				TurnOnCamera();
			}
		}

		ESCT_Logger.Info("Ambient Camera initalized.");
	}

	override void OnGameStateChanged(SCR_EGameModeState state)
	{
		super.OnGameStateChanged(state);

		if (RplSession.Mode() == RplMode.Dedicated)
			return;

		if (state == SCR_EGameModeState.GAME)
		{
			TurnOffCamera();
		}
		else if (state == SCR_EGameModeState.POSTGAME)
		{
			TurnOnCamera();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Turns on ambient camera for caller machine. Could work everywhere except dedicated server.
	void TurnOnCamera()
	{
		if (RplSession.Mode() == RplMode.Dedicated)
			return;

		if (m_Camera)
			return;

		if (!m_rCameraPrefab)
		{
			ESCT_Logger.Error("No camera prefab specified, check properties of entity in World Editor.");
			return;
		}

		if (!m_aCameraPositions || m_aCameraPositions.IsEmpty())
		{
			ESCT_Logger.Error("No camera positions specified, check properties of entity in World Editor.");
			return;
		}

		CameraManager cameraManager = GetGame().GetCameraManager();
		if (!cameraManager)
			return;

		Math.Randomize(-1);
		ESCT_CameraPosition cameraPosition = m_aCameraPositions.GetRandomElement();

		EntitySpawnParams spawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		spawnParams.Transform[3] = cameraPosition.GetOrigin();

		IEntity entity = GetGame().SpawnEntityPrefabLocal(Resource.Load(m_rCameraPrefab), params: spawnParams);
		if (!entity)
			return;

		CameraBase camera = CameraBase.Cast(entity);
		camera.SetName("AmbientCamera");
		camera.SetAngles(cameraPosition.GetAngles());
		camera.SetFOVDegree(cameraPosition.GetFOV());
		cameraManager.SetCamera(camera);

		m_Camera = camera;
	}

	//------------------------------------------------------------------------------------------------
	//! Turns off and removes ambient camera for caller machine. Could work everywhere except dedicated server.
	void TurnOffCamera()
	{
		if (RplSession.Mode() == RplMode.Dedicated)
			return;

		if (!m_Camera)
			return;

		SCR_EntityHelper.DeleteEntityAndChildren(m_Camera);
	}
}
