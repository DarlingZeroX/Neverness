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
#include <functional>
#include <string>
#include "../../VGEditorCoreConfig.h"

namespace VisionGal::Editor
{
    struct MenuItem {
        std::string label;
        std::string shortcut;
        bool* p_selected;
        bool enabled;
        std::function<void()> callback;
    };

    class VG_EDITOR_CORE_API EditorUIMenu
    {
    public:

        void OnGUI();
        void AddMenuItem(const MenuItem& item);
    private:
        std::vector<MenuItem> m_MenuItem;
    };
}