[EntityEditorProps("Escapists", description: "Handles helicrash helicopter state and other related things.", color: "0 0 255 255")]
class ESCT_HelicrashHelicopterComponentClass : ScriptComponentClass
{
}

sealed class ESCT_HelicrashHelicopterComponent : ScriptComponent
{
	[RplProp(onRplName: "OnWarningStateChanged")]
	protected bool m_bIsEnabled = true;

	protected IEntity m_Heli;
	protected SignalsManagerComponent m_SignalsManagerComponent;
	protected BaseLightManagerComponent m_BaseLightManager;
	protected SCR_BaseCompartmentManagerComponent m_VehicleCompartmentManager;

	private const string TRIGGER = "Trigger";

	bool IsEnabled()
	{
		return m_bIsEnabled;
	}

	void SetIsEnabled(bool value)
	{
		m_bIsEnabled = value;
		Replication.BumpMe();
	}

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (SCR_Global.IsEditMode()) return;

		SetEventMask(owner, EntityEvent.INIT);
	}

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (!GetGame().InPlayMode())
			return;

		m_Heli = owner;
		m_SignalsManagerComponent = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));
		m_BaseLightManager = BaseLightManagerComponent.Cast(owner.FindComponent(BaseLightManagerComponent));
		m_VehicleCompartmentManager = SCR_BaseCompartmentManagerComponent.Cast(owner.FindComponent(SCR_BaseCompartmentManagerComponent));

		//Mi-8 needs to be kicked into the ground to not hang in the air
		Physics physics = owner.GetPhysics();
		if (physics)
		{
			physics.ApplyImpulse(vector.Up * physics.GetMass() * 0.00001);
		}
		
		vector transform[4];
		owner.GetTransform(transform);
		SCR_TerrainHelper.SnapAndOrientToTerrain(transform, owner.GetWorld());
		owner.SetTransform(transform);
		
		GetGame().GetCallqueue().CallLater(SetHelicrashPresentation, 1200, false, 1);
	}

	void DisableWarningSound()
	{
		if (!m_SignalsManagerComponent || !m_Heli)
			return;

		m_SignalsManagerComponent.SetSignalValue(m_SignalsManagerComponent.AddOrFindSignal(TRIGGER), 0);
	}

	private void SetHelicrashPresentation(int value)
	{
		if (!m_Heli)
			return;
		
		SCR_PhysicsHelper.ChangeSimulationState(m_Heli, SimulationState.COLLISION, false);
		
		if (m_SignalsManagerComponent)
			m_SignalsManagerComponent.SetSignalValue(m_SignalsManagerComponent.AddOrFindSignal(TRIGGER), value);

		if (!m_BaseLightManager)
			return;
		
		TimeAndWeatherManagerEntity timeAndWeatherManager;
		ChimeraWorld world = ChimeraWorld.CastFrom(m_Heli.GetWorld());
		if (world)
			timeAndWeatherManager = world.GetTimeAndWeatherManager();
			
		if (!timeAndWeatherManager || !timeAndWeatherManager.IsSunSet())
			return;

		m_BaseLightManager.SetLightsState(ELightType.Hazard, true);
		m_BaseLightManager.SetLightsState(ELightType.Dashboard, true);
		
		//TODO: open cargo doors for heli	
	}

	private void OnWarningStateChanged()
	{
		if (m_bIsEnabled)
			return;

		DisableWarningSound();
	}
}
