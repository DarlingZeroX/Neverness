/*
* DialogueListEditorPanel 实现
*/

#include "DialogueListEditorPanel.h"

#include <algorithm>
#include <cstdio>
#include <string>

#include <VGGalgameRuntime/Include/DialogueListNodeData.h>

// ImGui
#include <VGImgui/Include/imgui/imgui.h>

namespace VisionGal::Editor
{
	using namespace Horizon::NodeGraphEditor;
	using namespace Horizon::NodeGraphRuntime;
	using namespace VisionGal::Runtime;

	DialogueListEditorPanel::~DialogueListEditorPanel() = default;

	static DialogueListEditorPanel* g_DialogueListPanel = nullptr;

	void SetDialogueListEditorPanelInstance(DialogueListEditorPanel* panel)
	{
		g_DialogueListPanel = panel;
	}

	DialogueListEditorPanel* GetDialogueListEditorPanelInstance()
	{
		return g_DialogueListPanel;
	}

	void DialogueListEditorPanel::ResetPreview()
	{
		m_PreviewRunning = false;
		m_PreviewIndex = 0;
		m_PreviewTyped = 0;
	}

	void DialogueListEditorPanel::Open(EditorNode* node)
	{
		if (!node)
			return;

		m_IsOpen = true;
		m_CurrentNodeId = node->id;
		m_CurrentNode = node;

		if (!m_Cache)
			m_Cache = std::make_unique<DialogueListNodeCache>();

		// 载入节点数据
		EnsureCacheLoadedFromNode();
		ResetPreview();
	}

	void DialogueListEditorPanel::Close()
	{
		m_IsOpen = false;
		m_CurrentNodeId = ax::NodeEditor::NodeId{};
		m_CurrentNode = nullptr;
		ResetPreview();

		if (m_Cache)
		{
			m_Cache->data = DialogueListNode{};
			m_Cache->dirty = false;
		}
	}

	void DialogueListEditorPanel::EnsureCacheLoadedFromNode()
	{
		if (!m_CurrentNode)
			return;

		constexpr const char* PROP_KEY = "dialogueListJson";
		Value& v = m_CurrentNode->GetProperty(PROP_KEY);
		if (v.type != ValueType::String)
			v = Value::FromString(v.AsString());

		m_Cache->data = DeserializeDialogueListNodeFromString(v.AsString());
		if (m_Cache->data.lines.empty())
		{
			// 保证至少有一行，避免编辑器 UI 空引用
			m_Cache->data.lines.push_back(DialogueLine{});
		}
		m_Cache->dirty = false;
	}

	void DialogueListEditorPanel::SaveIfDirty(EditorGraph& graph)
	{
		if (!m_CurrentNode || !m_Cache || !m_Cache->dirty)
			return;

		constexpr const char* PROP_KEY = "dialogueListJson";
		Value& v = m_CurrentNode->GetProperty(PROP_KEY);
		v = Value::FromString(SerializeDialogueListNodeToString(m_Cache->data));
		graph.dirty = true;

		m_Cache->dirty = false;
	}

