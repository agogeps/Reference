class ESCT_EntityHelperT<Class TType>
{
	//! Returns all children of entity with specified T type that has Hierarchy component.
	static void GetChildrenOfType(IEntity owner, notnull inout array<TType> typedEntities)
	{
		array<IEntity> children = {};
		ESCT_EntityHelper.GetAllChildren(owner, children);
		
		IEntity entity;
		for (int i = 0; i < children.Count(); i++)
		{
			entity = children[i];
			if (!entity || entity.Type() != TType)
				continue;
			
			typedEntities.Insert(TType.Cast(entity));
		}
	}
}

class ESCT_EntityHelper
{
	//! Returns all children of entity that has Hierarchy component.
	static void GetAllChildren(IEntity parent, notnull inout array<IEntity> allChildren, bool recursive = true)
	{
		if (!parent)
			return;
		
		IEntity child = parent.GetChildren();
		while (child)
		{
			allChildren.Insert(child);
			if (recursive)
			{
				GetAllChildren(child, allChildren, true);
			}
			child = child.GetSibling();
		}
	}
}
