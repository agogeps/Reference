[EntityEditorProps("Escapists/Components/Locations", description: "Handles location composition spawn/despawn.", color: "0 100 100 100")]
class ESCT_LocationCompositionComponentClass : ScriptComponentClass
{
}

class ESCT_LocationCompositionComponent : ScriptComponent
{
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Composition that will be spawned on point activation.", params: "et", category: "Escapists")]
	protected ResourceName m_CompositionResource;
	
	[Attribute("0", UIWidgets.Auto, desc: "Composition will use location rotation properties for composition.", category: "Escapists")]
	protected bool m_bUseLocationRotator;
	
	protected bool m_bUseLocationRotatorTemp = m_bUseLocationRotator;
	
	protected ResourceName m_CompositionPrefab;
	protected vector m_Rotator;
	protected IEntity m_Composition;
	protected ESCT_Location m_Location;
	protected bool m_bIsStatic = false;
	
	private SCR_AIWorld m_aiWorld;
	
	void SetUseRotator(bool value)
	{
		m_bUseLocationRotatorTemp = value;
	}
	
	void SetComposition(IEntity composition, bool isStatic = false)
	{
		m_Composition = composition;
		m_bIsStatic = isStatic;
	}
	
	IEntity GetComposition()
	{
		return m_Composition;
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

		if (SCR_Global.IsEditMode() || !Replication.IsServer())
			return;
		
		m_Location = ESCT_Location.Cast(owner);
	}
	
	void Spawn()
	{
		if (m_bIsStatic)
			return;
		
		if (!m_Location.IsVisited())
		{
			AssignComposition();
			
			if (m_bUseLocationRotatorTemp && !m_Rotator)
			{
				vector rotator = m_Location.GetYawPitchRoll();
				m_Rotator = rotator;
			}
			else
			{
				SetRotator(m_Location.GetYawPitchRoll());
			}
		}
		
		if (!m_CompositionPrefab)
			return;

		m_Composition = ESCT_SpawnHelpers.SpawnEntityPrefab(m_CompositionPrefab, m_Location.GetOrigin(), m_Rotator);
		
		SCR_AIWorld aiWorld = GetAiWorld();
		if (aiWorld)
			aiWorld.RequestNavmeshRebuildEntity(m_Composition);
	}
	
	void Despawn()
	{
		if (m_bIsStatic)
			return;
		
		RebuildNavmesh();
		
		SCR_EntityHelper.DeleteEntityAndChildren(m_Composition);
	}
	
	void SetRotator(vector rotator)
	{
		if (m_Rotator != vector.Zero)
			return;

		rotator[0] = Math.RandomFloat(0, 360);
		m_Rotator = rotator;
	}
	
	protected void AssignComposition()
	{
		if (m_CompositionPrefab || m_Composition)
			return;
		
		if (m_CompositionResource)
		{
			m_CompositionPrefab = m_CompositionResource;
			return;
		}
			
		ESCT_Faction faction = m_Location.GetOwnerFaction();
		if (!faction)
			return;
		
		ESCT_LocationCatalog catalog = new ESCT_LocationCatalog(faction);
		SCR_EntityCatalogEntry locationEntry = catalog.GetRandomLocation(m_Location.GetLocationType());
		if (!locationEntry)
		{
			ESCT_Logger.ErrorFormat("Location %1 had failed to assign resource name!", m_Location.GetName());
			return;
		}
		
		m_CompositionPrefab = locationEntry.GetPrefab();
	}
	
	protected SCR_AIWorld GetAiWorld()
	{
		if (!m_aiWorld)
			m_aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		
		return m_aiWorld;
	}
	
	protected void RebuildNavmesh()
	{
		if (!m_Composition)
			return;
		
		SCR_AIWorld aiWorld = GetAiWorld();
		if (!aiWorld)
			return;
			
		array<ref Tuple2<vector, vector>> areas = {}; // min, max
		array<bool> redoAreas = {};
		aiWorld.GetNavmeshRebuildAreas(m_Composition, areas, redoAreas);
		GetGame().GetCallqueue().CallLater(aiWorld.RequestNavmeshRebuildAreas, 1000, false, areas, redoAreas); //--- Called *before* the entity is deleted with a delay, ensures the regeneration doesn't accidentaly get anything from the entity
	}
}
