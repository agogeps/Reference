[EntityEditorProps(category: "Escapists/Entities", description: "Holds position for random location.")]
class ESCT_RandomLocationHolderClass : GenericEntityClass
{
}

sealed class ESCT_RandomLocationHolder : GenericEntity
{
	[Attribute("0", UIWidgets.Auto, desc: "If not ticked, the ordinary group will try to use location patrol points, if yes - additional group will be spawned for that.", category: "Escapists")]
	protected bool m_bSpawnAdditionalGroupForPoints;
	
	[Attribute(defvalue: "", uiwidget: UIWidgets.Auto, desc: "Custom patrol waypoints for location.", category: "Escapists")]
	protected ref array<ref ESCT_LocationPatrolPoint> m_aLocationPatrolPoints;
	
	[Attribute(defvalue: "", uiwidget: UIWidgets.Auto, desc: "Custom defend waypoint for location.", category: "Escapists")]
	protected ref ESCT_LocationPatrolPoint m_DefendPoint;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "If location should always spawn.", category: "Escapists")]
	protected bool m_bIsSpawnGuaranteed;
	
	[Attribute("0", UIWidgets.Auto, desc: "Composition will use this location rotation properties.", category: "Escapists")]
	protected bool m_bPreserveRotator;
	
	array<ref ESCT_LocationPatrolPoint> GetLocationPatrolPoints()
	{
		return m_aLocationPatrolPoints;
	}
	
	bool ShouldSpawnAdditionalGroupsForPoints()
	{
		return m_bSpawnAdditionalGroupForPoints;
	}
	
	ESCT_LocationPatrolPoint GetDefendPoint()
	{
		return m_DefendPoint;
	}
	
	bool IsRotationPreserved()
	{
		return m_bPreserveRotator;
	}
	
	bool IsSpawnGuaranteed()
	{
		return m_bIsSpawnGuaranteed;
	}

	void ESCT_RandomLocationHolder(IEntitySource src, IEntity parent)
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

		locationSystem.AddRandomLocationHolder(this);
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

		Color color = Color.Red;
		const string text = "Random Location";

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
		
		int index = 1;
		
		foreach (ESCT_LocationPatrolPoint point : m_aLocationPatrolPoints)
		{
			string pointText = string.Format("Patrol waypoint #%1", index);
			CreateWaypointPresentation(point, this, color, colorBackground, pointText);
			index++;
		}
		
		if (m_DefendPoint)
			CreateWaypointPresentation(m_DefendPoint, this, color, colorBackground, "Defend waypoint");
	}
	
	private void CreateWaypointPresentation(ESCT_LocationPatrolPoint point, IEntity owner, Color color, int colorBackground, string text)
	{
		vector pointPosition = point.GetWaypointWorldPosition(this);
		string pointText = text;

		Color newColor = new Color(color.R(), color.G(), color.B(), 0.75);
		
		ref DebugTextWorldSpace textShape = DebugTextWorldSpace.Create(
			owner.GetWorld(),
			pointText,
			DebugTextFlags.ONCE | DebugTextFlags.CENTER | DebugTextFlags.FACE_CAMERA,
			pointPosition[0],
			pointPosition[1] + 5,
			pointPosition[2],
			18.0,
			newColor.PackToInt(),
			colorBackground
		);

		Color transparentColor = new Color(color.R(), color.G(), color.B(), 0.4);

		ref Shape dbgShapeCenter = Shape.CreateCylinder(
			transparentColor.PackToInt(),
			ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOZWRITE | ShapeFlags.ONCE | ShapeFlags.NOOUTLINE,
			pointPosition,
			0.25,
			10
		);

		ref Shape dbgShapeOuter = Shape.CreateCylinder(
			transparentColor.PackToInt(),
			ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOZWRITE | ShapeFlags.ONCE | ShapeFlags.NOOUTLINE,
			pointPosition,
			4,
			2
		);
	}
#endif
}
