/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "ComponentRegistry/SequenceEditorRegistriesBootstrap.h"

#include "ComponentRegistry/SequenceComponentRegistry.h"
#include "Extensions/ISequenceEditorExtension.h"
#include "Extensions/SequenceExtensionRegistry.h"
#include "Inspector/BuiltinSequenceInspectors.h"
#include "Inspector/SequenceInspectorRegistry.h"

#include "Schema/SequenceGraphPortSchema.h"
#include "Schema/SequencePropertyFlags.h"
#include "Schema/SequencePropertySchema.h"
#include "Schema/SequencePropertyType.h"
#include "Schema/SequencePropertyValue.h"

#include <NNRuntimeImGui/Include/ImGuiEx/IconFont/IconsFontAwesome5Pro.h>

#include "VGGalgameSequenceRuntime/Include/Sequence/Components.h"
#include "VGGalgameSequenceRuntime/Interface/IVGSSequenceComponent.h"

#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace VisionGal::Editor
{
	namespace
	{
		void AppendLinearFlowPorts(SequenceComponentSchema& m)
		{
			m.InputPorts.clear();
			m.OutputPorts.clear();
			SequenceGraphPortSchema in;
			in.Name = "In";
			in.DisplayName = u8"流入";
			in.Direction = SequencePortDirection::Input;
			in.DataType = SequencePortDataType::Flow;
			SequenceGraphPortSchema out;
			out.Name = "Out";
			out.DisplayName = u8"流出";
			out.Direction = SequencePortDirection::Output;
			out.DataType = SequencePortDataType::Flow;
			m.InputPorts.push_back(in);
			m.OutputPorts.push_back(out);
		}

		void FillPropertySchemas(const std::string& id, SequenceComponentSchema& m)
		{
			m.Properties.clear();
			if (id == VGSSC_CommonDialogue::StaticGetTypeNameID())
			{
				SequencePropertySchema a;
				a.Type = SequencePropertyType::String;
				a.Name = "character";
				a.DisplayName = u8"角色名";
				a.Category = u8"对话";
				a.Flags = SequencePropertyFlags::None;
				a.Accessor.Getter = [](void* p) -> SequencePropertyValue
				{
					auto* d = static_cast<VGSSC_CommonDialogue*>(p);
					if (d == nullptr)
						return {};
					return SequencePropertyValue{std::in_place_type<std::string>, d->DialogueCharacterName};
				};
				a.Accessor.Setter = [](void* p, const SequencePropertyValue& v) -> bool
				{
					auto* d = static_cast<VGSSC_CommonDialogue*>(p);
					if (d == nullptr)
						return false;
					if (const auto* s = std::get_if<std::string>(&v))
					{
						d->DialogueCharacterName = *s;
						return true;
					}
					return false;
				};
				m.Properties.push_back(std::move(a));

				SequencePropertySchema b;
				b.Type = SequencePropertyType::String;
				b.Name = "dialogue";
				b.DisplayName = u8"对话文本";
				b.Category = u8"对话";
				b.Flags = SequencePropertyFlags::NotEmpty;
				b.Accessor.Getter = [](void* p) -> SequencePropertyValue
				{
					auto* d = static_cast<VGSSC_CommonDialogue*>(p);
					if (d == nullptr)
						return {};
					return SequencePropertyValue{std::in_place_type<std::string>, d->DialogueText};
				};
				b.Accessor.Setter = [](void* p, const SequencePropertyValue& v) -> bool
				{
					auto* d = static_cast<VGSSC_CommonDialogue*>(p);
					if (d == nullptr)
						return false;
					if (const auto* s = std::get_if<std::string>(&v))
					{
						d->DialogueText = *s;
						return true;
					}
					return false;
				};
				m.Properties.push_back(std::move(b));
				return;
			}
			if (id == VGSSC_ChangeFigure::StaticGetTypeNameID())
			{
				SequencePropertySchema t;
				t.Type = SequencePropertyType::ResourcePath;
				t.Name = "texture";
				t.DisplayName = u8"立绘纹理";
				t.Category = u8"立绘";
				t.Flags = SequencePropertyFlags::ResourcePathNotEmpty;
				t.Accessor.Getter = [](void* p) -> SequencePropertyValue
				{
					auto* f = static_cast<VGSSC_ChangeFigure*>(p);
					if (f == nullptr)
						return {};
					return SequencePropertyValue{std::in_place_type<std::string>, f->TextureResourcePath};
				};
				t.Accessor.Setter = [](void* p, const SequencePropertyValue& v) -> bool
				{
					auto* f = static_cast<VGSSC_ChangeFigure*>(p);
					if (f == nullptr)
						return false;
					if (const auto* s = std::get_if<std::string>(&v))
					{
						f->TextureResourcePath = *s;
						return true;
					}
					return false;
				};
				m.Properties.push_back(std::move(t));

				SequencePropertySchema sh;
				sh.Type = SequencePropertyType::Bool;
				sh.Name = "showState";
				sh.DisplayName = u8"显示立绘";
				sh.Category = u8"立绘";
				sh.Accessor.Getter = [](void* p) -> SequencePropertyValue
				{
					auto* f = static_cast<VGSSC_ChangeFigure*>(p);
					if (f == nullptr)
						return {};
					return SequencePropertyValue{std::in_place_type<bool>, f->ShowState};
				};
				sh.Accessor.Setter = [](void* p, const SequencePropertyValue& v) -> bool
				{
					auto* f = static_cast<VGSSC_ChangeFigure*>(p);
					if (f == nullptr)
						return false;
					if (const auto* b = std::get_if<bool>(&v))
					{
						f->ShowState = *b;
						return true;
					}
					return false;
				};
				m.Properties.push_back(std::move(sh));

				SequencePropertySchema w;
				w.Type = SequencePropertyType::Bool;
				w.Name = "wait";
				w.DisplayName = u8"等待";
				w.Category = u8"立绘";
				w.Accessor.Getter = [](void* p) -> SequencePropertyValue
				{
					auto* f = static_cast<VGSSC_ChangeFigure*>(p);
					if (f == nullptr)
						return {};
					return SequencePropertyValue{std::in_place_type<bool>, f->Wait};
				};
				w.Accessor.Setter = [](void* p, const SequencePropertyValue& v) -> bool
				{
					auto* f = static_cast<VGSSC_ChangeFigure*>(p);
					if (f == nullptr)
						return false;
					if (const auto* b = std::get_if<bool>(&v))
					{
						f->Wait = *b;
						return true;
					}
					return false;
				};
				m.Properties.push_back(std::move(w));
				return;
			}
			if (id == VGSSC_ChangeBackground::StaticGetTypeNameID())
			{
				SequencePropertySchema t;
				t.Type = SequencePropertyType::ResourcePath;
				t.Name = "texture";
				t.DisplayName = u8"背景纹理";
				t.Category = u8"背景";
				t.Flags = SequencePropertyFlags::ResourcePathNotEmpty;
				t.Accessor.Getter = [](void* p) -> SequencePropertyValue
				{
					auto* b = static_cast<VGSSC_ChangeBackground*>(p);
					if (b == nullptr)
						return {};
					return SequencePropertyValue{std::in_place_type<std::string>, b->TextureResourcePath};
				};
				t.Accessor.Setter = [](void* p, const SequencePropertyValue& v) -> bool
				{
					auto* bg = static_cast<VGSSC_ChangeBackground*>(p);
					if (bg == nullptr)
						return false;
					if (const auto* s = std::get_if<std::string>(&v))
					{
						bg->TextureResourcePath = *s;
						return true;
					}
					return false;
				};
				m.Properties.push_back(std::move(t));

				SequencePropertySchema sh;
				sh.Type = SequencePropertyType::Bool;
				sh.Name = "showState";
				sh.DisplayName = u8"显示背景";
				sh.Category = u8"背景";
				sh.Accessor.Getter = [](void* p) -> SequencePropertyValue
				{
					auto* b = static_cast<VGSSC_ChangeBackground*>(p);
					if (b == nullptr)
						return {};
					return SequencePropertyValue{std::in_place_type<bool>, b->ShowState};
				};
				sh.Accessor.Setter = [](void* p, const SequencePropertyValue& v) -> bool
				{
					auto* bg = static_cast<VGSSC_ChangeBackground*>(p);
					if (bg == nullptr)
						return false;
					if (const auto* bb = std::get_if<bool>(&v))
					{
						bg->ShowState = *bb;
						return true;
					}
					return false;
				};
				m.Properties.push_back(std::move(sh));

				SequencePropertySchema w;
				w.Type = SequencePropertyType::Bool;
				w.Name = "wait";
				w.DisplayName = u8"等待";
				w.Category = u8"背景";
				w.Accessor.Getter = [](void* p) -> SequencePropertyValue
				{
					auto* b = static_cast<VGSSC_ChangeBackground*>(p);
					if (b == nullptr)
						return {};
					return SequencePropertyValue{std::in_place_type<bool>, b->Wait};
				};
				w.Accessor.Setter = [](void* p, const SequencePropertyValue& v) -> bool
				{
					auto* bg = static_cast<VGSSC_ChangeBackground*>(p);
					if (bg == nullptr)
						return false;
					if (const auto* bb = std::get_if<bool>(&v))
					{
						bg->Wait = *bb;
						return true;
					}
					return false;
				};
				m.Properties.push_back(std::move(w));
			}
		}

		void FillPresentationForTypeNameID(const std::string& id, SequenceComponentSchema& m)
		{
			FillPropertySchemas(id, m);
			AppendLinearFlowPorts(m);
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
			SequenceComponentSchema m;
			m.TypeNameID = id;
			FillPresentationForTypeNameID(id, m);
			m.Priority = priority++;
			registry.Register(std::move(m));
		}
	}

	namespace
	{
		class NoopSequenceEditorExtension final : public ISequenceEditorExtension
		{
		public:
			[[nodiscard]] const char* GetExtensionId() const override { return "visiongal.sequence.phase8.noop"; }
		};
	}

	void BootstrapSequenceExtensions(SequenceExtensionRegistry& registry)
	{
		if (!registry.GetExtensions().empty())
			return;
		registry.Register(std::make_unique<NoopSequenceEditorExtension>());
	}

	void BootstrapSequenceInspectorRegistry(SequenceInspectorRegistry& inspectors, const SequenceComponentRegistry& components)
	{
		for (const SequenceComponentMetadata& meta : components.EnumerateOrdered())
			inspectors.Register(MakeSequenceInspectorForMetadata(meta));
	}
}
