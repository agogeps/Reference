[EntityEditorProps("Escapists", description: "Component that ammobox cartridge to magazine conversion.", color: "0 0 255 255")]
class ESCT_CartridgeAmmoboxComponentClass : ScriptComponentClass
{
}

sealed class ESCT_CartridgeAmmoboxComponent : ScriptComponent
{	
	[Attribute(defvalue: "3", UIWidgets.Slider, category: "Escapists", params: "1 10 1")]
	protected int m_iInitialTotalUses;

	[RplProp()]
	protected int m_iRemainingUses;
	
	[RplProp()]
	protected int m_iTotalUses;

	protected IEntity m_Ammobox;
	
	int GetRemainingUses()
	{
		return m_iRemainingUses;
	}
	
	int GetTotalUses()
	{
		return m_iTotalUses;
	}
	
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (SCR_Global.IsEditMode() || !Replication.IsServer()) return;

		SetEventMask(owner, EntityEvent.INIT);
	}
	
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (SCR_Global.IsEditMode() || !Replication.IsServer()) return;

		m_Ammobox = owner;
		
		m_iRemainingUses = m_iInitialTotalUses;
		m_iTotalUses = m_iInitialTotalUses;
		Replication.BumpMe();
	}
	
	void CreateMagazines(ResourceName resource, int magazines)
	{
		Rpc(Rpc_DoCreateMagazines, resource, magazines);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	private void Rpc_DoCreateMagazines(ResourceName resource, int magazines)
	{
		if (m_iRemainingUses == 0)
			return;
		
		for (int i = 0 ; i < magazines; i++)
		{
			vector position = m_Ammobox.GetOrigin() + "0 0.01 0";
			position[0] = position[0] + Math.RandomFloat(0.05, 0.75);
			position[2] = position[2] + Math.RandomFloat(0.05, 0.75);
			
			vector rotator = "0 0 -90";
			rotator[0] = Math.RandomFloat(0, 360);
			
			IEntity entity = ESCT_SpawnHelpers.SpawnEntityPrefab(resource, position, rotator);

			vector transform[4];
			entity.GetTransform(transform);
			SCR_TerrainHelper.SnapToTerrain(transform, entity.GetWorld());
		}
		
		m_iRemainingUses--;
		Replication.BumpMe();
	}	
}
