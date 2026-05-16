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
#include <NNCore/Include/Scene/HBaseComponent.h>
#include <NNCore/Include/Event/HEventDelegate.h>
//#include "Scene/Components.h"

namespace NN::Runtime
{
	void IGameActor::SetLabel(const String& label)
	{
		auto* com = GetComponent<NN::Core::HRelationship>();
		if (com)
		{
			com->Label = label;
		}
	}

	String IGameActor::GetLabel()
	{
		auto* com = GetComponent<NN::Core::HRelationship>();
		if (com)
		{
			return com->Label;
		}

		return "";
	}


}
