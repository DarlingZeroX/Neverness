/*
 * VGSFigureRuntimeSystem 实现
 */

#include "RuntimeSystem/VGSFigureRuntimeSystem.h"

#include "VGSTypeDefine.h"
#include "VisualSequence/SequenceComponents.h"
#include "SSExecutorResourceManager.h"
#include "VGGalgameCore/Interface/IGameObject.h"
#include "SSSequenceExecutionContext.h"

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

		GalGame::IGalCharacter* character = context.ResourceManager->GetCharacter(figure->characterID);
		if (character == nullptr)
			return;

		if (!figure->showState)
		{
			character->HideFigure();
			return;
		}

		std::string pathToShow = figure->textureResourcePath;

		if (pathToShow.empty() && figure->textureID != VGSS_INVALID_OBJECT_ID)
		{
			if (std::shared_ptr<GalGame::IGalSprite> sprite =
					context.ResourceManager->TryGetSpriteShared(figure->textureID))
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
		return figure->wait;
	}
}
