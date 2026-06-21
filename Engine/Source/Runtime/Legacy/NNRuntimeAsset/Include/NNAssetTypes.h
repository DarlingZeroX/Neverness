#pragma once

/**
 * @file NNAssetTypes.h
 * @brief 資產型別註冊系統。
 *
 * 使用 FNV-1a 64-bit 雜湊將型別名稱字串映射為 uint64 typeId。
 * 同時維護 typeId → 型別名稱的反向映射（除錯/序列化用）。
 */

#include <cstdint>
#include <string>
#include <unordered_map>

namespace NN::Runtime::Asset
{

/**
 * @brief FNV-1a 64-bit 雜湊。
 */
inline std::uint64_t NNFnv1a64(const char* data, std::size_t length) noexcept
{
	std::uint64_t hash = 14695981039346656037ULL;
	for (std::size_t i = 0; i < length; ++i)
	{
		hash ^= static_cast<std::uint64_t>(static_cast<unsigned char>(data[i]));
		hash *= 1099511628211ULL;
	}
	return hash;
}

inline std::uint64_t NNFnv1a64(const std::string& s) noexcept
{
	return NNFnv1a64(s.data(), s.size());
}

/**
 * @brief 資產型別註冊表。
 *
 * 在啟動時註冊所有支援的資產型別；查詢時由 typeId 得到型別名稱。
 */
class NNAssetTypeRegistry
{
public:
	static NNAssetTypeRegistry& Instance() noexcept
	{
		static NNAssetTypeRegistry instance;
		return instance;
	}

	/** @brief 註冊型別（名稱 → FNV-1a → typeId）。 */
	void RegisterType(const std::string& typeName, std::uint64_t predefinedId = 0)
	{
		const std::uint64_t id = predefinedId != 0 ? predefinedId : NNFnv1a64(typeName);
		nameToId_[typeName] = id;
		idToName_[id] = typeName;
	}

	/** @brief 依名稱取得 typeId。回傳 0 表示未註冊。 */
	std::uint64_t GetTypeId(const std::string& typeName) const
	{
		const auto it = nameToId_.find(typeName);
		return it != nameToId_.end() ? it->second : 0;
	}

	/** @brief 依 typeId 取得名稱。回傳空字串表示未知。 */
	const std::string& GetTypeName(std::uint64_t typeId) const
	{
		static const std::string empty;
		const auto it = idToName_.find(typeId);
		return it != idToName_.end() ? it->second : empty;
	}

	/** @brief 型別是否已註冊。 */
	bool IsRegistered(std::uint64_t typeId) const
	{
		return idToName_.find(typeId) != idToName_.end();
	}

private:
	NNAssetTypeRegistry() = default;

	std::unordered_map<std::string, std::uint64_t> nameToId_;
	std::unordered_map<std::uint64_t, std::string> idToName_;
};

} // namespace NN::Runtime::Asset
