/*
* Galgame NodeGraph Runtime 执行逻辑（VGGalgameRuntime）
*
* 说明：
* - 该文件只包含“运行时执行逻辑”，不依赖 Editor UI
* - 供 Editor 层在 NodeRegistry::Register(NodeMeta{..., execute }) 时引用
*/
#pragma once

#include "../VGGalgameRuntimeConfig.h"

#include <string>

#include <HNGRuntimeCore/Interface/Types.h>
#include <HNGRuntimeCore/Interface/Value.h>
#include <HNGRuntimeCore/Include/RuntimeContext.h>

namespace VisionGal::Runtime
{
	// 运行时变量名约定（Preview/UI 从 RuntimeContext.variables 读取）
	namespace Vars
	{
		// 当前对白：角色名/对白文本
		VG_GALGAME_RUNTIME_API extern const char* CurrentSpeaker;
		VG_GALGAME_RUNTIME_API extern const char* CurrentText;

		// 当前对白表现：立绘/表情（用于 Editor Preview 或后续渲染层对接）
		VG_GALGAME_RUNTIME_API extern const char* CurrentCharacterId;
		VG_GALGAME_RUNTIME_API extern const char* CurrentExpression;

		// 当前对白音频（如果希望 Preview 播放/渲染层播放）
		VG_GALGAME_RUNTIME_API extern const char* CurrentAudioClip;
	}

	// DialogueList 节点数据槽名（GraphCompiler -> RuntimeSlot）
	// - 该槽存放 dialogueListJson（DialogueListNodeData 序列化后的 JSON 字符串）
	static constexpr const char* PIN_LinesJson = "LinesJson";

	// ----------------------------
	// 节点执行函数（NodeExecuteFn）
	// ----------------------------
	// 说明：
	// - 签名必须与 HNGRuntimeCore 的 NodeExecuteFn 一致
	// - nodeIndex 参数在当前运行时实现中传入的是 NODE_ID（即节点 id）
	VG_GALGAME_RUNTIME_API Horizon::NodeGraphRuntime::ExecResult EntryExecute(
		Horizon::NodeGraphRuntime::RuntimeContext& ctx,
		Horizon::NodeGraphRuntime::NODE_ID nodeIndex);

	VG_GALGAME_RUNTIME_API Horizon::NodeGraphRuntime::ExecResult DialogueListExecute(
		Horizon::NodeGraphRuntime::RuntimeContext& ctx,
		Horizon::NodeGraphRuntime::NODE_ID nodeIndex);

	VG_GALGAME_RUNTIME_API Horizon::NodeGraphRuntime::ExecResult ChoiceExecute(
		Horizon::NodeGraphRuntime::RuntimeContext& ctx,
		Horizon::NodeGraphRuntime::NODE_ID nodeIndex);

	VG_GALGAME_RUNTIME_API Horizon::NodeGraphRuntime::ExecResult ShowCharacterExecute(
		Horizon::NodeGraphRuntime::RuntimeContext& ctx,
		Horizon::NodeGraphRuntime::NODE_ID nodeIndex);

	VG_GALGAME_RUNTIME_API Horizon::NodeGraphRuntime::ExecResult PlayBGMExecute(
		Horizon::NodeGraphRuntime::RuntimeContext& ctx,
		Horizon::NodeGraphRuntime::NODE_ID nodeIndex);

	VG_GALGAME_RUNTIME_API Horizon::NodeGraphRuntime::ExecResult SetBackgroundExecute(
		Horizon::NodeGraphRuntime::RuntimeContext& ctx,
		Horizon::NodeGraphRuntime::NODE_ID nodeIndex);
}

