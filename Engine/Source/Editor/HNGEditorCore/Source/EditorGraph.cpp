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
#include "EditorGraph.h"
#include <HNGRuntimeCore/Include/Core/RuntimeContext.h>

namespace Horizon::NodeGraphEditor
{
	void HandleCreateLink(EditorGraph& graph)
	{
		using namespace ax::NodeEditor;
		BeginCreate();

		PinId startPinId, endPinId;
		if (QueryNewLink(&startPinId, &endPinId))
		{
			EditorPin* startPin = graph.FindPin(startPinId);
			EditorPin* endPin = graph.FindPin(endPinId);
			if (!startPin || !endPin)
			{
				RejectNewItem();
				EndCreate();
				return;
			}

			// 校验方向
			if (startPin->isInput == endPin->isInput)
			{
				RejectNewItem();
			}
			// 校验类型
			else if (startPin->type != endPin->type)
			{
				RejectNewItem();
			}
			else if (AcceptNewItem())
			{
				EditorPin* outputPin = startPin->isInput ? endPin : startPin;
				EditorPin* inputPin = startPin->isInput ? startPin : endPin;
				EditorLink link;
				link.id = ax::NodeEditor::LinkId(static_cast<int>(graph.links.size() + 1));
				link.startPinId = outputPin->id;
				link.endPinId = inputPin->id;
				graph.links.push_back(link);
				graph.dirty = true;
			}
		}
		EndCreate();
	}

	void DrawEditorGraph(EditorGraph& graph, const Horizon::NodeGraphRuntime::RuntimeContext* runtimeCtx)
	{
		using namespace ax::NodeEditor;
		//SetCurrentEditor(graph.context);
		graph.context->SetContext();
		Begin("Node Graph");

		// 绘制所有节点
		for (auto& node : graph.nodes)
		{
			const Horizon::NodeGraphRuntime::NODE_ID runtimeNodeId = static_cast<Horizon::NodeGraphRuntime::NODE_ID>(node.id.Get());
			const bool executed = runtimeCtx ? runtimeCtx->WasNodeExecuted(runtimeNodeId) : false;
			if (executed)
			{
				PushStyleColor(StyleColor_NodeBg, ImColor(70, 120, 255, 200));
				PushStyleColor(StyleColor_NodeBorder, ImColor(120, 180, 255, 255));
				//PushStyleColor(StyleColor_No, ImColor(0, 0, 0, 0));
			}

			BeginNode(node.id);
			ImGui::Text("%s", node.name.c_str());

			// 输入区
			for (auto& pin : node.inputs)
			{
				BeginPin(pin.id, PinKind::Input);
				ImGui::Text("%s", pin.name.c_str());
				EndPin();
			}

			ImGui::SameLine();

			// 输出区
			for (auto& pin : node.outputs)
			{
				BeginPin(pin.id, PinKind::Output);
				ImGui::Text("%s", pin.name.c_str());
				EndPin();
			}

			EndNode();

			if (executed)
			{
				PopStyleColor(2);
			}
		}

		// 绘制所有连线
		for (auto& link : graph.links)
		{
			Link(link.id, link.startPinId, link.endPinId);
		}

		// 处理新建连线
		HandleCreateLink(graph);

		End();
	}
}