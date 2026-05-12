/*
 * ISubsystemBus — Galgame 子系统总线（服务定位器）
 *
 * 取代在 IGalGameEngine 上直接暴露 ShowSprite / PlayAudio / Get*System 等上帝接口；
 * 宿主 GalGameEngine 持有具体实现并返回本接口指针。
 *
 * CORE ABI STABLE
 * DO NOT MODIFY WITHOUT VERSION BUMP
 * 中文：变更须同步升版本、SaveArchive / Lua 绑定与文档。
 *
 * SubsystemBus owns NO lifetime. It is a pure dispatcher facade.
 * 中文：总线不拥有各子系统对象生命周期；Scene()/Dialogue() 等返回的指针由宿主引擎创建与销毁，
 * 总线仅负责按名称分发访问，不在此接口内实现具体玩法逻辑。
 */

#pragma once
#include "../VGGalCoreConfig.h"
#include "../Include/SubsystemBusSnapshot.h"

namespace VisionGal::GalGame
{
	struct ISceneSubsystem;
	struct IUISubsystem;
	struct IAudioSubsystem;
	struct IScriptSubsystem;
	struct IArchiveSubsystem;
	struct IDialogueSubsystem;

	struct  ISubsystemBus
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
	};
}
