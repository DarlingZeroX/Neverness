/*
 * VGSBackgroundRuntimeSystem 实现
 */

#include "RuntimeSystem/Background.h"

#include "VGSTypeDefine.h"
#include "Sequence/Components.h"
#include "ExecutorResourceManager.h"
#include "VGGalgameCore/Interface/IGameEngine.h"
#include "SequenceExecutionContext.h"

namespace VisionGal
{
	namespace
	{
		/// 与渲染管线默认背景层对齐（参见 RenderPipeline / GalGameEngine 常用命名）。
		constexpr const char* kDefaultBackgroundLayer = "Background";
	}

	bool VGSBackgroundRuntimeSystem::CanExecute(IVGSSequenceComponent* component) const
	{
		return component != nullptr && dynamic_cast<VGSSC_ChangeBackground*>(component) != nullptr;
	}

	void VGSBackgroundRuntimeSystem::Execute(IVGSSequenceComponent* component, SSSequenceExecutionContext& context)
	{
		auto* bg = dynamic_cast<VGSSC_ChangeBackground*>(component);
		if (bg == nullptr || context.Engine == nullptr)
			return;

		if (!bg->ShowState)
		{
			if (bg->TextureID != VGSS_INVALID_OBJECT_ID && context.ResourceManager != nullptr)
			{
				if (GalGame::IGalSprite* sprite = context.ResourceManager->GetSprite(bg->TextureID))
					context.Engine->RemoveSprite(sprite);
			}
			return;
		}

		std::string path = bg->TextureResourcePath;
		if (path.empty() && bg->TextureID != VGSS_INVALID_OBJECT_ID && context.ResourceManager != nullptr)
		{
			if (GalGame::IGalSprite* sprite = context.ResourceManager->GetSprite(bg->TextureID))
				path = sprite->GetResourcePath();
		}

		if (!path.empty())
			context.Engine->ShowSprite(kDefaultBackgroundLayer, path);
	}

	bool VGSBackgroundRuntimeSystem::ShouldHoldPlaybackAfterExecute(IVGSSequenceComponent* component) const
	{
		const auto* bg = dynamic_cast<const VGSSC_ChangeBackground*>(component);
		if (bg == nullptr)
			return false;
		return bg->Wait;
	}
}
