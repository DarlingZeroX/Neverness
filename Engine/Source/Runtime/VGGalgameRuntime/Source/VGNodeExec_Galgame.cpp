/*
* Galgame NodeGraph Runtime 执行逻辑（VGGalgameRuntime）
*
* 说明：
* - 该文件只包含“运行时执行逻辑”，不依赖 Editor UI
* - 通过 RuntimeContext::nodeStates 保存 DialogueListState，实现跨帧推进
*/

#include "VGNodeExec_Galgame.h"

#include <vector>

#include <HNGRuntimeCore/Include/RuntimeGraph.h>

namespace VisionGal::Runtime
{
	using namespace Horizon::NodeGraphRuntime;

	namespace Vars
	{
		const char* CurrentSpeaker = "__CurrentSpeaker";
		const char* CurrentText = "__CurrentText";
	}

	static constexpr const char* PIN_Text = "Text"; // DialogueList -> Text

	// ----------------------------
	// DialogueListState（封装在 Runtime 模块）
	// ----------------------------
	struct DialogueListLine
	{
		std::string speaker;
		std::string text;
	};

	struct DialogueListState
	{
		bool initialized = false;
		size_t currentLine = 0;
		std::vector<DialogueListLine> lines;
	};

	// 将一行解析为：speaker + text
	// 约定：
	// - 优先支持 "speaker|text"
	// - 其次支持 "speaker:text"
	// - 若无分隔符：speaker 为空，text 为整行
	static DialogueListLine ParseDialogueLine(const std::string& line)
	{
		DialogueListLine out;

		auto pos = line.find('|');
		if (pos != std::string::npos)
		{
			out.speaker = line.substr(0, pos);
			out.text = line.substr(pos + 1);
			return out;
		}

		pos = line.find(':');
		if (pos != std::string::npos)
		{
			out.speaker = line.substr(0, pos);
			out.text = line.substr(pos + 1);
			return out;
		}

		out.text = line;
		return out;
	}

	ExecResult EntryExecute(RuntimeContext& ctx, NODE_ID nodeIndex)
	{
		if (!ctx.graph) return ExecResult::Finished;
		RuntimeNode* node = GetNodeById(*ctx.graph, nodeIndex);
		if (!node) return ExecResult::Finished;

		// 激活所有 Exec 类型输出槽
		for (uint32_t i = 0; i < node->outputsCount; ++i)
		{
			const uint32_t outIndex = node->outputsBegin + i;
			RuntimeSlot& outSlot = ctx.graph->slots[outIndex];
			if (outSlot.type == SlotType::Exec)
				PushExec(ctx, outSlot.id);
		}

		return ExecResult::Finished;
	}

	ExecResult DialogueListExecute(RuntimeContext& ctx, NODE_ID nodeIndex)
	{
		if (!ctx.graph) return ExecResult::Finished;
		RuntimeNode* node = GetNodeById(*ctx.graph, nodeIndex);
		if (!node) return ExecResult::Finished;

		DialogueListState& state = ctx.GetOrCreateState<DialogueListState>(nodeIndex);

		// 读取 Text
		const SLOT_ID textSlotId = FindOutputSlot(*ctx.graph, *node, PIN_Text);
		if (textSlotId == 0 || textSlotId >= ctx.graph->slots.size())
			return ExecResult::Finished;

		const RuntimeSlot& textSlot = ctx.graph->slots[textSlotId];
		if (textSlot.value.type != ValueType::String)
			return ExecResult::Finished;

		const std::string& fullText = textSlot.value.AsString();

		// 初始化：拆分多行
		if (!state.initialized)
		{
			state.lines.clear();
			std::string current;
			for (char c : fullText)
			{
				if (c == '\n')
				{
					state.lines.push_back(ParseDialogueLine(current));
					current.clear();
				}
				else if (c != '\r')
				{
					current.push_back(c);
				}
			}
			state.lines.push_back(ParseDialogueLine(current));

			state.currentLine = 0;
			state.initialized = true;
		}

		if (state.lines.empty())
			return ExecResult::Finished;

		// 是否准备推进
		bool readyNext = false;
		auto itNext = ctx.variables.find("Next");
		if (itNext != ctx.variables.end() && itNext->second.type == ValueType::Bool)
			readyNext = itNext->second.AsBool();

		// 写入当前行到 RuntimeContext.variables（供 Preview/UI 读取）
		auto writeCurrent = [&]()
		{
			if (state.currentLine >= state.lines.size()) return;
			const auto& ln = state.lines[state.currentLine];
			ctx.variables[Vars::CurrentSpeaker] = Value::FromString(ln.speaker);
			ctx.variables[Vars::CurrentText] = Value::FromString(ln.text);
		};

		if (!readyNext)
		{
			writeCurrent();
			return ExecResult::Running;
		}

		// 消费 Next 并推进
		ctx.variables["Next"] = Value::FromBool(false);
		if (state.currentLine + 1 < state.lines.size())
		{
			++state.currentLine;
			writeCurrent();
			return ExecResult::Running;
		}

		// 播放完毕：激活输出 Exec "Next"
		const SLOT_ID nextSlotId = FindOutputSlot(*ctx.graph, *node, "Next");
		if (nextSlotId != 0) PushExec(ctx, nextSlotId);
		return ExecResult::Finished;
	}

