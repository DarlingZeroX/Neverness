#include "Utilities/builders.h"

#include "NNRuntimeImGui/Include/ImGui/imgui_stacklayout.h"

namespace ed = ax::NodeEditor;

// ----------------------------------------------------------------------------
// Stage 状态机核心：保证 Begin/End 的容器栈始终匹配
// ----------------------------------------------------------------------------
bool BlueprintNodeBuilder::SetStage(Stage stage)
{
	if (stage == m_CurrentStage)
		return false;

	const Stage oldStage = m_CurrentStage;
	m_CurrentStage = stage;

	// 1) 先处理“离开旧 stage”时需要关闭的容器/弹出样式
	switch (oldStage)
	{
	case Stage::Begin:
		break;

	case Stage::Header:
		// 关闭 header 横向容器，并测量 header rect，用于 End() 绘制背景。
		ImGui::EndHorizontal();
		m_HeaderMin = ImGui::GetItemRectMin();
		m_HeaderMax = ImGui::GetItemRectMax();

		// header 与 content 间距：保持稳定的垂直节奏（不依赖 SameLine/宽度计算）
		ImGui::Spring(0.0f, ImGui::GetStyle().ItemSpacing.y * 2.0f);
		break;

	case Stage::Content:
		break;

	case Stage::Input:
		// 退出 inputs：弹出 pins 对齐样式，并关闭 inputs 容器
		ed::PopStyleVar(2); // PivotAlignment + PivotSize
		ImGui::Spring(1.0f, 0.0f);
		ImGui::EndVertical();
		break;

	case Stage::Middle:
		ImGui::EndVertical();
		break;

	case Stage::Output:
		ed::PopStyleVar(2); // PivotAlignment + PivotSize
		ImGui::Spring(1.0f, 0.0f);
		ImGui::EndVertical();
		break;

	case Stage::End:
		break;

	case Stage::Invalid:
		break;
	}

	// 2) 再处理“进入新 stage”时需要打开的容器
	switch (stage)
	{
	case Stage::Invalid:
		break;

	case Stage::Begin:
		// 整体节点布局容器：垂直堆叠 Header + Content
		ImGui::BeginVertical("node");
		break;

	case Stage::Header:
		m_HasHeader = true;
		ImGui::BeginHorizontal("header");
		break;

	case Stage::Content:
		// 从 Begin 直接进入 Content（无 header 情况）时，需要先让 StackLayout 正常初始化
		if (oldStage == Stage::Begin)
			ImGui::Spring(0.0f);

		// Content 是横向容器：内部依次放 inputs / middle / outputs（由后续 stage 依次打开）
		ImGui::BeginHorizontal("content");
		ImGui::Spring(0.0f, 0.0f);
		break;

	case Stage::Input:
		// Inputs：左对齐，宽度不抢占横向弹性空间
		ImGui::BeginVertical("inputs", ImVec2(0, 0), 0.0f);

		// 核心要求：使用 PivotAlignment 做左对齐（Pivot 在 x=0 边对齐）
		ed::PushStyleVar(ed::StyleVar_PivotAlignment, ImVec2(0.0f, 0.5f));
		ed::PushStyleVar(ed::StyleVar_PivotSize, ImVec2(0.0f, 0.0f)); // 让 pivot 驱动布局更“贴边”

		// 无 header 情况：让 inputs 在可视区域内更自然地开始（参考旧 demo 的策略）
		if (!m_HasHeader)
			ImGui::Spring(1.0f, 0.0f);
		break;

	case Stage::Middle:
		// Middle：居中并自动撑开（横向弹性权重）
		ImGui::Spring(1.0f);
		ImGui::BeginVertical("middle", ImVec2(0, 0), 1.0f);
		break;

	case Stage::Output:
		// Outputs：右对齐并保持与 Middle 对齐的弹性节奏
		if (oldStage == Stage::Middle || oldStage == Stage::Input)
			ImGui::Spring(1.0f);
		else
			ImGui::Spring(1.0f, 0.0f);

		ImGui::BeginVertical("outputs", ImVec2(0, 0), 1.0f);

		// 核心要求：使用 PivotAlignment 做右对齐（Pivot 在 x=1 边对齐）
		ed::PushStyleVar(ed::StyleVar_PivotAlignment, ImVec2(1.0f, 0.5f));
		ed::PushStyleVar(ed::StyleVar_PivotSize, ImVec2(0.0f, 0.0f));

		if (!m_HasHeader)
			ImGui::Spring(1.0f, 0.0f);
		break;

	case Stage::End:
		// 当 End 发生在 Input 阶段时：先补一个弹性段，避免 stacklayout 容器宽度计算“塌缩”
		if (oldStage == Stage::Input)
			ImGui::Spring(1.0f, 0.0f);

		// Content（横向容器）需要收口
		if (oldStage != Stage::Begin)
			ImGui::EndHorizontal();

		// 最外层 node 容器关闭
		ImGui::EndVertical();
		break;
	}

	return true;
}

