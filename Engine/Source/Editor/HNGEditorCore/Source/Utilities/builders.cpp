#include "Utilities/builders.h"

// 注意：
// 该 cpp 文件用于承载 BlueprintNodeBuilder 的具体实现。
// 把实现从 builders.h 拆出，能降低头文件编译开销，并避免过多 inline 代码膨胀。

void BlueprintNodeBuilder::Begin(ed::NodeId id)
{
	m_CurrentNode = id;
	ed::BeginNode(id);

	ImGui::PushID(id.AsPointer());

	//ImGui::BeginGroup(); // 整体节点
}

void BlueprintNodeBuilder::End()
{
	const ed::NodeId nodeId = m_CurrentNode;

	// 如果打开了 Body 容器（BeginBody），End() 里确保把 Body 外层 BeginGroup 关闭，
	// 从而保证：
	// - Begin/End 的组栈始终匹配（避免 Missing EndGroup）
	// - MiddleContent 在 EndBody 之后仍处于 Body 外层布局容器内部（避免跑到 Node 外）
	if (m_BodyActive)
	{
		ImGui::EndGroup(); // close Outer body group
		m_BodyActive = false;
	}

	//ImGui::EndGroup();

	ImGui::PopID();
	ed::EndNode();

	// Header 背景在 End 阶段绘制到节点背景层（避免 BeginChild 导致的布局/脱离问题）
	if (m_HeaderBgEnabled && nodeId.Get() != 0 && m_HeaderFinalHeightScreen > 0.0f)
	{
		ImDrawList* drawList = ed::GetNodeBackgroundDrawList(nodeId);
		if (drawList)
		{
			// 关键修复：
			// GetNodePosition()/GetNodeSize() 返回的是节点在当前编辑器的“绘制坐标系”（与 Node::Draw 使用的 m_Bounds 一致），
			// 不应再做 CanvasToScreen 转换，否则会在拖拽/缩放时造成 x/width 不匹配。
			const ImVec2 nodeTL = ed::GetNodePosition(nodeId);
			const ImVec2 nodeSize = ed::GetNodeSize(nodeId);

			// header 背景的 Y 起点以 Header() 时的布局起点为准，
			// 避免 background 绘制到节点外侧边界（缩放/拖拽时更稳定）
			const float headerTopY = m_HeaderStartScreenPos.y;
			const ImVec2 pMin(nodeTL.x, headerTopY);
			const ImVec2 pMax(nodeTL.x + nodeSize.x, headerTopY + m_HeaderFinalHeightScreen);

			const float rounding = ed::GetStyle().NodeRounding;
			if (!m_HeaderGradientEnabled)
			{
				drawList->AddRectFilled(
					pMin,
					pMax,
					ImGui::ColorConvertFloat4ToU32(m_HeaderSolidColor),
					rounding,
					ImDrawFlags_RoundCornersTop
				);
			}
			else
			{
				// MultiColor 不支持圆角参数（内部 API 不提供 rounding 参数），但能保证缩放/边界正确贴合。
				const ImU32 cLeft = ImGui::ColorConvertFloat4ToU32(m_HeaderLeftColor);
				const ImU32 cRight = ImGui::ColorConvertFloat4ToU32(m_HeaderRightColor);
				drawList->AddRectFilledMultiColor(pMin, pMax, cLeft, cRight, cRight, cLeft);
			}
		}
	}

	// 清理 header 状态
	m_HeaderActive = false;
	m_HeaderBgEnabled = false;
	m_HeaderGradientEnabled = false;
	m_HeaderFinalHeightScreen = 0.0f;

	m_CurrentNode = 0;
}

void BlueprintNodeBuilder::Header(const ImVec4& color, float height)
{
	m_HeaderSolidColor = color;
	m_HeaderGradientEnabled = false;
	m_HeaderBgEnabled = true;
	m_HeaderRequestedHeight = height;
	m_HeaderStartScreenPos = ImGui::GetCursorScreenPos();
	m_HeaderNodeTLScreenPos = ed::CanvasToScreen(ed::GetNodePosition(m_CurrentNode));
	m_HeaderStartCursorLocalY = ImGui::GetCursorPos().y;
	m_HeaderFinalHeightScreen = 0.0f;
	m_HeaderActive = true;
}