	void DialogueListEditorPanel::Draw(EditorGraph& graph)
	{
		if (!m_IsOpen)
			return;

		// 防御：节点可能被删除或图被重建，尝试按 id 重新获取
		m_CurrentNode = graph.FindNode(m_CurrentNodeId);
		if (!m_CurrentNode)
		{
			Close();
			return;
		}

		if (!m_Cache)
			m_Cache = std::make_unique<DialogueListNodeCache>();

		// 窗口 UI
		bool open = true;
		ImGui::SetNextWindowSize(ImVec2(860, 760), ImGuiCond_FirstUseEver);
		if (!ImGui::Begin("Dialogue Editor", &open))
		{
			ImGui::End();
			if (!open) Close();
			return;
		}

		// 若 cache 为空（首次打开或被 Close 清空），从节点加载一次
		if (m_Cache->data.lines.empty())
			EnsureCacheLoadedFromNode();

		DialogueListNode& data = m_Cache->data;

		// ----------------------------
		// 预览：面板内最小打字机模拟
		// ----------------------------
		if (ImGui::Button("Preview"))
		{
			m_PreviewRunning = true;
			m_PreviewIndex = 0;
			m_PreviewTyped = 0;
		}
		ImGui::SameLine();
		if (ImGui::Button("Stop"))
		{
			m_PreviewRunning = false;
		}

		const auto& io = ImGui::GetIO();
		if (m_PreviewRunning && !data.lines.empty() && m_PreviewIndex < data.lines.size())
		{
			const DialogueLine& pln = data.lines[m_PreviewIndex];
			const size_t fullLen = pln.text.size();
			const size_t add = static_cast<size_t>(io.DeltaTime * 40.0f);
			if (add > 0 && m_PreviewTyped < fullLen)
				m_PreviewTyped = std::min(fullLen, m_PreviewTyped + add);

			ImGui::Separator();
			ImGui::TextWrapped("Preview: %s", pln.speakerId.empty() ? "?" : pln.speakerId.c_str());
			ImGui::TextWrapped("%s", pln.text.substr(0, m_PreviewTyped).c_str());

			if (m_PreviewTyped >= fullLen && ImGui::Button("Next Line"))
			{
				++m_PreviewIndex;
				m_PreviewTyped = 0;
				if (m_PreviewIndex >= data.lines.size())
					m_PreviewRunning = false;
			}
		}

		// ----------------------------
		// 行级编辑：添加/删除/排序/字段
		// ----------------------------
		ImGui::Separator();
		ImGui::TextUnformatted("Lines:");
		for (int i = 0; i < static_cast<int>(data.lines.size()); ++i)
		{
			ImGui::PushID(i);
			auto& ln = data.lines[static_cast<size_t>(i)];

			ImGui::BeginGroup();
			ImGui::Separator();

			ImGui::TextUnformatted("---- Line ----");
			ImGui::SameLine();
			if (ImGui::Button("Up") && i > 0)
			{
				std::swap(data.lines[static_cast<size_t>(i)], data.lines[static_cast<size_t>(i - 1)]);
				m_Cache->dirty = true;
				ImGui::EndGroup();
				ImGui::PopID();
				continue;
			}
			ImGui::SameLine();
			if (ImGui::Button("Down") && (i + 1) < static_cast<int>(data.lines.size()))
			{
				std::swap(data.lines[static_cast<size_t>(i)], data.lines[static_cast<size_t>(i + 1)]);
				m_Cache->dirty = true;
				ImGui::EndGroup();
				ImGui::PopID();
				continue;
			}
			ImGui::SameLine();
			if (ImGui::Button("Del") && data.lines.size() > 1)
			{
				data.lines.erase(data.lines.begin() + i);
				m_Cache->dirty = true;
				ImGui::EndGroup();
				ImGui::PopID();
				--i;
				continue;
			}

			char bufSpeaker[128];
			std::snprintf(bufSpeaker, sizeof(bufSpeaker), "%s", ln.speakerId.c_str());
			if (ImGui::InputText("SpeakerId", bufSpeaker, sizeof(bufSpeaker)))
			{
				ln.speakerId = bufSpeaker;
				m_Cache->dirty = true;
			}

			char bufText[1024];
			std::snprintf(bufText, sizeof(bufText), "%s", ln.text.c_str());
			if (ImGui::InputText("Text", bufText, sizeof(bufText)))
			{
				ln.text = bufText;
				m_Cache->dirty = true;
			}

			char bufChar[128];
			std::snprintf(bufChar, sizeof(bufChar), "%s", ln.characterId.c_str());
			if (ImGui::InputText("CharacterId", bufChar, sizeof(bufChar)))
			{
				ln.characterId = bufChar;
				m_Cache->dirty = true;
			}

			char bufExpr[128];
			std::snprintf(bufExpr, sizeof(bufExpr), "%s", ln.expression.c_str());
			if (ImGui::InputText("Expression", bufExpr, sizeof(bufExpr)))
			{
				ln.expression = bufExpr;
				m_Cache->dirty = true;
			}

			char bufAudio[256];
			std::snprintf(bufAudio, sizeof(bufAudio), "%s", ln.audioClip.c_str());
			if (ImGui::InputText("AudioClip", bufAudio, sizeof(bufAudio)))
			{
				ln.audioClip = bufAudio;
				m_Cache->dirty = true;
			}

			//ImGui::Separator();
			ImGui::TextUnformatted("Presentation:");

			bool usePos = ln.presentation.usePosition;
			if (ImGui::Checkbox("UsePosition", &usePos))
			{
				ln.presentation.usePosition = usePos;
				m_Cache->dirty = true;
			}
			if (ln.presentation.usePosition)
			{
				float pos[2] = { ln.presentation.positionX, ln.presentation.positionY };
				if (ImGui::InputFloat2("Position", pos))
				{
					ln.presentation.positionX = pos[0];
					ln.presentation.positionY = pos[1];
					m_Cache->dirty = true;
				}
			}

			bool useAnim = ln.presentation.useAnimation;
			if (ImGui::Checkbox("UseAnimation", &useAnim))
			{
				ln.presentation.useAnimation = useAnim;
				m_Cache->dirty = true;
			}
			if (ln.presentation.useAnimation)
			{
				int anim = static_cast<int>(ln.presentation.animation);
				const char* items[] = { "FadeIn", "Move", "Shake" };
				if (ImGui::Combo("Animation", &anim, items, 3))
				{
					ln.presentation.animation = static_cast<DialogueAnimation>(anim);
					m_Cache->dirty = true;
				}

				float dur = ln.presentation.duration;
				if (ImGui::InputFloat("Duration", &dur))
				{
					ln.presentation.duration = dur;
					m_Cache->dirty = true;
				}
			}

			//ImGui::Separator();
			ImGui::TextUnformatted("Events:");
			for (int e = 0; e < static_cast<int>(ln.events.size()); ++e)
			{
				ImGui::PushID(e);
				char bufEvt[128];
				std::snprintf(bufEvt, sizeof(bufEvt), "%s", ln.events[static_cast<size_t>(e)].c_str());
				ImGui::TextUnformatted("Event");
				ImGui::SameLine();
				if (ImGui::InputText("##evt", bufEvt, sizeof(bufEvt)))
				{
					ln.events[static_cast<size_t>(e)] = bufEvt;
					m_Cache->dirty = true;
				}
				ImGui::SameLine();
				if (ImGui::Button("X"))
				{
					ln.events.erase(ln.events.begin() + e);
					m_Cache->dirty = true;
					ImGui::PopID();
					--e;
					continue;
				}
				ImGui::PopID();
			}

			static char s_EventAddBuf[128] = {};
			ImGui::InputText("AddEvent", s_EventAddBuf, sizeof(s_EventAddBuf));
			ImGui::SameLine();
			if (ImGui::Button("+"))
			{
				if (s_EventAddBuf[0] != '\0')
				{
					ln.events.push_back(s_EventAddBuf);
					s_EventAddBuf[0] = '\0';
					m_Cache->dirty = true;
				}
			}

			ImGui::EndGroup();
			ImGui::PopID();
		}

		if (ImGui::Button("+ Add Line"))
		{
			data.lines.push_back(DialogueLine{});
			m_Cache->dirty = true;
		}

		// 即时保存：一旦 dirty 立即写回节点 properties
		SaveIfDirty(graph);

		ImGui::End();

		if (!open)
			Close();
	}
}

