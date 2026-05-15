/*
 * ISequenceAction — 通用序列图「可执行单元」契约（Phase 8）
 *
 * 中文：Sequence 内核只认 Execute/Suspend/Resume/Cancel；具体对白 / 资源 / 条件由宿主注册实现。
 * 当前为首批接口拆分，与 IRuntimeExecutionServices 配合逐步替代节点内硬编码。
 */

#pragma once

namespace VisionGal::GalGame
{
	struct IRuntimeExecutionServices;

	struct ISequenceAction
	{
		virtual ~ISequenceAction() = default;

		virtual void Execute(IRuntimeExecutionServices& services) = 0;
		virtual void Suspend() {}
		virtual void Resume() {}
		virtual void Cancel() {}
	};
}
