#pragma once

#include "../NNRAssetsConfig.h"
#include <NNRuntimeCore/NNObject.h>
#include <string>
#include <vector>
#include <unordered_map>

namespace NN::Runtime::Assets
{
    using namespace NN::Runtime::Core;

    // ========================================================================
    //  NNShaderSource — Manages shader source files
    //  Loads .hlsl files from directory, provides lookup by name
    //  Supports hot-reload
    // ========================================================================

    class NNShaderSource : public INNObject
    {
    public:
        NNShaderSource() = default;
        ~NNShaderSource() override = default;

        // === Load all shader files from directory ===
        static NNRef<NNShaderSource> LoadFromDirectory(const char* dir);

        // === Add shader source manually ===
        void AddSource(const char* name, const char* source);

        // === Get shader source by name ===
        const char* GetSource(const char* name) const;
        bool HasSource(const char* name) const;

        // === Get all shader names ===
        std::vector<std::string> GetShaderNames() const;

        // === Reload from disk (for hot-reload) ===
        bool Reload();

        // === Get directory ===
        const char* GetDirectory() const { return m_Directory.c_str(); }

        // === INNObject ===
        uint32_t AddRef() override { return ++m_RefCount; }
        uint32_t Release() override { uint32_t r = --m_RefCount; if (r == 0) delete this; return r; }
        uint32_t GetRefCount() const override { return m_RefCount; }

    private:
        struct ShaderEntry
        {
            std::string Name;     // e.g., "BasicVS"
            std::string FilePath; // Full path on disk
            std::string Source;   // HLSL source
        };

        std::vector<ShaderEntry> m_Shaders;
        std::unordered_map<std::string, size_t> m_NameToIndex;
        std::string m_Directory;
        std::atomic<uint32_t> m_RefCount{0};

        void LoadFilesFromDir(const char* dir);
    };

} // namespace NN::Runtime::Assets
