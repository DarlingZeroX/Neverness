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
#include <string>
//#include <NNEditorFrameworkLegacy/Include/EditorCore/EditorCore.h>
namespace NN::Runtime::Application
{
	struct EditorVFSPath
	{
		std::string assets;
		std::string library;
		std::string build;
		std::string packages;

		std::string editor;
		std::string engine;
		std::string projectSettings;
		std::string projectIntermediate;
	};

	struct EditorInitializer
	{
		static bool CheckProjectRootDir(const std::string& projectRootDir);

		static void PakResource(const EditorVFSPath& path);

		static void InitializeVFS(const EditorVFSPath& path);
	};
}