void BlueprintNodeBuilder::Begin(ed::NodeId id)
{
	// reset
	m_CurrentNode = id;
	m_CurrentStage = Stage::Invalid;
	m_HasHeader = false;
	m_HeaderBgEnabled = false;
	m_HeaderGradientEnabled = false;
	m_HeaderSolidColor = ImVec4(0, 0, 0, 0);
	m_HeaderLeftColor = ImVec4(0, 0, 0, 0);
	m_HeaderRightColor = ImVec4(0, 0, 0, 0);
	m_HeaderMin = ImVec2(0, 0);
	m_HeaderMax = ImVec2(0, 0);

	// NodeEditor 需要的节点级 padding 调整：保持旧版风格（可根据需要再统一抽到 ed::Style）
	ed::PushStyleVar(ed::StyleVar_NodePadding, ImVec4(8, 4, 8, 8));

	ed::BeginNode(id);
	ImGui::PushID(id.AsPointer());

	SetStage(Stage::Begin);
}

void BlueprintNodeBuilder::End()
{
	// 为了满足“三段式结构”，在异常调用顺序下也尽量补齐空 middle。
	if (m_CurrentStage == Stage::Begin || m_CurrentStage == Stage::Header)
		SetStage(Stage::Content);

	if (m_CurrentStage == Stage::Content || m_CurrentStage == Stage::Input)
		SetStage(Stage::Middle);

	SetStage(Stage::End);

	const ed::NodeId nodeId = m_CurrentNode;

	ImGui::PopID();

	// -----------------------------------------------------------------
	// 1. 提交节点，此时节点编辑器会计算出该节点的最终完整大小
	ed::EndNode(); 
	// -----------------------------------------------------------------

	// -----------------------------------------------------------------
	// 2. 核心修复：在这里获取计算完成后的节点最终外框，并修正 Header 坐标
	if (ImGui::IsItemVisible()) // 优化：只在节点可见时计算
	{
		// ed::EndNode() 实际上向 ImGui 提交了一个 Item，可以直接获取它的 Rect
		ImVec2 nodeMin = ImGui::GetItemRectMin();
		ImVec2 nodeMax = ImGui::GetItemRectMax();

		// 修正 Header 背景区域的左右边界，使其完全贴合节点外沿
		m_HeaderMin.x = nodeMin.x;
		m_HeaderMax.x = nodeMax.x;

		// 修正 Header 的上边界，覆盖掉 NodePadding 导致的顶部留白
		m_HeaderMin.y = nodeMin.y;

		// m_HeaderMax.y 保持原样，因为这代表 Header 底部的位置
		// 如果觉得底部太贴文字，可以稍微加一点 Padding：
		// m_HeaderMax.y += 4.0f; 
	}
	// -----------------------------------------------------------------

	// 3. 绘制 Header 背景到 NodeEditor 背景层
	if (m_HeaderBgEnabled && nodeId.Get() != 0 && (m_HeaderMax.x > m_HeaderMin.x) && (m_HeaderMax.y > m_HeaderMin.y))
	{
		ImDrawList* drawList = ed::GetNodeBackgroundDrawList(nodeId);
		if (drawList)
		{
			const float rounding = ed::GetStyle().NodeRounding;
			if (!m_HeaderGradientEnabled)
			{
				drawList->AddRectFilled(
					m_HeaderMin,
					m_HeaderMax,
					ImGui::ColorConvertFloat4ToU32(m_HeaderSolidColor),
					rounding,
					ImDrawFlags_RoundCornersTop
				);
			}
			else
			{
				// 暂时用单色版本，渐变版本在缩放时会出现边界不贴合的问题（可能是 ImGui 的 AddRectFilledMultiColor 实现问题）
				drawList->AddRectFilled(
					m_HeaderMin,
					m_HeaderMax,
					ImGui::ColorConvertFloat4ToU32(m_HeaderLeftColor),
					rounding,
					ImDrawFlags_RoundCornersTop
				);
				
				//const ImU32 cLeft = ImGui::ColorConvertFloat4ToU32(m_HeaderLeftColor);
				//const ImU32 cRight = ImGui::ColorConvertFloat4ToU32(m_HeaderRightColor);
				//// MultiColor 版本不支持 rounding 参数；为了稳定优先保证缩放边界贴合
				//drawList->AddRectFilledMultiColor(m_HeaderMin, m_HeaderMax, cLeft, cRight, cRight, cLeft);
			}
		}
	}

	// 清理
	m_CurrentNode = 0;
	m_CurrentStage = Stage::Invalid;

	ed::PopStyleVar(); // StyleVar_NodePadding
}

