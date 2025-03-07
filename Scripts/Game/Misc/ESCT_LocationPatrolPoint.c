[BaseContainerProps()]
class ESCT_LocationPatrolPoint
{
	[Attribute(defvalue: "0 0 0", uiwidget: UIWidgets.Coords, params: "inf inf purpose=coords space=entity")]
	protected vector m_vWaypointPosition;
	
	[Attribute(defvalue: "-1", uiwidget: UIWidgets.Auto)]
	protected float m_fWaypointRadius;
	
	float GetWaypointRadius()
	{
		return m_fWaypointRadius;
	}
	
	vector GetWaypointPosition()
	{
		return m_vWaypointPosition;
	}
	
	vector GetWaypointWorldPosition(notnull IEntity owner)
	{
		vector ownerTransformMatrix[4];
		owner.GetWorldTransform(ownerTransformMatrix);
		
		vector worldPosition = m_vWaypointPosition.Multiply4(ownerTransformMatrix);
		worldPosition[1] = SCR_TerrainHelper.GetTerrainY(worldPosition, owner.GetWorld());
		
		return worldPosition;
	}
}
