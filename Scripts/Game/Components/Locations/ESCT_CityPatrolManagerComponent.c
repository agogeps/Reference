[EntityEditorProps("Escapists/Location", description: "This component handles location patrols (for cities).", color: "0 0 255 255")]
class ESCT_CityPatrolManagerComponentClass : ESCT_LocationPatrolManagerComponentClass
{
}

class ESCT_CityPatrolManagerComponent : ESCT_LocationPatrolManagerComponent
{
	protected override ResourceName SelectGroupPrefab(notnull ESCT_Faction faction)
	{
		ESCT_EGroupSize groupSize = ESCT_GroupDirector.GetGroupSize(m_Location.GetLocationType());
		if (groupSize == ESCT_EGroupSize.Undefined)
			return ResourceName.Empty;
		
		array<ESCT_EMilitaryBranchType> branches = {ESCT_EMilitaryBranchType.MilitaryPolice, ESCT_EMilitaryBranchType.Military};
		int index = SCR_ArrayHelper.GetWeightedIndex({15, 25}, Math.RandomFloat01());

		return faction.GetRandomGroupOfBranchAndSize(branches[index], groupSize);
	}
}
