[EntityEditorProps(category: "Escapists/Entities/Locations", visible: false, color: "0 255 255 255")]
class ESCT_LocationClass : ESCT_LocationBaseClass
{
}

class ESCT_Location : ESCT_LocationBase
{
	protected ESCT_LocationCompositionComponent m_LocationCompositionComponent;

	protected ref ScriptInvokerInt m_OnEntityChange = new ScriptInvokerInt();
	protected ESCT_MapDescriptorComponent m_MapMarker;

	private int m_iEntitiesToClear;

	ESCT_Faction GetOwnerFaction()
	{
		if (!m_FactionAffiliation)
			return null;

		ESCT_Faction faction = ESCT_Faction.Cast(m_FactionAffiliation.GetAffiliatedFaction());
		if (!faction)
			faction = ESCT_FactionManager.GetInstance().GetEnemyFaction();

		return faction;
	}
	
	IEntity GetStaticComposition()
	{
		if (!m_LocationCompositionComponent)
			return null;
		
		return m_LocationCompositionComponent.GetComposition();
	}

	void SetStaticComposition(IEntity composition)
	{
		if (!m_LocationCompositionComponent)
			return;

		m_LocationCompositionComponent.SetComposition(composition, isStatic: true);
	}

	void SetUseLocationRotator(bool value)
	{
		if (!m_LocationCompositionComponent)
			return;

		m_LocationCompositionComponent.SetUseRotator(value);
	}

	array<ESCT_LocationComponent> GetLocationComponents()
	{
		return m_aLocationComponents;
	}

	bool AddToLocationComponents(notnull ESCT_LocationComponent component)
	{
		if (!Replication.IsServer())
			return false;
		
		return m_aLocationComponents.Insert(component);
	}

	bool IsVisited()
	{
		return m_bIsVisited;
	}

	void ChangeEntitiesToClear(int incrementValue)
	{
		m_iEntitiesToClear = m_iEntitiesToClear + incrementValue;
	}

	ScriptInvokerInt GetOnEntityChange()
	{
		return m_OnEntityChange;
	}

	IEntity GetComposition()
	{
		if (!m_LocationCompositionComponent)
			return null;

		return m_LocationCompositionComponent.GetComposition();
	}

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (!GetGame().InPlayMode())
			return;

		m_MapMarker = ESCT_MapDescriptorComponent.Cast(FindComponent(ESCT_MapDescriptorComponent));
		m_LocationCompositionComponent = ESCT_LocationCompositionComponent.Cast(FindComponent(ESCT_LocationCompositionComponent));

		if (!Replication.IsServer())
			return;

		ESCT_LocationSystem locationSystem = ESCT_LocationSystem.GetSystem();
		if (locationSystem)
			locationSystem.Register(this);
	}

	void Initialize()
	{
		SetSpawnState(ESCT_ESpawnState.Idle);

		ESCT_FactionManager factionManager = ESCT_FactionManager.GetInstance();
		if (m_bIsInsurgentLocation && ESCT_EscapistsConfigComponent.GetInstance().IsInsurgencyEnabled())
			m_FactionAffiliation.SetAffiliatedFactionByKey(factionManager.GetInsurgentFactionKey());
		else
			m_FactionAffiliation.SetAffiliatedFactionByKey(factionManager.GetEnemyFactionKey());

		if (m_MapMarker)
			m_MapMarker.Init();
	}

	bool Spawn()
	{
		if (!Replication.IsServer() || m_SpawnState == ESCT_ESpawnState.Disabled || m_SpawnState == ESCT_ESpawnState.Spawned)
			return false;

		SetSpawnState(ESCT_ESpawnState.Spawned);

		if (m_LocationCompositionComponent)
			m_LocationCompositionComponent.Spawn();

		if (m_MapMarker)
			m_MapMarker.SetPollMarkerReveal(true);

		InitiateLocationComponentsSpawn();

		if (m_eLocationType != ESCT_ELocationType.StartArea)
			m_OnEntityChange.Insert(OnEntityChange);

		return true;
	}

	bool Despawn()
	{
		if (!Replication.IsServer() || m_SpawnState == ESCT_ESpawnState.Idle)
			return false;

		SetSpawnState(ESCT_ESpawnState.Idle);

		if (m_MapMarker)
			m_MapMarker.SetPollMarkerReveal(false);

		m_OnEntityChange.Clear();

		foreach (ESCT_LocationComponent locationComponent : m_aLocationComponents)
		{
			locationComponent.Despawn();
		}

		if (m_LocationCompositionComponent)
			m_LocationCompositionComponent.Despawn();

		return true;
	}

	private void OnEntityChange(int additiveValue)
	{
		m_iEntitiesToClear+=additiveValue;

		if (m_iEntitiesToClear <= ESCT_EscapistsConfigComponent.GetInstance().GetLocationClearThreshold())
			m_OnEntityChange.Remove(OnEntityChange);

		if (m_MapMarker)
		{
			m_MapMarker.Clear();
		}
	}
}