void BlueprintNodeBuilder::Header(const ImVec4& color, float /*height*/)
{
	m_HeaderSolidColor = color;
	m_HeaderGradientEnabled = false;
	m_HeaderBgEnabled = true;

	SetStage(Stage::Header);
}

void BlueprintNodeBuilder::HeaderGradient(const ImVec4& leftColor, const ImVec4& rightColor, float /*height*/)
{
	m_HeaderLeftColor = leftColor;
	m_HeaderRightColor = rightColor;
	m_HeaderGradientEnabled = true;
	m_HeaderBgEnabled = true;

	SetStage(Stage::Header);
}

void BlueprintNodeBuilder::EndHeader()
{
	// Header -> Content：开启三段式内容区域容器
	SetStage(Stage::Content);

	// 防止 header/content 边界出现“空行抖动”
	ImGui::Spacing();
}

void BlueprintNodeBuilder::Input(ed::PinId id, const char* label, Horizon::NodeGraphRuntime::SlotType slotType)
{
	// 如果调用方没走 Header->EndHeader，确保 content/inputs 容器存在
	if (m_CurrentStage == Stage::Begin || m_CurrentStage == Stage::Header)
		SetStage(Stage::Content);

	if (m_CurrentStage != Stage::Input)
	{
		// 若用户还没进入 Middle，而直接画 Input，则按结构要求先进入 Input stage
		SetStage(Stage::Input);
	}

	Pin(id, ed::PinKind::Input, label, slotType);
}

void BlueprintNodeBuilder::EndInput()
{
	if (m_CurrentStage == Stage::Input)
		SetStage(Stage::Middle);
}

void BlueprintNodeBuilder::Middle(std::function<void()> func)
{
	if (m_CurrentStage == Stage::Begin || m_CurrentStage == Stage::Header)
		SetStage(Stage::Content);

	if (m_CurrentStage == Stage::Content || m_CurrentStage == Stage::Input)
		SetStage(Stage::Middle);

	// 中间区域本身允许任意复杂 UI（DialogueList 在这里绘制）
	ImGui::Spacing();
	if (func) func();
	ImGui::Spacing();
}

