[EntityEditorProps("Escapists/Server/Components", description: "Component for ambient event spawner in editor/GM mode.", color: "0 0 255 255")]
class ESCT_EditableAmbientEventComponentClass : ScriptComponentClass
{
}

sealed class ESCT_EditableAmbientEventComponent : ScriptComponent
{
	[Attribute(uiwidget: UIWidgets.Auto, desc: "Event prefab that will be used for this ambient event type.", params: "et", category: "Escapists")]
	protected ResourceName m_rEventPrefab;
	
	private ESCT_AmbientEventBase m_AmbientEvent;
	
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
		if (!m_rEventPrefab)
			return;
		
		ESCT_AmbientEventSystem ambientEventSystem = ESCT_AmbientEventSystem.GetSystem();
		if (!ambientEventSystem)
			return;
		
		vector targetPosition = GetOwner().GetOrigin();
		
		IEntity eventEntity = ESCT_SpawnHelpers.SpawnEntityPrefab(m_rEventPrefab, targetPosition, global: false);
		if (!eventEntity)
		{
			ESCT_Logger.ErrorFormat("[ESCT_EditableAmbientEventComponent] Failed to spawn %1 ambient event prefab at %2 position!", m_rEventPrefab, targetPosition.ToString());
			return;
		}

		m_AmbientEvent = ESCT_AmbientEventBase.Cast(eventEntity);
		if (!m_AmbientEvent)
		{
			ESCT_Logger.ErrorFormat("[ESCT_EditableAmbientEventComponent] Failed to cast %1 ambient event at %2 position!", m_rEventPrefab, targetPosition.ToString());
			return;
		}

		if (!m_AmbientEvent.Launch())
		{
			ESCT_Logger.ErrorFormat("[ESCT_EditableAmbientEventComponent] Failed to launch %1 ambient event at %2 position!", m_rEventPrefab, targetPosition.ToString());
			return;
		}
		
		m_AmbientEvent.GetOnAmbientEventDespawned().Insert(OnEventDespawned);
		ambientEventSystem.AddExternalEvent(m_AmbientEvent);
		
		ESCT_Logger.InfoFormat("[ESCT_EditableAmbientEventComponent] Successfully created %1 ambient event ambient event at %2 position.", ESCT_Misc.GetPrefabName(eventEntity), targetPosition.ToString());
	}
	
	private void OnEventDespawned()
	{
		SCR_EntityHelper.DeleteEntityAndChildren(GetOwner());
	}
	
	void ~ESCT_EditableAmbientEventComponent()
	{
		if (!m_AmbientEvent)
			return;
		
		m_AmbientEvent.Despawn();
	}
}
