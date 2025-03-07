class ESCT_MissionHeader : SCR_MissionHeader
{
	[Attribute("", UIWidgets.Auto, "Occupying faction key. Supported values: US, USSR, RANDOM.", category: "Escapists")]
	string m_sOccupyingFaction;
	
	[Attribute("", UIWidgets.Auto, "Player faction key. Should be not the same as occupying faction! Supported values: US, USSR, RANDOM.", category: "Escapists")]
	string m_sPlayerFaction;

	[Attribute("0", uiwidget: UIWidgets.ComboBox, desc: "Enemy AI skill. Cylon and None values are not supported.", enums: ParamEnumArray.FromEnum(EAISkill), category: "Escapists")]
	EAISkill m_eAiSkill;

	[Attribute("0", uiwidget: UIWidgets.ComboBox, desc: "Player character survivability.", enums: ParamEnumArray.FromEnum(ESCT_EHealthType), category: "Escapists")]
	ESCT_EHealthType m_eHealthType;

	[Attribute("", UIWidgets.EditBox, "Starting weather. Supported values: Clear, Cloudy, Overcast, Rainy, Random.", category: "Escapists")]
	string m_sStartingWeather;

	[Attribute("-1", UIWidgets.Slider, "Starting time of day (hours) (Escapists)", "0 25 1")]
	int m_iStartHours;

	[Attribute(defvalue: "-1.0", uiwidget: UIWidgets.Slider, params: "0.1 12.0 1.0", precision: 2, desc: "General time acceleration.", category: "Escapists")]
	float m_fTimeAcceleration;

	[Attribute(defvalue: "-1.0", uiwidget: UIWidgets.Slider, params: "0.1 1 0.05", precision: 2, desc: "How many locations will be disabled in the particular run.", category: "Escapists")]
	float m_fDisabledLocationsRatio;
	
	[Attribute("-1", UIWidgets.Slider, "Civilian Presence", "0 1 1")]
	int m_iCivilianPresence;
	
	[Attribute("-1", UIWidgets.Slider, "Insurgent faction.", "0 1 1")]
	int m_iInsurgency;
	
	[Attribute("-1", UIWidgets.Slider, "How frequent ambient events would happen (seconds per tick).", "-1 240 1")]
	int m_iAmbientEventsFrequency;
	
	[Attribute(defvalue: "-1.0", UIWidgets.Slider, desc: "Chance of static location (radio station, hospital etc) to be present in the game.", category: "Escapists", params: "0.1 1 0.125")]
	float m_fStaticLocationChance;
	
	[Attribute(defvalue: "-1.0", UIWidgets.Slider, desc: "Items spawn chance (storages and horizontal surfaces).", category: "Escapists", params: "0.1 1 0.125")]
	float m_fItemSpawnChance;
	
	[Attribute(defvalue: "-1.0", UIWidgets.Slider, desc: "Characters spawn chance (on character spawn points on location compositions).", category: "Escapists", params: "0.1 1 0.125")]
	float m_fCharacterSpawnChance;
	
	[Attribute(defvalue: "-1.0", UIWidgets.Slider, desc: "Chances of map item in enemy character's loadout.", category: "Escapists", params: "0.1 1 0.125")]
	float m_fMapItemChance;
	
	[Attribute(defvalue: "-1.0", UIWidgets.Slider, desc: "Chances of armed car alarm on ambient cars.", category: "Escapists", params: "0.1 1 0.125")]
	float m_fCarAlarmChance;
	
	[Attribute(defvalue: "-1.0", UIWidgets.Slider, desc: "Chance that vehicle spawn on vehicle spawn points.", category: "Escapists", params: "0.1 1 0.125")]
	float m_fVehicleSpawnChance;
	
	[Attribute("1", UIWidgets.CheckBox, "When enabled, handovers admin menu to the logged in admin. Bypasses other mission header settings.")]
	bool m_bUseSetupMenu;
	
	[Attribute("0", uiwidget: UIWidgets.ComboBox, desc: "Start type. Supported values: Hideout, Prison, Random.", enums: ParamEnumArray.FromEnum(ESCT_EStartType), category: "Escapists")]
	ESCT_EStartType m_eStartType;
	
	[Attribute("0", UIWidgets.CheckBox, "When enabled, randomizes start time. Bypasses m_iStartHours if enabled.", category: "Escapists")]
	bool m_bRandomStartingTime;
	
	[Attribute("0", UIWidgets.CheckBox, "When enabled, players have the ability to withstand death with adminestring epinephrine on themselves.", category: "Escapists")]
	bool m_bUseWithstand;
}
