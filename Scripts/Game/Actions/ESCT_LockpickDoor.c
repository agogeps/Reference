class ESCT_LockpickDoor : ESCT_HoldAction
{
	private ESCT_PrisonDoorComponent m_PrisonDoor;
	private IEntity m_Door;
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);
		
		if (SCR_Global.IsEditMode()) return;
		
		m_PrisonDoor = ESCT_PrisonDoorComponent.Cast(pOwnerEntity.FindComponent(ESCT_PrisonDoorComponent));
		m_Door = pOwnerEntity;
	}
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!pOwnerEntity || !pUserEntity)
			return;
		
		m_PrisonDoor.UnlockDoor(pUserEntity);
		
		super.PerformAction(pOwnerEntity, pUserEntity);
	}

	override bool CanBePerformedScript(IEntity user)
	{
		if (!super.CanBePerformedScript(user))
			return false;

		if (ESCT_GameStateManagerComponent.GetInstance().GetGameState() != ESCT_EGameState.Start)
			return false;

		return true;
	}

	override bool CanBeShownScript(IEntity user)
	{
		if (ESCT_GameStateManagerComponent.GetInstance().GetGameState() != ESCT_EGameState.Start)
			return false;
		
		return !m_PrisonDoor.GetDoorUnlockState();
	}
}
