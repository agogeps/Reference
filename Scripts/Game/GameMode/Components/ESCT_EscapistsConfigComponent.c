[ComponentEditorProps(category: "Escapists/GameMode/Components", description: "Components that holds configuration values for most things in this game mode allow for fine-tuning of used assets.")]
class ESCT_EscapistsConfigComponentClass : ESCT_GameModeBaseComponentClass
{
}

sealed class ESCT_EscapistsConfigComponent : ESCT_GameModeBaseComponent
{
	[Attribute("", UIWidgets.Auto, category: "Escapists")]
	protected ref ESCT_GeneralConfig m_GeneralConfig;

	[Attribute("", UIWidgets.Auto, category: "Escapists")]
	protected ref ESCT_SetupConfig m_SetupConfig;

	[RplProp()]
	protected float m_fVehicleSpawnChance;

	[RplProp()]
	protected float m_fCarAlarmChance;

	[RplProp()]
	protected float m_fMapItemChance;

	[RplProp()]
	protected float m_fCharacterSpawnChance;

	[RplProp()]
	protected float m_fItemSpawnChance;

	[RplProp()]
	protected float m_fStaticLocationChance;

	[RplProp()]
	protected float m_fTimeAcceleration;

	[RplProp()]
	protected float m_fDisabledLocationsRatio;

	[RplProp()]
	protected int m_iStartingHours;
	
	[RplProp()]
	protected int m_iAmbientEventsFrequency;
	
	[RplProp()]
	protected bool m_bRandomStartingTime;

	[RplProp()]
	protected ESCT_EHealthType m_eHealthType;

	[RplProp()]
	protected EAISkill m_eSkill;

	[RplProp()]
	protected bool m_bCivilianPresence;

	[RplProp()]
	protected bool m_bInsurgency;
	
	[RplProp()]
	protected bool m_bUseWithstand;

	[RplProp()]
	protected string m_sStartingWeather;
	
	[RplProp()]
	protected ESCT_EStartType m_eStartType;

	[RplProp(onRplName: "OnEnemyFactionKeyChanged")]
	protected string m_sEnemyFactionKey;

	[RplProp(onRplName: "OnPlayerFactionKeyChanged")]
	protected string m_sPlayerFactionKey;

