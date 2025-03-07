sealed class ESCT_SupportSystem : GameSystem
{
	[Attribute("", UIWidgets.Auto, category: "Escapists")]
	protected ref ESCT_SupportSystemConfig m_SupportConfig;
	
	protected ref array<ESCT_SupportBase> m_aActiveSupports = {};
	protected ref map<ESCT_ESupportType, ref ESCT_SupportInfo> m_mSupportInfoMap = new map<ESCT_ESupportType, ref ESCT_SupportInfo>();

	private ref ESCT_SupportPicker m_SupportPicker;
	private float m_fLastExecutionTime = 0;
	
	private static const float SUPPORT_UPDATE_TICK = 10000;

	static ESCT_SupportSystem GetSystem()
	{
		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return null;

		return ESCT_SupportSystem.Cast(world.FindSystem(ESCT_SupportSystem));
	}
	
	void AddExternalSupport(notnull ESCT_SupportBase support)
	{
		m_aActiveSupports.Insert(support);
	}
	
	array<ESCT_SupportBase> GetActiveSupports()
	{
		return m_aActiveSupports;
	}

	bool HasSimilarActiveSupport(SCR_AIGroup group, vector position)
	{
		if (position == vector.Zero)
		{
			ESCT_Logger.ErrorFormat("Can't check support position - zero vector provided.");
			return false;
		}

		bool hasSupport = false;

		ESCT_SupportBase support = null;
		for (int i = 0; i < m_aActiveSupports.Count(); i++)
		{
			support = m_aActiveSupports[i];
			if (!support)
				continue;

			SCR_AIGroup callerGroup = support.GetCallerGroup();
			if (support.GetCallerGroup() != null && callerGroup == group)
			{
				ESCT_Logger.InfoFormat("%1 group already has active support - %2", group.ToString(), support.GetSupportToString());
				hasSupport = true;
				break;
			}
//
//			if (vector.Distance(position, support.GetTargetPosition()) < 25.0 && support.GetSupportType() == type)
//			{
//				ESCT_Logger.InfoFormat("There is already support of %1 type on %2 position.", typename.EnumToString(ESCT_ESupportType, type), position.ToString());
//				hasSupport = true;
//				break;
//			}
		}

		return hasSupport;
	}

	void CreateSupport(IEntity target, vector targetPosition, IEntity caller = null, ESCT_ESupportType desiredSupportType = ESCT_ESupportType.UNDEFINED, bool followTarget = false, bool isSilent = false)
	{
		if (!target && targetPosition == vector.Zero)
		{
			ESCT_Logger.ErrorFormat("Failed to create %1 support due to zero vector position and no target!", typename.EnumToString(ESCT_ESupportType, desiredSupportType));
			return;
		}

		ESCT_SupportInfo supportInfo = SelectSupport(desiredSupportType);
		if (!supportInfo)
		{
			ESCT_Logger.ErrorFormat("Failed to create support due to inability to select support.", typename.EnumToString(ESCT_ESupportType, desiredSupportType));
			return;
		}

		ESCT_Logger.InfoFormat("Selected support with type %1.", typename.EnumToString(ESCT_ESupportType, supportInfo.GetSupportType()));

		IEntity supportEntity = ESCT_SpawnHelpers.SpawnEntityPrefab(supportInfo.GetSupportPrefab(), targetPosition, global: false);
		if (!supportEntity)
		{
			ESCT_Logger.ErrorFormat("Failed to spawn %1 support prefab at %2 position!", supportInfo.GetSupportPrefab(), targetPosition.ToString());
			return;
		}

		ESCT_SupportBase support = ESCT_SupportBase.Cast(supportEntity);
		if (!support)
		{
			ESCT_Logger.ErrorFormat("Failed to cast %1 entity to base support class.", support.ToString());
			return;
		}

		if (!support.Setup(target, targetPosition, caller, followTarget))
		{
			ESCT_Logger.ErrorFormat("Failed to setup %1 support for some reason, check corresponding support entity to debug.", support.ToString());
			return;
		}

		support.Launch();

		if (!isSilent)
			ShowFeedbackAboutUpcomingSupport(support.GetSupportType());

		m_aActiveSupports.Insert(support);
	}
	
	void ShowFeedbackAboutUpcomingSupport(ESCT_ESupportType type)
	{
		PlayerManager plrManager = GetGame().GetPlayerManager();

		array<int> playerIds = {};
		plrManager.GetPlayers(playerIds);

		foreach (int id : playerIds)
		{
			PlayerController controller = plrManager.GetPlayerController(id);
			if (!id)
				continue;

			ESCT_EscapistsNetworkComponent networkComponent = ESCT_EscapistsNetworkComponent.Cast(controller.FindComponent(ESCT_EscapistsNetworkComponent));
			if (!networkComponent)
				continue;

			networkComponent.ShowSupportNotification(type);
		}
	}

	protected override void OnInit()
	{
		super.OnInit();

		if (SCR_Global.IsEditMode() || !Replication.IsServer())
			return;

		ESCT_Logger.Info("Initializing Support System.");

		FillSupportMap();
		
		m_SupportPicker = new ESCT_SupportPicker(m_aActiveSupports, m_mSupportInfoMap);

		ESCT_Logger.Info("Support System initialized.");
	}

	protected override void OnUpdate(ESystemPoint point)
	{
		super.OnUpdate(point);

		float currentTime = GetWorld().GetWorldTime();

		if (currentTime - m_fLastExecutionTime < 1)
			return;

		m_fLastExecutionTime = currentTime + SUPPORT_UPDATE_TICK;
		
		ESCT_SupportBase support = null;

		for (int i = m_aActiveSupports.Count() - 1; i >= 0; i--)
		{
			support = m_aActiveSupports[i];
			if (!support || !support.CheckDespawn())
				continue;

			ESCT_Logger.InfoFormat("%1 support will be despawned - %2 type, %3 state.",
				support.ToString(),
				typename.EnumToString(ESCT_ESupportType, support.GetSupportType()),
				typename.EnumToString(ESCT_ESupportState, support.GetSupportState())
			);

			support.Despawn();
			m_aActiveSupports.Remove(i);
		}
	}

	private ESCT_SupportInfo SelectSupport(ESCT_ESupportType desiredSupportType = ESCT_ESupportType.UNDEFINED)
	{
		if (desiredSupportType != ESCT_ESupportType.UNDEFINED)
			return m_mSupportInfoMap.Get(desiredSupportType);

		return m_SupportPicker.PickSupportType();
	}

	private void FillSupportMap()
	{
		if (!m_SupportConfig)
			return;
		
		foreach (ESCT_SupportInfo supportInfo : m_SupportConfig.GetSupportInfos())
		{
			ESCT_ESupportType type = supportInfo.GetSupportType();
			m_mSupportInfoMap.Insert(type, supportInfo);
			ESCT_Logger.InfoFormat("[ESCT_SupportSystem] %1 support added to support types map.", typename.EnumToString(ESCT_ESupportType, type));
		}
	}
}
