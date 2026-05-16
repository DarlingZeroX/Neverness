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
#include <NNEditorFramework/Include/EditorCore/EditorCore.h>

struct VGDesktopApplicationVFSPath
{
	std::string assets;
	std::string editor;
	std::string engine;
	std::string projectSettings;
	std::string projectIntermediate;
};

struct VGDesktopApplicationInitializer
{
	static void PakResource(const VGDesktopApplicationVFSPath& path);

	static void InitializeVFS(const VGDesktopApplicationVFSPath& path);
};