	ExecResult ChoiceExecute(RuntimeContext& ctx, NODE_ID nodeIndex)
	{
		if (!ctx.graph) return ExecResult::Finished;
		RuntimeNode* node = GetNodeById(*ctx.graph, nodeIndex);
		if (!node) return ExecResult::Finished;

		// 简化策略：默认 Option1（后续可替换为 UI/变量驱动）
		const SLOT_ID opt1 = FindOutputSlot(*ctx.graph, *node, "Option1");
		if (opt1 != 0) PushExec(ctx, opt1);
		return ExecResult::Finished;
	}

	ExecResult ShowCharacterExecute(RuntimeContext& ctx, NODE_ID nodeIndex)
	{
		if (!ctx.graph) return ExecResult::Finished;
		RuntimeNode* node = GetNodeById(*ctx.graph, nodeIndex);
		if (!node) return ExecResult::Finished;

		auto readString = [&](const std::string& outName) -> std::string
		{
			const SLOT_ID sid = FindOutputSlot(*ctx.graph, *node, outName);
			if (sid == 0 || sid >= ctx.graph->slots.size()) return {};
			const RuntimeSlot& s = ctx.graph->slots[sid];
			return (s.value.type == ValueType::String) ? s.value.AsString() : std::string{};
		};

		const std::string name = readString("Name");
		const std::string expr = readString("Expression");
		const std::string pos = readString("Position");

		ctx.variables["__CharacterName"] = Value::FromString(name);
		ctx.variables["__CharacterExpression"] = Value::FromString(expr);
		ctx.variables["__CharacterPosition"] = Value::FromString(pos);

		const SLOT_ID outSlotId = FindOutputSlot(*ctx.graph, *node, "Out");
		if (outSlotId != 0) PushExec(ctx, outSlotId);
		return ExecResult::Finished;
	}

	ExecResult PlayBGMExecute(RuntimeContext& ctx, NODE_ID nodeIndex)
	{
		if (!ctx.graph) return ExecResult::Finished;
		RuntimeNode* node = GetNodeById(*ctx.graph, nodeIndex);
		if (!node) return ExecResult::Finished;

		const SLOT_ID bgmSlot = FindOutputSlot(*ctx.graph, *node, "BgmName");
		std::string bgm;
		if (bgmSlot != 0 && bgmSlot < ctx.graph->slots.size() &&
			ctx.graph->slots[bgmSlot].value.type == ValueType::String)
		{
			bgm = ctx.graph->slots[bgmSlot].value.AsString();
		}

		ctx.variables["__BgmName"] = Value::FromString(bgm);

		const SLOT_ID outSlotId = FindOutputSlot(*ctx.graph, *node, "Out");
		if (outSlotId != 0) PushExec(ctx, outSlotId);
		return ExecResult::Finished;
	}

	ExecResult SetBackgroundExecute(RuntimeContext& ctx, NODE_ID nodeIndex)
	{
		if (!ctx.graph) return ExecResult::Finished;
		RuntimeNode* node = GetNodeById(*ctx.graph, nodeIndex);
		if (!node) return ExecResult::Finished;

		const SLOT_ID bgSlot = FindOutputSlot(*ctx.graph, *node, "BackgroundName");
		std::string bg;
		if (bgSlot != 0 && bgSlot < ctx.graph->slots.size() &&
			ctx.graph->slots[bgSlot].value.type == ValueType::String)
		{
			bg = ctx.graph->slots[bgSlot].value.AsString();
		}

		ctx.variables["__BackgroundName"] = Value::FromString(bg);

		const SLOT_ID outSlotId = FindOutputSlot(*ctx.graph, *node, "Out");
		if (outSlotId != 0) PushExec(ctx, outSlotId);
		return ExecResult::Finished;
	}
}

