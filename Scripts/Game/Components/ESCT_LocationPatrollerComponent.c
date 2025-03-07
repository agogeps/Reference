[EntityEditorProps("Escapists/Server/Components", description: "Component that helps in handling patrol persistence mechanics per-individual AI units.", color: "0 0 255 255")]
class ESCT_LocationPatrollerComponentClass : ScriptComponentClass
{
}

sealed class ESCT_LocationPatrollerComponent : ScriptComponent
{
	private ChimeraCharacter m_Character;
	private ESCT_LocationPatrolManagerComponent m_LocationPatrolManager;
	private SCR_AIGroup m_Group;

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (SCR_Global.IsEditMode() || !Replication.IsServer() || EntityUtils.IsPlayer(owner)) return;

		SetEventMask(owner, EntityEvent.INIT);
	}

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (SCR_Global.IsEditMode() || !Replication.IsServer() || EntityUtils.IsPlayer(owner)) return;

		m_Character = ChimeraCharacter.Cast(owner);
	}


	void Register(notnull ESCT_LocationPatrolManagerComponent patrolManager, notnull SCR_AIGroup group)
	{
		m_LocationPatrolManager = patrolManager;
		m_Group = group;
		
		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(m_Character.GetDamageManager());
		if (damageManager)
			damageManager.GetOnDamageStateChanged().Insert(OnDeath);
	}

	private	void OnDeath(EDamageState state)
	{
		if (state != EDamageState.DESTROYED || !m_Character || !m_LocationPatrolManager)
			return;

//		SCR_CharacterControllerComponent charController = SCR_CharacterControllerComponent.Cast(playerEntity.FindComponent(SCR_CharacterControllerComponent));
//			if (!charController)
//				continue;
//
//		ECharacterLifeState lifeState = charController.GetLifeState();

		array<AIAgent> agents = {};
		m_Group.GetAgents(agents);

		if (agents.Count() > m_LocationPatrolManager.GetClearThreshold())
			return;
		
		//clear event subscribers since location is already marked as cleared
		foreach (AIAgent agent : agents)
		{
			IEntity agentEntity = agent.GetControlledEntity();
			if (!agentEntity)
				continue;
			
			SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(agentEntity.FindComponent(SCR_CharacterDamageManagerComponent));
			if (damageManager)
				damageManager.GetOnDamageStateChanged().Remove(OnDeath);
		}
		
		m_LocationPatrolManager.ClearGroup(m_Group);
	}
}
