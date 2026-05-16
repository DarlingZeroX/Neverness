#pragma once

#include <functional>

# include <VGImgui/Include/Imgui/imgui.h>
# include <VGImgui/Include/ImNodeEditor/imgui_node_editor.h>
#include <NNNodeGraphCore/Interface/Types.h>

// SlotType -> Icon
#include "widgets.h"

namespace ed = ax::NodeEditor;

// ----------------------------------------------------------------------------
// BlueprintNodeBuilder（架构升级版）
// ----------------------------------------------------------------------------
// 目标：接近 Unreal Engine Blueprint 的三段式节点布局（Header / Middle / Pins）。
//
// 布局核心约束：
// - 使用 Stage 状态机统一控制容器打开/关闭，避免 BeginGroup/SameLine 拼接不稳定
// - 使用 ImGui StackLayout（BeginHorizontal/BeginVertical/Spring）完成水平对齐：
//   * Input：左对齐（PivotAlignment=(0,0.5)）
//   * Middle：居中并自动撑开
//   * Output：右对齐（PivotAlignment=(1,0.5)）
// - 节点内部不再手动计算宽度，也不直接使用 ImGui::SameLine
// ----------------------------------------------------------------------------
class BlueprintNodeBuilder
{
public:
	BlueprintNodeBuilder() = default;

	void Begin(ed::NodeId id);
	void End();

	//========================
	// Header
	//========================
	// 实现要点：Header 内部的内容由调用方绘制（例如：node name 文本），
	//           EndHeader() 负责切换到 Content stage，并为 Header 背景绘制提供测量值。
	void Header(const ImVec4& color, float height = 28.0f);

	//========================
	// Header Gradient（从左到右渐变）
	//========================
	void HeaderGradient(const ImVec4& leftColor, const ImVec4& rightColor, float height = 28.0f);

	void EndHeader();

	//========================
	// Input 区域（左对齐）
	//========================
	void Input(ed::PinId id, const char* label, Horizon::NodeGraphRuntime::SlotType slotType);
	void EndInput();

	//========================
	// Middle 区域（居中并自动撑开）
	//========================
	// Middle 内支持任意复杂 UI（DialogueList 多行、按钮、事件连接等），
	// 只要调用方不要使用 ImGui Table 且不要破坏外部容器栈即可。
	void Middle(std::function<void()> func);

	//========================
	// Output 区域（右对齐）
	//========================
	void Output(ed::PinId id, const char* label, Horizon::NodeGraphRuntime::SlotType slotType);
	void EndOutput();

private:
	// 与用户需求一致：Stage 状态机
	enum class Stage
	{
		Invalid,
		Begin,
		Header,
		Content,
		Input,
		Middle,
		Output,
		End
	};

	bool SetStage(Stage stage);

	void Pin(ed::PinId id, ed::PinKind kind, const char* label, Horizon::NodeGraphRuntime::SlotType slotType);
	void EndPin();

	static ax::Drawing::IconType GetSlotIconType(Horizon::NodeGraphRuntime::SlotType slotType);
	static ImVec4 GetSlotIconColor(Horizon::NodeGraphRuntime::SlotType slotType);

private:
	ed::NodeId m_CurrentNode = 0;
	Stage       m_CurrentStage = Stage::Invalid;

	// Header 状态（用于 End() 绘制背景）
	bool  m_HasHeader = false;
	bool  m_HeaderBgEnabled = false;
	bool  m_HeaderGradientEnabled = false;
	ImVec4 m_HeaderSolidColor = ImVec4(0, 0, 0, 0);
	ImVec4 m_HeaderLeftColor = ImVec4(0, 0, 0, 0);
	ImVec4 m_HeaderRightColor = ImVec4(0, 0, 0, 0);

	// 在 SetStage(oldStage==Header) 关闭 header 容器时测量
	ImVec2 m_HeaderMin = ImVec2(0, 0);
	ImVec2 m_HeaderMax = ImVec2(0, 0);
};