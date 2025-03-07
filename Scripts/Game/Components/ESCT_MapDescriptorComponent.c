class ESCT_MapDescriptorComponentClass : SCR_MapDescriptorComponentClass
{
}

sealed class ESCT_MapDescriptorComponent : SCR_MapDescriptorComponent
{
	[Attribute(uiwidget: UIWidgets.EditBox, desc: "Quad name from imageset defined on MapWidget in Map.layout.", category: "Escapists")]
	protected string m_ItemDef;

	[Attribute("0.375", uiwidget: UIWidgets.Slider, "Icon size coefficient.", category: "Escapists", params: "0 2 0.125")]
	protected float m_ItemScale;

	[RplProp(onRplName: "OnRevealed")]
	protected bool m_bIsRevealed = false;

	[RplProp(onRplName: "OnInitialized")]
	private bool m_bIsInitialized = false;

	[RplProp(onRplName: "OnCleared")]
	private bool m_bIsCleared = false;

	private float m_fRevealDistance;

	private ref MapItem m_MapItem;
	private ESCT_Location m_Location;

	private static const string UNKNOWN_ICON_QUAD = "AR_icon_question_mark_02";
	private static const Color UNKNOWN_ICON_COLOR = Color.Black;
	private static const float UNKNOWN_ICON_SCALE = 2;
	private static const float CHECK_INTERVAL = 1000.0;

	void ESCT_MapDescriptorComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		if (!GetGame().InPlayMode())
			return;

		m_Location = ESCT_Location.Cast(ent);
	}

	void Init()
	{
		if (m_MapItem)
			return;

		RpcDo_Initialize();
		m_bIsInitialized = true;
		m_fRevealDistance = ESCT_EscapistsConfigComponent.GetInstance().GetMarkerRevealDistance();
		Replication.BumpMe();

		Rpc(RpcDo_Initialize);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_Initialize()
	{
		m_MapItem = Item();
		if (!m_MapItem)
			return;

		m_MapItem.SetBaseType(0);
		m_MapItem.SetImageDef(UNKNOWN_ICON_QUAD);

		MapDescriptorProps props = m_MapItem.GetProps();
		props.SetFrontColor(UNKNOWN_ICON_COLOR);
		props.SetOutlineColor(UNKNOWN_ICON_COLOR);
		props.SetIconSize(UNKNOWN_ICON_SCALE, UNKNOWN_ICON_SCALE, UNKNOWN_ICON_SCALE);
		props.SetVisible(true);
		props.Activate(true);
		m_MapItem.SetProps(props);
		m_MapItem.SetVisible(true);
	}

	void Reveal()
	{
		if (!Replication.IsServer())
			return;

		//host reveal
		RpcDo_Reveal();

		//memorizing reveal fact for JIPs
		m_bIsRevealed = true;
		Replication.BumpMe();

		//reveal for already connected clients
		Rpc(RpcDo_Reveal);
	}

	void Clear()
	{
		//host clear
		RpcDo_Clear();

		//memorizing clear fact for JIPs
		m_bIsCleared = true;
		Replication.BumpMe();

		//clear for already connected clients
		Rpc(RpcDo_Clear);
	}

	void SetPollMarkerReveal(bool enable)
	{
		if (!m_MapItem)
			return;

		if (enable)
		{
			if (!m_MapItem.IsVisible() || m_bIsRevealed)
				return;

			GetGame().GetCallqueue().CallLater(PollMarkerReveal, CHECK_INTERVAL, true);
		}
		else
		{
			GetGame().GetCallqueue().Remove(PollMarkerReveal);
		}
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_Reveal()
	{
		if (!m_MapItem)
			return;

		Color markerColor = UNKNOWN_ICON_COLOR; //fallback color
		
		FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(m_Location.FindComponent(FactionAffiliationComponent));
		if (factionAffiliation)
		{
			Faction faction = factionAffiliation.GetAffiliatedFaction();
			if (faction)
				markerColor = faction.GetFactionColor();
		}

		m_MapItem.SetImageDef(m_ItemDef);
		MapDescriptorProps props = m_MapItem.GetProps();
		props.SetIconSize(m_ItemScale, m_ItemScale, m_ItemScale);
		props.SetFrontColor(markerColor);
		props.SetOutlineColor(markerColor);

		m_MapItem.SetProps(props);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_Clear()
	{
		if (!m_MapItem)
			return;

		MapDescriptorProps props = m_MapItem.GetProps();
		props.SetFrontColor(UNKNOWN_ICON_COLOR);
		props.SetOutlineColor(UNKNOWN_ICON_COLOR);
		m_MapItem.SetProps(props);
	}

	private void PollMarkerReveal()
	{
		bool isInteresected = false;

		array<IEntity> players = {};
		ESCT_Player.GetAlivePlayers(players, true);

		for (int i = 0; i < players.Count(); i++)
		{
			if (ESCT_Math.IsPointInRadiusXZ(players[i].GetOrigin(), m_Location.GetOrigin(), m_fRevealDistance))
			{
				isInteresected = true;
				break;
			}
		}

		if (!isInteresected)
			return;

		Reveal();

		GetGame().GetCallqueue().Remove(PollMarkerReveal);
	}

	private void OnInitialized()
	{
		if (m_MapItem)
			return;

		RpcDo_Initialize();
	}

	private void OnCleared()
	{
		if (!m_MapItem)
		{
			RpcDo_Initialize();
		}

		RpcDo_Clear();
	}

	private void OnRevealed()
	{
		if (!m_MapItem)
		{
			RpcDo_Initialize();
		}

		RpcDo_Reveal();
	}
}
