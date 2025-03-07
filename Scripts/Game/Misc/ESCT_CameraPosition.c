//! Container class acting as camera positions storage.
class ESCT_CameraPosition : ScriptAndConfig
{
	[Attribute("0 0 0", UIWidgets.Coords)]
	protected vector m_vPosition;

	[Attribute("0 0 0", UIWidgets.Coords)]
	protected vector m_vAngles;
	
	[Attribute("60", UIWidgets.Slider, "Camera's field of view", "0 180 1")]
	protected float m_fFOV;
	
	vector GetOrigin()
	{
		return m_vPosition;
	}
	
	vector GetAngles()
	{
		return m_vAngles;
	}
	
	float GetFOV()
	{
		return m_fFOV;
	}
}
