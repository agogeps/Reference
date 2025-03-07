[EntityEditorProps("Escapists/Components/Locations", description: "Handles location composition spawn/despawn (special checkpoint type).", color: "0 100 100 100")]
class ESCT_CheckpointCompositionComponentClass : ESCT_LocationCompositionComponentClass
{
}

class ESCT_CheckpointCompositionComponent : ESCT_LocationCompositionComponent
{
	override void SetRotator(vector rotator)
	{
		if (m_Rotator != vector.Zero)
			return;

		//roadblocks doesn't need randomization as in base class as they should align with slot direction
		m_Rotator = rotator;
	}
}
