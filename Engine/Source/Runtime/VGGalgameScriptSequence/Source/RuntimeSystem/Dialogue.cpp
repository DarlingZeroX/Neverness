/*
 * VGSDialogueRuntimeSystem 实现
 */

#include "RuntimeSystem/Dialogue.h"
#include "Sequence/Components.h"
#include "VGGalgameCore/Interface/IGameObject.h"
#include "SequenceExecutionContext.h"

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
		if (dialogue->CharacterID != VGSS_INVALID_OBJECT_ID && context.ResourceManager != nullptr)
			character = context.ResourceManager->GetCharacter(dialogue->CharacterID);

		if (character != nullptr)
		{
			if (!dialogue->DialogueCharacterName.empty())
				character->SetName(dialogue->DialogueCharacterName);

			if (!dialogue->DialogueText.empty())
				character->Say(dialogue->DialogueText);
		}
		else
		{
			context.Engine->GetDialogueSystem()->CharacterSay(dialogue->DialogueCharacterName,dialogue->DialogueText);
		}
	}

	bool VGSDialogueRuntimeSystem::ShouldHoldPlaybackAfterExecute(IVGSSequenceComponent* component) const
	{
		const auto* dialogue = dynamic_cast<const VGSSC_CommonDialogue*>(component);
		if (dialogue == nullptr)
			return false;
		return dialogue->wait;
	}
}
