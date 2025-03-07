[EntityEditorProps("Escapists/Components/SupportStations", description: "Support station for enabling wounded person transfer to any vehicle in vicinity.", color: "0 0 255 255")]
class ESCT_LoadCasualtyVehicleSupportStationComponentClass : SCR_BaseSupportStationComponentClass
{
}

class ESCT_LoadCasualtyVehicleSupportStationComponent : SCR_BaseSupportStationComponent
{
	protected SCR_BaseCompartmentManagerComponent m_CompartmentManager;
	protected IEntity m_CompartmentManagerOwner; 
	
	//------------------------------------------------------------------------------------------------
	protected override void DelayedInit(IEntity owner)
	{
		if (!owner)
			return;
		
		super.DelayedInit(owner);
		
		m_CompartmentManager = SCR_BaseCompartmentManagerComponent.Cast(owner.FindComponent(SCR_BaseCompartmentManagerComponent));	
		if (!m_CompartmentManager)
		{
			IEntity parent = owner.GetParent();
			if (parent)
			{
				m_CompartmentManager = SCR_BaseCompartmentManagerComponent.Cast(parent.FindComponent(SCR_BaseCompartmentManagerComponent));
				if (m_CompartmentManager)
					m_CompartmentManagerOwner = parent;
			}			
		}
		else 
		{
			m_CompartmentManagerOwner = owner;
		}
		
		//~ Could not find compartment manager
		if (!m_CompartmentManager)
		{
			Print("'ESCT_LoadCasualtyVehicleSupportStationComponent': '" + GetOwner().GetName() + "' has no CompartmentManager!", LogLevel.ERROR);
			return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsValid(IEntity actionOwner, IEntity actionUser, SCR_BaseUseSupportStationAction action, vector actionPosition, out ESupportStationReasonInvalid reasonInvalid, out int supplyCost)
	{
		if (!super.IsValid(actionOwner, actionUser, action, actionPosition, reasonInvalid, supplyCost))
			return false;
		
		//~ Has no fuel to provide so not valid
		if (!m_CompartmentManager || !m_CompartmentManager.GetFirstFreeCompartmentOfType(CargoCompartmentSlot))
		{
			reasonInvalid = ESupportStationReasonInvalid.LOAD_CASUALTY_NO_SPACE;
			return false;
		}
	
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override ESupportStationType GetSupportStationType()
	{
		return ESupportStationType.LOAD_CASUALTY_VEHICLE;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnExecutedServer(notnull IEntity actionOwner, notnull IEntity actionUser, notnull SCR_BaseUseSupportStationAction action)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(actionOwner);
		if (!char)
			return;
		
		SCR_CompartmentAccessComponent accessComp = SCR_CompartmentAccessComponent.Cast(char.GetCompartmentAccessComponent());
		if (!accessComp)
			return;
		
		//~ Load casualty in vehicle
		if (m_CompartmentManager)
		{
			BaseCompartmentSlot compartmentSlot = m_CompartmentManager.GetFirstFreeCompartmentOfType(CargoCompartmentSlot);
			if (compartmentSlot)
			{
				//~ Successfully moved the character
				ChimeraWorld world = GetGame().GetWorld();
				if (accessComp.MoveInVehicle(m_CompartmentManagerOwner, ECompartmentType.CARGO, world.IsGameTimePaused(), compartmentSlot))
					super.OnExecutedServer(actionOwner, actionUser, action);
			}
		}
	}	
}
