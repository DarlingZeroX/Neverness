/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "ComponentRegistry/SequenceEditorRegistriesBootstrap.h"

#include "ComponentRegistry/SequenceComponentRegistry.h"
#include "Inspector/BuiltinSequenceInspectors.h"
#include "Inspector/SequenceInspectorRegistry.h"
#include "Properties/SequencePropertyDescriptor.h"

#include "Commands/EditSequencePropertyCommand.h"

#include <VGImgui/Include/ImGuiEx/IconFont/IconsFontAwesome5Pro.h>

#include "VGGalgameScriptSequence/Include/Sequence/Components.h"
#include "VGGalgameScriptSequence/Interface/IVGSSequenceComponent.h"

#include <memory>
#include <string>
#include <vector>

namespace VisionGal::Editor
{
	namespace
	{
		void FillPropertyDescriptors(const std::string& id, SequenceComponentMetadata& m)
		{
			m.PropertyDescriptors.clear();
			if (id == VGSSC_CommonDialogue::StaticGetTypeNameID())
			{
				SequencePropertyDescriptor a;
				a.Kind = SequencePropertyKind::String;
				a.Id = "character";
				a.Label = u8"角色名";
				a.HasEditField = true;
				a.EditField = SequenceEditFieldId::CommonDialogue_CharacterName;
				m.PropertyDescriptors.push_back(a);
				SequencePropertyDescriptor b;
				b.Kind = SequencePropertyKind::String;
				b.Id = "dialogue";
				b.Label = u8"对话文本";
				b.HasEditField = true;
				b.EditField = SequenceEditFieldId::CommonDialogue_DialogueText;
				m.PropertyDescriptors.push_back(b);
				return;
			}
			if (id == VGSSC_ChangeFigure::StaticGetTypeNameID())
			{
				SequencePropertyDescriptor t;
				t.Kind = SequencePropertyKind::AssetRef;
				t.Id = "texture";
				t.Label = u8"立绘纹理";
				t.HasEditField = true;
				t.EditField = SequenceEditFieldId::ChangeFigure_TextureResourcePath;
				m.PropertyDescriptors.push_back(t);
				return;
			}
			if (id == VGSSC_ChangeBackground::StaticGetTypeNameID())
			{
				SequencePropertyDescriptor t;
				t.Kind = SequencePropertyKind::AssetRef;
				t.Id = "texture";
				t.Label = u8"背景纹理";
				t.HasEditField = true;
				t.EditField = SequenceEditFieldId::ChangeBackground_TextureResourcePath;
				m.PropertyDescriptors.push_back(t);
			}
		}

		void FillPresentationForTypeNameID(const std::string& id, SequenceComponentMetadata& m)
		{
			FillPropertyDescriptors(id, m);
			if (id == VGSSC_CommonDialogue::StaticGetTypeNameID())
			{
				m.DisplayName = u8"普通对话";
				m.Icon = ICON_FA_COMMENT_ALT;
				m.Category = u8"常规演出";
				return;
			}
			if (id == VGSSC_ChangeFigure::StaticGetTypeNameID())
			{
				m.DisplayName = u8"切换立绘";
				m.Icon = ICON_FA_USER;
				m.Category = u8"常规演出";
				return;
			}
			if (id == VGSSC_ChangeBackground::StaticGetTypeNameID())
			{
				m.DisplayName = u8"切换背景";
				m.Icon = ICON_FA_IMAGES;
				m.Category = u8"常规演出";
				return;
			}
			m.DisplayName = id;
			m.Category = u8"序列组件";
			m.Icon = ICON_FA_CUBE;
		}
	}

	void BootstrapSequenceComponentRegistry(SequenceComponentRegistry& registry)
	{
		std::vector<std::string> types;
		VisionGal::IVGSSequenceComponentManager::Get().EnumerateRegisteredTypeNameIDs(types);

		int priority = 0;
		for (const std::string& id : types)
		{
			SequenceComponentMetadata m;
			m.TypeNameID = id;
			FillPresentationForTypeNameID(id, m);
			m.Priority = priority++;
			registry.Register(std::move(m));
		}
	}

	void BootstrapSequenceInspectorRegistry(SequenceInspectorRegistry& inspectors, const SequenceComponentRegistry& components)
	{
		for (const SequenceComponentMetadata& meta : components.EnumerateOrdered())
			inspectors.Register(MakeSequenceInspectorForMetadata(meta));
	}
}
