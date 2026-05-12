/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Schema/SequencePropertyValue.h"

#include <functional>

namespace VisionGal::Editor
{
	/// 将具体组件实例指针（`IVGSSequenceComponent*`）与 Schema 解耦的反射存取器。
	/// Getter/Setter 由宿主模块（如 VGEditorGalgameSequence Bootstrap）绑定 lambda。
	struct SequencePropertyAccessor
	{
		std::function<SequencePropertyValue(void*)> Getter;
		std::function<bool(void*, const SequencePropertyValue&)> Setter;
	};
}
