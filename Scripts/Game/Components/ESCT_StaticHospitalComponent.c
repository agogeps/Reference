[EntityEditorProps("Prison", description: "Component for static hospitals.", color: "0 0 255 255")]
class ESCT_StaticHospitalComponentClass : ESCT_StaticLocationComponentClass
{
}

sealed class ESCT_StaticHospitalComponent : ESCT_StaticLocationComponent
{
	override ESCT_ELocationType GetLocationType()
	{
		return ESCT_ELocationType.Hospital;
	}
}
