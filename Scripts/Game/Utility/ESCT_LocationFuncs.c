class ESCT_LocationFuncs<Class TType>
{
	static TType GetLocationComponent(notnull ESCT_Location location)
	{
		TType resultComponent;
		array<ESCT_LocationComponent> locationComponents = location.GetLocationComponents();
		
		foreach (ESCT_LocationComponent locComponent : locationComponents)
		{
			if (locComponent.Type() != TType)
				continue;
			
			resultComponent = TType.Cast(locComponent);
			break;
		}
		
		return resultComponent;
	}
}
