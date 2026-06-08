// NNShaderSource.cpp — Shader source management

#include "../../Shader/NNShaderSource.h"
#include <fstream>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

namespace NN::Runtime::Assets
{
    // ========================================================================
    //  Factory: Load all shader files from directory
    // ========================================================================

    NNRef<NNShaderSource> NNShaderSource::LoadFromDirectory(const char* dir)
    {
        auto* source = new NNShaderSource();
        source->m_Directory = dir;
        source->LoadFilesFromDir(dir);
        return NNRef<NNShaderSource>(source);
    }

    // ========================================================================
    //  Add shader source manually
    // ========================================================================

    void NNShaderSource::AddSource(const char* name, const char* source)
    {
        auto it = m_NameToIndex.find(name);
        if (it != m_NameToIndex.end())
        {
            // Update existing
            m_Shaders[it->second].Source = source;
        }
        else
        {
            // Add new
            size_t idx = m_Shaders.size();
            m_Shaders.push_back({name, "", source});
            m_NameToIndex[name] = idx;
        }
    }

    // ========================================================================
    //  Get shader source by name
    // ========================================================================

    const char* NNShaderSource::GetSource(const char* name) const
    {
        auto it = m_NameToIndex.find(name);
        if (it != m_NameToIndex.end())
        {
            return m_Shaders[it->second].Source.c_str();
        }
        return nullptr;
    }

    bool NNShaderSource::HasSource(const char* name) const
    {
        return m_NameToIndex.find(name) != m_NameToIndex.end();
    }

    // ========================================================================
    //  Get all shader names
    // ========================================================================

    std::vector<std::string> NNShaderSource::GetShaderNames() const
    {
        std::vector<std::string> names;
        names.reserve(m_Shaders.size());
        for (const auto& entry : m_Shaders)
        {
            names.push_back(entry.Name);
        }
        return names;
    }

    // ========================================================================
    //  Reload from disk
    // ========================================================================

    bool NNShaderSource::Reload()
    {
        if (m_Directory.empty()) return false;

        // Clear and reload
        m_Shaders.clear();
        m_NameToIndex.clear();
        LoadFilesFromDir(m_Directory.c_str());
        return true;
    }

    // ========================================================================
    //  Internal: Load files from directory
    // ========================================================================

    void NNShaderSource::LoadFilesFromDir(const char* dir)
    {
        if (!fs::exists(dir) || !fs::is_directory(dir))
        {
            std::cerr << "[NNShaderSource] Directory not found: " << dir << std::endl;
            return;
        }

        for (const auto& entry : fs::directory_iterator(dir))
        {
            if (!entry.is_regular_file()) continue;

            auto ext = entry.path().extension().string();
            // Accept .hlsl, .glsl, .vert, .frag
            if (ext != ".hlsl" && ext != ".glsl" && ext != ".vert" && ext != ".frag")
            {
                continue;
            }

            auto path = entry.path();
            auto name = path.stem().string(); // filename without extension

            std::ifstream file(path, std::ios::binary | std::ios::ate);
            if (!file.is_open())
            {
                std::cerr << "[NNShaderSource] Failed to open: " << path << std::endl;
                continue;
            }

            auto size = file.tellg();
            file.seekg(0, std::ios::beg);

            std::string source(static_cast<size_t>(size), '\0');
            file.read(source.data(), size);
            file.close();

            size_t idx = m_Shaders.size();
            m_Shaders.push_back({name, path.string(), std::move(source)});
            m_NameToIndex[name] = idx;

            std::cout << "[NNShaderSource] Loaded: " << name << " (" << size << " bytes)" << std::endl;
        }
    }

} // namespace NN::Runtime::Assets
