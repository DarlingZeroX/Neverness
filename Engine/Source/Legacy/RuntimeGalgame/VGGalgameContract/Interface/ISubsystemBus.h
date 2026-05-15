/*
 * ISubsystemBus — Galgame 子系统总线（服务定位器）（Phase 8：Contract）
 *
 * 取代在 IGalGameEngine 上直接暴露 ShowSprite / PlayAudio / Get*System 等上帝接口；
 * 宿主 GalGameEngine 持有具体实现并返回本接口指针。
 *
 * SubsystemBus owns NO lifetime. It is a pure dispatcher facade.
 * 中文：总线不拥有各子系统对象生命周期；Scene()/Dialogue() 等返回的指针由宿主引擎创建与销毁，
 * 总线仅负责按名称分发访问，不在此接口内实现具体玩法逻辑。
 *
 * Phase 8：与 `SubsystemBusSnapshot` 一并置于 **VGGalgameContract**，不依赖存档或 Lua 类型。
 */

#pragma once
#include "../Include/SubsystemBusSnapshot.h"

namespace VisionGal::GalGame
{
	struct ISceneSubsystem;
	struct IUISubsystem;
	struct IAudioSubsystem;
	struct IScriptSubsystem;
	struct IArchiveSubsystem;
	struct IDialogueSubsystem;
	struct IPlaybackSubsystem;

	struct ISubsystemBus
	{
		virtual ~ISubsystemBus() = default;

		/// 预留：Sequence 回放 / 调试；默认实现返回空快照，不复制子系统。
		[[nodiscard]] virtual SubsystemBusSnapshot Snapshot() const { return {}; }

		/// 预留：与 Snapshot 配对；默认空操作。
		virtual void Restore(const SubsystemBusSnapshot&) {}

		virtual ISceneSubsystem* Scene() = 0;
		virtual IUISubsystem* UI() = 0;
		virtual IAudioSubsystem* Audio() = 0;
		virtual IScriptSubsystem* Script() = 0;
		virtual IArchiveSubsystem* Archive() = 0;
		virtual IDialogueSubsystem* Dialogue() = 0;
		/// 中文：Wait / 节拍；与 IScriptSubsystem::Wait 语义分离后的统一入口（Phase 8）。
		virtual IPlaybackSubsystem* Playback() = 0;
	};
}
