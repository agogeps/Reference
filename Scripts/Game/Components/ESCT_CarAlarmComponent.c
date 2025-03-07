[EntityEditorProps("Escapists", description: "Component that handles car alarms.", color: "0 0 255 255")]
class ESCT_CarAlarmComponentClass : ScriptComponentClass
{
}

enum ESCT_CarAlarmState
{
	Disabled = 0,
	EnabledButHidden = 2,
	Revealed = 4,
	Active = 8
}

sealed class ESCT_CarAlarmComponent : ScriptComponent
{
	[Attribute("0", UIWidgets.Auto, desc: "Determines if car should has locked seats (for example, open vehicles should be able to be boarded even if car alarm is armed.", category: "Escapists")]
	protected bool m_bShouldLockSeats;

	[RplProp(onRplName: "OnCarAlarmStateChanged")]
	protected ESCT_CarAlarmState m_eCarAlarmState = ESCT_CarAlarmState.Disabled;

	[RplProp()]
	protected bool m_bIsLocked = false;

	protected IEntity m_Vehicle;
	protected BaseLightManagerComponent m_BaseLightManager;
	protected SCR_VehicleSoundComponent m_SoundComponent;
	protected SignalsManagerComponent m_SignalsManager;
	protected SCR_WheeledDamageManagerComponent m_DamageManager;

	private float m_fSupportCallChance = 0.35;
	private bool m_bIsEnemyReacted = false;

	private ref ScriptInvokerVoid m_OnAlarmTriggered;

	ScriptInvokerVoid GetOnAlarmTriggered()
	{
		if (!m_OnAlarmTriggered)
			m_OnAlarmTriggered = new ScriptInvokerVoid();

		return m_OnAlarmTriggered;
	}

	ESCT_CarAlarmState GetAlarmState()
	{
		return m_eCarAlarmState;
	}

	protected void SetAlarmState(ESCT_CarAlarmState newState)
	{
		m_eCarAlarmState = newState;
		Replication.BumpMe();
	}

	bool IsLocked()
	{
		return m_bIsLocked;
	}

	protected void LockCar(bool newValue)
	{
		m_bIsLocked = newValue;
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

		if (SCR_Global.IsEditMode()) return;

		m_Vehicle = owner;
		m_SoundComponent = SCR_VehicleSoundComponent.Cast(m_Vehicle.FindComponent(SCR_VehicleSoundComponent));
		m_BaseLightManager = BaseLightManagerComponent.Cast(m_Vehicle.FindComponent(BaseLightManagerComponent));
		m_SignalsManager = SignalsManagerComponent.Cast(m_Vehicle.FindComponent(SignalsManagerComponent));
		m_DamageManager = SCR_WheeledDamageManagerComponent.Cast(m_Vehicle.FindComponent(SCR_WheeledDamageManagerComponent));

		CarControllerComponent carController = CarControllerComponent.Cast(m_Vehicle.FindComponent(CarControllerComponent));
		if (carController)
			carController.GetOnEngineStart().Insert(OnEngineStarted);
	}

	void ArmAlarm()
	{
		SetAlarmState(ESCT_CarAlarmState.EnabledButHidden);

		if (m_DamageManager)
		{
			array<HitZone> hitZones = {};
			m_DamageManager.GetAllHitZones(hitZones);

			for (int i = 0; i < hitZones.Count(); i++)
			{
				SCR_VehicleHitZone vehHitZone = SCR_VehicleHitZone.Cast(hitZones[i]);
				if (vehHitZone && !vehHitZone.IsOnHitSubscribed())
				{
					//FIXME: it conflicts with other EHs as they can't subscribe as flag already set
					vehHitZone.SetSubscribe(true);
					vehHitZone.GetOnHit().Insert(OnHit);
				}
			}
		}

		//damage manager seems to be global (all cars with this component will be affected), so we should use this
		EventHandlerManagerComponent ev = EventHandlerManagerComponent.Cast(m_Vehicle.FindComponent(EventHandlerManagerComponent));
		if (ev)
			ev.RegisterScriptHandler("OnDestroyed", this, OnDestroyed, false);

		if (m_bShouldLockSeats)
			LockCar(true);
	}

