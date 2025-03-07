[EntityEditorProps("Escapists/Location", visible: false)]
class ESCT_LocationComponentClass : ScriptComponentClass
{
}

enum ESCT_ESpawnOrder
{
	Irrelevant = 0,
	Late = 2
} 

class ESCT_LocationComponent : ScriptComponent
{
	[Attribute("0", uiwidget: UIWidgets.ComboBox, desc: "Handles order of component spawn.", enums: ParamEnumArray.FromEnum(ESCT_ESpawnOrder))]
	protected ESCT_ESpawnOrder m_eSpawnOrder;
	
	protected ESCT_Location m_Location;
	
	ESCT_ESpawnOrder GetSpawnOrder()
	{
		return m_eSpawnOrder;
	}
	
	ESCT_Location GetLocation()
	{
		return m_Location;
	}
	
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		SetEventMask(owner, EntityEvent.INIT);
	}
	
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		m_Location = ESCT_Location.Cast(owner);
		m_Location.AddToLocationComponents(this);
	}
	
	void Spawn();
	void Despawn();
}
