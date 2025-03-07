class ESCT_Faction : SCR_Faction
{
	[Attribute(desc: "Faction info related to players.", uiwidget: UIWidgets.Object, category: "Escapists")]
	protected ref ESCT_PlayerFactionInfo m_PlayerFactionInfo;

	[Attribute("", UIWidgets.Auto, "Factions that can't be chosen as player faction if this faction is selected as enemy.", category: "Escapists")]
	protected ref array<string> m_aExcludedPlayerFactionIds;

	[Attribute("", UIWidgets.ResourceAssignArray, "List of helicopter with flyby capabilities.", "et", category: "Escapists")]
	protected ref array<ResourceName> m_aFlybyHelicopters;

	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Radio prefab.", params: "et", category: "Escapists")]
	protected ResourceName m_Radio;

	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Group that will be used for spawned characters (usually individual chars).", params: "et", category: "Escapists")]
	protected ResourceName m_BaseFactionGroup;

	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Group that will be used for spawned characters with RTO (radio/support call capabilities).", params: "et", category: "Escapists")]
	protected ResourceName m_BaseFactionRadioGroup;

	protected ref ESCT_GroupLookup m_GroupLookup;
	protected ref ESCT_CharacterLookup m_CharacterLookup;

	private ESCT_EUnitRole m_UnitRole;

	ESCT_PlayerFactionInfo GetPlayerFactionInfo()
	{
		return m_PlayerFactionInfo;
	}

	ResourceName GetRadio()
	{
		return m_Radio;
	}

	array<ResourceName> GetFlybyHelicopterPrefabs()
	{
		return m_aFlybyHelicopters;
	}

	ResourceName GetBaseFactionGroup()
	{
		return m_BaseFactionGroup;
	}

	ResourceName GetBaseFactionRadioGroup()
	{
		return m_BaseFactionRadioGroup;
	}

	array<string> GetExcludedPlayerFactionIds()
	{
		return m_aExcludedPlayerFactionIds;
	}

	ResourceName GetRandomCharacterOfBranch(ESCT_EMilitaryBranchType branch)
	{
		if (!m_CharacterLookup)
			return ResourceName.Empty;

		return m_CharacterLookup.GetRandomCharacterOfBranch(branch);
	}

	ResourceName GetRandomCharacterOfRole(ESCT_EUnitRole role)
	{
		if (!m_CharacterLookup)
			return ResourceName.Empty;

		return m_CharacterLookup.GetRandomCharacterOfRole(role);
	}

	ResourceName GetRandomGroupOfBranchAndSize(ESCT_EMilitaryBranchType branch, ESCT_EGroupSize size)
	{
		if (!m_GroupLookup)
			return ResourceName.Empty;

		return m_GroupLookup.GetRandomGroupOfBranchAndSize(branch, size);
	}

	void BuildFactionLookups()
	{
		m_GroupLookup = new ESCT_GroupLookup(this);
		m_GroupLookup.Build();

		m_CharacterLookup = new ESCT_CharacterLookup(this);
		m_CharacterLookup.Build();
	}
}
