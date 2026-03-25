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
#include "../../VGEditorCoreConfig.h"
#include <string>
#include <functional>
#include <VGCore/Include/Core/Core.h>

namespace VisionGal::Editor
{
    struct VG_EDITOR_CORE_API EditorScene
    {
        static void OpenSaveCurrentSceneDialog(const std::function<void(int)>& callback);

        static void NewScene();

        static bool OpenNewScene(const VGPath& path);

        static void OpenSceneByFileDialog();

        static bool SaveCurrentScene();

        static bool SaveCurrentSceneAs();
    };
}