void BlueprintNodeBuilder::HeaderGradient(const ImVec4& leftColor, const ImVec4& rightColor, float height)
{
	m_HeaderLeftColor = leftColor;
	m_HeaderRightColor = rightColor;
	m_HeaderGradientEnabled = true;
	m_HeaderBgEnabled = true;
	m_HeaderRequestedHeight = height;
	m_HeaderStartScreenPos = ImGui::GetCursorScreenPos();
	m_HeaderNodeTLScreenPos = ed::CanvasToScreen(ed::GetNodePosition(m_CurrentNode));
	m_HeaderStartCursorLocalY = ImGui::GetCursorPos().y;
	m_HeaderFinalHeightScreen = 0.0f;
	m_HeaderActive = true;
}

void BlueprintNodeBuilder::EndHeader()
{
	if (m_HeaderBgEnabled && m_HeaderActive)
	{
		// 让 header 背景贴合实际布局高度（参考 ImNodeEditor 内部做法），
		// 避免为了“强行补齐到 requested height”在拖拽过程中触发布局抖动。
		const float finalScreenH = ImGui::GetCursorScreenPos().y - m_HeaderStartScreenPos.y;
		m_HeaderFinalHeightScreen = finalScreenH;
	}

	m_HeaderActive = false;
	ImGui::Spacing();
}

void BlueprintNodeBuilder::BeginBody()
{
	m_BodyActive = true;

	ImGui::BeginGroup();

	float fullWidth = ImGui::GetContentRegionAvail().x;

	m_LeftWidth = fullWidth * 0.5f;
	m_RightWidth = fullWidth * 0.5f;

	// 左列（Inputs）
	ImGui::BeginGroup();
}

void BlueprintNodeBuilder::NextColumn()
{
	ImGui::EndGroup();

	ImGui::SameLine();

	// 右列（Outputs）
	ImGui::BeginGroup();
}

void BlueprintNodeBuilder::EndBody()
{
	// BeginBody() -> NextColumn() 开启的 Body 容器里：
	// - 这里只需要关闭当前列（Right/Left）的 BeginGroup
	// - Body 的“外层 BeginGroup”保持开启，供 EndBody 之后的 MiddleContent 继续位于节点内部布局中
	ImGui::EndGroup(); // close Right/Left column group

	// 结束列组后，将光标重置到 Body 的起始位置。
	// 否则 MiddleContent() 可能从“上一列的光标 X 位置”继续绘制，
	// 导致文本/控件看起来跑出节点边界（例如在 Dialogue 的 InputTextMultiline 上表现为内容出框）。
	ImGui::NewLine();
	ImGui::SetCursorPosX(0.0f);
}

void BlueprintNodeBuilder::Input(ed::PinId id, const char* label, Horizon::NodeGraphRuntime::SlotType slotType)
{
	ed::BeginPin(id, ed::PinKind::Input);

	ImGui::AlignTextToFramePadding();

	const float iconSize = ImGui::GetTextLineHeight();
	const bool filled = ed::PinHadAnyLinks(id);
	ax::Widgets::Icon(
		ImVec2(iconSize, iconSize),
		GetSlotIconType(slotType),
		filled,
		GetSlotIconColor(slotType)
	);
	ImGui::SameLine();
	ImGui::Text("%s", label);

	ed::EndPin();
}

void BlueprintNodeBuilder::Output(ed::PinId id, const char* label, Horizon::NodeGraphRuntime::SlotType slotType)
{
	ed::BeginPin(id, ed::PinKind::Output);

	// 注意：新版 ImNodeEditor 拖拽/重建时，content region 尺寸可能随帧变化。
	// 若依赖 GetContentRegionAvail() 做右对齐偏移，会导致 pin 文本偏移从而“推大/推小”
	// 节点的实际宽度，表现为节点在拖动时向左扩/缩。
	// 这里改为稳定的纯文本布局（不依赖动态宽度），避免节点宽度抖动。
	ImGui::AlignTextToFramePadding();

	const float iconSize = ImGui::GetTextLineHeight();
	const bool filled = ed::PinHadAnyLinks(id);

	// 输出 pin：Icon 显示在文字右边
	ImGui::Text("%s", label);
	ImGui::SameLine();
	ax::Widgets::Icon(
		ImVec2(iconSize, iconSize),
		GetSlotIconType(slotType),
		filled,
		GetSlotIconColor(slotType)
	);

	ed::EndPin();
}

void BlueprintNodeBuilder::MiddleContent(std::function<void()> func)
{
	ImGui::Spacing();
	func();
	ImGui::Spacing();
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
