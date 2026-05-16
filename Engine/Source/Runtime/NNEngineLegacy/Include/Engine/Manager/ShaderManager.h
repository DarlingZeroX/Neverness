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
#include "../../EngineConfig.h"
#include "NNRuntimeCore/Include/Core/Core.h"
//#include "../../Graphics/Interface/VGFX.h"
#include <NNRuntimeRHI/Interface/VGFX.h>

namespace VisionGal
{
    class VG_ENGINE_API ShaderManager
    {
    public:
        enum class ShaderBlockType {
            None,
            VS,
            PS
        };

        ShaderManager();
        ~ShaderManager() = default;
        ShaderManager(const ShaderManager&) = delete;
        ShaderManager& operator=(const ShaderManager&) = delete;
        ShaderManager(ShaderManager&&) noexcept = default;
        ShaderManager& operator=(ShaderManager&&) noexcept = default;

        static ShaderManager* Get();
        VGFX::IShaderProgram* GetBuiltinProgram(const String& name);
    private:
        std::string Trim(const std::string& s);
        void ExtractShaders(const std::string& source, std::string& vs, std::string& ps);
        void Initialize();
        void AddBuiltInShader(const std::string& name, const std::string& path = "");
    private:
        std::unordered_map<std::string, Ref<VGFX::IShaderProgram>> m_BuiltinProgram;
    };
}
