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

#include "NodeFactory.h"

namespace Horizon::NodeGraphEditor
{
	EditorNode CreateNodeFromMeta(
		const Horizon::NodeGraphRuntime::NodeMeta& meta,
		GraphIdGenerator& idGen
	)
	{
		EditorNode node;
		node.id = idGen.NewNodeId();
		node.type = meta.type;
		node.name = meta.name;

		for (const auto& in : meta.inputs)
		{
			EditorPin pin;
			pin.id = idGen.NewPinId();
			pin.name = in.name;
			pin.type = in.type;
			pin.isInput = true;
			node.inputs.push_back(std::move(pin));
		}

		for (const auto& out : meta.outputs)
		{
			EditorPin pin;
			pin.id = idGen.NewPinId();
			pin.name = out.name;
			pin.type = out.type;
			pin.isInput = false;
			node.outputs.push_back(std::move(pin));
		}

		return node;
	}
}