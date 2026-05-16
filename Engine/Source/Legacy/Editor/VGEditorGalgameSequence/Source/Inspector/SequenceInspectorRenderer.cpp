/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Inspector/SequenceInspectorRenderer.h"

#include "ComponentRegistry/SequenceComponentMetadata.h"
#include "Core/SequenceEditorContext.h"
#include "Inspector/SequenceSchemaPropertyCommandDispatch.h"

#include <NNRuntimeImGui/IncludeImGui.h>
#include <NNRuntimeImGui/IncludeImGuiEx.h>

#include <climits>
#include <cstdint>
#include <unordered_map>

#include "VGGalgameSequenceRuntime/Interface/IVGSSequenceComponent.h"

namespace VisionGal::Editor
{
	namespace
	{
		using StagingKey = std::uint64_t;

		StagingKey MakeStringStagingKey(const unsigned index, const std::string& propName)
		{
			const std::hash<std::string> h;
			return (static_cast<std::uint64_t>(index) << 32)
				^ static_cast<std::uint64_t>(static_cast<unsigned>(h(propName) & 0xFFFFFFFFu));
		}

		std::unordered_map<StagingKey, std::string>& StagingStrings()
		{
			static std::unordered_map<StagingKey, std::string> s;
			return s;
		}

		bool TryGetStringFromValue(const SequencePropertyValue& v, std::string& out)
		{
			if (const auto* s = std::get_if<std::string>(&v))
			{
				out = *s;
				return true;
			}
			return false;
		}

		bool DrawStringLike(
			const SequencePropertySchema& prop,
			const unsigned index,
			void* component,
			const std::string& typeNameId,
			SequenceEditorContext* context,
			const bool multiline)
		{
			if (component == nullptr || !prop.Accessor.Getter)
				return false;
			const StagingKey key = MakeStringStagingKey(index, prop.Name);
			std::string& staging = StagingStrings()[key];
			if (staging.empty())
			{
				SequencePropertyValue cur = prop.Accessor.Getter(component);
				(void)TryGetStringFromValue(cur, staging);
			}

			const char* label = prop.DisplayName.empty() ? prop.Name.c_str() : prop.DisplayName.c_str();
			ImGui::PushID(static_cast<int>(reinterpret_cast<intptr_t>(&prop) ^ static_cast<int>(index)));
			if (multiline)
				ImGuiEx::InputTextMultiline(label, staging, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 4));
			else
				ImGuiEx::InputText(label, staging);
			if (ImGui::IsItemDeactivatedAfterEdit())
			{
				(void)TryDispatchSchemaPropertyEdit(
					typeNameId,
					prop.Name,
					index,
					SequencePropertyValue{std::in_place_type<std::string>, staging},
					context);
			}
			ImGui::PopID();
			return true;
		}

		bool DrawBool(
			const SequencePropertySchema& prop,
			const unsigned index,
			void* component,
			const std::string& typeNameId,
			SequenceEditorContext* context)
		{
			if (component == nullptr || !prop.Accessor.Getter)
				return false;
			bool v = false;
			{
				SequencePropertyValue cur = prop.Accessor.Getter(component);
				if (const auto* b = std::get_if<bool>(&cur))
					v = *b;
			}
			const char* label = prop.DisplayName.empty() ? prop.Name.c_str() : prop.DisplayName.c_str();
			ImGui::PushID(static_cast<int>(reinterpret_cast<intptr_t>(&prop) ^ static_cast<int>(index)));
			if (ImGui::Checkbox(label, &v))
			{
				(void)TryDispatchSchemaPropertyEdit(
					typeNameId,
					prop.Name,
					index,
					SequencePropertyValue{std::in_place_type<bool>, v},
					context);
			}
			ImGui::PopID();
			return true;
		}

		bool DrawFloat(const SequencePropertySchema& prop, const unsigned index, void* component, SequenceEditorContext* context)
		{
			(void)context;
			if (component == nullptr || !prop.Accessor.Getter || !prop.Accessor.Setter)
				return false;
			double v = 0.0;
			{
				SequencePropertyValue cur = prop.Accessor.Getter(component);
				if (const auto* d = std::get_if<double>(&cur))
					v = *d;
				else if (const auto* i = std::get_if<std::int64_t>(&cur))
					v = static_cast<double>(*i);
			}
			float vf = static_cast<float>(v);
			float min = 0.f;
			float max = 0.f;
			bool clamp = false;
			if (prop.Range.has_value())
			{
				if (prop.Range->Min.has_value())
				{
					min = static_cast<float>(*prop.Range->Min);
					clamp = true;
				}
				if (prop.Range->Max.has_value())
				{
					max = static_cast<float>(*prop.Range->Max);
					clamp = true;
				}
			}
			const char* label = prop.DisplayName.empty() ? prop.Name.c_str() : prop.DisplayName.c_str();
			ImGui::PushID(static_cast<int>(reinterpret_cast<intptr_t>(&prop) ^ static_cast<int>(index)));
			bool changed = false;
			if (clamp)
				changed = ImGui::SliderFloat(label, &vf, min, max);
			else
				changed = ImGui::DragFloat(label, &vf, 0.01f);
			if (changed)
				(void)prop.Accessor.Setter(component, SequencePropertyValue{std::in_place_type<double>, static_cast<double>(vf)});
			ImGui::PopID();
			return true;
		}

