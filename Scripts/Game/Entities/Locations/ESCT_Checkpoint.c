[EntityEditorProps(category: "Escapists/Entities/Locations", description: "Roadblock location type.")]
class ESCT_CheckpointClass : ESCT_LocationClass
{
}

sealed class ESCT_Checkpoint: ESCT_Location
{
	void SetRotator(vector rotator)
	{
		ESCT_CheckpointCompositionComponent checkpointComposition = ESCT_CheckpointCompositionComponent.Cast(m_LocationCompositionComponent);
		if (!checkpointComposition)
			return;

		//roadblocks doesn't need randomization as in base class as they should align with slot direction
		checkpointComposition.SetRotator(rotator);
	}
	
#ifdef WORKBENCH
	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		super._WB_AfterWorldUpdate(timeSlice);
		
		vector mat[4];
		GetWorldTransform(mat);

		// Draw arrow
		vector arrowPosition = mat[3];
		arrowPosition[1] = arrowPosition[1] + 0.5;
		Shape arrowShape = Shape.CreateArrow(arrowPosition, arrowPosition + mat[2], 0.2, Color.Green.PackToInt(), ShapeFlags.ONCE);
	}
	
	void _WB_SnapToTerrain(IEntitySource entitySource)
	{
		vector pos = GetOrigin();
		pos[1] = GetWorld().GetSurfaceY(pos[0], pos[2]);
		if (entitySource.GetParent())
			pos = SCR_BaseContainerTools.GetLocalCoords(entitySource.GetParent(), pos);
		
		WorldEditorAPI api = _WB_GetEditorAPI();
		IEntitySource src = api.EntityToSource(this);
		api.SetVariableValue(src, null, "coords", pos.ToString(false));
	}
	
	void _WB_OrientToTerrain(IEntitySource entitySource)
	{
		WorldEditorAPI api = _WB_GetEditorAPI();
		IEntitySource src = api.EntityToSource(this);
		
		api.SetVariableValue(src, null, "angleX", "0");
		api.SetVariableValue(src, null, "angleZ", "0");
		
		vector transform[4];
		GetWorldTransform(transform);
		vector pos = transform[3];
		
		IEntity child;
		vector min, max, posChild;
		
		for (int i = 0, count = entitySource.GetNumChildren(); i < count; i++)
		{
			IEntitySource childSrc = entitySource.GetChild(i);
			child = api.SourceToEntity(childSrc);
			child.GetBounds(min, max);
			
			float posY = -float.MAX;
			float angleZ = _WB_GetAngle(api, pos + transform[0] * min[0], pos + transform[0] * max[0], posY);
			float angleX = _WB_GetAngle(api, pos + transform[2] * min[2], pos + transform[2] * max[2], posY);
			
			posChild = pos;
			posChild[1] = posY;
			posChild = CoordToLocal(posChild);
			
			api.SetVariableValue(childSrc, null, "coords", posChild.ToString(false));
			api.SetVariableValue(childSrc, null, "angleX", angleX.ToString());
			api.SetVariableValue(childSrc, null, "angleZ", (-angleZ).ToString());
		}
	}
	
	float _WB_GetAngle(WorldEditorAPI api, vector posA, vector posB, out float posY)
	{
		float dis = vector.Distance(posA, posB);
		if (dis == 0)
		{
			Print(string.Format("Slot entity %1 at position %2 has zero size!", this, GetWorldTransformAxis(3)), LogLevel.WARNING);
			return 0;
		}
		
		posA[1] = api.GetTerrainSurfaceY(posA[0], posA[2]);
		posB[1] = api.GetTerrainSurfaceY(posB[0], posB[2]);
		
		vector posCenter = vector.Lerp(posA, posB, 0.5);
		posY = Math.Max(posY, posCenter[1]);
		
		return Math.Tan((posB[1] - posA[1]) / dis) * Math.RAD2DEG;
	}

	override array<ref WB_UIMenuItem> _WB_GetContextMenuItems()
	{
		array<ref WB_UIMenuItem> items = { new WB_UIMenuItem("Snap and orient to terrain", 0) };
		
		if (GetName().IsEmpty())
			items.Insert(new WB_UIMenuItem("Auto-assign slot name", 1));
		
		return items;
	}
	
	override void _WB_OnContextMenu(int id)
	{
		switch (id)
		{
			case 0:
			{
				WorldEditorAPI api = _WB_GetEditorAPI();
				vector mat[4];
				api.BeginEntityAction();
				IEntitySource entitySource = _WB_GetEditorAPI().EntityToSource(this);
				_WB_SnapToTerrain(entitySource);
				_WB_OrientToTerrain(entitySource);
				api.EndEntityAction();
				break;
			}
			case 1:
			{
				WorldEditorAPI api = _WB_GetEditorAPI();
				api.BeginEntityAction();
				api.EndEntityAction();
				break;
			}
		}
	}

	override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		if (key == "coords")
		{
			WorldEditorAPI api = _WB_GetEditorAPI();
			if (!api.UndoOrRedoIsRestoring())
			{
				IEntitySource entitySource = src.ToEntitySource();
				_WB_OrientToTerrain(entitySource);
			}
		}
		return false;
	}
#endif
}
