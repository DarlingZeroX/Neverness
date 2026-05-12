/*
 * IArchiveSubsystem — 存档槽位与读档 / 变量容器（由 ISubsystemBus::Archive() 访问）
 *
 * 职责从 IGalGameEngine::LoadArchive / GetArchiveDataContainer / GetArchiveSystem 迁移。
 */

#pragma once
#include "IGameSystem.h"
#include "../Include/SaveArchive.h"
#include "../VGGalCoreConfig.h"

namespace VisionGal::GalGame
{
	struct ArchiveDataContainer;

	struct IArchiveSubsystem
	{
		virtual ~IArchiveSubsystem() = default;

		virtual bool LoadArchive(const SaveArchive& archive) = 0;
		virtual ArchiveDataContainer* GetArchiveDataContainer() const = 0;
		virtual IArchiveSystem* GetArchiveSystem() = 0;
	};
}
