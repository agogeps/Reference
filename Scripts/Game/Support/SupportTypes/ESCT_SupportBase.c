[EntityEditorProps(category: "Escapists/Entities", description: "Base entity for support.", visible: false)]
class ESCT_SupportBaseClass : GenericEntityClass
{
}

[BaseContainerProps(), BaseContainerCustomStringTitleField("DO NOT USE BASE CLASS!")]
class ESCT_SupportBase : GenericEntity
{
	[Attribute("0", desc: "Support type.", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ESCT_ESupportType))]
	protected ESCT_ESupportType m_eSupportType;
	
	[Attribute(defvalue: "600", UIWidgets.Slider, desc: "Distance to despawn support.", params: "0 2000 1")]
	protected float m_fDespawnDistance;

	[Attribute(defvalue: "1800", UIWidgets.Slider, desc: "Time before despawn (if other despawn conditions weren't fullfilled yet).", params: "0 10000 1")]
	protected float m_fDespawnTimeout;

	protected SCR_AIGroup m_callerGroup;
	protected ESCT_ESupportState m_eSupportState = ESCT_ESupportState.New;
	protected vector m_vTargetPosition;
	protected IEntity m_Target;
	
	protected WorldTimestamp m_fLaunchTimestamp;
	protected WorldTimestamp m_fDespawnTimestamp;
	
	protected ref ScriptInvokerVoid m_OnSupportLaunched;
	protected ref ScriptInvokerVoid m_OnSupportDespawned; 
	
	private static const float GRACE_PERIOD_SECONDS = 10;
	
	ScriptInvokerVoid GetOnSupportLaunched()
	{
		if (!m_OnSupportLaunched)
		{
			m_OnSupportLaunched = new ScriptInvokerVoid();
		}

		return m_OnSupportLaunched;
	}
	
	ScriptInvokerVoid GetOnSupportDespawned()
	{
		if (!m_OnSupportDespawned)
		{
			m_OnSupportDespawned = new ScriptInvokerVoid();
		}

		return m_OnSupportDespawned;
	}

	SCR_AIGroup GetCallerGroup()
	{
		//maybe it's not needed, but I'm encountering frequent crashes on null referenced groups
		if (!m_callerGroup)
			return null;
		
		return m_callerGroup;
	}

	ESCT_ESupportType GetSupportType()
	{
		return m_eSupportType;
	}

	ESCT_ESupportState GetSupportState()
	{
		return m_eSupportState;
	}
	
	vector GetTargetPosition()
	{
		return m_vTargetPosition;
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

	bool Setup(IEntity target, vector targetPosition = vector.Zero, IEntity caller = null, bool followTarget = false)
	{
		if (!SetTarget(target, targetPosition))
			return false;
		
		if (caller)
			m_callerGroup = ESCT_CharacterHelper.GetAIGroup(caller);
		
		return true;
	}

	bool Launch()
	{
		if (m_eSupportState != ESCT_ESupportState.Ready)
			return false;
		
		ChimeraWorld world = GetWorld();
		if (!world)
			return false;
		
		m_fLaunchTimestamp = world.GetServerTimestamp();
		
		if (m_fDespawnTimeout > 0)
			m_fDespawnTimestamp = world.GetServerTimestamp().PlusSeconds(m_fDespawnTimeout);
		
		m_eSupportState = ESCT_ESupportState.InProgress;
		
		GetOnSupportLaunched().Invoke();
		
		return true;
	}

	bool CheckDespawn()
	{		
		ESCT_Logger.InfoFormat("[ESCT_SupportBase] Despawn check for %1 support on %2 position.", 
			typename.EnumToString(ESCT_ESupportType, m_eSupportType), 
			GetOrigin().ToString()
		);
		
		if (m_eSupportState == ESCT_ESupportState.Finished)
			return true;

		if (m_fDespawnTimestamp)
		{
			ChimeraWorld world = GetWorld();
			if (world && world.GetServerTimestamp().GreaterEqual(m_fDespawnTimestamp))
				return true;
		}
		
		bool isPlayerNear = false;

		array<IEntity> players = {};
		ESCT_Player.GetAlivePlayers(players, true);

		for (int i = 0; i < players.Count(); i++)
		{
			if (ESCT_Math.IsPointInRadiusXZ(players[i].GetOrigin(), GetOrigin(), m_fDespawnDistance))
			{
				isPlayerNear = true;
				break;
			}
		}

		if (!isPlayerNear)
			return true;

		return false;
	}

	void Despawn()
	{
		GetOnSupportDespawned().Invoke();
		SCR_EntityHelper.DeleteEntityAndChildren(this);
	}
	
	string GetSupportToString()
	{
		return string.Format("%1 at %2 called by %3.", GetPrefabData().GetPrefabName(), m_vTargetPosition.ToString(true), m_callerGroup.ToString());
	}

	protected bool SetTarget(IEntity target, vector targetPosition)
	{
		if (targetPosition != vector.Zero)
			m_vTargetPosition = targetPosition;

		if (target)
			m_Target = target;

		if (!target && targetPosition == vector.Zero)
		{
			ESCT_Logger.Error("Both target and target positions are empty, failed to setup support.");
			return false;
		}

		return true;
	}
}
