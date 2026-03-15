/*
* This source file is part of VisionGal, the Visual Novel Engine
*
* For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
* GitHub page: https://github.com/DarlingZeroX/VisionGal
*
* Copyright (c) 2025-present 梦旅缘心
*
* See the LICENSE file in the project root for details.
*/

#include "SceneInterface.h"
#include <HCore/Include/Scene/HBaseComponent.h>
#include <HCore/Include/Event/HEventDelegate.h>
//#include "Scene/Components.h"

namespace VisionGal
{
	void GameActor::SetLabel(const String& label)
	{
		auto* com = GetComponent<Horizon::HRelationship>();
		if (com)
		{
			com->Label = label;
		}
	}

	String GameActor::GetLabel()
	{
		auto* com = GetComponent<Horizon::HRelationship>();
		if (com)
		{
			return com->Label;
		}

		return "";
	}

	void GameActor::SetVisible(bool visible)
	{
		//auto* com = GetComponent<TransformComponent>();
		//if (com)
		//{
		//	com->visible = visible;
		//}
	}

	bool GameActor::GetVisible()
	{
		//auto* com = GetComponent<TransformComponent>();
		//if (com)
		//{
		//	return com->visible;
		//}

		return true;
	}

	IComponent* GameActor::GetComponentByType(const String& type)
	{
		//if (type == "Transform")
		//{
		//	return GetComponent<TransformComponent>();
		//}

		return nullptr;
	}

	IComponent* GameActor::AddComponentByType(const String& type)
	{
		return nullptr;
	}
}
