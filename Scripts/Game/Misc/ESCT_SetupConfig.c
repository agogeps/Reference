[BaseContainerProps(configRoot: true)]
class ESCT_SetupConfig
{
	[Attribute(defvalue: "0.35", UIWidgets.Slider, category: "Escapists", params: "0.1 1 0.125")]
	float m_fVehicleSpawnChance;

	[Attribute(defvalue: "0.4", UIWidgets.Slider, desc: "Chance that ambient car will have armed car alarm.", category: "Escapists", params: "0.1 1 0.125")]
	float m_fCarAlarmChance;

	[Attribute(defvalue: "0.2", UIWidgets.Slider, desc: "Chance of map item in enemy character's loadout,", category: "Escapists", params: "0.1 1 0.05")]
	float m_fMapItemChance;

	[Attribute(defvalue: "0.35", UIWidgets.Slider, desc: "Characters spawn chance (on character spawn points on location compositions and military buildings).", category: "Escapists", params: "0.1 1 0.125")]
	float m_fCharacterSpawnChance;

	[Attribute(defvalue: "0.4", UIWidgets.Slider, desc: "Items spawn chance (storages and horizontal surfaces).", category: "Escapists", params: "0 100 1")]
	float m_fItemSpawnChance;

	[Attribute(defvalue: "0.35", UIWidgets.Slider, desc: "Chance of static location (radio station, hospital etc) to be present in the game.", category: "Escapists", params: "0.1 1 0.125")]
	float m_fStaticLocationChance;

