[EntityEditorProps(category: "Escapists/Entities/Locations", visible: false, color: "0 255 255 255")]
class ESCT_LocationBaseClass : GenericEntityClass
{
}

class ESCT_LocationBase : GenericEntity
{
#ifdef WORKBENCH
	[Attribute(UIWidgets.CheckBox, desc: "Show spawn/activation radius.", category: "Escapists")]
	protected bool m_bDrawDebugSphere;
#endif

	[Attribute("0", desc: "Support type.", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ESCT_ELocationType), category: "Escapists")]
	protected ESCT_ELocationType m_eLocationType;

	[Attribute(defvalue: "700", UIWidgets.Slider, desc: "Distance to spawn/activate location.", category: "Escapists", params: "0 2000 1")]
	protected float m_fSpawnDistance;

	[Attribute("0", UIWidgets.Auto, desc: "Location will be not removed by randomizer and will be always present.", category: "Escapists")]
	protected bool m_bIsSpawnGuaranteed;

	[Attribute("0", UIWidgets.Auto, desc: "Location will use insurgent faction.", category: "Escapists")]
	protected bool m_bIsInsurgentLocation;

	protected FactionAffiliationComponent m_FactionAffiliation;

	protected bool m_bIsSpawningLocationComponents = false;
	protected ref array<ESCT_LocationComponent> m_aLocationComponentsQueue;
	protected ref array<ESCT_LocationComponent> m_aLocationComponents = {};
	protected bool m_bIsSpawnGuaranteedTemp = m_bIsSpawnGuaranteed;
	protected ESCT_ELocationSpawnReason m_eSpawnReasons;
	protected ESCT_ESpawnState m_SpawnState = ESCT_ESpawnState.Disabled;
	protected bool m_bIsVisited = false;

	bool IsInsurgentLocation()
	{
		return m_bIsInsurgentLocation;
	}

	ESCT_ELocationType GetLocationType()
	{
		return m_eLocationType;
	}

	void SetSpawnGuaranteed(bool value)
	{
		m_bIsSpawnGuaranteedTemp = value;
	}

	float GetSpawnDistance()
	{
		return m_fSpawnDistance;
	}

	bool IsSpawnGuaranteed()
	{
		return m_bIsSpawnGuaranteedTemp;
	}

	ESCT_ESpawnState GetSpawnState()
	{
		return m_SpawnState;
	}

	void SetSpawnState(ESCT_ESpawnState newState)
	{
		m_SpawnState = newState;
	}

	//------------------------------------------------------------------------------------------------
	//! Adds spawn reason flag.
	void AddSpawnReason(ESCT_ELocationSpawnReason spawnReason)
	{
		m_eSpawnReasons = SCR_Enum.SetFlag(m_eSpawnReasons, spawnReason);
	}

	//------------------------------------------------------------------------------------------------
	//! Removes spawn reason flag.
	void RemoveSpawnReason(ESCT_ELocationSpawnReason spawnReason)
	{
		m_eSpawnReasons = SCR_Enum.RemoveFlag(m_eSpawnReasons, spawnReason);
	}

	//------------------------------------------------------------------------------------------------
	//! Gets current spawn reasons of the location.
	//! \return Bitshifted enum value that contains spawn reasons.
	ESCT_ELocationSpawnReason GetSpawnReasons()
	{
		return m_eSpawnReasons;
	}

	void ESCT_LocationBase(IEntitySource src, IEntity parent)
	{
		if (!GetGame().InPlayMode())
			return;

		SetEventMask(EntityEvent.INIT);
		SetFlags(EntityFlags.ACTIVE, true);
	}

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		m_FactionAffiliation = FactionAffiliationComponent.Cast(FindComponent(FactionAffiliationComponent));
	}

	protected void InitiateLocationComponentsSpawn()
	{
		m_bIsSpawningLocationComponents = true;
		m_aLocationComponentsQueue = {};

		//sort components by spawn order
		for (int i = m_aLocationComponents.Count() - 1; i >= 0; i--)
		{
			ESCT_LocationComponent component = m_aLocationComponents[i];
			if (component.GetSpawnOrder() == ESCT_ESpawnOrder.Late)
				m_aLocationComponentsQueue.Insert(component);
			else
				m_aLocationComponentsQueue.InsertAt(component, 0);
		}

		if (m_aLocationComponentsQueue.IsEmpty())
		{
			FinishSpawningLocationComponents(true);
			return;
		}

		GetGame().GetCallqueue().CallLater(SpawnLocationComponentEachFrame, 0, true);
	}

	protected void SpawnLocationComponentEachFrame()
	{
		ESCT_LocationComponent locationComponentToSpawn = null;

		for (int i = 0; i < m_aLocationComponentsQueue.Count(); i++)
		{
			//~ Invalid compartment so remove it from the list
			if (!m_aLocationComponentsQueue[i])
			{
				m_aLocationComponentsQueue.RemoveOrdered(i);
				i--;
				continue;
			}

			locationComponentToSpawn = m_aLocationComponentsQueue[i];
			m_aLocationComponentsQueue.RemoveOrdered(i);
			break;
		}

		//~ No location component to spawn found
		if (!locationComponentToSpawn)
		{
			FinishSpawningLocationComponents(false);
			return;
		}

		locationComponentToSpawn.Spawn();

		if (m_aLocationComponentsQueue.IsEmpty())
			FinishSpawningLocationComponents(false);
	}


	protected void FinishSpawningLocationComponents(bool wasCanceled)
	{
		m_bIsSpawningLocationComponents = false;
		m_aLocationComponentsQueue.Clear();
		GetGame().GetCallqueue().Remove(SpawnLocationComponentEachFrame);

		if (!m_bIsVisited)
			m_bIsVisited = true;
	}

	void ~ESCT_LocationBase()
	{
		if (m_bIsSpawningLocationComponents)
			FinishSpawningLocationComponents(true);
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

		int color;
		string text = SCR_Enum.GetEnumName(ESCT_ELocationType, m_eLocationType);

		if (m_bIsInsurgentLocation)
		{
			color = Color.Green.PackToInt();
		}
		else
		{
			switch (m_eLocationType)
			{
				case ESCT_ELocationType.Checkpoint:
				{
					color = Color.DarkYellow.PackToInt();
					break;
				}
				case ESCT_ELocationType.Helipad:
				{
					color = Color.Blue.PackToInt();
					break;
				}
				case ESCT_ELocationType.RadioStation:
				{
					color = Color.Yellow.PackToInt();
					break;
				}
				case ESCT_ELocationType.StartArea:
				{
					color = Color.Cyan.PackToInt();
					break;
				}
				case ESCT_ELocationType.City:
				{
					color = Color.Pink.PackToInt();
					break;
				}
				case ESCT_ELocationType.Hospital:
				{
					color = Color.Red.PackToInt();
					break;
				}
				case ESCT_ELocationType.Outpost:
				{
					color = Color.Violet.PackToInt();
					break;
				}
				case ESCT_ELocationType.Hideout:
				{
					color = Color.Green.PackToInt();
					break;
				}
				case ESCT_ELocationType.Patrol:
				{
					color = Color.DodgerBlue.PackToInt();
					break;
				}
				case ESCT_ELocationType.VehicleDepot:
				{
					color = Color.Chartreuse.PackToInt();
					break;
				}
				default:
				{
					color = Color.White.PackToInt();
					break;
				}
			}
		}

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
			18.0,
			color,
			colorBackground
		);

		ref Shape pointShape = Shape.CreateSphere(color, ShapeFlags.ONCE | ShapeFlags.NOOUTLINE, position, 0.2);
		ref Shape arrowShape = Shape.CreateArrow(position, position + outMat[2], 0.2, color, ShapeFlags.ONCE);

		if (!m_bDrawDebugSphere)
			return;

		Shape distanceShape = null;
		distanceShape = Shape.CreateSphere(
			ARGB(100, 0x99, 0x10, 0xF2),
			ShapeFlags.ONCE | ShapeFlags.NOOUTLINE | ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOZWRITE,
			GetOrigin(),
			m_fSpawnDistance
		);
	}
#endif
}
