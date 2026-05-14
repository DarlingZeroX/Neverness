#pragma once

#include <filesystem>
#include <memory>

/**
 * Encapsulates nethost + hostfxr + load_assembly_and_get_function_pointer.
 * No public engine headers include hostfxr — this header stays in the module Private/ include path only.
 */
class CoreCLRLoader
{
public:
	CoreCLRLoader();
	~CoreCLRLoader();

	CoreCLRLoader(const CoreCLRLoader&) = delete;
	CoreCLRLoader& operator=(const CoreCLRLoader&) = delete;

	bool initialize(
		const std::filesystem::path& runtime_config_json,
		const std::filesystem::path* assembly_path_hint);

	void shutdown();

	bool is_initialized() const;

	void* host_context_opaque() const;

	void* cached_load_assembly_and_get_function_pointer() const;

	bool resolve_unmanaged_callers_only(
		const std::filesystem::path& assembly_path,
		const char* assemblyQualifiedTypeNameUtf8,
		const char* methodNameUtf8,
		void** out_function) const;
private:
	struct Impl;
	std::unique_ptr<Impl> impl_;
};
