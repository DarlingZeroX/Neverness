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
#include "../../Config.h"
#include "CodeDocument.h"
#include "NNRuntimeCore/Include/Core/Core.h"

namespace VisionGal::Editor
{
	struct DocumentManager
	{
		std::set<std::string> FileList;
		std::vector<Ref<CodeDocument>> Documents;
		std::vector<CodeDocument*> CloseQueue;
		int NextUID;
		DocumentManager() : NextUID(1) {}

		bool OpenTextFile(const VGPath& path);

		CodeDocument* GetDocument(const VGPath& path);

		void AddDocument(const Ref<CodeDocument>& document);

		bool ExistDocument(const std::string& path);

		bool CloseDocument(const std::string& path);
		bool CloseDocument(CodeDocument* doc);

		bool SaveDocument(const std::string& path);
		bool SaveDocument(CodeDocument* doc);

		bool ForceRemoveDocument(const std::string& path);
		bool ForceRemoveDocument(CodeDocument* doc);

		void SaveAllDocument();

		bool SetDocumentActive(const VGPath& path);
	};
}
