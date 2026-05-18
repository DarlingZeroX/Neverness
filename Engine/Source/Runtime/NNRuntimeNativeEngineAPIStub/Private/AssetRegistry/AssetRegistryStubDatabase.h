#pragma once

/**
 * @file AssetRegistryStubDatabase.h
 * @brief Stub **NNAssetRegistryAPI** 之路徑 ↔ GUID 假資料庫。
 */

#include <cstddef>
#include <cstdint>

#include "NNNativeEngineAPI/Include/EngineTypes.h"

namespace NN::StubRuntime::AssetRegistry
{
int RegisterAsset(const char* virtualPathUtf8, NNGuid guid);
int UnregisterByGuid(NNGuid guid);
int UnregisterByPath(const char* virtualPathUtf8);
int ResolvePathByGuid(NNGuid guid, char* outUtf8, std::size_t outCapacity);
int ResolveGuidByPath(const char* virtualPathUtf8, NNGuid* outGuid);
std::uint32_t GetDependencyCount(NNGuid guid);
int GetDependencyAt(NNGuid guid, std::uint32_t index, NNGuid* outDependency);
NNGuid ImportAsset(const char* virtualPathUtf8);
} // namespace NN::StubRuntime::AssetRegistry
