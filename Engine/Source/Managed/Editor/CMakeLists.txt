# =============================================================================
# Engine/Source/Managed/Editor — 托管 Editor 构建
# -----------------------------------------------------------------------------
# 入口项目：NervernessEditor（NevernessEditor 程序集）
#   - 当前为独立 exe 项目，尚未通过 ProjectReference 引用 Runtime.Host
#   - 输出路径与 Runtime 相同：Build/bin/<Config>/（见 NervernessEditor.csproj）
#
# CMake 目标：neverness_managed_editor（ALL，默认构建）
#
# 依赖顺序：
#   1. neverness_managed_runtime — 先完成 Runtime 程序集链（若已启用托管构建）
#   2. VGLauncher / VGEditor       — 与 Runtime 目标相同，确保 C++ 主产物优先
# =============================================================================

neverness_add_dotnet_build(neverness_managed_editor
	"${CMAKE_CURRENT_SOURCE_DIR}/NervernessEditor/NervernessEditor.csproj")

if(TARGET neverness_managed_editor)
	add_dependencies(neverness_managed_editor
		neverness_managed_runtime
		VGLauncher
		VGEditor)
endif()
