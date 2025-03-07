[EntityEditorProps(category: "Escapists/Entities", description: "Possible spawn point for entity of some kind.")]
class ESCT_CharacterSpawnPointClass : GenericEntityClass
{
}

sealed class ESCT_CharacterSpawnPoint : GenericEntity
{
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Probability of spawn will be not checked and item will be always present on this point.", category: "Escapists")]
	bool m_IsSpawnGuaranteed;

	protected IEntity m_Entity;
	protected int m_SpawnPointId;
	protected ESCT_BaseCharacterManagerComponent m_CharacterManager;

	IEntity GetEntity()
	{
		return m_Entity;
	}

	int GetEntitySpawnPointId()
	{
		return m_SpawnPointId;
	}

	void SetCharacterManager(ESCT_BaseCharacterManagerComponent manager)
	{
		m_CharacterManager = manager;
	}

	void ESCT_CharacterSpawnPoint(IEntitySource src, IEntity parent)
	{
		if (SCR_Global.IsEditMode() || !Replication.IsServer()) return;

		m_SpawnPointId = ESCT_IdentifierGenerator.GenerateIntId();
	}

	IEntity SpawnCharacter(ResourceName prefab, SCR_AIGroup group = null, bool enableAI = true)
	{
		if (m_Entity)
		{
			PrintFormat("[Escapists] SpawnPoint %1 is already populated with %2.", this, m_Entity);
			return null;
		}

		vector spawnPosition = GetOrigin();
		vector spawnRotator = GetYawPitchRoll();

		if (group)
		{
			m_Entity = ESCT_CharacterHelper.SpawnCharacterForGroup(prefab, group, spawnPosition, spawnRotator, enableAI);
		}
		else
		{
			m_Entity = ESCT_CharacterHelper.SpawnCharacter(prefab, spawnPosition, spawnRotator, enableAI);
		}
		
		if (!m_Entity)
			return null;

		ChimeraCharacter character = ChimeraCharacter.Cast(m_Entity);
		if (!character)
			return null;
		
		SCR_CharacterDamageManagerComponent damageMan = SCR_CharacterDamageManagerComponent.Cast(character.GetDamageManager());
		if (damageMan)
			damageMan.GetOnDamageStateChanged().Insert(OnDeath);

		return m_Entity;
	}

	private void OnDeath(EDamageState state)
	{
		//free entity from spawn point
		m_Entity = null;

		if (m_CharacterManager)
		{
			m_CharacterManager.UpdateState(new ESCT_CharacterState(m_SpawnPointId, ResourceName.Empty, true));
		}
	}

#ifdef WORKBENCH
	private const float _DRAW_DISTANCE = 100 * 100;

	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		WorldEditorAPI api = this._WB_GetEditorAPI();
		if (!api)
			return;

		IEntitySource src = api.EntityToSource(this);
		if (!src)
			return;

		if (!api.IsEntityVisible(src) || !api.IsEntityLayerVisible(src.GetSubScene(), src.GetLayerID()))
			return;

		const float radius = 0.1;

		vector outMat[4];
		GetWorldTransform(outMat);

		vector cameraTransform[4];
		GetWorld().GetCurrentCamera(cameraTransform);

		int colorBackground = 0x00000000;
		if (api.IsEntitySelected(src))
		{
			colorBackground = ARGBF(1, 0, 0, 0);
		}
		else
		{
			//--- Draw only if near when not selected
			if (vector.DistanceSq(GetOrigin(), cameraTransform[3]) > _DRAW_DISTANCE)
				return;
		}

		int color = COLOR_RED;
		const string text = "Character";

		vector position = outMat[3];
		ref Shape pointShape = Shape.CreateSphere(color, ShapeFlags.ONCE | ShapeFlags.NOOUTLINE, position, radius);
		ref Shape arrowShape = Shape.CreateArrow(position, position + outMat[2], 0.2, color, ShapeFlags.ONCE);
		ref DebugTextWorldSpace textShape = DebugTextWorldSpace.Create(
			GetWorld(),
			text,
			DebugTextFlags.ONCE | DebugTextFlags.CENTER | DebugTextFlags.FACE_CAMERA,
			position[0],
			position[1] + 0.2,
			position[2],
			14.0,
			color,
			colorBackground
		);

		if (m_IsSpawnGuaranteed)
		{
			ref Shape sphere = Shape.CreateSphere(0, ShapeFlags.ONCE | ShapeFlags.NOOUTLINE, position + "0 0.1 0", radius / 2);
		}

		super._WB_AfterWorldUpdate(timeSlice);
	}
#endif
}
