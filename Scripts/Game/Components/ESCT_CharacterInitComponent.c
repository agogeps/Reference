[EntityEditorProps("Escapists", description: "Component that inits characters to be suitable for Escapists gameplay needs.", color: "0 0 255 255")]
class ESCT_CharacterInitComponentClass : ScriptComponentClass
{
}

class ESCT_CharacterInitComponent : ScriptComponent
{
	protected static ref array<string> NAVIGATION_PREFABS =
	{
		"{13772C903CB5E4F7}Prefabs/Items/Equipment/Maps/PaperMap_01_folded.et"
	};

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (SCR_Global.IsEditMode() || !Replication.IsServer()) return;

		SetEventMask(owner, EntityEvent.INIT);
	}

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (SCR_Global.IsEditMode() || !Replication.IsServer()) return;

		//inventory has not all initial items in spawn frame and m_eSkill is null for some reason
		GetGame().GetCallqueue().Call(LateApply);
	}

	protected void LateApply()
	{
		ESCT_GameStateManagerComponent manager = ESCT_GameStateManagerComponent.GetInstance();
		if (!manager)
			return;

		ESCT_EGameState state = manager.GetGameState();

		float mapChance = ESCT_EscapistsConfigComponent.GetInstance().GetMapItemChance();
		IEntity character = GetOwner();
		if (!character)
			return;

		SetSkill(character);
		RemoveMapItem(character, mapChance);

		if (state <= ESCT_EGameState.Start)
		{
			AIControlComponent ctrl = AIControlComponent.Cast(GetOwner().FindComponent(AIControlComponent));
			if (ctrl)
			{
				SCR_ChimeraAIAgent agent = SCR_ChimeraAIAgent.Cast(ctrl.GetAIAgent());
				if (agent)
					agent.DeactivateAI();
			}

			ESCT_GameStateManagerComponent.GetInstance().GetOnGameStateChanged().Insert(HandleGameStateChanged);
		}
	}

	protected void SetSkill(IEntity character)
	{
		SCR_AICombatComponent combatComponent = SCR_AICombatComponent.Cast(character.FindComponent(SCR_AICombatComponent));
		if (!combatComponent)
			return;

		EAISkill skill = ESCT_EscapistsConfigComponent.GetInstance().GetAiSkill();

		combatComponent.SetAISkill(skill);
	}

	protected void RemoveMapItem(IEntity character, float apperanceChance)
	{
		if (Math.RandomFloat01() < apperanceChance)
			return;

		SCR_InventoryStorageManagerComponent invManager = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!invManager)
			return;

		if (NAVIGATION_PREFABS.Count() == 1)
		{
			FindAndDeleteNavigationItem(invManager, NAVIGATION_PREFABS[0]);
			return;
		}

		foreach (string navPrefab : NAVIGATION_PREFABS)
		{
			FindAndDeleteNavigationItem(invManager, navPrefab);
		}
	}

	private static void FindAndDeleteNavigationItem(SCR_InventoryStorageManagerComponent invManager, string navPrefab)
	{
		SCR_PrefabNamePredicate predicate = new SCR_PrefabNamePredicate();
		predicate.prefabName = navPrefab;

		IEntity mapItem = invManager.FindItem(predicate);
		if (!mapItem)
			return;

		invManager.TryDeleteItem(mapItem);
	}

	private void HandleGameStateChanged(ESCT_EGameState gameState)
	{
		if (gameState != ESCT_EGameState.RadioStationSearch)
			return;

		AIControlComponent ctrl = AIControlComponent.Cast(GetOwner().FindComponent(AIControlComponent));
		if (ctrl)
		{
			SCR_ChimeraAIAgent agent = SCR_ChimeraAIAgent.Cast(ctrl.GetAIAgent());
			if (agent)
				agent.ActivateAI();
		}

		ESCT_GameStateManagerComponent.GetInstance().GetOnGameStateChanged().Remove(HandleGameStateChanged);
	}
}
