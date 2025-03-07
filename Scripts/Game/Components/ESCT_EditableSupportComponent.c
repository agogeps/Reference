[EntityEditorProps("Escapists/Server/Components", description: "Component for support spawner in editor/GM mode.", color: "0 0 255 255")]
class ESCT_EditableSupportComponentClass : ScriptComponentClass
{
}

sealed class ESCT_EditableSupportComponent : ScriptComponent
{
	[Attribute(uiwidget: UIWidgets.Auto, desc: "Support prefab that will be used for this support type.", params: "et", category: "Escapists")]
	protected ResourceName m_rSupportPrefab;

	private ESCT_SupportBase m_Support;
	
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (SCR_Global.IsEditMode() || !Replication.IsServer()) return;

		SetEventMask(owner, EntityEvent.INIT);
	}

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		GetGame().GetCallqueue().CallLater(SpawnEvent);
	}

	private void SpawnEvent()
	{
		if (!m_rSupportPrefab)
			return;

		ESCT_SupportSystem supportSystem = ESCT_SupportSystem.GetSystem();
		if (!supportSystem)
			return;

		vector targetPosition = GetOwner().GetOrigin();

		IEntity supportEntity = ESCT_SpawnHelpers.SpawnEntityPrefab(m_rSupportPrefab, targetPosition, global: false);
		if (!supportEntity)
		{
			ESCT_Logger.ErrorFormat("Failed to spawn %1 support prefab at %2 position!", m_rSupportPrefab, targetPosition.ToString());
			return;
		}

		m_Support = ESCT_SupportBase.Cast(supportEntity);
		if (!m_Support)
		{
			ESCT_Logger.ErrorFormat("Failed to cast %1 entity to base support class.", m_Support.ToString());
			return;
		}

		if (!m_Support.Setup(null, targetPosition))
		{
			ESCT_Logger.ErrorFormat("Failed to setup %1 support for some reason, check corresponding support entity to debug.", m_Support.ToString());
			return;
		}

		m_Support.Launch();
		m_Support.GetOnSupportDespawned().Insert(OnSupportDespawned);

		supportSystem.AddExternalSupport(m_Support);
		supportSystem.ShowFeedbackAboutUpcomingSupport(m_Support.GetSupportType());

		ESCT_Logger.InfoFormat("[ESCT_EditableSupportComponent] Successfully created %1 support at %2 position.", ESCT_Misc.GetPrefabName(supportEntity), targetPosition.ToString());
	}

	private void OnSupportDespawned()
	{
		SCR_EntityHelper.DeleteEntityAndChildren(GetOwner());
	}
	
	void ~ESCT_EditableSupportComponent()
	{
		if (!m_Support)
			return;
		
		m_Support.Despawn();
	}
}
