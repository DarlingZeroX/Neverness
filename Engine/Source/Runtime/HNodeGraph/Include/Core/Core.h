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

#pragma once
#include "../Config.h"
#include "Types.h"
#include <HCore/Interface/HConfig.h>
#include <HCore/Interface/HCoreTypes.h>

namespace Horizon::NodeGraph
{
	struct H_NODE_GRAPH_API Link
	{
		LINK_ID LinkID = 0;
		PIN_ID StartPinID = 0;
		PIN_ID EndPinID = 0;

		float4 LinkColor = float4(1.0f,1.0f,1.0f,1.0f);
		float LinkThickness = 2.0f;
	};

	struct NodeComponent
	{
		virtual ~NodeComponent() = default;

		NODE_COMPONENT_ID ComponentID = 0;
		std::string ComponentType;
	};

	struct Pin: public NodeComponent
	{
		~Pin() override = default;

		PIN_ID PinID = 0;
		std::string PinTitle;
		int PinSize = 24;
		bool Connected = false;
		PinDirection Direction = PinDirection::Input;

		float4 PinColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	};

	struct Node
	{
		virtual ~Node() = default;

		NODE_ID NodeID = 0;
		std::string NodeType;
		std::string NodeTitle;
		float4 NodeColor = float4(1.0f, 1.0f, 1.0f, 1.0f);

		float2 Size;
	};

	struct BlueprintNode: public Node
	{
		~BlueprintNode() override = default;

		std::vector<Ref<NodeComponent>> Components[static_cast<unsigned>(NodeComponentPosition::Size)];
	};

	using SLOT_ID = size_t;
	using EDGE_ID = size_t;
	using NODE_RUNTIME_ID = size_t;

	struct RuntimeEdge
	{
		EDGE_ID ID;
		SLOT_ID From;
		SLOT_ID To;
	};

	struct RuntimeSlot
	{
		SLOT_ID ID;
		std::string Type;
	};

	struct RuntimeNode
	{
		NODE_RUNTIME_ID ID;
		std::string Type;

		std::vector<RuntimeSlot> Inputs;
		std::vector<RuntimeSlot> Outputs;

		//void (*Execute)(RuntimeContext&);
	};
}
