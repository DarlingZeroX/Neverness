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
#include <string>
#include "../HNGEditorCoreConfig.h"
#include "../Interface/NodeEditorCore.h"
#include "EditorGraph.h"
#include "HNGRuntimeCore/Include/Core/RuntimeGraph.h"

namespace Horizon::NodeGraphEditor
{
	HNG_EDITOR_CORE_API NodeGraphRuntime::RuntimeGraph Compile(const EditorGraph& editor, const NodeGraphRuntime::NodeRegistry& registry);
}