	void EnableAlarm()
	{
		if (m_bIsLocked && m_SoundComponent)
			m_SoundComponent.SoundEvent("SOUND_CAR_HANDLE");

		Rpc(Rpc_DoEnableAlarm);
	}

	void DisableAlarm()
	{
		Rpc(Rpc_DoDisableAlarm);
	}

	void UnlockVehicle()
	{
		if (m_SoundComponent)
			m_SoundComponent.SoundEvent("SOUND_CAR_UNLOCK");

		LockCar(false);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	private void Rpc_DoEnableAlarm()
	{
		if (!m_Vehicle || m_eCarAlarmState == ESCT_CarAlarmState.Active || m_eCarAlarmState == ESCT_CarAlarmState.Disabled)
			return;

		SetAlarmState(ESCT_CarAlarmState.Active);

		GetGame().GetCallqueue().CallLater(SetAlarmPresentation, 600, param1: true);
		GetGame().GetCallqueue().CallLater(MakeAlarmDormant, Math.RandomInt(15000, 30000), param1: false);

		GetOnAlarmTriggered().Invoke();

		TryCallForSupport();
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	private void Rpc_DoDisableAlarm()
	{
		if (!m_Vehicle || m_eCarAlarmState == ESCT_CarAlarmState.Disabled)
			return;

		SetAlarmState(ESCT_CarAlarmState.Disabled);

		GetGame().GetCallqueue().CallLater(SetAlarmPresentation, 600, param1: false);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	private void Rpc_DoSetLightLoop(bool value)
	{
		SetLights(value);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	private void Rpc_DoSetHazardLights(bool value)
	{
		if (!m_BaseLightManager)
			return;
		
		m_BaseLightManager.SetLightsState(ELightType.Hazard, value);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	private void Rpc_DoSetAudio(bool value)
	{
		if (!m_SignalsManager)
			return;

		m_SignalsManager.SetSignalValue(m_SignalsManager.AddOrFindSignal("Trigger"), value);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	private void Rpc_DoDisableLights()
	{
		if (!m_BaseLightManager)
			return;

		m_BaseLightManager.SetLightsState(ELightType.Head, false);
		m_BaseLightManager.SetLightsState(ELightType.Brake, false);
		m_BaseLightManager.SetLightsState(ELightType.Hazard, false);
	}

	private void TryCallForSupport()
	{
		if (m_bIsEnemyReacted || !m_Vehicle)
			return;

		//"enemy is lazy" simulation so it wouldn't proc every time
		if (Math.RandomFloat(0, 1) < m_fSupportCallChance)
		{
			if (m_fSupportCallChance > 0)
				m_fSupportCallChance -= 0.05;

			return;
		}

		ESCT_LocationSystem locationSystem = ESCT_LocationSystem.GetSystem();
		if (!locationSystem)
			return;

		array<ESCT_Location> locations = {};
		locationSystem.GetLocations(locations);

		bool isSomeLocationNear = false;
		for (int i = 0; i < locations.Count(); i++)
		{
			if (ESCT_Math.IsPointInRadiusXZ(locations[i].GetOrigin(), m_Vehicle.GetOrigin(), 1000))
			{
				isSomeLocationNear = true;
				break;
			}
		}

		if (!isSomeLocationNear)
			return;

		ESCT_SupportSystem supportSystem = ESCT_SupportSystem.GetSystem();
		if (!supportSystem)
		{
			ESCT_Logger.Error("Can't call for support on car alarm - Support System wasn't found! Check if call was made by the server!");
			return;
		}
		
		supportSystem.CreateSupport(m_Vehicle, m_Vehicle.GetOrigin(), desiredSupportType: ESCT_ESupportType.QRF, followTarget: true, isSilent: true);
		m_bIsEnemyReacted = true;

		ESCT_Logger.InfoFormat("Something triggered an %1 car alarm at %2 position, executed support call.", m_Vehicle.ToString(), m_Vehicle.GetOrigin().ToString());
	}

	private void SetAlarmPresentation(bool state)
	{
		if (!m_Vehicle)
			return;

		if (state == true)
		{
			Rpc_DoSetAudio(true);
			Rpc(Rpc_DoSetAudio, true);

			Rpc_DoSetLightLoop(true);
			Rpc(Rpc_DoSetLightLoop, true);
			
			Rpc_DoSetHazardLights(true);
			Rpc(Rpc_DoSetHazardLights, true);
		}
		else
		{
			Rpc_DoSetAudio(false);
			Rpc(Rpc_DoSetAudio, false);

			Rpc_DoSetLightLoop(false);
			Rpc(Rpc_DoSetLightLoop, false);
			
			Rpc_DoSetHazardLights(false);
			Rpc(Rpc_DoSetHazardLights, false);
		}
	}

	private void OnCarAlarmStateChanged()
	{
		if (m_eCarAlarmState != ESCT_CarAlarmState.Active)
			return;

		Rpc_DoSetAudio(true);
		Rpc_DoSetLightLoop(true);
	}

	private void SetLights(bool value)
	{
		if (!m_BaseLightManager || m_eCarAlarmState != ESCT_CarAlarmState.Active)
			return;

		m_BaseLightManager.SetLightsState(ELightType.Head, value);
		m_BaseLightManager.SetLightsState(ELightType.Brake, value);

		GetGame().GetCallqueue().CallLater(SetLights, 500, false, !value);
	}

	private void OnDestroyed(IEntity ent)
	{
		if (m_eCarAlarmState == ESCT_CarAlarmState.Disabled)
			return;

		if (m_eCarAlarmState == ESCT_CarAlarmState.Active)
			SetAlarmPresentation(false);

		SetAlarmState(ESCT_CarAlarmState.Disabled);
	}

	private void OnHit(IEntity vehicle, BaseDamageContext damageContext)
	{
		if (!m_Vehicle || m_eCarAlarmState == ESCT_CarAlarmState.Active)
			return;

		if (m_DamageManager.IsDestroyed() || m_eCarAlarmState == ESCT_CarAlarmState.Disabled)
		{
			array<HitZone> hitZones = {};
			m_DamageManager.GetAllHitZones(hitZones);

			for (int i = 0; i < hitZones.Count(); i++)
			{
				SCR_VehicleHitZone vehHitZone = SCR_VehicleHitZone.Cast(hitZones[i]);
				if (!vehHitZone)
					continue;

				vehHitZone.GetOnHit().Remove(OnHit);
			}
			return;
		}

		Rpc(Rpc_DoEnableAlarm);
	}

	private void MakeAlarmDormant()
	{
		if (!m_Vehicle || m_eCarAlarmState == ESCT_CarAlarmState.Disabled || m_eCarAlarmState == ESCT_CarAlarmState.Revealed)
			return;

		SetAlarmState(ESCT_CarAlarmState.Revealed);
		SetAlarmPresentation(false);
		
		GetGame().GetCallqueue().CallLater(DisableAlarmPresentation, 510, false, param1: false);
	}
	
	private void DisableAlarmPresentation()
	{	
		Rpc_DoDisableLights();
		Rpc(Rpc_DoDisableLights);
	}
	
	private void OnEngineStarted()
	{
		if (!m_Vehicle)
			return;
		
		GetGame().GetCallqueue().CallLater(DisableAlarm, Math.RandomInt(8000, 16000), false);
		
		CarControllerComponent carController = CarControllerComponent.Cast(m_Vehicle.FindComponent(CarControllerComponent));
		if (carController)
			carController.GetOnEngineStart().Remove(OnEngineStarted);
	}
}
