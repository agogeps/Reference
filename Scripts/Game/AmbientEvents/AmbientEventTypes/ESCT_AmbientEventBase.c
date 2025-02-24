[EntityEditorProps(category: "Escapists/Entities", description: "Base entity for ambient event.", visible: false)]
class ESCT_AmbientEventBaseClass : GenericEntityClass
{
}

[BaseContainerProps(), BaseContainerCustomStringTitleField("DO NOT USE BASE CLASS!")]
class ESCT_AmbientEventBase : GenericEntity
{
	[Attribute("0", desc: "Support type.", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ESCT_EAmbientEventType))]
	protected ESCT_EAmbientEventType m_eAmbientEventType;
	
	[Attribute(defvalue: "600", UIWidgets.Slider, desc: "Distance to despawn support.", params: "0 4000 1")]
	protected float m_fDespawnDistance;

	[Attribute(defvalue: "1200", UIWidgets.Slider, desc: "Time before despawn (if other despawn conditions weren't fullfilled yet).", params: "0 10000 1")]
	protected float m_fDespawnTimeout;

	protected WorldTimestamp m_fLaunchTimestamp;
	protected WorldTimestamp m_fDespawnTimestamp;
	
	private static const float GRACE_PERIOD_SECONDS = 10;
	
	protected ref ScriptInvokerVoid m_OnAmbientEventLaunched;
	protected ref ScriptInvokerVoid m_OnAmbientEventDespawned; 

	ScriptInvokerVoid GetOnAmbientEventLaunched()
	{
		if (!m_OnAmbientEventLaunched)
		{
			m_OnAmbientEventLaunched = new ScriptInvokerVoid();
		}

		return m_OnAmbientEventLaunched;
	}
	
	ScriptInvokerVoid GetOnAmbientEventDespawned()
	{
		if (!m_OnAmbientEventDespawned)
		{
			m_OnAmbientEventDespawned = new ScriptInvokerVoid();
		}

		return m_OnAmbientEventDespawned;
	}
	
	ESCT_EAmbientEventType GetAmbientEventType()
	{
		return m_eAmbientEventType;
	}
	
	bool HasGracePeriod()
	{
		//hasn't assigned yet, so pass
		if (!m_fLaunchTimestamp)
			return true;
		
		ChimeraWorld world = GetWorld();
		
		return world.GetServerTimestamp().Less(m_fLaunchTimestamp.PlusSeconds(GRACE_PERIOD_SECONDS));
	}
	
	WorldTimestamp GetLaunchTimestamp()
	{
		return m_fLaunchTimestamp;
	}

	bool Launch()
	{	
		ChimeraWorld world = GetWorld();
		if (!world)
			return false;
		
		m_fLaunchTimestamp = world.GetServerTimestamp();
		
		if (m_fDespawnTimeout > 0)
			m_fDespawnTimestamp = world.GetServerTimestamp().PlusSeconds(m_fDespawnTimeout);
		
		
		GetOnAmbientEventLaunched().Invoke();
		
		return true;
	}

	bool CheckDespawn()
	{		
		ESCT_Logger.InfoFormat("[ESCT_AmbientEventBase] Despawn check for %1 event on %2 position.", 
			typename.EnumToString(ESCT_EAmbientEventType, m_eAmbientEventType), 
			GetOrigin().ToString()
		);
		
		if (m_fDespawnTimestamp)
		{
			ChimeraWorld world = GetWorld();
			if (world && world.GetServerTimestamp().GreaterEqual(m_fDespawnTimestamp))
				return true;
		}
		
		return false;
	}

	void Despawn()
	{
		GetOnAmbientEventDespawned().Invoke();
		SCR_EntityHelper.DeleteEntityAndChildren(this);
	}
}