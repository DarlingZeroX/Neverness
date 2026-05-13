/*
 * IArchiveSubsystem — 存档槽位与读档 / 变量容器（Phase 8：Contract）
 *
 * 职责从 IGalGameEngine::LoadArchive / GetArchiveDataContainer / GetArchiveSystem 迁移。
 * SaveArchive / ArchiveDataContainer 完整类型在 **VGGalgameRuntimeCore**。
 */

#pragma once

namespace VisionGal::GalGame
{
	struct SaveArchive;
	struct ArchiveDataContainer;
	struct IArchiveSystem;

	struct IArchiveSubsystem
	{
		virtual ~IArchiveSubsystem() = default;

		virtual bool LoadArchive(const SaveArchive& archive) = 0;
		virtual ArchiveDataContainer* GetArchiveDataContainer() const = 0;
		virtual IArchiveSystem* GetArchiveSystem() = 0;
	};
}
