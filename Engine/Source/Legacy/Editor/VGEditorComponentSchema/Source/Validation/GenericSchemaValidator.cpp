/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Validation/GenericSchemaValidator.h"

#include "Schema/SequencePropertyFlags.h"

#include <cctype>

namespace VisionGal::Editor
{
	namespace
	{
		bool IsWhitespaceOnly(const std::string& s)
		{
			for (unsigned char c : s)
			{
				if (!std::isspace(c))
					return false;
			}
			return true;
		}

		void PushNotEmpty(std::vector<SequenceSchemaValidationNote>& out, const char* msg)
		{
			SequenceSchemaValidationNote n;
			n.Message = msg;
			out.push_back(std::move(n));
		}
	}

	std::vector<SequenceSchemaValidationNote> ValidatePropertyValue(
		const SequencePropertySchema& prop,
		const SequencePropertyValue& current)
	{
		std::vector<SequenceSchemaValidationNote> out;

		const bool required = EnumHasAny(prop.Flags, SequencePropertyFlags::Required);
		const bool notEmpty = EnumHasAny(prop.Flags, SequencePropertyFlags::NotEmpty);
		const bool resNotEmpty = EnumHasAny(prop.Flags, SequencePropertyFlags::ResourcePathNotEmpty);
		const bool useRange = EnumHasAny(prop.Flags, SequencePropertyFlags::UseRange);

		if (required)
		{
			if (std::holds_alternative<std::monostate>(current))
				PushNotEmpty(out, u8"缺少必填属性值");
		}

		if (const auto* s = std::get_if<std::string>(&current))
		{
			if (notEmpty && (s->empty() || IsWhitespaceOnly(*s)))
				PushNotEmpty(out, u8"文本不得为空");
			if (resNotEmpty && s->empty())
				PushNotEmpty(out, u8"资源路径不得为空");
		}

		if (const auto* i = std::get_if<std::int64_t>(&current))
		{
			if (useRange && prop.Range.has_value())
			{
				const double v = static_cast<double>(*i);
				if (prop.Range->Min.has_value() && v < *prop.Range->Min)
					PushNotEmpty(out, u8"整型值低于最小值");
				if (prop.Range->Max.has_value() && v > *prop.Range->Max)
					PushNotEmpty(out, u8"整型值高于最大值");
			}
		}

		if (const auto* f = std::get_if<double>(&current))
		{
			if (useRange && prop.Range.has_value())
			{
				if (prop.Range->Min.has_value() && *f < *prop.Range->Min)
					PushNotEmpty(out, u8"浮点值低于最小值");
				if (prop.Range->Max.has_value() && *f > *prop.Range->Max)
					PushNotEmpty(out, u8"浮点值高于最大值");
			}
		}

		return out;
	}
}
