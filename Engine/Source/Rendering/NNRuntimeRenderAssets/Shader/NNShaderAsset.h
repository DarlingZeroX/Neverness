#pragma once

#include "../NNRAssetsConfig.h"
#include <NNRuntimeRender/Pipeline/INNShader.h>
#include <NNRuntimeRender/Device/INNRenderDevice.h>
#include <NNRuntimeCore/NNObject.h>
#include <string>
#include <cstdint>

namespace NN::Runtime::Assets
{
    using namespace NN::Runtime::Render;
    using namespace NN::Runtime::Core;

    // ========================================================================
    //  NNShaderAsset — Shader source/bytecode asset
    //  Loads shader from file or memory, creates INNShader on demand
    // ========================================================================

    class NNShaderAsset : public INNObject
    {
    public:
        NNShaderAsset() = default;
        ~NNShaderAsset() override = default;

        // === Factory methods ===

        // Load from file (HLSL source)
        static NNRef<NNShaderAsset> LoadFromFile(const char* path, NNShaderStage stage,
                                                  const char* entryPoint = "main");

        // Load from memory (HLSL source)
        static NNRef<NNShaderAsset> LoadFromMemory(const char* source, uint32_t length,
                                                     NNShaderStage stage,
                                                     const char* name = "unnamed",
                                                     const char* entryPoint = "main");

        // Load pre-compiled bytecode (SPIR-V / DXIL)
        static NNRef<NNShaderAsset> LoadFromByteCode(const uint32_t* byteCode, uint32_t size,
                                                       NNShaderStage stage,
                                                       const char* name = "unnamed");

        // === Create shader for specific backend ===
        NNRef<INNShader> CreateShader(INNRenderDevice* device);

        // === Accessors ===
        NNShaderStage GetStage() const { return m_Stage; }
        const char* GetSource() const { return m_Source.c_str(); }
        const char* GetName() const { return m_Name.c_str(); }
        const char* GetEntryPoint() const { return m_EntryPoint.c_str(); }
        bool HasSource() const { return !m_Source.empty(); }
        bool HasByteCode() const { return m_ByteCodeSize > 0; }

        // === INNObject ===
        uint32_t AddRef() override { return ++m_RefCount; }
        uint32_t Release() override { uint32_t r = --m_RefCount; if (r == 0) delete this; return r; }
        uint32_t GetRefCount() const override { return m_RefCount; }

    private:
        std::string m_Source;
        std::string m_Name;
        std::string m_EntryPoint = "main";
        std::vector<uint32_t> m_ByteCode;
        uint32_t m_ByteCodeSize = 0;
        NNShaderStage m_Stage = NNShaderStage::Vertex;
        std::atomic<uint32_t> m_RefCount{0};

        // Cached shader per-device (simple single cache)
        INNRenderDevice* m_CachedDevice = nullptr;
        NNRef<INNShader> m_CachedShader;
    };

} // namespace NN::Runtime::Assets
