[EntityEditorProps(category: "Escapists/Vehicles", description: "Moves characters out of vehicle on explosions.", color: "0 0 255 255")]
class ESCT_VehicleForceEjectComponentClass : ScriptComponentClass
{
}

class ESCT_VehicleForceEjectComponent : ScriptComponent
{
	protected IEntity m_Vehicle;
	protected SCR_BaseCompartmentManagerComponent m_CompartmentManager;
	protected SCR_VehicleDamageManagerComponent m_VehicleDamageManager;

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (SCR_Global.IsEditMode()) return;

		SetEventMask(owner, EntityEvent.INIT);
	}

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (SCR_Global.IsEditMode()) return;

		m_Vehicle = owner;

		m_CompartmentManager = SCR_BaseCompartmentManagerComponent.Cast(m_Vehicle.FindComponent(SCR_BaseCompartmentManagerComponent));
		if (!m_CompartmentManager)
			return;

		m_VehicleDamageManager = SCR_VehicleDamageManagerComponent.Cast(m_Vehicle.FindComponent(SCR_VehicleDamageManagerComponent));
		if (!m_VehicleDamageManager)
			return;

		m_VehicleDamageManager.GetOnDamageStateChanged().Insert(OnVehicleDestroyed);
	}

	protected void OnVehicleDestroyed(EDamageState state)
	{
		if (state != EDamageState.DESTROYED || !m_CompartmentManager || !m_Vehicle)
			return;

		array<BaseCompartmentSlot> compartments = {};
		m_CompartmentManager.GetCompartments(compartments);

		foreach (BaseCompartmentSlot compartment : compartments)
		{
			Eject(compartment);
		}
	}

	private void Eject(notnull BaseCompartmentSlot compartment)
	{
		//copy of BaseCompartmentSlot.EjectOccupant with some small changes
		ChimeraCharacter character = ChimeraCharacter.Cast(compartment.GetOccupant());
		if (!character)
			return;

		// don't know why but it breaks animations and character teleports under the vehicle
//		RplComponent rpl = character.GetRplComponent();
//		if (rpl && rpl.IsProxy())
//			return;
//
		// Ignore characters that only began to get in the vehicle
		CompartmentAccessComponent access = character.GetCompartmentAccessComponent();
		if (!access)
			return;

		access.GetOutVehicle(EGetOutType.TELEPORT, -1, ECloseDoorAfterActions.INVALID, false);

		SCR_CharacterDamageManagerComponent damageMan = SCR_CharacterDamageManagerComponent.Cast(character.GetDamageManager());
		if (damageMan)
			damageMan.ForceUnconsciousness();
	}
}
