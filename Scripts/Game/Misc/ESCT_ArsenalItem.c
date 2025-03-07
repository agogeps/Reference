[BaseContainerProps(configRoot: true), BaseContainerCustomCheckIntTitleField("m_bEnabled", "Arsenal Data", "DISABLED - Arsenal Data", 1)]
modded class SCR_ArsenalItem : SCR_BaseEntityCatalogData
{
	[Attribute(defvalue: "0.5", uiwidget: UIWidgets.Slider, params: "0 1 0.05", precision: 2, desc: "Random selection weight")]
	protected float m_fWeight;

	[Attribute("0", uiwidget: UIWidgets.CheckBox, "Could item spawn in prisons?", category: "Game Mode")]
	protected bool m_bEligibleForPrison;

	float GetWeight()
	{
		return m_fWeight;
	}

	bool IsEligibleForPrison()
	{
		return m_bEligibleForPrison;
	}
}

[BaseContainerProps(configRoot: true), BaseContainerCustomCheckIntTitleField("m_bEnabled", "Branch Data", "DISABLED - Branch Data", 1)]
class ESCT_MilitaryBranchData : SCR_BaseEntityCatalogData
{
	[Attribute("0", uiwidget: UIWidgets.ComboBox, desc: "Character type.", enums: ParamEnumArray.FromEnum(ESCT_EMilitaryBranchType))]
	protected ESCT_EMilitaryBranchType m_eBranch;
	
	ESCT_EMilitaryBranchType GetBranch()
	{
		return m_eBranch;
	}
}

[BaseContainerProps(configRoot: true), BaseContainerCustomCheckIntTitleField("m_bEnabled", "Unit Role Data", "DISABLED - Unit Role Data", 1)]
class ESCT_UnitRoleData : SCR_BaseEntityCatalogData
{
	[Attribute("0", uiwidget: UIWidgets.ComboBox, desc: "Unit type (role).", enums: ParamEnumArray.FromEnum(ESCT_EUnitRole))]
	protected ESCT_EUnitRole m_eUnitRole;
	
	ESCT_EUnitRole GetUnitRole()
	{
		return m_eUnitRole;
	}
}

[BaseContainerProps(configRoot: true), BaseContainerCustomCheckIntTitleField("m_bEnabled", "Location Data", "DISABLED - Location", 1)]
class ESCT_LocationData : SCR_BaseEntityCatalogData
{
	[Attribute("4", uiwidget: UIWidgets.ComboBox, desc: "Group size.", enums: ParamEnumArray.FromEnum(ESCT_ELocationType))]
	protected ESCT_ELocationType m_eLocationType;

	ESCT_ELocationType GetLocationType()
	{
		return m_eLocationType;
	}
}

[BaseContainerProps(configRoot: true), BaseContainerCustomCheckIntTitleField("m_bEnabled", "Exclude from Character Lookup Map", "DISABLED - Exclude from Character Lookup Map", 1)]
class ESCT_ExcludeFromCharacterLookupData : SCR_BaseEntityCatalogData
{
	[Attribute("1", uiwidget: UIWidgets.CheckBox)]
	protected bool m_bExcludeFromCharacterRoleLookup;
	
	[Attribute("1", uiwidget: UIWidgets.CheckBox)]
	protected bool m_bExcludeFromCharacterBranchLookup;
	
	bool IsExcludedFromRoleLookup()
	{
		return m_bExcludeFromCharacterRoleLookup;
	}
	
	bool IsExcludedFromBranchLookup()
	{
		return m_bExcludeFromCharacterBranchLookup;
	}
}

//groups can be still used by various other systems (ambient patrols etc) but we don't need them in entity catalog pool for faction group lookups
[BaseContainerProps(configRoot: true), BaseContainerCustomCheckIntTitleField("m_bEnabled", "Exclude from Group Lookup Map", "DISABLED - Exclude from Group Lookup Map", 1)]
class ESCT_ExcludeFromGroupLookupData : SCR_BaseEntityCatalogData
{
}

[BaseContainerProps(), BaseContainerCustomDoubleCheckIntResourceNameTitleField("m_bEnabled", "m_sEntityPrefab", 1, "%1", "DISABLED - %1")]
modded class SCR_EntityCatalogEntry
{
	[Attribute(defvalue: "0.5", uiwidget: UIWidgets.Slider, params: "0 1 0.05", precision: 2, desc: "Random selection weight")]
	protected float m_fWeight;

	float GetWeight()
	{
		return m_fWeight;
	}
}

modded enum EEntityCatalogType
{
	LOCATION = 500,
	STORAGE = 501
}
