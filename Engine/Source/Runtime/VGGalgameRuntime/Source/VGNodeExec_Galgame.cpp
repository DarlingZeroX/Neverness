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

	// ----------------------------
	// DialogueListState：逐帧/可中断播放状态
	// ----------------------------
	struct DialogueListState
	{
		bool initialized = false;
		size_t index = 0;

		// typewriter progress
		size_t typedChars = 0;
		bool linePresented = false; // 当前行表现是否已“触发”（变量写入）

		DialogueListNode nodeData;
	};

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

		// 1) 初始化：解析 DialogueListNodeData（JSON 字符串）
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
			// 空对话：直接触发 Next
			const SLOT_ID nextSlotId = FindOutputSlot(*ctx.graph, *node, "Next");
			if (nextSlotId != 0) PushExec(ctx, nextSlotId);
			return ExecResult::Finished;
		}

		const DialogueLine& curLine = lines[state.index];

		// 2) 读取 Next（玩家点击继续/跳过）
		bool readyNext = false;
		auto itNext = ctx.variables.find("Next");
		if (itNext != ctx.variables.end() && itNext->second.type == ValueType::Bool)
			readyNext = itNext->second.AsBool();

		// 3) 如果这一行尚未触发表现：写入运行时变量（供 Preview/Renders/扩展层读取）
		if (!state.linePresented)
		{
			ctx.variables[Vars::CurrentSpeaker] = Value::FromString(curLine.speakerId);
			ctx.variables[Vars::CurrentCharacterId] = Value::FromString(curLine.characterId);
			ctx.variables[Vars::CurrentExpression] = Value::FromString(curLine.expression);
			ctx.variables[Vars::CurrentAudioClip] = Value::FromString(curLine.audioClip);

			// 行级事件：以 JSON 字符串形式挂到变量，便于后续渲染层/脚本层取用
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

		// 4) 自动打字机推进（每帧）
		if (state.typedChars < fullLen && !readyNext)
		{
			// chars per second：可按 presentation/duration 扩展（此处用固定值）
			const float charsPerSecond = 40.0f;
			const float dt = (ctx.deltaTime > 0.0f) ? ctx.deltaTime : 0.016f;
			const size_t add = static_cast<size_t>(dt * charsPerSecond);
			if (add > 0) state.typedChars = std::min(fullLen, state.typedChars + add);
		}

		// 5) 写入当前可见文本（typewriter）
		const std::string visible = curLine.text.substr(0, static_cast<size_t>(state.typedChars));
		ctx.variables[Vars::CurrentText] = Value::FromString(visible);

		// 6) 若玩家按了 Next：先跳过打字机，再进入下一行；最后触发 Next exec
		if (readyNext)
		{
			// 消费 Next
			ctx.variables["Next"] = Value::FromBool(false);

			// 1) 若当前行未打完：直接补全文本，不推进 index
			if (state.typedChars < fullLen)
			{
				state.typedChars = fullLen;
				const std::string fullVisible = curLine.text;
				ctx.variables[Vars::CurrentText] = Value::FromString(fullVisible);
				return ExecResult::Running;
			}

			// 2) 已打完：推进到下一行或结束
			state.index++;
			state.typedChars = 0;
			state.linePresented = false;

			if (state.index < lines.size())
			{
				// 继续运行，让下一帧触发表现/显示
				return ExecResult::Running;
			}

			// 结束：激活输出 Exec "Next"
			const SLOT_ID nextSlotId = FindOutputSlot(*ctx.graph, *node, "Next");
			if (nextSlotId != 0) PushExec(ctx, nextSlotId);
			// 可选：重置 index 以便 Stop/Play 后的行为更一致
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

