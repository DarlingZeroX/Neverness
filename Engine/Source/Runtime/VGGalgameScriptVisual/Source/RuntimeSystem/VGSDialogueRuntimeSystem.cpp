/*
 * VGSDialogueRuntimeSystem 实现
 */

#include "RuntimeSystem/VGSDialogueRuntimeSystem.h"
#include "VisualSequence/SequenceComponents.h"
#include "VGGalgameCore/Interface/IGameObject.h"
#include "SSSequenceExecutionContext.h"

namespace VisionGal
{
	bool VGSDialogueRuntimeSystem::CanExecute(IVGSSequenceComponent* component) const
	{
		return component != nullptr && dynamic_cast<VGSSC_CommonDialogue*>(component) != nullptr;
	}

	void VGSDialogueRuntimeSystem::Execute(IVGSSequenceComponent* component, SSSequenceExecutionContext& context)
	{
		auto* dialogue = dynamic_cast<VGSSC_CommonDialogue*>(component);
		if (dialogue == nullptr)
			return;

		GalGame::IGalCharacter* character = nullptr;
		if (dialogue->characterID != VGSS_INVALID_OBJECT_ID && context.ResourceManager != nullptr)
			character = context.ResourceManager->GetCharacter(dialogue->characterID);

		if (character != nullptr)
		{
			if (!dialogue->dialogueCharacterName.empty())
				character->SetName(dialogue->dialogueCharacterName);

			if (!dialogue->dialogueText.empty())
				character->Say(dialogue->dialogueText);
		}
		// 若无绑定角色，上层可选择接入全局字幕系统；此处保持 RuntimeSystem 职责最小化。
	}

	bool VGSDialogueRuntimeSystem::ShouldHoldPlaybackAfterExecute(IVGSSequenceComponent* component) const
	{
		const auto* dialogue = dynamic_cast<const VGSSC_CommonDialogue*>(component);
		if (dialogue == nullptr)
			return false;
		return dialogue->wait;
	}
}
