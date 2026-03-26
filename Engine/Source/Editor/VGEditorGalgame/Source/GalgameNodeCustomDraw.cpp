/*
* Galgame 节点自定义 UI 绘制实现（业务层）
*/

#include "GalgameNodeCustomDraw.h"

#include <algorithm>
#include <cstdio>

#include <VGGalgameRuntime/Include/DialogueListNodeData.h>
#include "DialogueListEditorPanel.h"

// ImGui / ImNodeEditor
#include <VGImgui/Include/imgui/imgui.h>
#include <VGImgui/Include/imgui/imgui_internal.h>
#include <VGImgui/Include/ImNodeEditor/imgui_node_editor.h>

namespace VisionGal::Editor
{
	using namespace Horizon::NodeGraphEditor;
	using namespace Horizon::NodeGraphRuntime;
	using namespace VisionGal::Runtime;

	void DrawDialogueListCustomUI(EditorGraph& graph, EditorNode& node, const RuntimeContext* /*runtimeCtx*/)
	{
		constexpr const char* PROP_KEY = "dialogueListJson";

		// 仅对拥有该属性的节点启用（避免误注册/误调用）
		auto itProp = node.properties.find(PROP_KEY);
		if (itProp == node.properties.end())
			return;

		Value& dlgValue = node.GetProperty(PROP_KEY);
		if (dlgValue.type != ValueType::String)
			dlgValue = Value::FromString(dlgValue.AsString());

		DialogueListNode data = DeserializeDialogueListNodeFromString(dlgValue.AsString());
		if (data.lines.empty())
			data.lines.push_back(DialogueLine{});

		// ------------------------------------------------------------
		// 1) Node 上预览：只显示前两行
		// ------------------------------------------------------------
		ImGui::TextUnformatted("DialogueList (Preview):");
		const int showCount = (data.lines.size() >= 2) ? 2 : static_cast<int>(data.lines.size());
		for (int i = 0; i < showCount; ++i)
		{
			const auto& ln = data.lines[static_cast<size_t>(i)];
			std::string lineText = ln.speakerId;
			if (!ln.text.empty())
			{
				if (!lineText.empty()) lineText += ": ";
				lineText += ln.text;
			}
			ImGui::TextWrapped("%s", lineText.c_str());
		}
		if (data.lines.size() > 2)
			ImGui::TextUnformatted("...");

		// 2) 节点内只保留按钮：点击后由 NGEditorGalgame 的面板打开独立窗口
		if (ImGui::Button("Edit Dialogue"))
		{
			if (DialogueListEditorPanel* panel = GetDialogueListEditorPanelInstance())
			{
				panel->Open(&node);
			}
		}
	}
}

