/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <string>

namespace VisionGal::Editor
{
	/// 第三方或内置包注册的序列编辑器扩展（Phase 8：静态注册为主）。
	class ISequenceEditorExtension
	{
	public:
		virtual ~ISequenceEditorExtension() = default;
		[[nodiscard]] virtual const char* GetExtensionId() const = 0;
		virtual void OnEditorSessionBegin() {}
		virtual void OnEditorSessionEnd() {}
	};
}
