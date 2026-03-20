#pragma once
#include <functional>
# include <VGImgui/Include/Imgui/imgui.h>
# include <VGImgui/Include/ImNodeEditor/imgui_node_editor.h>
#include <HNGRuntimeCore/Interface/Types.h>

// SlotType -> Icon
#include "widgets.h"

namespace ed = ax::NodeEditor;

class BlueprintNodeBuilder
{
public:
	BlueprintNodeBuilder() = default;

	void Begin(ed::NodeId id);
	void End();

	//========================
	// Header
	//========================
	void Header(const ImVec4& color, float height = 28.0f);

	//========================
	// Header Gradient（从左到右渐变）
	//========================
	void HeaderGradient(const ImVec4& leftColor, const ImVec4& rightColor, float height = 28.0f);

	void EndHeader();

	//========================
	// Body Begin
	//========================
	void BeginBody();

	void NextColumn();

	void EndBody();

	//========================
	// Input Pin
	//========================
	void Input(ed::PinId id, const char* label, Horizon::NodeGraphRuntime::SlotType slotType);

	//========================
	// Output Pin（右对齐）
	//========================
	void Output(ed::PinId id, const char* label, Horizon::NodeGraphRuntime::SlotType slotType);

	//========================
	// 中间内容（可选）
	//========================
	void MiddleContent(std::function<void()> func);

private:
	ed::NodeId m_CurrentNode = 0;

	static ax::Drawing::IconType GetSlotIconType(Horizon::NodeGraphRuntime::SlotType slotType);
	static ImVec4 GetSlotIconColor(Horizon::NodeGraphRuntime::SlotType slotType);

	bool m_HeaderActive = false;

	// Header 背景绘制（在 End() 中写入 node background draw list）
	bool m_HeaderBgEnabled = false;
	bool m_HeaderGradientEnabled = false;
	ImVec4 m_HeaderSolidColor = ImVec4(0, 0, 0, 0);
	ImVec4 m_HeaderLeftColor = ImVec4(0, 0, 0, 0);
	ImVec4 m_HeaderRightColor = ImVec4(0, 0, 0, 0);
	float m_HeaderRequestedHeight = 0.0f;
	ImVec2 m_HeaderStartScreenPos = ImVec2(0, 0);
	ImVec2 m_HeaderNodeTLScreenPos = ImVec2(0, 0);
	float m_HeaderStartCursorLocalY = 0.0f;
	float m_HeaderFinalHeightScreen = 0.0f;

	bool m_BodyActive = false;

	float m_LeftWidth = 0.0f;
	float m_RightWidth = 0.0f;
};