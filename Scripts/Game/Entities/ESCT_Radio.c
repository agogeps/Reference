[EntityEditorProps(category: "Escapists/Entities", description: "Handles Escapists radio to call for evacuation.")]
class ESCT_RadioClass : GenericEntityClass
{
}

enum ESCT_RadioState 
{
	Idle,
	NoFrequency,
	FrequencySet,
	WaitingForResponse,
	ReadyForResponse,
	Done
}

void ScriptInvokerRadioMethod(ESCT_Radio radio);
typedef func ScriptInvokerRadioMethod;
typedef ScriptInvokerBase<ScriptInvokerRadioMethod> ScriptInvokerRadio;

sealed class ESCT_Radio : GenericEntity
{
	[RplProp()]
	protected ESCT_RadioState m_eRadioState;
	
	protected bool m_bIsSubscribedToTask = false;

	private IEntity m_Owner;
	private ref ScriptInvokerRadio m_OnSignalSent;
	private SoundComponent m_SoundComponent;
	private SignalsManagerComponent m_SignalsManagerComponent;
	
	ESCT_RadioState GetRadioState()
	{
		return m_eRadioState;
	}
	
	void SetRadioState(ESCT_RadioState newState)
	{
		m_eRadioState = newState;
		Replication.BumpMe();
	}
	
	ScriptInvokerRadio GetOnSignalSent()
	{
		if (!m_OnSignalSent)
		{
			m_OnSignalSent = new ScriptInvokerRadio();
		}

		return m_OnSignalSent;
	}
	
	bool GetIsSubscribedToTask()
	{
		return m_bIsSubscribedToTask;
	}
	
	void SetIsSubscribedToTask(bool value)
	{
		m_bIsSubscribedToTask = value;
	}
	
	void ESCT_Radio(IEntitySource src, IEntity parent)
	{
		if (!GetGame().InPlayMode())
			return;
		
		SetEventMask(EntityEvent.INIT);
	}

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		if (!GetGame().InPlayMode())
			return;
		
		m_Owner = owner;
		m_SoundComponent = SoundComponent.Cast(m_Owner.FindComponent(SoundComponent));
		m_SignalsManagerComponent = SignalsManagerComponent.Cast(m_Owner.FindComponent(SignalsManagerComponent));

		ESCT_GameStateManagerComponent gameState = ESCT_GameStateManagerComponent.GetInstance();
		if (gameState && gameState.GetGameState() > ESCT_EGameState.RadioStationSearch)
			return;
		
		GetGame().GetCallqueue().CallLater(SetRadioSound, 100, false, 1, "Trigger");
	}
	
	void SendDistressSignal()
	{
		if (m_SoundComponent)
			m_SoundComponent.SoundEvent("RADIO_SOUND_TURN_OFF");
			
		GetGame().GetCallqueue().CallLater(ReceiveFirstMessage, 100);
		
		SetRadioState(ESCT_RadioState.WaitingForResponse);
	}
	
	void ReceiveHqResponse()
	{	
		SetRadioState(ESCT_RadioState.Done);
		
		if (m_SoundComponent)
			m_SoundComponent.SoundEvent("RADIO_SOUND_TURN_OFF");
			
		GetGame().GetCallqueue().CallLater(ReceiveSecondMessage, 100);
	}
	
	void NoFrequency()
	{
		SetRadioState(ESCT_RadioState.NoFrequency);
		SetRadioSound(0, "Trigger");
		
		if (m_SignalsManagerComponent)
			m_SignalsManagerComponent.SetSignalValue(m_SignalsManagerComponent.AddOrFindSignal("StaticNoise"), 1);
	}
	
	void TuneFrequency()
	{
		SetRadioState(ESCT_RadioState.FrequencySet);
		
		if (m_SoundComponent)
			m_SoundComponent.SoundEvent("RADIO_SOUND_TURN_ON");
	}
	
	private void SetRadioSound(int value, string signal)
	{
		if (!m_Owner)
			return;
		
		if (m_SignalsManagerComponent)
			m_SignalsManagerComponent.SetSignalValue(m_SignalsManagerComponent.AddOrFindSignal(signal), value);
	}	
	
	private void ReceiveFirstMessage()
	{	
		ESCT_Faction faction = ESCT_FactionManager.GetInstance().GetPlayerFaction();
		if (m_SoundComponent && faction)
		{
			ESCT_PlayerFactionInfo info = faction.GetPlayerFactionInfo();
			if (info)
			{
				GetGame().GetCallqueue().CallLater(m_SoundComponent.SoundEvent, Math.RandomInt(1500, 2500), false, info.GetFirstRadioMessage());
			}
		}
		
		GetGame().GetCallqueue().CallLater(FirstMessage, 21000, false);
	}
	
		
	private void FirstMessage()
	{
		if (!m_OnSignalSent)
			return;
		
		//fire event to create defend radio task
		m_OnSignalSent.Invoke(this);
		
		ESCT_GameStateManagerComponent.GetInstance().SetGameState(ESCT_EGameState.RadioStationDefend);
	}
	
	private void ReceiveSecondMessage()
	{	
		ESCT_Faction faction = ESCT_FactionManager.GetInstance().GetPlayerFaction();
		if (m_SoundComponent && faction)
		{
			ESCT_PlayerFactionInfo info = faction.GetPlayerFactionInfo();
			if (info)
			{
				GetGame().GetCallqueue().CallLater(m_SoundComponent.SoundEvent, Math.RandomInt(800, 1400), false, info.GetSecondRadioMessage());
			}
		}
		
		GetGame().GetCallqueue().CallLater(SecondMessage, 10000, false);
	}
	
	private void SecondMessage()
	{
		ESCT_GameStateManagerComponent.GetInstance().SetGameState(ESCT_EGameState.Transport);
	}
}
