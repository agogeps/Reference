class ESCT_PrisonDoorUserAction : SCR_DoorUserAction
{
	private ESCT_PrisonDoorComponent m_PrisonDoor;

	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);
		m_PrisonDoor = ESCT_PrisonDoorComponent.Cast(pOwnerEntity.FindComponent(ESCT_PrisonDoorComponent));
	}
	
	override bool CanBePerformedScript(IEntity user)
	{
		DoorComponent doorComponent = GetDoorComponent();
		if (!doorComponent)
		{
			return false;
		}
		
		if (!m_PrisonDoor.GetDoorUnlockState())
		{
			return false;
		}
		
		return true;
	}
	
	override bool CanBeShownScript(IEntity user)
	{
		return m_PrisonDoor.GetDoorUnlockState();
	}
}