void BlueprintNodeBuilder::Output(ed::PinId id, const char* label, Horizon::NodeGraphRuntime::SlotType slotType)
{
	// 输出需要 Middle 容器已存在（哪怕它是空内容）
	if (m_CurrentStage == Stage::Begin || m_CurrentStage == Stage::Header)
		SetStage(Stage::Content);

	if (m_CurrentStage == Stage::Content)
		SetStage(Stage::Middle); // 空 Middle 补齐
	else if (m_CurrentStage == Stage::Input)
		SetStage(Stage::Middle); // EndInput 没调用时：补空 Middle

	if (m_CurrentStage != Stage::Output)
		SetStage(Stage::Output);

	Pin(id, ed::PinKind::Output, label, slotType);
}

void BlueprintNodeBuilder::EndOutput()
{
	if (m_CurrentStage == Stage::Output)
		SetStage(Stage::End);
}

void BlueprintNodeBuilder::Pin(ed::PinId id, ed::PinKind kind, const char* label, Horizon::NodeGraphRuntime::SlotType slotType)
{
	ed::BeginPin(id, kind);

	ImGui::AlignTextToFramePadding();

	const float iconSize = ImGui::GetTextLineHeight();
	const bool filled = ed::PinHadAnyLinks(id);

	// 禁止 SameLine：用 StackLayout 的 BeginHorizontal 来保持图标与文本水平排列。
	if (kind == ed::PinKind::Input)
	{
		ImGui::BeginHorizontal(id.AsPointer());
		ax::Widgets::Icon(
			ImVec2(iconSize, iconSize),
			GetSlotIconType(slotType),
			filled,
			GetSlotIconColor(slotType)
		);
		ImGui::Spacing();
		ImGui::TextUnformatted(label ? label : "");
		ImGui::EndHorizontal();
	}
	else
	{
		// Output：label 在左，icon 在右（视觉更贴近现有实现习惯）
		ImGui::BeginHorizontal(id.AsPointer());
		ImGui::TextUnformatted(label ? label : "");
		ImGui::Spacing();
		ax::Widgets::Icon(
			ImVec2(iconSize, iconSize),
			GetSlotIconType(slotType),
			filled,
			GetSlotIconColor(slotType)
		);
		ImGui::EndHorizontal();
	}

	ed::EndPin();
}

void BlueprintNodeBuilder::EndPin()
{
	// 该 Builder 的实现里：Pin() 已经封装了 BeginPin/EndPin。
	// EndPin() 目前仅保留为私有接口占位（避免调用方误用旧版分离逻辑）。
}

ax::Drawing::IconType BlueprintNodeBuilder::GetSlotIconType(Horizon::NodeGraphRuntime::SlotType slotType)
{
	using SlotType = Horizon::NodeGraphRuntime::SlotType;
	switch (slotType)
	{
	case SlotType::Exec: return ax::Drawing::IconType::Flow;
	case SlotType::Bool: return ax::Drawing::IconType::Circle;
	case SlotType::Int: return ax::Drawing::IconType::Square;
	case SlotType::Float: return ax::Drawing::IconType::Grid;
	case SlotType::String: return ax::Drawing::IconType::Diamond;
	default: return ax::Drawing::IconType::Circle;
	}
}

ImVec4 BlueprintNodeBuilder::GetSlotIconColor(Horizon::NodeGraphRuntime::SlotType slotType)
{
	using SlotType = Horizon::NodeGraphRuntime::SlotType;
	switch (slotType)
	{
	case SlotType::Exec: return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // pure white
	case SlotType::Bool: return ImVec4(0.2f, 0.8f, 1.0f, 1.0f);   // cyan
	case SlotType::Int: return ImVec4(0.3f, 0.9f, 0.4f, 1.0f);    // green
	case SlotType::Float: return ImVec4(0.6f, 0.3f, 1.0f, 1.0f);  // purple
	case SlotType::String: return ImVec4(0.2f, 0.6f, 1.0f, 1.0f); // blue
	default: return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	}
}
