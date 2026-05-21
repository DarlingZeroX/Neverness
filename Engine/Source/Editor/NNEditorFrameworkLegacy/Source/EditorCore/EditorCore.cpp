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

#include "EditorCore/EditorCore.h"
#include "NNEngineLegacy/Include/Project/ProjectSettings.h"

namespace NN::Editor
{
	struct EditorCoreImp
	{
		EditorCoreImp() = default;
		~EditorCoreImp()
		{
			EditorPreferences::Save(Preferences);
			Runtime::ProjectSettings::SaveProjectSettings();
		}

		static EditorCoreImp& GetInstance()
		{
			static EditorCoreImp imp;
			return imp;
		}

		EditorPreferences Preferences;
	};

    std::string EditorCore::GetEditorResourcePathVFS()
    {
        return "/editor/";
    }

    void EditorCore::LoadEditorPreferences()
    {
		EditorPreferences::Load(EditorCoreImp::GetInstance().Preferences);
    }

    EditorPreferences& EditorCore::GetEditorPreferences()
    {
		return EditorCoreImp::GetInstance().Preferences;
    }
}
