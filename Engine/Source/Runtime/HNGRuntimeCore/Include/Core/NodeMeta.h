/*
* 运行时 NodeGraph 实现（无虚函数、POD 风格、函数指针驱动）
* 节点元数据：用于 Editor 自动构建节点（数据驱动）
*/

#pragma once

#include <string>
#include <vector>
#include "../../Interface/Types.h"
#include "../../Interface/RuntimeCore.h"

namespace Horizon::NodeGraphRuntime
{
	struct NodeMeta
	{
		NodeType type;
		std::string name;

		struct SlotMeta
		{
			std::string name;
			SlotType type;
			bool isInput;
		};

		std::vector<SlotMeta> inputs;
		std::vector<SlotMeta> outputs;

		NodeExecuteFn execute;
	};
}
