[EntityEditorProps("Prison", description: "Component for static radio stations.", color: "0 0 255 255")]
class ESCT_StaticInsurgentRadioStationComponentClass : ESCT_StaticRadioStationComponentClass
{
}

class ESCT_StaticInsurgentRadioStationComponent : ESCT_StaticRadioStationComponent
{
	override ESCT_Faction GetOwnerFaction()
	{
		ESCT_FactionManager factionManager = ESCT_FactionManager.GetInstance();
		if (!factionManager)
			return null;
	
		if (!ESCT_EscapistsConfigComponent.GetInstance().IsInsurgencyEnabled()) 
			return factionManager.GetEnemyFaction();
			
		return factionManager.GetInsurgentFaction();
	}
	
}
