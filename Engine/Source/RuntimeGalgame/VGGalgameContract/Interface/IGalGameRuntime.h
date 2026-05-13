/*
 * IGalGameRuntime — Gal 多子域运行时聚合入口（Phase 8 Contract）
 *
 * 中文：将「执行 / 存档 / 播放 / 变量」从 IGalGameEngine 上帝对象中剥离；
 * 宿主通过 GetRuntime() 暴露，各域指针由实现装配，未实现子域可返回 nullptr。
 *
 * Phase 11 前本接口仍可演进；勿标注为最终冻结 ABI。
 */

#pragma once

namespace VisionGal::GalGame
{
	struct IStoryScriptSystem;
	struct IArchiveSystem;
	struct IPlaybackSubsystem;
	struct IVariableRuntime;

	struct IGalGameRuntime
	{
		virtual ~IGalGameRuntime() = default;

		/// 中文：剧情脚本执行子系统（当前即 IStoryScriptSystem，后续可收窄为 IExecutionRuntime）。
		virtual IStoryScriptSystem* GetExecutionRuntime() noexcept = 0;

		/// 中文：存档读写子系统（磁盘槽位、元数据等）。
		virtual IArchiveSystem* GetSaveRuntime() noexcept = 0;

		/// 中文：Wait / 节拍相关（见 IPlaybackSubsystem）。
		virtual IPlaybackSubsystem* GetPlaybackRuntime() noexcept = 0;

		/// 中文：变量子域；占位阶段可恒为 nullptr。
		virtual IVariableRuntime* GetVariableRuntime() noexcept = 0;
	};
}
