//FIXME: don't forget to remove it once BI fix it
modded class SCR_DoorUserAction : DoorUserAction
{
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!pOwnerEntity || !pOwnerEntity)
		{
			ESCT_Logger.Error("[modded SCR_DoorUserAction] No owner/user entity reference, aborting door user action!"); 
			return;
		}
		
		super.PerformAction(pOwnerEntity, pUserEntity);
	}
}
