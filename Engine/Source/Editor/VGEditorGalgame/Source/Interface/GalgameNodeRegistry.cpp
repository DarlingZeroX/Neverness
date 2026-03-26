/*
* Galgame 节点注册器实现
*/

#include <VGEditorGalgame/Interface/GalgameNodeRegistry.h>
#include "GalgameNodeCustomDraw.h"

#include <VGGalgameRuntime/Include/VGNodeExec_Galgame.h>

namespace VisionGal::Editor
{
	using namespace Horizon::NodeGraphRuntime;
	using namespace Horizon::NodeGraphEditor;

	static void RegisterCoreNodes(NodeRegistry& registry, NodeEditorRegistry& editorRegistry)
	{
		const NodeTypeId entryTypeId = registry.FindType("Entry");

		// Entry
		registry.Register(NodeMeta{
			entryTypeId,
			"Entry",
			{},
			{ { "Next", SlotType::Exec, false } },
			VisionGal::Runtime::EntryExecute
		});

		editorRegistry.Register(NodeEditorMeta{
			entryTypeId,
			"Entry",
			"Core",
			{}
		});
	}

	static void RegisterFlowNodes(NodeRegistry& registry, NodeEditorRegistry& editorRegistry)
	{
		const NodeTypeId dialogueTypeId = registry.RegisterType("DialogueList");
		const NodeTypeId choiceTypeId = registry.RegisterType("Choice");

		// DialogueList
		registry.Register(NodeMeta{
			dialogueTypeId,
			"DialogueList",
			{ { "In", SlotType::Exec, true } },
			{
				{ "Next", SlotType::Exec, false },
				{ VisionGal::Runtime::PIN_LinesJson, SlotType::String, false }
			},
			VisionGal::Runtime::DialogueListExecute
		});

		editorRegistry.Register(NodeEditorMeta{
			dialogueTypeId,
			"DialogueList",
			"Galgame",
			{
				PropertyMeta{
					"dialogueListJson",
					"dialogueList",
					ValueType::String,
					PropertyWidgetType::MultilineText,
					Value::FromString(R"({"type":"DialogueList","lines":[]})"),
					{}
				}
			},
			&VisionGal::Editor::DrawDialogueListCustomUI
		});

		// Choice
		registry.Register(NodeMeta{
			choiceTypeId,
			"Choice",
			{ { "In", SlotType::Exec, true } },
			{
				{ "Option1", SlotType::Exec, false },
				{ "Option2", SlotType::Exec, false }
			},
			VisionGal::Runtime::ChoiceExecute
		});

		editorRegistry.Register(NodeEditorMeta{
			choiceTypeId,
			"Choice",
			"Galgame",
			{}
		});
	}

	static void RegisterAssetNodes(NodeRegistry& registry, NodeEditorRegistry& editorRegistry)
	{
		const NodeTypeId showCharacterTypeId = registry.RegisterType("ShowCharacter");
		const NodeTypeId playBGMTypeId = registry.RegisterType("PlayBGM");
		const NodeTypeId setBackgroundTypeId = registry.RegisterType("SetBackground");

		// ShowCharacter
		registry.Register(NodeMeta{
			showCharacterTypeId,
			"ShowCharacter",
			{ { "In", SlotType::Exec, true } },
			{
				{ "Out", SlotType::Exec, false },
				{ "Name", SlotType::String, false },
				{ "Expression", SlotType::String, false },
				{ "Position", SlotType::String, false }
			},
			VisionGal::Runtime::ShowCharacterExecute
		});

		editorRegistry.Register(NodeEditorMeta{
			showCharacterTypeId,
			"ShowCharacter",
			"Galgame",
			{
				PropertyMeta{ "name", "name", ValueType::String, PropertyWidgetType::InputText, Value::FromString(""), {} },
				PropertyMeta{ "value", "expression", ValueType::String, PropertyWidgetType::InputText, Value::FromString(""), {} },
				PropertyMeta{ "position", "position", ValueType::String, PropertyWidgetType::InputText, Value::FromString(""), {} }
			}
		});

		// PlayBGM
		registry.Register(NodeMeta{
			playBGMTypeId,
			"PlayBGM",
			{ { "In", SlotType::Exec, true } },
			{
				{ "Out", SlotType::Exec, false },
				{ "BgmName", SlotType::String, false }
			},
			VisionGal::Runtime::PlayBGMExecute
		});

		editorRegistry.Register(NodeEditorMeta{
			playBGMTypeId,
			"PlayBGM",
			"Galgame",
			{
				PropertyMeta{ "bgmName", "bgmName", ValueType::String, PropertyWidgetType::InputText, Value::FromString(""), {} }
			}
		});

		// SetBackground
		registry.Register(NodeMeta{
			setBackgroundTypeId,
			"SetBackground",
			{ { "In", SlotType::Exec, true } },
			{
				{ "Out", SlotType::Exec, false },
				{ "BackgroundName", SlotType::String, false }
			},
			VisionGal::Runtime::SetBackgroundExecute
		});

		editorRegistry.Register(NodeEditorMeta{
			setBackgroundTypeId,
			"SetBackground",
			"Galgame",
			{
				PropertyMeta{ "backgroundName", "backgroundName", ValueType::String, PropertyWidgetType::InputText, Value::FromString(""), {} }
			}
		});
	}

	void GalgameNodeRegistry::RegisterAll(NodeRegistry& registry, NodeEditorRegistry& editorRegistry)
	{
		RegisterCoreNodes(registry, editorRegistry);
		RegisterFlowNodes(registry, editorRegistry);
		RegisterAssetNodes(registry, editorRegistry);
	}
}

