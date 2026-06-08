// NNShaderAsset.cpp — Shader asset implementation

#include "../../Shader/NNShaderAsset.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>

namespace NN::Runtime::Assets
{
    // ========================================================================
    //  Factory: Load from file
    // ========================================================================

    NNRef<NNShaderAsset> NNShaderAsset::LoadFromFile(const char* path, NNShaderStage stage,
                                                      const char* entryPoint)
    {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open())
        {
            std::cerr << "[NNShaderAsset] Failed to open: " << path << std::endl;
            return nullptr;
        }

        auto size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::string source(static_cast<size_t>(size), '\0');
        file.read(source.data(), size);
        file.close();

        auto* asset = new NNShaderAsset();
        asset->m_Source = std::move(source);
        asset->m_Stage = stage;
        asset->m_EntryPoint = entryPoint ? entryPoint : "main";

        // Extract filename as name
        std::string pathStr(path);
        auto lastSlash = pathStr.find_last_of("/\\");
        auto lastDot = pathStr.find_last_of('.');
        if (lastSlash == std::string::npos) lastSlash = 0; else lastSlash++;
        if (lastDot == std::string::npos || lastDot < lastSlash) lastDot = pathStr.size();
        asset->m_Name = pathStr.substr(lastSlash, lastDot - lastSlash);

        return NNRef<NNShaderAsset>(asset);
    }

    // ========================================================================
    //  Factory: Load from memory
    // ========================================================================

    NNRef<NNShaderAsset> NNShaderAsset::LoadFromMemory(const char* source, uint32_t length,
                                                         NNShaderStage stage,
                                                         const char* name,
                                                         const char* entryPoint)
    {
        auto* asset = new NNShaderAsset();
        asset->m_Source.assign(source, length);
        asset->m_Stage = stage;
        asset->m_Name = name ? name : "unnamed";
        asset->m_EntryPoint = entryPoint ? entryPoint : "main";
        return NNRef<NNShaderAsset>(asset);
    }

    // ========================================================================
    //  Factory: Load pre-compiled bytecode
    // ========================================================================

    NNRef<NNShaderAsset> NNShaderAsset::LoadFromByteCode(const uint32_t* byteCode, uint32_t size,
                                                           NNShaderStage stage,
                                                           const char* name)
    {
        auto* asset = new NNShaderAsset();
        asset->m_ByteCode.assign(byteCode, byteCode + size / sizeof(uint32_t));
        asset->m_ByteCodeSize = size;
        asset->m_Stage = stage;
        asset->m_Name = name ? name : "unnamed";
        return NNRef<NNShaderAsset>(asset);
    }

    // ========================================================================
    //  Create shader for specific backend
    // ========================================================================

    NNRef<INNShader> NNShaderAsset::CreateShader(INNRenderDevice* device)
    {
        if (!device) return nullptr;

        // Return cached shader if same device
        if (m_CachedDevice == device && m_CachedShader)
        {
            return m_CachedShader;
        }

        // Build shader desc
        NNShaderDesc desc;
        desc.Stage = m_Stage;
        desc.EntryPoint = m_EntryPoint.c_str();
        desc.DebugName = m_Name.c_str();

        if (HasSource())
        {
            desc.SourceCode = m_Source.c_str();
            desc.SourceLength = static_cast<uint32_t>(m_Source.size());
            desc.Language = NNShaderLanguage::HLSL;
        }
        else if (HasByteCode())
        {
            desc.ByteCode = m_ByteCode.data();
            desc.ByteCodeSize = m_ByteCodeSize;
            desc.Language = NNShaderLanguage::SPIRV;
        }
        else
        {
            std::cerr << "[NNShaderAsset] No source or bytecode: " << m_Name << std::endl;
            return nullptr;
        }

        // Create shader via device
        auto shader = device->CreateShader(desc);
        if (shader)
        {
            m_CachedDevice = device;
            m_CachedShader = shader;
        }

        return shader;
    }

} // namespace NN::Runtime::Assets
