/*
 * VGSBackgroundRuntimeSystem 实现
 */

#include "RuntimeSystem/Background.h"

#include "Runtime/SequenceComponentTypeId.h"
#include "VGSTypeDefine.h"
#include "Sequence/Components.h"
#include "ExecutorResourceManager.h"
#include "VGGalgameCore/Interface/ISubsystemBus.h"
#include "VGGalgameCore/Interface/ISceneSubsystem.h"
#include "SequenceExecutionContext.h"

namespace VisionGal
{
	namespace
	{
		/// 与渲染管线默认背景层对齐（参见 RenderPipeline / GalGameEngine 常用命名）。
		constexpr const char* kDefaultBackgroundLayer = "Background";
	}

	bool VGSBackgroundRuntimeSystem::SupportsType(const SequenceComponentTypeID id) const
	{
		return id == MakeSequenceComponentTypeIDFromTypeName("ChangeBackground");
	}

	bool VGSBackgroundRuntimeSystem::CanExecute(IVGSSequenceComponent* component) const
	{
		return component != nullptr && dynamic_cast<VGSSC_ChangeBackground*>(component) != nullptr;
	}

	void VGSBackgroundRuntimeSystem::Execute(IVGSSequenceComponent* component, SequenceRuntimeExecutionContext& context)
	{
		auto* bg = dynamic_cast<VGSSC_ChangeBackground*>(component);
		if (bg == nullptr || context.SharedContext == nullptr || context.SharedContext->SubsystemBus == nullptr)
			return;

		SSSequenceExecutionContext& shared = *context.SharedContext;

		if (!bg->ShowState)
		{
			if (bg->TextureID != VGSS_INVALID_OBJECT_ID && shared.ResourceManager != nullptr)
			{
				if (GalGame::IGalSprite* sprite = shared.ResourceManager->GetSprite(bg->TextureID))
					shared.SubsystemBus->Scene()->RemoveSprite(sprite);
			}
			return;
		}

		std::string path = bg->TextureResourcePath;
		if (path.empty() && bg->TextureID != VGSS_INVALID_OBJECT_ID && shared.ResourceManager != nullptr)
		{
			if (GalGame::IGalSprite* sprite = shared.ResourceManager->GetSprite(bg->TextureID))
				path = sprite->GetResourcePath();
		}

		if (!path.empty())
			shared.SubsystemBus->Scene()->ShowSprite(kDefaultBackgroundLayer, path);
	}

	bool VGSBackgroundRuntimeSystem::ShouldHoldPlaybackAfterExecute(IVGSSequenceComponent* component) const
	{
		const auto* bg = dynamic_cast<const VGSSC_ChangeBackground*>(component);
		if (bg == nullptr)
			return false;
		return bg->Wait;
	}
}
