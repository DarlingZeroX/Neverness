/*
 * VGSFigureRuntimeSystem 实现
 */

#include "RuntimeSystem/Figure.h"

#include "VGSTypeDefine.h"
#include "Sequence/Components.h"
#include "ExecutorResourceManager.h"
#include "VGGalgameCore/Interface/IGameObject.h"
#include "SequenceExecutionContext.h"

namespace VisionGal
{
	bool VGSFigureRuntimeSystem::CanExecute(IVGSSequenceComponent* component) const
	{
		return component != nullptr && dynamic_cast<VGSSC_ChangeFigure*>(component) != nullptr;
	}

	void VGSFigureRuntimeSystem::Execute(IVGSSequenceComponent* component, SSSequenceExecutionContext& context)
	{
		auto* figure = dynamic_cast<VGSSC_ChangeFigure*>(component);
		if (figure == nullptr || context.ResourceManager == nullptr)
			return;

		GalGame::IGalCharacter* character = context.ResourceManager->GetCharacter(figure->CharacterID);
		if (character == nullptr)
			return;

		if (!figure->ShowState)
		{
			character->HideFigure();
			return;
		}

		std::string pathToShow = figure->TextureResourcePath;

		if (pathToShow.empty() && figure->TextureID != VGSS_INVALID_OBJECT_ID)
		{
			if (std::shared_ptr<GalGame::IGalSprite> sprite =
					context.ResourceManager->TryGetSpriteShared(figure->TextureID))
				pathToShow = sprite->GetResourcePath();
		}

		if (!pathToShow.empty())
			character->ShowFigure(pathToShow);
	}

	bool VGSFigureRuntimeSystem::ShouldHoldPlaybackAfterExecute(IVGSSequenceComponent* component) const
	{
		const auto* figure = dynamic_cast<const VGSSC_ChangeFigure*>(component);
		if (figure == nullptr)
			return false;
		return figure->Wait;
	}
}
