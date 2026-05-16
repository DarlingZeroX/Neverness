/*
 * Galgame 节点图运行时执行逻辑（VGGalgameNodeGraph）
 *
 * 中文说明：
 * - 本头文件仅声明「与 HNGRuntimeCore NodeExecuteFn 签名一致」的节点执行入口，
 *   不包含任何编辑器 UI；编辑器在 NodeRegistry 注册 NodeMeta 时绑定这些函数指针。
 * - 运行时通过 RuntimeContext::variables 与约定变量名（见 Vars 命名空间）向预览/UI/渲染层
 *   暴露当前对白状态；DialogueList 的逐行状态保存在 RuntimeContext::nodeStates 中（见 .cpp）。
 */
#pragma once

#include "VGGalgameNodeGraph/VGGalgameNodeGraphConfig.h"

#include <string>

#include <NNNodeGraphCore/Interface/Types.h>
#include <NNNodeGraphCore/Interface/Value.h>
#include <NNNodeGraphCore/Include/RuntimeContext.h>

namespace VisionGal::Runtime
{
	/**
	 * 中文：运行时变量名约定——预览或游戏层从 RuntimeContext.variables 读取当前对白快照。
	 * 与编辑器 Preview 使用的键名保持一致，便于无耦合串联。
	 */
	namespace Vars
	{
		VG_GALGAME_NODEGRAPH_API extern const char* CurrentSpeaker;
		VG_GALGAME_NODEGRAPH_API extern const char* CurrentText;

		VG_GALGAME_NODEGRAPH_API extern const char* CurrentCharacterId;
		VG_GALGAME_NODEGRAPH_API extern const char* CurrentExpression;

		VG_GALGAME_NODEGRAPH_API extern const char* CurrentAudioClip;
	}

	/**
	 * 中文：DialogueList 节点上存放「整表 JSON 字符串」的数据槽名；
	 * GraphCompiler 编译后该槽的字符串可由 DeserializeDialogueListNodeFromString 解析。
	 */
	static constexpr const char* PIN_LinesJson = "LinesJson";

	// ----------------------------
	// 节点执行函数（NodeExecuteFn）
	// ----------------------------
	// 中文：nodeIndex 在当前运行时实现中传入的是 NODE_ID（即节点 id），与 HNGRuntimeCore 文档一致。

	VG_GALGAME_NODEGRAPH_API Horizon::NodeGraphRuntime::ExecResult EntryExecute(
		Horizon::NodeGraphRuntime::RuntimeContext& ctx,
		Horizon::NodeGraphRuntime::NODE_ID nodeIndex);

	VG_GALGAME_NODEGRAPH_API Horizon::NodeGraphRuntime::ExecResult DialogueListExecute(
		Horizon::NodeGraphRuntime::RuntimeContext& ctx,
		Horizon::NodeGraphRuntime::NODE_ID nodeIndex);

	VG_GALGAME_NODEGRAPH_API Horizon::NodeGraphRuntime::ExecResult ChoiceExecute(
		Horizon::NodeGraphRuntime::RuntimeContext& ctx,
		Horizon::NodeGraphRuntime::NODE_ID nodeIndex);

	VG_GALGAME_NODEGRAPH_API Horizon::NodeGraphRuntime::ExecResult ShowCharacterExecute(
		Horizon::NodeGraphRuntime::RuntimeContext& ctx,
		Horizon::NodeGraphRuntime::NODE_ID nodeIndex);

	VG_GALGAME_NODEGRAPH_API Horizon::NodeGraphRuntime::ExecResult PlayBGMExecute(
		Horizon::NodeGraphRuntime::RuntimeContext& ctx,
		Horizon::NodeGraphRuntime::NODE_ID nodeIndex);

	VG_GALGAME_NODEGRAPH_API Horizon::NodeGraphRuntime::ExecResult SetBackgroundExecute(
		Horizon::NodeGraphRuntime::RuntimeContext& ctx,
		Horizon::NodeGraphRuntime::NODE_ID nodeIndex);
}
