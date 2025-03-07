[EntityEditorProps(category: "Escapists/PlayerController", description: "Handles player's self-revive capabilities.", color: "0 0 255 255")]
class ESCT_SelfReviveComponentClass : ScriptComponentClass
{
}

class ESCT_SelfReviveComponent : ScriptComponent
{
	protected ref array<IEntity> m_aEpinephrinesLocal;

	protected SCR_ChimeraCharacter m_PlayerCharacter;
	protected SCR_CharacterControllerComponent m_CharacterController;
	protected ESCT_UnconsciousInfoDisplay m_UnconDisplay;
	protected SCR_InventoryStorageManagerComponent m_InventoryStorageManager;

	private const string REVIVE_ACTION = "EscapistsWithstand";

	//------------------------------------------------------------------------------------------------
	//! \return
	static ESCT_SelfReviveComponent GetLocalInstance()
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return null;

		return ESCT_SelfReviveComponent.Cast(pc.FindComponent(ESCT_SelfReviveComponent));
	}

	void TryAddSelfRevive()
	{
		if (!ESCT_EscapistsConfigComponent.GetInstance().IsWithstandEnabled())
			return;

		SCR_PlayerController controller = SCR_PlayerController.Cast(GetOwner());
		if (!controller)
			return;

		IEntity playerEntity = controller.GetControlledEntity();
		if (!playerEntity)
		{
			ESCT_Logger.ErrorFormat(
				"[ESCT_SelfReviveComponent][TryAddSelfRevive] Failed to get controlled entity on %1 machine. Self-revive behaviour will be not attached.",
				controller.GetPlayerId().ToString()
			);
			return;
		}

		AddSelfReviveBehaviour(playerEntity);
	}

	protected void AddSelfReviveBehaviour(IEntity playerEntity)
	{
		m_PlayerCharacter = SCR_ChimeraCharacter.Cast(playerEntity);
		if (!m_PlayerCharacter)
			return;

		m_UnconDisplay = AssignUnconsciousInfoDisplay();
		if (!m_UnconDisplay)
		{
			ESCT_Logger.Error(
				"[ESCT_SelfReviveComponent][OnPossessed] Failed to find ESCT_UnconsciousInfoDisplay! Check HUD manager component in player controller prefab!");
			return;
		}

		m_aEpinephrinesLocal = {};

		m_CharacterController = SCR_CharacterControllerComponent.Cast(m_PlayerCharacter.FindComponent(SCR_CharacterControllerComponent));
		if (m_CharacterController)
			m_CharacterController.m_OnLifeStateChanged.Insert(OnLifeStateChanged);

		m_InventoryStorageManager = SCR_InventoryStorageManagerComponent.Cast(m_PlayerCharacter.FindComponent(SCR_InventoryStorageManagerComponent));
	}

	protected void OnLifeStateChanged(ECharacterLifeState previousLifeState, ECharacterLifeState newLifeState)
	{
		if (newLifeState == ECharacterLifeState.INCAPACITATED)
		{
			//probably not needed, but there might be cases when UI may show up for conscious but wounded characters
			if (!m_CharacterController.IsUnconscious())
				return;

			GetCharacterReviveItems(m_InventoryStorageManager, m_aEpinephrinesLocal);

			m_InventoryStorageManager.m_OnItemAddedInvoker.Insert(OnItemAdded);
			m_InventoryStorageManager.m_OnItemRemovedInvoker.Insert(OnItemRemoved);

			if (!m_aEpinephrinesLocal.IsEmpty())
			{
				m_UnconDisplay.Show(true, 0.5, EAnimationCurve.EASE_IN_CIRC);
				GetGame().GetInputManager().AddActionListener(REVIVE_ACTION, EActionTrigger.DOWN, ReviveSelf);
			}
		}
		else
		{
			m_InventoryStorageManager.m_OnItemAddedInvoker.Remove(OnItemAdded);
			m_InventoryStorageManager.m_OnItemRemovedInvoker.Remove(OnItemRemoved);
			
			m_UnconDisplay.Show(false);
			GetGame().GetInputManager().RemoveActionListener(REVIVE_ACTION, EActionTrigger.DOWN, ReviveSelf);
		}
	}

	protected void ReviveSelf()
	{
		if (!m_PlayerCharacter || m_aEpinephrinesLocal.IsEmpty() || !m_InventoryStorageManager)
			return;

		SCR_CharacterControllerComponent charController = SCR_CharacterControllerComponent.Cast(m_PlayerCharacter.FindComponent(SCR_CharacterControllerComponent));
		if (!charController)
			return;

		//can't heal already dead or already conscious character
		ECharacterLifeState lifeState = charController.GetLifeState();
		if (lifeState != ECharacterLifeState.INCAPACITATED)
			return;

		SCR_PlayerController controller = SCR_PlayerController.Cast(GetOwner());
		if (!controller)
			return;

		int pid = controller.GetPlayerId();

		if (RplSession.Mode() == RplMode.Client)
		{
			//seems that heal should be serverside as there are some bugs on client otherwise
			Rpc(Rpc_Revive_SA, pid);
		}
		else
		{
			Rpc_Revive_SA(pid);
		}

		//cleanup
		m_InventoryStorageManager.m_OnItemAddedInvoker.Remove(OnItemAdded);
		m_InventoryStorageManager.m_OnItemRemovedInvoker.Remove(OnItemRemoved);
		m_aEpinephrinesLocal.Clear();
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void Rpc_Revive_SA(int pid)
	{
		IEntity serverPlayerEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(pid);
		if (!serverPlayerEntity)
			return;

		SCR_ChimeraCharacter serverPlayerCharacter = SCR_ChimeraCharacter.Cast(serverPlayerEntity);
		if (!serverPlayerCharacter)
			return;

		SCR_CharacterControllerComponent charController = SCR_CharacterControllerComponent.Cast(serverPlayerCharacter.FindComponent(SCR_CharacterControllerComponent));
		if (!charController)
			return;

		//can't heal already dead or already conscious character
		ECharacterLifeState lifeState = charController.GetLifeState();
		if (lifeState != ECharacterLifeState.INCAPACITATED)
			return;

		SCR_InventoryStorageManagerComponent inventoryStorage =
			SCR_InventoryStorageManagerComponent.Cast(serverPlayerCharacter.FindComponent(SCR_InventoryStorageManagerComponent));
		array<IEntity> epinephrines = {};
		GetCharacterReviveItems(inventoryStorage, epinephrines);

		if (epinephrines.IsEmpty())
			return;

		IEntity epi = epinephrines.GetRandomElement();
		if (!epi)
			return;

		SCR_ConsumableItemComponent consumable = SCR_ConsumableItemComponent.Cast(epi.FindComponent(SCR_ConsumableItemComponent));
		if (!consumable)
			return;

		consumable.ApplyItemEffect(serverPlayerCharacter, serverPlayerCharacter, new ItemUseParameters(), epi, deleteItem: true);
	}

	private array<IEntity> GetCharacterReviveItems(notnull SCR_InventoryStorageManagerComponent inventoryStorage, notnull out array<IEntity> epinephrines)
	{
		array<IEntity> items = {};
		inventoryStorage.GetItems(items);

		IEntity item = null;
		for (int i = 0; i < items.Count(); i++)
		{
			item = items[i];
			if (!item)
				continue;

			InventoryItemComponent inventoryItem = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
			if (!inventoryItem)
				continue;

			if (inventoryItem.GetAttributes().GetCommonType() == ECommonItemType.EPINEPHRINE)
			{
				epinephrines.Insert(item);
			}
		}

		return epinephrines;
	}

	private ESCT_UnconsciousInfoDisplay AssignUnconsciousInfoDisplay()
	{
		HUDManagerComponent hudManager = GetGame().GetHUDManager();
		array<BaseInfoDisplay> infoDisplays = {};
		hudManager.GetInfoDisplays(infoDisplays);

		ESCT_UnconsciousInfoDisplay displayResult = null;

		foreach (BaseInfoDisplay display : infoDisplays)
		{
			if (display.Type() == ESCT_UnconsciousInfoDisplay)
			{
				displayResult = ESCT_UnconsciousInfoDisplay.Cast(display);
				break;
			}
		}

		return displayResult;
	}

	private void OnItemAdded(IEntity item, BaseInventoryStorageComponent storageOwner)
	{
		InventoryItemComponent inventoryItem = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!inventoryItem)
			return;

		if (inventoryItem.GetAttributes().GetCommonType() == ECommonItemType.EPINEPHRINE)
		{
			m_aEpinephrinesLocal.Insert(item);
		}

		if (!m_aEpinephrinesLocal.IsEmpty())
		{
			ESCT_PlayerDamageManagerComponent damageManager = ESCT_PlayerDamageManagerComponent.Cast(m_PlayerCharacter.GetDamageManager());
			if (damageManager)
				damageManager.UpdateLoseConditions();

			if (!m_UnconDisplay.IsShown())
			{
				m_UnconDisplay.Show(true, 0.5, EAnimationCurve.EASE_IN_CIRC);
				GetGame().GetInputManager().AddActionListener(REVIVE_ACTION, EActionTrigger.DOWN, ReviveSelf);
			}
		}
	}

	private void OnItemRemoved(IEntity item, BaseInventoryStorageComponent storageOwner)
	{
		InventoryItemComponent inventoryItem = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!inventoryItem)
			return;

		if (inventoryItem.GetAttributes().GetCommonType() == ECommonItemType.EPINEPHRINE)
		{
			m_aEpinephrinesLocal.RemoveItem(item);
		}

		if (m_aEpinephrinesLocal.IsEmpty())
		{
			ESCT_PlayerDamageManagerComponent damageManager = ESCT_PlayerDamageManagerComponent.Cast(m_PlayerCharacter.GetDamageManager());
			if (damageManager)
				damageManager.UpdateLoseConditions();

			if (m_UnconDisplay.IsShown())
			{
				m_UnconDisplay.Show(false);
				GetGame().GetInputManager().RemoveActionListener(REVIVE_ACTION, EActionTrigger.DOWN, ReviveSelf);
			}
		}
	}
}
