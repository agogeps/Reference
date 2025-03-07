[EntityEditorProps("Escapists/Location", description: "Handles turrets and it's gunners on locations.", color: "0 0 255 255")]
class ESCT_LocationTurretManagerComponentClass : ESCT_LocationComponentClass
{
}

class ESCT_LocationTurretManagerComponent : ESCT_LocationComponent
{
	private ref array<IEntity> m_aTurretGunners = {};
	private ref array<IEntity> m_aTurrets = {};
	private int m_iActiveTurrets = 0;
	private bool m_bIsCleared = false;

	override void Spawn()
	{
		if (m_bIsCleared)
			return;

		IEntity composition = m_Location.GetComposition();
		if (!composition)
			return;

		GetCompositionEntities(composition);
		PopulateTurrets();
	}

	override void Despawn()
	{
		foreach (IEntity entity : m_aTurretGunners)
		{
			if (!entity)
				continue;

			SCR_EntityHelper.DeleteEntityAndChildren(entity);
		}
		m_aTurretGunners.Clear();
		m_aTurrets.Clear();
	}

	private void GetCompositionEntities(IEntity composition)
	{
		array<IEntity> children = {};
		ESCT_EntityHelper.GetAllChildren(composition, children);

		foreach (IEntity entity : children)
		{
			if (!entity)
				continue;

			if (entity.Type() == Turret)
			{
				m_aTurrets.Insert(entity);
			}
		}
	}

	private void PopulateTurrets()
	{
		ESCT_Faction faction = m_Location.GetOwnerFaction();
		if (!faction)
			return;

		foreach (IEntity turret : m_aTurrets)
		{
			AIGroup group = ESCT_CharacterHelper.CreateGroup(faction.GetFactionKey(), faction.GetBaseFactionGroup());
			if (!group)
				continue;

			ResourceName characterPrefab = faction.GetRandomCharacterOfBranch(ESCT_EMilitaryBranchType.Military);
			if (!characterPrefab || characterPrefab == ResourceName.Empty)
				continue;

			ESCT_CharacterHelper.SpawnGunner(turret, faction, characterPrefab, group);
			SCR_AIGroup scrGroup = SCR_AIGroup.Cast(group);
			scrGroup.GetOnAgentAdded().Insert(RegisterGunner);
		}
	}

	private void RegisterGunner(AIAgent aiAgent)
	{
		if (!aiAgent)
			return;

		IEntity charEntity = aiAgent.GetControlledEntity();
		if (!charEntity)
			return;

		m_aTurretGunners.Insert(charEntity);
		m_Location.ChangeEntitiesToClear(1);

		ChimeraCharacter character = ChimeraCharacter.Cast(charEntity);
		if (!character)
			return;

		SCR_CharacterDamageManagerComponent damageMan = SCR_CharacterDamageManagerComponent.Cast(character.GetDamageManager());
		if (damageMan)
			damageMan.GetOnDamageStateChanged().Insert(OnTurretOccupantDeath);
	}

	private void OnTurretOccupantDeath(EDamageState state)
	{
		m_iActiveTurrets = m_iActiveTurrets - 1;
		if (m_iActiveTurrets <= 0)
		{
			m_bIsCleared = true;
		}
	}
}
