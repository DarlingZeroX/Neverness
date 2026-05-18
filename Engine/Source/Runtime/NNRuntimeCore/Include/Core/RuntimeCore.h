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
#include "CoreTypes.h"
#include "../../RuntimeCoreExport.h"
#include "../../Interface/FileInterface.h"

namespace NN::Runtime
{
	struct VGObject
	{
		virtual ~VGObject() = default;

		virtual  std::string ToString() { return ""; }
	};

	struct VGEngineResource: public VGObject
	{
		const String& GetResourcePath() { return m_ResourcePath; }
		void SetResourcePath(const String& path) { m_ResourcePath = path; }
	private:
		String m_ResourcePath;
	};

	struct NN_RUNTIME_CORE_API RuntimeCore
	{
		static void Initialize();
		static float GetCurrentTime();

		static void SetFileInterface(FileInterface* file_interface);
		static FileInterface* GetFileInterface();

		static std::string GetDefaultSpriteTexturePath();

		static std::string GetAssetsPathVFS();
		static std::string GetProjectIntermediatePathVFS();
		static std::string GetProjectSettingsPathVFS();
		static std::string GetEngineResourcePathVFS();

		static std::string GetResourcePathVFS(const std::string& absolutePath);

		static std::filesystem::path GetExecutableDirectory();
	};


}
