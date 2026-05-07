/*
* This source file is part of VisionGal, the Visual Novel Engine
*
* For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
* GitHub page: https://github.com/DarlingZeroX/VisionGal
*
* Copyright (c) 2025-present 梦旅缘心
*
* See the LICENSE file in the project root for details.
*/
#pragma once

#include <memory>
#include <string>
#include <unordered_map>

namespace VisionGal::Editor
{
	struct SequenceEntryUIData
	{
		std::string FullLabel;
		std::string Label;
		std::string IconLabel;
		std::string Description;
		std::string Category;

		std::string TypeNameID;

		SequenceEntryUIData() = default;
		SequenceEntryUIData(const std::string& iconLabel, const std::string& label);
	};

	struct SequenceEntryUIDataManager
	{
		SequenceEntryUIDataManager();

		static SequenceEntryUIDataManager& GetInstance();

		static SequenceEntryUIData GetDataByTypeNameID(const std::string& typeNameID);

		std::unordered_map<std::string, SequenceEntryUIData> DataMap;
	};
}
