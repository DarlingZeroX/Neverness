/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Schema/SequencePropertyValue.h"

#include <string>

namespace VisionGal::Editor
{
	struct SequenceEditorContext;

	/// 将 Schema 属性名与取值转为现有撤销命令（字符串 / 布尔字段）。
	/// @return 是否已识别并执行（未识别则返回 false，由调用方决定降级策略）。
	bool TryDispatchSchemaPropertyEdit(
		const std::string& typeNameId,
		const std::string& propertyName,
		unsigned entryIndex,
		const SequencePropertyValue& value,
		SequenceEditorContext* context);
}
