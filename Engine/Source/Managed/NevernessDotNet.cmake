# =============================================================================
# NevernessDotNet.cmake
# -----------------------------------------------------------------------------
# 作用：为 Engine/Source/Managed 下的 C# 项目提供统一的 CMake 集成。
#
# 设计要点：
#   - 通过 dotnet build 编译 .csproj，与 C++ 目标同属一次 cmake --build。
#   - 输出目录与 .csproj 中 BaseOutputPath/OutputPath 一致：
#       $(SolutionDir)Build\bin\$(Configuration)\
#     即仓库根目录下的 Build/bin/<Config>/（与根 CMakeLists 中 NN_OUTPUT_ROOT 对齐）。
#   - 命名约定：本模块新增的 target/option/函数均使用 neverness_ / NEVERNESS_ 前缀，
#     不使用 visiongal / VisionGal。
#
# 用法（在子目录 CMakeLists.txt 中）：
#   include(.../NevernessDotNet.cmake)   # 通常由 Managed/CMakeLists.txt 统一 include
#   neverness_add_dotnet_build(<target名> <csproj绝对或相对路径>)
#
# 关闭托管编译：cmake -DNEVERNESS_BUILD_MANAGED=OFF
# =============================================================================

# 防止同一 configure 周期内被多次 include 时重复定义 option / function
if(NOT DEFINED NEVERNESS_DOTNET_INCLUDED)
	set(NEVERNESS_DOTNET_INCLUDED TRUE)

	# 总开关：OFF 时 neverness_add_dotnet_build 直接 return，不创建任何 C# 构建目标
	option(NEVERNESS_BUILD_MANAGED
		"是否编译 Engine/Source/Managed 下的 C# 项目（dotnet build）"
		ON)

	# 查找 .NET SDK 命令行工具；未找到时各子目录会跳过托管目标（configure 不失败）
	find_program(NEVERNESS_DOTNET_EXECUTABLE dotnet DOC ".NET SDK CLI")

	# -------------------------------------------------------------------------
	# neverness_add_dotnet_build(TARGET_NAME CSPROJ_PATH)
	#
	# 创建一个 ALL 自定义目标，在构建阶段执行：
	#   dotnet build <csproj> -c <Config> -p:SolutionDir=<仓库根>/
	#
	# 参数：
	#   TARGET_NAME  — CMake 目标名（如 neverness_managed_runtime）
	#   CSPROJ_PATH  — .csproj 文件路径（必须存在，否则 FATAL_ERROR）
	#
	# 配置映射：
	#   - 多配置生成器（Visual Studio）：$<CONFIG> 为 Debug / Release 等
	#   - 单配置生成器（Ninja + CMAKE_BUILD_TYPE）：$<CONFIG> 与 CMAKE_BUILD_TYPE 一致
	# -------------------------------------------------------------------------
	function(neverness_add_dotnet_build TARGET_NAME CSPROJ_PATH)
		if(NOT NEVERNESS_BUILD_MANAGED)
			return()
		endif()

		if(NOT NEVERNESS_DOTNET_EXECUTABLE)
			message(STATUS "未找到 dotnet，跳过托管目标: ${TARGET_NAME}")
			return()
		endif()

		if(NOT EXISTS "${CSPROJ_PATH}")
			message(FATAL_ERROR
				"neverness_add_dotnet_build: 找不到 csproj: ${CSPROJ_PATH}")
		endif()

		# MSBuild 要求 SolutionDir 以路径分隔符结尾；与 .csproj 中 $(SolutionDir) 用法一致
		file(TO_CMAKE_PATH "${CMAKE_SOURCE_DIR}" _neverness_solution_dir)
		set(_neverness_solution_dir "${_neverness_solution_dir}/")

		add_custom_target(${TARGET_NAME} ALL
			COMMAND "${NEVERNESS_DOTNET_EXECUTABLE}" build "${CSPROJ_PATH}"
				-c $<CONFIG>
				-p:SolutionDir=${_neverness_solution_dir}
			WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
			COMMENT "正在编译托管程序集: ${TARGET_NAME}"
			VERBATIM
		)

		# 修改 .csproj 时触发 CMake 重新 configure，便于 IDE 感知工程变更
		set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" APPEND
			PROPERTY CMAKE_CONFIGURE_DEPENDS "${CSPROJ_PATH}")
	endfunction()

endif()
