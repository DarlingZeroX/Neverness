/*
 * Galgame 节点图运行时执行逻辑实现（VGGalgameNodeGraph）
 *
 * 中文：通过 RuntimeContext::GetOrCreateState 持久化 DialogueListState，实现跨帧打字机与玩家 Next。
 */

#include "VGNodeExec_Galgame.h"

#include <vector>

#include <NNNodeGraphCore/Include/RuntimeGraph.h>

#include "DialogueListNodeData.h"

namespace VisionGal::Runtime
{
	using namespace Horizon::NodeGraphRuntime;

	namespace Vars
	{
		const char* CurrentSpeaker = "__CurrentSpeaker";
		const char* CurrentText = "__CurrentText";
		const char* CurrentCharacterId = "__CurrentCharacterId";
		const char* CurrentExpression = "__CurrentExpression";
		const char* CurrentAudioClip = "__CurrentAudioClip";
	}

	/**
	 * 中文：DialogueList 节点在单图实例内的可中断播放状态。
	 * - index：当前播放到第几行；
	 * - typedChars：打字机已显示字符数；
	 * - linePresented：本行是否已把「表现层变量」写入 ctx（避免同一行重复触发）。
	 */
	struct DialogueListState
	{
		bool initialized = false;
		size_t index = 0;

		size_t typedChars = 0;
		bool linePresented = false;

		DialogueListNode nodeData;
	};

	ExecResult EntryExecute(RuntimeContext& ctx, NODE_ID nodeIndex)
	{
		if (!ctx.graph) return ExecResult::Finished;
		RuntimeNode* node = GetNodeById(*ctx.graph, nodeIndex);
		if (!node) return ExecResult::Finished;

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

		// 中文：首帧从 LinesJson 槽读取 JSON 字符串并反序列化
		if (!state.initialized)
		{
			const SLOT_ID linesSlotId = FindOutputSlot(*ctx.graph, *node, PIN_LinesJson);
			if (linesSlotId == 0 || linesSlotId >= ctx.graph->slots.size())
				return ExecResult::Finished;

			const RuntimeSlot& linesSlot = ctx.graph->slots[linesSlotId];
			if (linesSlot.value.type != ValueType::String)
				return ExecResult::Finished;

			const DialogueListNode data =
				DeserializeDialogueListNodeFromString(linesSlot.value.AsString());
			state.nodeData = std::move(data);
			state.index = 0;
			state.typedChars = 0;
			state.linePresented = false;
			state.initialized = true;
		}

		const auto& lines = state.nodeData.lines;
		if (lines.empty() || state.index >= lines.size())
		{
			const SLOT_ID nextSlotId = FindOutputSlot(*ctx.graph, *node, "Next");
			if (nextSlotId != 0) PushExec(ctx, nextSlotId);
			return ExecResult::Finished;
		}

		const DialogueLine& curLine = lines[state.index];

		// 中文：外部（UI）将 variables["Next"] 置 true 表示玩家请求继续/跳过打字机
		bool readyNext = false;
		auto itNext = ctx.variables.find("Next");
		if (itNext != ctx.variables.end() && itNext->second.type == ValueType::Bool)
			readyNext = itNext->second.AsBool();

		// 中文：新行首次进入时写入「当前对白」相关变量，供预览与渲染读取
		if (!state.linePresented)
		{
			ctx.variables[Vars::CurrentSpeaker] = Value::FromString(curLine.speakerId);
			ctx.variables[Vars::CurrentCharacterId] = Value::FromString(curLine.characterId);
			ctx.variables[Vars::CurrentExpression] = Value::FromString(curLine.expression);
			ctx.variables[Vars::CurrentAudioClip] = Value::FromString(curLine.audioClip);

			if (!curLine.events.empty())
			{
				nlohmann::json ev = curLine.events;
				ctx.variables["__CurrentDialogueEventsJson"] = Value::FromString(ev.dump());
			}
			else
			{
				ctx.variables["__CurrentDialogueEventsJson"] = Value::FromString(std::string{});
			}

			state.typedChars = 0;
			state.linePresented = true;
		}

		const size_t fullLen = curLine.text.size();

		// 中文：每帧推进打字机（未收到 Next 时）
		if (state.typedChars < fullLen && !readyNext)
		{
			const float charsPerSecond = 40.0f;
			const float dt = (ctx.deltaTime > 0.0f) ? ctx.deltaTime : 0.016f;
			const size_t add = static_cast<size_t>(dt * charsPerSecond);
			if (add > 0) state.typedChars = std::min(fullLen, state.typedChars + add);
		}

		const std::string visible = curLine.text.substr(0, static_cast<size_t>(state.typedChars));
		ctx.variables[Vars::CurrentText] = Value::FromString(visible);

		// 中文：收到 Next 时——先瞬间打完本行，再切下一行；最后一行打完则触发 Exec 出口 Next
		if (readyNext)
		{
			ctx.variables["Next"] = Value::FromBool(false);

			if (state.typedChars < fullLen)
			{
				state.typedChars = fullLen;
				const std::string fullVisible = curLine.text;
				ctx.variables[Vars::CurrentText] = Value::FromString(fullVisible);
				return ExecResult::Running;
			}

			state.index++;
			state.typedChars = 0;
			state.linePresented = false;

			if (state.index < lines.size())
			{
				return ExecResult::Running;
			}

			const SLOT_ID nextSlotId = FindOutputSlot(*ctx.graph, *node, "Next");
			if (nextSlotId != 0) PushExec(ctx, nextSlotId);
			state.index = 0;
			return ExecResult::Finished;
		}

		return ExecResult::Running;
	}

	ExecResult ChoiceExecute(RuntimeContext& ctx, NODE_ID nodeIndex)
	{
		if (!ctx.graph) return ExecResult::Finished;
		RuntimeNode* node = GetNodeById(*ctx.graph, nodeIndex);
		if (!node) return ExecResult::Finished;

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
