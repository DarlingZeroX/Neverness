/*
 * DialogueList 节点数据结构与 JSON 序列化（VGGalgameNodeGraph）
 *
 * 中文说明：
 * - 节点图运行时槽位类型 Runtime::Value 不支持嵌套对象/数组，因此「一整段对白列表」在资产侧
 *   以 JSON 字符串形式挂在 EditorGraph 的 properties 上，经 GraphCompiler 写入槽位，
 *   运行时在 DialogueListExecute 中再反序列化为本头文件定义的结构体。
 * - 命名空间保持 VisionGal::Runtime，与 HNGRuntimeCore 节点执行上下文中的类型习惯一致，
 *   避免与 GalGame 业务层命名冲突。
 */
#pragma once

#include <string>
#include <vector>

#include <HCore/Include/File/nlohmann/json.hpp>

namespace VisionGal::Runtime
{
	/** 中文：对白行可选的立绘/文本动效类型（与 JSON 中 animation 字段对应）。 */
	enum class DialogueAnimation : int
	{
		FadeIn = 0,
		Move = 1,
		Shake = 2
	};

	/** 中文：单条对白在「表现层」上的附加参数（坐标、动效、时长等）。 */
	struct DialoguePresentation
	{
		bool usePosition = false;
		float positionX = 0.0f;
		float positionY = 0.0f;

		bool useAnimation = false;
		DialogueAnimation animation = DialogueAnimation::FadeIn;

		float duration = 0.2f;
	};

	/** 中文：对白列表中的一行（说话人、正文、立绘/音频、表现参数、行级事件名列表）。 */
	struct DialogueLine
	{
		std::string speakerId;     // 角色ID（如 alice）
		std::string text;          // 对话文本（支持变量表达式：由渲染层/脚本层决定）

		// 表现层（立绘）
		std::string characterId;
		std::string expression;

		// 音频
		std::string audioClip;

		// 表现控制（核心）
		DialoguePresentation presentation;

		// 行级事件（非 Exec pin）：字符串列表，序列化进 JSON 供脚本/渲染扩展读取
		std::vector<std::string> events;
	};

	/** 中文：DialogueList 节点在内存中的完整载荷（多行对白）。 */
	struct DialogueListNode
	{
		std::vector<DialogueLine> lines;
	};

	// ----------------------------
	// JSON 序列化/反序列化（供编辑器预览与运行时共用）
	// ----------------------------
	inline nlohmann::json SerializeDialoguePresentation(const DialoguePresentation& p)
	{
		const char* animName = "FadeIn";
		switch (p.animation)
		{
		case DialogueAnimation::FadeIn: animName = "FadeIn"; break;
		case DialogueAnimation::Move: animName = "Move"; break;
		case DialogueAnimation::Shake: animName = "Shake"; break;
		}

		return nlohmann::json{
			{ "usePosition", p.usePosition },
			{ "position", { p.positionX, p.positionY } },
			{ "useAnimation", p.useAnimation },
			{ "animation", animName },
			{ "duration", p.duration }
		};
	}

	inline DialoguePresentation DeserializeDialoguePresentation(const nlohmann::json& j)
	{
		DialoguePresentation out;
		out.usePosition = j.value("usePosition", false);

		if (j.contains("position") && j["position"].is_array() && j["position"].size() >= 2)
		{
			out.positionX = j["position"][0].get<float>();
			out.positionY = j["position"][1].get<float>();
		}

		out.useAnimation = j.value("useAnimation", false);

		// animation 支持 string（推荐）与 int（兼容）
		if (j.contains("animation"))
		{
			if (j["animation"].is_string())
			{
				const std::string s = j["animation"].get<std::string>();
				if (s == "FadeIn") out.animation = DialogueAnimation::FadeIn;
				else if (s == "Move") out.animation = DialogueAnimation::Move;
				else if (s == "Shake") out.animation = DialogueAnimation::Shake;
			}
			else
			{
				const int anim = j.value("animation", static_cast<int>(DialogueAnimation::FadeIn));
				out.animation = static_cast<DialogueAnimation>(anim);
			}
		}
		out.duration = j.value("duration", out.duration);
		return out;
	}

	inline nlohmann::json SerializeDialogueLine(const DialogueLine& ln)
	{
		return nlohmann::json{
			{ "speakerId", ln.speakerId },
			{ "text", ln.text },
			{ "characterId", ln.characterId },
			{ "expression", ln.expression },
			{ "audioClip", ln.audioClip },
			{ "presentation", SerializeDialoguePresentation(ln.presentation) },
			{ "events", ln.events }
		};
	}

	inline DialogueLine DeserializeDialogueLine(const nlohmann::json& j)
	{
		DialogueLine out;
		out.speakerId = j.value("speakerId", std::string{});
		out.text = j.value("text", std::string{});
		out.characterId = j.value("characterId", std::string{});
		out.expression = j.value("expression", std::string{});
		out.audioClip = j.value("audioClip", std::string{});
		if (j.contains("presentation") && j["presentation"].is_object())
		{
			out.presentation = DeserializeDialoguePresentation(j["presentation"]);
		}
		if (j.contains("events") && j["events"].is_array())
		{
			for (const auto& evt : j["events"])
			{
				if (evt.is_string())
					out.events.push_back(evt.get<std::string>());
			}
		}
		return out;
	}

	inline nlohmann::json SerializeDialogueListNode(const DialogueListNode& node)
	{
		nlohmann::json root;
		root["type"] = "DialogueList";
		root["lines"] = nlohmann::json::array();
		for (const auto& ln : node.lines)
			root["lines"].push_back(SerializeDialogueLine(ln));
		return root;
	}

	inline DialogueListNode DeserializeDialogueListNode(const nlohmann::json& j)
	{
		DialogueListNode out;
		if (!j.is_object()) return out;
		if (j.contains("lines") && j["lines"].is_array())
		{
			for (const auto& ln : j["lines"])
			{
				if (ln.is_object())
					out.lines.push_back(DeserializeDialogueLine(ln));
			}
		}
		return out;
	}

	inline std::string SerializeDialogueListNodeToString(const DialogueListNode& node)
	{
		return SerializeDialogueListNode(node).dump();
	}

	inline DialogueListNode DeserializeDialogueListNodeFromString(const std::string& jsonStr)
	{
		DialogueListNode out;
		if (jsonStr.empty()) return out;
		try
		{
			const auto j = nlohmann::json::parse(jsonStr);
			return DeserializeDialogueListNode(j);
		}
		catch (...)
		{
			return out;
		}
	}
}