	[Attribute(defvalue: "1.0", uiwidget: UIWidgets.Slider, params: "0.1 12.0 1.0", precision: 2, desc: "General time acceleration.", category: "Escapists")]
	float m_fTimeAcceleration;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.Slider, params: "0.1 1 0.05", precision: 2, desc: "How many locations will be disabled in the particular run.", category: "Escapists")]
	float m_fDisabledLocationsRatio;

	[Attribute("8", UIWidgets.Slider, "Starting time of day (hours) (Escapists)", "0 23 1")]
	int m_iStartingHours;

	[Attribute("0", uiwidget: UIWidgets.ComboBox, desc: "Player character survivability.", enums: ParamEnumArray.FromEnum(ESCT_EHealthType), category: "Escapists")]
	ESCT_EHealthType m_eHealthType;

	[Attribute("50", uiwidget: UIWidgets.ComboBox, desc: "Enemy AI skill. Cylon and None values are not supported.", enums: ParamEnumArray.FromEnum(EAISkill), category: "Escapists")]
	EAISkill m_eSkill;

	[Attribute("1", UIWidgets.CheckBox, desc: "Civlian presence (cars and pedestrians).", category: "Escapists")]
	bool m_bCivilianPresence;
	
	[Attribute("1", UIWidgets.CheckBox, desc: "Insurgency being present in the run.", category: "Escapists")]
	bool m_bInsurgency;

	[Attribute("Clear", UIWidgets.EditBox, "Starting weather. Supported values: Clear, Cloudy, Overcast, Rainy. ", category: "Escapists")]
	string m_sStartingWeather;

	[Attribute("US", UIWidgets.Auto, "Occupying faction key. Supported values: US, USSR, RANDOM.", category: "Escapists")]
	string m_sEnemyFactionKey;

	[Attribute("USSR", UIWidgets.Auto, "Player faction key. Supported values: US, USSR, RANDOM.", category: "Escapists")]
	string m_sPlayerFactionKey;
	
	[Attribute("0", uiwidget: UIWidgets.ComboBox, desc: "Start location and it's surrounding conditions for escapees.", enums: ParamEnumArray.FromEnum(ESCT_EStartType), category: "Escapists")]
	ESCT_EStartType m_eStartType;
	
	[Attribute("0", uiwidget: UIWidgets.ComboBox, desc: "Start location and it's surrounding conditions for escapees.", enums: ParamEnumArray.FromEnum(ESCT_EStartType), category: "Escapists")]
	bool m_bRandomStartingTime;
	
	[Attribute("45", UIWidgets.Slider, "How frequent ambient events would happen (seconds per tick).", "-1 240 1")]
	int m_iAmbientEventsFrequency;
	
	[Attribute("1", UIWidgets.CheckBox, desc: "Ability to withstand death by applying ", category: "Escapists")]
	bool m_bUseWithstand;

	static bool Extract(ESCT_SetupConfig instance, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.SerializeFloat(instance.m_fVehicleSpawnChance);
		snapshot.SerializeFloat(instance.m_fCarAlarmChance);
		snapshot.SerializeFloat(instance.m_fMapItemChance);
		snapshot.SerializeFloat(instance.m_fCharacterSpawnChance);
		snapshot.SerializeFloat(instance.m_fItemSpawnChance);
		snapshot.SerializeFloat(instance.m_fStaticLocationChance);
		snapshot.SerializeFloat(instance.m_fTimeAcceleration);
		snapshot.SerializeFloat(instance.m_fDisabledLocationsRatio);
		snapshot.SerializeInt(instance.m_iStartingHours);
		snapshot.SerializeInt(instance.m_eHealthType);
		snapshot.SerializeInt(instance.m_eSkill);
		snapshot.SerializeInt(instance.m_eStartType);
		snapshot.SerializeInt(instance.m_iAmbientEventsFrequency);
		snapshot.SerializeBool(instance.m_bCivilianPresence);
		snapshot.SerializeBool(instance.m_bInsurgency);
		snapshot.SerializeBool(instance.m_bRandomStartingTime);
		snapshot.SerializeBool(instance.m_bUseWithstand);
		snapshot.SerializeString(instance.m_sStartingWeather);
		snapshot.SerializeString(instance.m_sEnemyFactionKey);
		snapshot.SerializeString(instance.m_sPlayerFactionKey);

		return true;
	}

	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, ESCT_SetupConfig instance)
	{
		snapshot.SerializeFloat(instance.m_fVehicleSpawnChance);
		snapshot.SerializeFloat(instance.m_fCarAlarmChance);
		snapshot.SerializeFloat(instance.m_fMapItemChance);
		snapshot.SerializeFloat(instance.m_fCharacterSpawnChance);
		snapshot.SerializeFloat(instance.m_fItemSpawnChance);
		snapshot.SerializeFloat(instance.m_fStaticLocationChance);
		snapshot.SerializeFloat(instance.m_fTimeAcceleration);
		snapshot.SerializeFloat(instance.m_fDisabledLocationsRatio);
		snapshot.SerializeInt(instance.m_iStartingHours);
		snapshot.SerializeInt(instance.m_eHealthType);
		snapshot.SerializeInt(instance.m_eSkill);
		snapshot.SerializeInt(instance.m_eStartType);
		snapshot.SerializeInt(instance.m_iAmbientEventsFrequency);
		snapshot.SerializeBool(instance.m_bCivilianPresence);
		snapshot.SerializeBool(instance.m_bInsurgency);
		snapshot.SerializeBool(instance.m_bRandomStartingTime);
		snapshot.SerializeBool(instance.m_bUseWithstand);
		snapshot.SerializeString(instance.m_sStartingWeather);
		snapshot.SerializeString(instance.m_sEnemyFactionKey);
		snapshot.SerializeString(instance.m_sPlayerFactionKey);

		return true;
	}

	static void Encode(SSnapSerializerBase snapshot, ScriptCtx hint, ScriptBitSerializer packet)
	{
		snapshot.EncodeFloat(packet);		//m_fVehicleSpawnChance
		snapshot.EncodeFloat(packet);		//m_fCarAlarmChance
		snapshot.EncodeFloat(packet);		//m_fMapItemChance
		snapshot.EncodeFloat(packet);		//m_fCharacterSpawnChance
		snapshot.EncodeFloat(packet);		//m_fItemSpawnChance
		snapshot.EncodeFloat(packet);		//m_fStaticLocationChance
		snapshot.EncodeFloat(packet);		//m_fTimeAcceleration
		snapshot.EncodeFloat(packet);		//m_fDisabledLocationsRatio
		snapshot.EncodeInt(packet);	 		//m_iStartingHours
		snapshot.EncodeInt(packet);	 		//m_eHealthType
		snapshot.EncodeInt(packet); 		//m_eSkill
		snapshot.EncodeInt(packet);			//m_eStartType
		snapshot.EncodeInt(packet); 		//m_iAmbientEventsFrequency
		snapshot.EncodeBool(packet); 		//m_bCivilianPresence
		snapshot.EncodeBool(packet); 		//m_bInsurgency
		snapshot.EncodeBool(packet); 		//m_bRandomStartingTime
		snapshot.EncodeBool(packet); 		//m_bUseWithstand
		snapshot.EncodeString(packet);		//m_sStartingWeather
		snapshot.EncodeString(packet);		//m_sEnemyFactionKey
		snapshot.EncodeString(packet);		//m_sPlayerFactionKey
	}

	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.DecodeFloat(packet);		//m_fVehicleSpawnChance
		snapshot.DecodeFloat(packet);		//m_fCarAlarmChance
		snapshot.DecodeFloat(packet);		//m_fMapItemChance
		snapshot.DecodeFloat(packet);		//m_fCharacterSpawnChance
		snapshot.DecodeFloat(packet);		//m_fItemSpawnChance
		snapshot.DecodeFloat(packet);		//m_fStaticLocationChance
		snapshot.DecodeFloat(packet);		//m_fTimeAcceleration
		snapshot.DecodeFloat(packet);		//m_fDisabledLocationsRatio
		snapshot.DecodeInt(packet); 		//m_iStartingHours
		snapshot.DecodeInt(packet); 		//m_eHealthType
		snapshot.DecodeInt(packet); 		//m_eSkill
		snapshot.DecodeInt(packet);			//m_eStartType
		snapshot.DecodeInt(packet);			//m_iAmbientEventsFrequency
		snapshot.DecodeBool(packet); 		//m_bCivilianPresence
		snapshot.DecodeBool(packet); 		//m_bInsurgency
		snapshot.DecodeBool(packet); 		//m_bRandomStartingTime
		snapshot.DecodeBool(packet); 		//m_bUseWithstand
		snapshot.DecodeString(packet);		//m_sStartingWeather
		snapshot.DecodeString(packet);		//m_sEnemyFactionKey
		snapshot.DecodeString(packet);		//m_sPlayerFactionKey

		return true;
	}

	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx ctx)
	{
		return lhs.CompareSnapshots(rhs, 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4) 		//m_fVehicleSpawnChance, m_fCarAlarmChance, m_fMapItemChance, m_fCharacterSpawnChance, m_fItemSpawnChance, m_fStaticLocationChance, m_fTimeAcceleration, m_fDisabledLocationsRatio
			&& lhs.CompareSnapshots(rhs, 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4) 		//m_iStartingHours, m_eHealthType, m_eSkill, m_eStartType, m_bCivilianPresence, m_bInsurgency, m_bRandomStartingTime, m_bUseWithstand, m_iAmbientEventsFrequency
			&& lhs.CompareStringSnapshots(rhs)									//m_sStartingWeather
			&& lhs.CompareStringSnapshots(rhs)									//m_sEnemyFactionKey
			&& lhs.CompareStringSnapshots(rhs);									//m_sPlayerFactionKey
	}

	static bool PropCompare(ESCT_SetupConfig instance, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{
		bool compareFloat = snapshot.CompareFloat(instance.m_fVehicleSpawnChance)
			&& snapshot.CompareFloat(instance.m_fCarAlarmChance)
			&& snapshot.CompareFloat(instance.m_fMapItemChance)
			&& snapshot.CompareFloat(instance.m_fCharacterSpawnChance)
			&& snapshot.CompareFloat(instance.m_fItemSpawnChance)
			&& snapshot.CompareFloat(instance.m_fStaticLocationChance)
			&& snapshot.CompareFloat(instance.m_fTimeAcceleration)
			&& snapshot.CompareFloat(instance.m_fDisabledLocationsRatio);

		bool compareInt = snapshot.CompareInt(instance.m_iStartingHours)
			&& snapshot.CompareInt(instance.m_eHealthType)
			&& snapshot.CompareInt(instance.m_eSkill)
			&& snapshot.CompareInt(instance.m_eStartType)
			&& snapshot.CompareInt(instance.m_iAmbientEventsFrequency);

		bool compareBool = snapshot.CompareBool(instance.m_bCivilianPresence) 
			&& snapshot.CompareBool(instance.m_bInsurgency)
			&& snapshot.CompareBool(instance.m_bRandomStartingTime)
			&& snapshot.CompareBool(instance.m_bUseWithstand);

		bool compareString = snapshot.CompareString(instance.m_sStartingWeather)
			&& snapshot.CompareString(instance.m_sEnemyFactionKey)
			&& snapshot.CompareString(instance.m_sPlayerFactionKey);

		return compareFloat && compareInt && compareBool && compareString;
	}
}
