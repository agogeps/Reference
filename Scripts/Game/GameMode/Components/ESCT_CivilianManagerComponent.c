[ComponentEditorProps(category: "Escapists/GameMode/Components", description: "Manages civilians.")]
class ESCT_CivilianManagerComponentClass : ESCT_GameModeBaseComponentClass
{
}

sealed class ESCT_CivilianManagerComponent : ESCT_GameModeBaseComponent
{
	private ref array<SCR_EntityCatalogEntry> m_aCharacters = {};
	private ref array<SCR_EntityCatalogEntry> m_aVehicles = {};
	private SCR_Faction m_CivilianFaction;
	
	private static ESCT_CivilianManagerComponent s_Instance = null;
	static ESCT_CivilianManagerComponent GetInstance()
	{
		if (!s_Instance)
		{
			BaseGameMode gameMode = GetGame().GetGameMode();
			if (gameMode)
			{
				s_Instance = ESCT_CivilianManagerComponent.Cast(gameMode.FindComponent(ESCT_CivilianManagerComponent));
			}
		}

		return s_Instance;
	}

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		if (SCR_Global.IsEditMode() || !Replication.IsServer())
			return;

		SetEventMask(owner, EntityEvent.INIT);
	}

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		if (SCR_Global.IsEditMode() || !Replication.IsServer()) return;
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (factionManager)
			m_CivilianFaction = SCR_Faction.Cast(factionManager.GetFactionByKey("CIV"));	
		
		SCR_EntityCatalog characterCatalog = m_CivilianFaction.GetFactionEntityCatalogOfType(EEntityCatalogType.CHARACTER);
		characterCatalog.GetEntityList(m_aCharacters);
		
		SCR_EntityCatalog vehicleCatalog = m_CivilianFaction.GetFactionEntityCatalogOfType(EEntityCatalogType.VEHICLE);
		vehicleCatalog.GetEntityList(m_aVehicles);
	}
	
	SCR_EntityCatalogEntry GetRandomCharacter()
	{
		if (!m_aCharacters || m_aCharacters.IsEmpty())
			return null;
		
		return m_aCharacters.GetRandomElement();
	}
	
	SCR_EntityCatalogEntry GetRandomVehicle()
	{
		if (!m_aVehicles || m_aVehicles.IsEmpty())
			return null;
		
		return m_aVehicles.GetRandomElement();
	}
}