	private static ESCT_EscapistsConfigComponent s_Instance = null;
	static ESCT_EscapistsConfigComponent GetInstance()
	{
		if (!s_Instance)
		{
			BaseGameMode gameMode = GetGame().GetGameMode();
			if (gameMode)
			{
				s_Instance = ESCT_EscapistsConfigComponent.Cast(gameMode.FindComponent(ESCT_EscapistsConfigComponent));
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

		if (!m_SetupConfig)
		{
			ESCT_Logger.Error("Initial setup config is not specified!");
			return;
		}

		ApplyConfig(m_SetupConfig, "default");
	}

	void ApplyConfig(notnull ESCT_SetupConfig incomingSetupConfig, string title)
	{
		if (!Replication.IsServer())
			return;

		ESCT_Logger.Info("-------------------------------------------------");
		ESCT_Logger.InfoFormat("Applying %1 Escapists config values...", title);

		ESCT_Logger.InfoFormat("Weather: %1", incomingSetupConfig.m_sStartingWeather);
		m_sStartingWeather = incomingSetupConfig.m_sStartingWeather;

		ESCT_Logger.InfoFormat("Civilian Presence: %1", incomingSetupConfig.m_bCivilianPresence.ToString());
		m_bCivilianPresence = incomingSetupConfig.m_bCivilianPresence;

		ESCT_Logger.InfoFormat("Insurgency: %1", incomingSetupConfig.m_bInsurgency.ToString());
		m_bInsurgency = incomingSetupConfig.m_bInsurgency;

		ESCT_Logger.InfoFormat("Withstand: %1", incomingSetupConfig.m_bUseWithstand.ToString());
		m_bUseWithstand = incomingSetupConfig.m_bUseWithstand;
		
		ESCT_Logger.InfoFormat("AI Skill: %1", typename.EnumToString(EAISkill, incomingSetupConfig.m_eSkill));
		m_eSkill = incomingSetupConfig.m_eSkill;

		ESCT_Logger.InfoFormat("Health Type: %1", typename.EnumToString(ESCT_EHealthType, incomingSetupConfig.m_eHealthType));
		m_eHealthType = incomingSetupConfig.m_eHealthType;

		ESCT_Logger.InfoFormat("Starting hours: %1", incomingSetupConfig.m_iStartingHours.ToString());
		m_iStartingHours = incomingSetupConfig.m_iStartingHours;
		
		ESCT_Logger.InfoFormat("Ambient Events Frequency (seconds per tick): %1", incomingSetupConfig.m_iAmbientEventsFrequency.ToString());
		m_iAmbientEventsFrequency = incomingSetupConfig.m_iAmbientEventsFrequency;

		ESCT_Logger.InfoFormat("Time Acceleration: %1", incomingSetupConfig.m_fTimeAcceleration.ToString());
		m_fTimeAcceleration = incomingSetupConfig.m_fTimeAcceleration;

		ESCT_Logger.InfoFormat("Disabled Locations Ratio: %1", incomingSetupConfig.m_fDisabledLocationsRatio.ToString());
		m_fDisabledLocationsRatio = incomingSetupConfig.m_fDisabledLocationsRatio;

		ESCT_Logger.InfoFormat("Static Location Spawn Chance: %1", incomingSetupConfig.m_fStaticLocationChance.ToString());
		m_fStaticLocationChance = incomingSetupConfig.m_fStaticLocationChance;

		ESCT_Logger.InfoFormat("Item Spawn Chance: %1", incomingSetupConfig.m_fItemSpawnChance.ToString());
		m_fItemSpawnChance = incomingSetupConfig.m_fItemSpawnChance;

		ESCT_Logger.InfoFormat("Character Spawn Chance: %1", incomingSetupConfig.m_fCharacterSpawnChance.ToString());
		m_fCharacterSpawnChance = incomingSetupConfig.m_fCharacterSpawnChance;

		ESCT_Logger.InfoFormat("Vehicle Spawn Chance: %1", incomingSetupConfig.m_fVehicleSpawnChance.ToString());
		m_fVehicleSpawnChance = incomingSetupConfig.m_fVehicleSpawnChance;

		ESCT_Logger.InfoFormat("Map Item Chance: %1", incomingSetupConfig.m_fMapItemChance.ToString());
		m_fMapItemChance = incomingSetupConfig.m_fMapItemChance;

		ESCT_Logger.InfoFormat("Car Alarm Chance: %1", incomingSetupConfig.m_fCarAlarmChance.ToString());
		m_fCarAlarmChance = incomingSetupConfig.m_fCarAlarmChance;
		
		ESCT_Logger.InfoFormat("Start Type: %1", typename.EnumToString(ESCT_EStartType, incomingSetupConfig.m_eHealthType));
		m_eStartType = incomingSetupConfig.m_eStartType;

		ESCT_Logger.InfoFormat("Enemy Faction Key : %1", incomingSetupConfig.m_sEnemyFactionKey);
		m_sEnemyFactionKey = incomingSetupConfig.m_sEnemyFactionKey;

		ESCT_Logger.InfoFormat("Enemy Faction Key : %1", incomingSetupConfig.m_sPlayerFactionKey);
		m_sPlayerFactionKey = incomingSetupConfig.m_sPlayerFactionKey;
		
		ESCT_Logger.InfoFormat("Random starting time: %1", incomingSetupConfig.m_bRandomStartingTime.ToString());
		m_bRandomStartingTime = incomingSetupConfig.m_bRandomStartingTime;

		ESCT_FactionManager.GetInstance().SetFactions(incomingSetupConfig.m_sEnemyFactionKey, incomingSetupConfig.m_sPlayerFactionKey);

		Replication.BumpMe();

		ESCT_Logger.InfoFormat("Escapists %1 config values have been applied.", title);
		ESCT_Logger.Info("-------------------------------------------------");
	}

	ESCT_SetupConfig GetSetupConfig()
	{
		return m_SetupConfig;
	}

	float GetMarkerRevealDistance()
	{
		return m_GeneralConfig.m_fMarkerRevealDistance;
	}

	int GetLocationClearThreshold()
	{
		return m_GeneralConfig.m_iLocationClearThreshold;
	}

	array<ref ESCT_LocationConfig> GetRandomLocationTypes()
	{
		return m_GeneralConfig.m_RandomLocationTypes;
	}

	string GetStartingWeather()
	{
		return m_sStartingWeather;
	}

	float GetTimeAcceleration()
	{
		return m_fTimeAcceleration;
	}

	float GetLocationsQuantityMultiplier()
	{
		return m_fDisabledLocationsRatio;
	}

	bool GetCivilianPresence()
	{
		return m_bCivilianPresence;
	}

	float GetStartingHours()
	{
		return m_iStartingHours;
	}
	
	bool IsStartingTimeRandomized()
	{
		return m_bRandomStartingTime;
	}

	EAISkill GetAiSkill()
	{
		return m_eSkill;
	}

	ESCT_EHealthType GetHealthType()
	{
		return m_eHealthType;
	}

	float GetStaticLocationChance()
	{
		return m_fStaticLocationChance;
	}

	float GetItemSpawnChance()
	{
		return m_fItemSpawnChance;
	}

	float GetCharacterSpawnChance()
	{
		return m_fCharacterSpawnChance;
	}

	float GetVehicleSpawnChance()
	{
		return m_fVehicleSpawnChance;
	}

	float GetMapItemChance()
	{
		return m_fMapItemChance;
	}

	float GetCarAlarmChance()
	{
		return m_fCarAlarmChance;
	}

	bool IsInsurgencyEnabled()
	{
		return m_bInsurgency;
	}
	
	bool IsWithstandEnabled()
	{
		return m_bUseWithstand;
	}
	
	ESCT_EStartType GetStartType()
	{
		return m_eStartType;
	}
	
	int GetAmbientEventsFrequency()
	{
		return m_iAmbientEventsFrequency;
	}

	void LoadHeaderSettings()
	{
		if (RplSession.Mode() != RplMode.Dedicated)
			return;

		ESCT_MissionHeader escapistsHeader = GetGame().GetEscapistsMissionHeader();
		if (!escapistsHeader)
			return;

		ESCT_SetupConfig missionConfig = ESCT_SetupConfig.Cast(m_SetupConfig.Clone());

		if (escapistsHeader.m_bUseSetupMenu)
		{
			ESCT_Logger.Info("Mission header will be not used as setup UI admin handover is enabled.");
			return;
		}

		ESCT_Logger.Info("-------------------------------------------------");
		ESCT_Logger.Info("Setting mission header options...");
		
		if (escapistsHeader.m_iStartHours > 0)
		{
			ESCT_Logger.InfoFormat("Starting Hours: %1", escapistsHeader.m_iStartHours.ToString());
			missionConfig.m_iStartingHours = escapistsHeader.m_iStartHours;
		}
		
		if (escapistsHeader.m_iAmbientEventsFrequency != -1)
		{
			ESCT_Logger.InfoFormat("Ambient events frequency (seconds per tick): %1", escapistsHeader.m_iAmbientEventsFrequency.ToString());
			missionConfig.m_iAmbientEventsFrequency = escapistsHeader.m_iAmbientEventsFrequency;
		}
		
		if (escapistsHeader.m_bRandomStartingTime)
		{
			ESCT_Logger.InfoFormat("Random Starting Time: %1", escapistsHeader.m_bRandomStartingTime.ToString());
			missionConfig.m_bRandomStartingTime = escapistsHeader.m_bRandomStartingTime;
		}
		
		if (escapistsHeader.m_bUseWithstand)
		{
			ESCT_Logger.InfoFormat("Withstand: %1", escapistsHeader.m_bUseWithstand.ToString());
			missionConfig.m_bUseWithstand = escapistsHeader.m_bUseWithstand;
		}

		if (escapistsHeader.m_eAiSkill != EAISkill.NONE)
		{
			ESCT_Logger.InfoFormat("AI Skill: %1", typename.EnumToString(EAISkill, escapistsHeader.m_eAiSkill));
			missionConfig.m_eSkill = escapistsHeader.m_eAiSkill;
		}

		if (escapistsHeader.m_eHealthType != ESCT_EHealthType.Default)
		{
			ESCT_Logger.InfoFormat("Health Type: %1", typename.EnumToString(ESCT_EHealthType, escapistsHeader.m_eHealthType));
			missionConfig.m_eHealthType = escapistsHeader.m_eHealthType;
		}
		
		if (escapistsHeader.m_eStartType != ESCT_EStartType.Random)
		{
			ESCT_Logger.InfoFormat("Start Type: %1", typename.EnumToString(ESCT_EStartType, escapistsHeader.m_eStartType));
			missionConfig.m_eStartType = escapistsHeader.m_eStartType;
		}

		if (escapistsHeader.m_sStartingWeather != string.Empty)
		{
			ESCT_Logger.InfoFormat("Weather: %1", escapistsHeader.m_sStartingWeather);

			string weatherValue = escapistsHeader.m_sStartingWeather;
			weatherValue.Trim();
			weatherValue.ToUpper();

			if (weatherValue == "RANDOM")
			{
				array<string> weathers = { "Clear", "Cloudy", "Overcast", "Rainy" };
				missionConfig.m_sStartingWeather = weathers.GetRandomElement();
			}
			else
			{
				missionConfig.m_sStartingWeather = escapistsHeader.m_sStartingWeather;
			}
		}

		if (escapistsHeader.m_fTimeAcceleration > 0)
		{
			ESCT_Logger.InfoFormat("Time Acceleration: %1", escapistsHeader.m_fTimeAcceleration.ToString());
			missionConfig.m_fTimeAcceleration = escapistsHeader.m_fTimeAcceleration;
		}

		if (escapistsHeader.m_fDisabledLocationsRatio > 0)
		{
			ESCT_Logger.InfoFormat("Disabled Locations Ratio: %1", escapistsHeader.m_fDisabledLocationsRatio.ToString());
			missionConfig.m_fDisabledLocationsRatio = escapistsHeader.m_fDisabledLocationsRatio;
		}

		if (escapistsHeader.m_iCivilianPresence != -1)
		{
			ESCT_Logger.InfoFormat("Civilian Presence: %1", escapistsHeader.m_iCivilianPresence.ToString());
			missionConfig.m_bCivilianPresence = escapistsHeader.m_iCivilianPresence != 0;
		}

		if (escapistsHeader.m_iInsurgency != -1)
		{
			ESCT_Logger.InfoFormat("Insurgency: %1", escapistsHeader.m_iInsurgency.ToString());
			missionConfig.m_bInsurgency = escapistsHeader.m_iInsurgency != 0;
		}

		if (escapistsHeader.m_fStaticLocationChance != -1)
		{
			ESCT_Logger.InfoFormat("Static Location Chance: %1", escapistsHeader.m_fStaticLocationChance.ToString());
			missionConfig.m_fStaticLocationChance = escapistsHeader.m_fStaticLocationChance;
		}

		if (escapistsHeader.m_fItemSpawnChance != -1)
		{
			ESCT_Logger.InfoFormat("Item Spawn Chance: %1", escapistsHeader.m_fItemSpawnChance.ToString());
			missionConfig.m_fItemSpawnChance = escapistsHeader.m_fItemSpawnChance;
		}

		if (escapistsHeader.m_fCharacterSpawnChance != -1)
		{
			ESCT_Logger.InfoFormat("Character Spawn Chance: %1", escapistsHeader.m_fCharacterSpawnChance.ToString());
			missionConfig.m_fCharacterSpawnChance = escapistsHeader.m_fCharacterSpawnChance;
		}

		if (escapistsHeader.m_fVehicleSpawnChance != -1)
		{
			ESCT_Logger.InfoFormat("Vehicle Spawn Chance: %1", escapistsHeader.m_fVehicleSpawnChance.ToString());
			missionConfig.m_fVehicleSpawnChance = escapistsHeader.m_fVehicleSpawnChance;
		}

		if (escapistsHeader.m_fMapItemChance != -1)
		{
			ESCT_Logger.InfoFormat("Character Spawn Chance: %1", escapistsHeader.m_fMapItemChance.ToString());
			missionConfig.m_fMapItemChance = escapistsHeader.m_fMapItemChance;
		}

		if (escapistsHeader.m_fCarAlarmChance != -1)
		{
			ESCT_Logger.InfoFormat("Car Alarm Chance: %1", escapistsHeader.m_fCarAlarmChance.ToString());
			missionConfig.m_fCarAlarmChance = escapistsHeader.m_fCarAlarmChance;
		}

		string enemyFactionKey = GetEnemyFactionKeyFromConfig(escapistsHeader);
		if (enemyFactionKey == string.Empty)
		{
			ESCT_Logger.InfoFormat("Unable to select enemy faction! Mission header value: %1", escapistsHeader.m_sOccupyingFaction);
			return;
		}
		
		string playerFactionKey = GetPlayerFactionKeyFromConfig(escapistsHeader, enemyFactionKey);
		if (enemyFactionKey == string.Empty)
		{
			ESCT_Logger.InfoFormat("Unable to select enemy faction! Mission header value: %1", escapistsHeader.m_sOccupyingFaction);
			return;
		}

		if (playerFactionKey == string.Empty)
		{
			ESCT_Logger.InfoFormat("Unable to select player faction! Mission header value: %1", escapistsHeader.m_sPlayerFaction);
			return;
		}

		missionConfig.m_sEnemyFactionKey = enemyFactionKey;
		missionConfig.m_sPlayerFactionKey = playerFactionKey;

		ApplyConfig(missionConfig, "mission header");

		ESCT_Logger.Info("Mission header options are set.");
		ESCT_Logger.Info("-------------------------------------------------");
	}

	private string GetEnemyFactionKeyFromConfig(ESCT_MissionHeader escapistsHeader)
	{
		ESCT_FactionManager factionManager = ESCT_FactionManager.GetInstance();
		if (!factionManager)
			return string.Empty;

		string factionKey;
		if (!escapistsHeader.m_sOccupyingFaction || escapistsHeader.m_sOccupyingFaction == string.Empty)
			factionKey = "RANDOM";
		else
		{
			factionKey = escapistsHeader.m_sOccupyingFaction.Trim();
			factionKey.ToUpper();
		}

		array<Faction> factions = {};
		factionManager.GetFactionsList(factions);

		for (int i = factions.Count() - 1; i >= 0; i--)
		{
			ESCT_Faction faction = ESCT_Faction.Cast(factions[i]);
			if (!faction)
				continue;
			
			string key = faction.GetFactionKey();
			if (key == "FIA" || !faction.IsMilitary())
			{
				factions.Remove(i);
			}
		}

		if (factionKey == "RANDOM")
		{
			Math.Randomize(-1);
			Faction selectedFaction = factions.GetRandomElement();

			return selectedFaction.GetFactionKey();
		}
		else if (factionManager.GetFactionByKey(factionKey))
		{
			return factionKey;
		}

		ESCT_Logger.ErrorFormat("Unsupported faction in config. Config value: %1.", factionKey);
		return string.Empty;
	}

	private string GetPlayerFactionKeyFromConfig(ESCT_MissionHeader escapistsHeader, string enemyFactionKey)
	{
		ESCT_FactionManager factionManager = ESCT_FactionManager.GetInstance();
		if (!factionManager)
			return string.Empty;
		
		ESCT_Faction enemyFaction = ESCT_Faction.Cast(factionManager.GetFactionByKey(enemyFactionKey));
		if (!enemyFaction)
			return string.Empty;

		string factionKey;
		if (!escapistsHeader.m_sPlayerFaction || escapistsHeader.m_sPlayerFaction == string.Empty)
		{
			factionKey = "RANDOM";
		}
		else
		{
			factionKey = escapistsHeader.m_sPlayerFaction.Trim();
			factionKey.ToUpper();
		}

		if (factionKey != "RANDOM")
		{
			if (factionKey == enemyFactionKey)
			{
				ESCT_Logger.ErrorFormat("Same player faction key as enemy faction detected! Config value: %1.", factionKey);
				return string.Empty;
			}

			if (!factionManager.GetFactionByKey(factionKey))
			{
				ESCT_Logger.ErrorFormat("Unsupported faction in config. Config value: %1.", factionKey);
				return string.Empty;
			}

			if (enemyFaction.GetExcludedPlayerFactionIds().Contains(factionKey))
			{
				ESCT_Logger.ErrorFormat("This faction is not compatible with %1 enemy faction. Config value: %2.", factionManager.GetEnemyFactionKey(), factionKey);
				return string.Empty;
			}

			return factionKey;
		}
			
		array<Faction> factions = {};
		factionManager.GetFactionsList(factions);

		for (int i = factions.Count() - 1; i >= 0; i--)
		{
			ESCT_Faction faction = ESCT_Faction.Cast(factions[i]);
			if (!faction)
				continue;

			string key = faction.GetFactionKey();

			if ({"FIA", enemyFaction.GetFactionKey()}.Contains(key) || enemyFaction.GetExcludedPlayerFactionIds().Contains(key) || !faction.IsMilitary())
			{
				factions.Remove(i);
			}
		}

		if (factions.IsEmpty())
		{
			ESCT_Logger.ErrorFormat("Random factions array is empty due to player/enemy faction incompatibility, nothing to choose from. Config value: %1.", factionKey);
			return string.Empty;
		}

		Faction selectedFaction = factions.GetRandomElement();
		return selectedFaction.GetFactionKey();
	}

	private void OnEnemyFactionKeyChanged()
	{
		ESCT_FactionManager.GetInstance().TrySetEnemyFaction_CA(m_sEnemyFactionKey);
	}

	private void OnPlayerFactionKeyChanged()
	{
		ESCT_FactionManager.GetInstance().TrySetPlayerFaction_CA(m_sPlayerFactionKey);
	}
}