		bool DrawInt(const SequencePropertySchema& prop, const unsigned index, void* component, SequenceEditorContext* context)
		{
			(void)context;
			if (component == nullptr || !prop.Accessor.Getter || !prop.Accessor.Setter)
				return false;
			int iv = 0;
			{
				SequencePropertyValue cur = prop.Accessor.Getter(component);
				if (const auto* i = std::get_if<std::int64_t>(&cur))
					iv = static_cast<int>(*i);
				else if (const auto* d = std::get_if<double>(&cur))
					iv = static_cast<int>(*d);
			}
			const char* label = prop.DisplayName.empty() ? prop.Name.c_str() : prop.DisplayName.c_str();
			ImGui::PushID(static_cast<int>(reinterpret_cast<intptr_t>(&prop) ^ static_cast<int>(index)));
			bool changed = false;
			if (prop.Range.has_value() && prop.Range->Min.has_value() && prop.Range->Max.has_value())
			{
				const int minV = static_cast<int>(*prop.Range->Min);
				const int maxV = static_cast<int>(*prop.Range->Max);
				changed = ImGui::SliderInt(label, &iv, minV, maxV);
			}
			else
				changed = ImGui::DragInt(label, &iv);
			if (changed)
				(void)prop.Accessor.Setter(
					component,
					SequencePropertyValue{std::in_place_type<std::int64_t>, static_cast<std::int64_t>(iv)});
			ImGui::PopID();
			return true;
		}

		bool DrawEnum(const SequencePropertySchema& prop, const unsigned index, void* component, SequenceEditorContext* context)
		{
			(void)context;
			if (component == nullptr || !prop.Enum.has_value() || !prop.Accessor.Getter || !prop.Accessor.Setter)
				return false;
			std::int64_t current = 0;
			{
				SequencePropertyValue cur = prop.Accessor.Getter(component);
				if (const auto* i = std::get_if<std::int64_t>(&cur))
					current = *i;
			}
			const char* preview = "?";
			int previewIndex = -1;
			int idx = 0;
			for (const SequenceEnumItem& it : prop.Enum->Items)
			{
				if (it.Value == current)
				{
					preview = it.DisplayLabel.c_str();
					previewIndex = idx;
					break;
				}
				++idx;
			}
			const char* label = prop.DisplayName.empty() ? prop.Name.c_str() : prop.DisplayName.c_str();
			ImGui::PushID(static_cast<int>(reinterpret_cast<intptr_t>(&prop) ^ static_cast<int>(index)));
			if (ImGui::BeginCombo(label, preview))
			{
				idx = 0;
				for (const SequenceEnumItem& it : prop.Enum->Items)
				{
					const bool selected = (idx == previewIndex);
					if (ImGui::Selectable(it.DisplayLabel.c_str(), selected))
						(void)prop.Accessor.Setter(
							component,
							SequencePropertyValue{std::in_place_type<std::int64_t>, it.Value});
					if (selected)
						ImGui::SetItemDefaultFocus();
					++idx;
				}
				ImGui::EndCombo();
			}
			ImGui::PopID();
			return true;
		}

		using Drawer = std::function<bool(
			const SequencePropertySchema&,
			unsigned,
			void*,
			const std::string&,
			SequenceEditorContext*)>;

		bool DrawOneProperty(
			const SequencePropertySchema& prop,
			const unsigned index,
			void* component,
			const std::string& typeNameId,
			SequenceEditorContext* context)
		{
			if (!prop.Editable)
				return false;
			switch (prop.Type)
			{
			case SequencePropertyType::String:
			case SequencePropertyType::LocalizedText:
				return DrawStringLike(
					prop,
					index,
					component,
					typeNameId,
					context,
					prop.Name == "dialogue");
			case SequencePropertyType::ResourcePath:
				return DrawStringLike(prop, index, component, typeNameId, context, false);
			case SequencePropertyType::Bool:
				return DrawBool(prop, index, component, typeNameId, context);
			case SequencePropertyType::Float:
				return DrawFloat(prop, index, component, context);
			case SequencePropertyType::Int:
				return DrawInt(prop, index, component, context);
			case SequencePropertyType::Enum:
				return DrawEnum(prop, index, component, context);
			default:
				return false;
			}
		}
	}

	bool SequenceInspectorRenderer::DrawFromSchema(
		const SequenceComponentMetadata& schema,
		const unsigned index,
		void* component,
		SequenceEditorContext* context)
	{
		if (component == nullptr || context == nullptr || context->document == nullptr || context->undo == nullptr)
			return false;
		if (schema.Properties.empty())
			return false;

		bool drew = false;
		const auto& props = schema.Properties;
		if (props.size() <= 40)
		{
			for (const SequencePropertySchema& p : props)
			{
				if (DrawOneProperty(p, index, component, schema.TypeNameID, context))
					drew = true;
			}
		}
		else
		{
			ImGuiListClipper clipper;
			clipper.Begin(static_cast<int>(props.size()));
			while (clipper.Step())
			{
				for (int ri = clipper.DisplayStart; ri < clipper.DisplayEnd; ++ri)
				{
					if (DrawOneProperty(props[static_cast<size_t>(ri)], index, component, schema.TypeNameID, context))
						drew = true;
				}
			}
		}
		return drew;
	}

	bool SequenceInspectorRenderer::DrawFromDescriptors(
		const SequenceComponentMetadata& meta,
		const unsigned index,
		VisionGal::IVGSSequenceComponent* component,
		SequenceEditorContext* context)
	{
		return DrawFromSchema(meta, index, static_cast<void*>(component), context);
	}
}
