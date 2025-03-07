[BaseContainerProps(configRoot: true)]
class ESCT_PlayerFactionInfo
{
	[Attribute()]
	protected ref array<ref ESCT_PlayerCharacter> m_Characters;

	[Attribute("", desc: "Pairs of pants and jacket.")]
	protected ref array<ref ESCT_EquipmentInfo> m_Pairs;

	[Attribute(defvalue: "", uiwidget: UIWidgets.EditBox, desc: "Audio project key for first radio message")]
	protected string m_sFirstRadioMessage;

	[Attribute(defvalue: "", uiwidget: UIWidgets.EditBox, desc: "Audio project key for second radio message")]
	protected string m_sSecondRadioMessage;

	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Flashlight prefab.", params: "et")]
	protected ResourceName m_Flashlight;
	
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Helicrash prefab.", params: "et")]
	protected ResourceName m_HelicrashComposition;
	
	[Attribute(uiwidget: UIWidgets.Auto, desc: "Helicrash pilot equipment.")]
	protected ref ESCT_EquipmentInfo m_PilotEquipment;

	private ESCT_Faction m_Faction;

	ESCT_PlayerCharacter GetPlayerCharacterByType(ESCT_EHealthType type)
	{
		if (!m_Characters)
			return null;

		ESCT_PlayerCharacter character = null;

		for (int i = 0; i < m_Characters.Count(); i++)
		{
			character = m_Characters[i];
			if (character.GetHealthType() == type)
			{
				break;
			}
		}

		return character;
	}

	ResourceName GetFlashlightPrefab()
	{
		return m_Flashlight;
	}
	
	ResourceName GetHelicrashPrefab()
	{
		return m_HelicrashComposition;
	}

	array<ref ESCT_PlayerCharacter> GetPlayerCharacters()
	{
		return m_Characters;
	}

	array<ref ESCT_EquipmentInfo> GetEquipmentPairs()
	{
		return m_Pairs;
	}
	
	ESCT_EquipmentInfo GetPilotEquipment()
	{
		return m_PilotEquipment;
	}

	string GetFirstRadioMessage()
	{
		return m_sFirstRadioMessage;
	}

	string GetSecondRadioMessage()
	{
		return m_sSecondRadioMessage;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] faction Owner faction of this player faction info.
	void SetFaction(notnull ESCT_Faction faction)
	{
		m_Faction = faction;
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ESCT_EHealthType, "m_eHealthType")]
class ESCT_PlayerCharacter
{
	[Attribute(uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ESCT_EHealthType))]
	protected ESCT_EHealthType m_eHealthType;

	[Attribute(desc: "Player character prefab.", UIWidgets.ResourcePickerThumbnail, params: "et")]
	protected ResourceName m_CharacterPrefab;

	[Attribute(desc: "Player loadout name. Should be present in loadout manager.", uiwidget: UIWidgets.Auto)]
	protected string m_sLoadoutName;

	ESCT_EHealthType GetHealthType() { return m_eHealthType; }

	ResourceName GetCharacterPrefab() { return m_CharacterPrefab; }

	string GetLoadoutName() { return m_sLoadoutName; }
}
