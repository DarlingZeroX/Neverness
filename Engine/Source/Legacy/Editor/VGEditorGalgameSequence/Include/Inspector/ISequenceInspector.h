/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <VGImgui/IncludeImGui.h>

#include <string>

#include "Core/SequenceEditorContext.h"

namespace VisionGal
{
	class IVGSSequenceComponent;
}

namespace VisionGal::Editor
{
	/// Extensible inspector surface (property panel, later timeline row / graph node).
	/// 可扩展的检查器界面（属性面板，后续可扩展时间轴行/图节点等）。
	class ISequenceInspector
	{
	public:
		virtual ~ISequenceInspector() = default;

		virtual void OnInspectorGUI(unsigned int index, VisionGal::IVGSSequenceComponent* component, SequenceEditorContext* context) = 0;

		virtual void OnHeaderGUI(unsigned int index, VisionGal::IVGSSequenceComponent* component, SequenceEditorContext* context) {}

		virtual void OnTimelineGUI(unsigned int index, VisionGal::IVGSSequenceComponent* component, SequenceEditorContext* context) {}

		virtual void OnContextMenu(unsigned int index, VisionGal::IVGSSequenceComponent* component, SequenceEditorContext* context) {}

		virtual std::string GetDisplayName() const = 0;

		virtual ImVec4 GetThemeColor() const { return ImVec4(0.55f, 0.55f, 0.60f, 1.f); }

		virtual bool Validate(unsigned int index, VisionGal::IVGSSequenceComponent* component) const { return true; }

		virtual std::string GetBoundTypeNameID() const = 0;
	};
}
