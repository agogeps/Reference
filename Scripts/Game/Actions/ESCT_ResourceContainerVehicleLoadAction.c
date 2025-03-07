modded class SCR_ResourceContainerVehicleLoadAction : SCR_ScriptedUserAction
{
	override event bool CanBePerformedScript(IEntity user)
	{
		//escapists doesn't need supply system and there is no other way to disable this action
		return false;
	}
	
	override event bool CanBeShownScript(IEntity user)
	{
		//escapists doesn't need supply system and there is no other way to disable this action
		return false;
	}
}
