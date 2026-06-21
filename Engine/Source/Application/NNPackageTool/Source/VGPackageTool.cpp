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

#include "VGPackageTool.h"
#include "NNRuntimePak/Include/PakReader.h"
#include "NNRuntimePak/Include/PakWriter.h"
#include <NNRuntimeCore/Include/Core/Core.h>
#include "NNRuntimeVFS/Include/VFSService.h"
#include <VGEditorCore/Include/EditorCore/EditorCore.h>
#include "NNPlatformCore/Interface/FileSystem/HFileSystem.h"

namespace VisionGal::Editor
{
	void VGPackageTool::PakEngineEditorResources()
	{	PakFileReader reader;
		PakFileWriter writer;
		String engineResourcePath = VFSService::GetInstance()->AbsolutePath(Core::GetEngineResourcePathVFS());
		String editorResourcePath = VFSService::GetInstance()->AbsolutePath(EditorCore::GetEditorResourcePathVFS());

		Horizon::HFileSystem::CreateDirectoryWhenNoExist("Data");

		writer.WriteDirectoryToPakFile(engineResourcePath, "Data/engine.pak", "");
		writer.WriteDirectoryToPakFile(editorResourcePath, "Data/editor.pak", "");
	}
}
