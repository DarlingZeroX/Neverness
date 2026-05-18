# Regenerates *Sources.cmake for all large Core/Runtime modules.
$ErrorActionPreference = 'Stop'
$gen = Join-Path $PSScriptRoot 'Generate-CMakeModuleSources.ps1'

$modules = @(
    @{
        ModuleRoot = 'Engine/Source/Core/NNCore'
        VariableName = 'NNCore'
        RelativeGlobs = @(
            '.Generated/*.h', '.Generated/*.cpp',
            '.Generated/Event/*.h', '.Generated/Event/*.cpp',
            '.Generated/System/*.h', '.Generated/System/*.cpp',
            'Source/Asset/*.cpp', 'Source/Core/*.cpp', 'Source/File/*.cpp', 'Source/File/XML/*.cpp',
            'Source/Scene/*.cpp', 'Source/Utils/*.cpp',
            'Interface/*.h',
            'Include/Event/*.h', 'Include/Event/*.cpp',
            'Include/File/*.h', 'Include/File/nlohmann/*.hpp', 'Include/File/XML/*.hpp',
            'Include/Math/GLM/*.h', 'Include/Math/GLM/*.cpp',
            'Include/Math/GLM/detail/*.h', 'Include/Math/GLM/detail/*.cpp',
            'Include/Math/GLM/ext/*.h', 'Include/Math/GLM/ext/*.cpp',
            'Include/Math/GLM/gtc/*.h', 'Include/Math/GLM/gtc/*.cpp',
            'Include/Math/GLM/gtx/*.h', 'Include/Math/GLM/gtx/*.cpp',
            'Include/Math/GLM/simd/*.h', 'Include/Math/GLM/simd/*.cpp',
            'Include/Math/*.h', 'Include/Math/*.cpp',
            'Include/Scene/*.h', 'Include/Stb/*.h', 'Include/Utils/*.h',
            'Include/VFS/*.h', 'Include/VFS/*.hpp',
            'Include/*.h', 'Include/*.cpp',
            '*.h'
        )
    },
    @{
        ModuleRoot = 'Engine/Source/Core/NNPlatformCore'
        VariableName = 'NNPlatformCore'
        RelativeGlobs = @(
            'Interface/*.h',
            'Include/*.h', 'Include/Input/*.h', 'Include/*.cpp',
            'Include/FileWatcher/include/*.h', 'Include/FileWatcher/include/*.cpp',
            'Include/FileWatcher/src/*.h', 'Include/FileWatcher/src/*.cpp',
            'Include/FileWatcher/src/platform/*.h', 'Include/FileWatcher/src/platform/*.cpp',
            'Include/FileWatcher/src/platform/win/*.h', 'Include/FileWatcher/src/platform/win/*.cpp',
            'Include/FileWatcher/src/platform/posix/*.h', 'Include/FileWatcher/src/platform/posix/*.cpp',
            'Include/FileWatcher/src/test/*.h', 'Include/FileWatcher/src/test/*.cpp',
            'Include/NativeFileDialog/*.h', 'Include/NativeFileDialog/*.cpp',
            'Include/SDL3/*.h', 'Include/SDL3/*.cpp',
            'Source/*.cpp', 'Source/Input/*.cpp'
        )
    },
    @{
        ModuleRoot = 'Engine/Source/Core/NNMeta'
        VariableName = 'NNMeta'
        RelativeGlobs = @(
            '.Generated/*.h', '.Generated/*.cpp',
            '.Generated/Event/*.h', '.Generated/Event/*.cpp',
            '.Generated/System/*.h', '.Generated/System/*.cpp',
            'Source/Asset/*.cpp', 'Source/Core/*.cpp', 'Source/File/*.cpp', 'Source/File/XML/*.cpp',
            'Source/Scene/*.cpp', 'Source/Utils/*.cpp',
            'Interface/*.h',
            'Include/Event/*.h', 'Include/Event/*.cpp',
            'Include/File/*.h', 'Include/File/nlohmann/*.hpp', 'Include/File/XML/*.hpp',
            'Include/Math/GLM/*.h', 'Include/Math/GLM/*.cpp',
            'Include/Math/GLM/detail/*.h', 'Include/Math/GLM/detail/*.cpp',
            'Include/Math/GLM/ext/*.h', 'Include/Math/GLM/ext/*.cpp',
            'Include/Math/GLM/gtc/*.h', 'Include/Math/GLM/gtc/*.cpp',
            'Include/Math/GLM/gtx/*.h', 'Include/Math/GLM/gtx/*.cpp',
            'Include/Math/GLM/simd/*.h', 'Include/Math/GLM/simd/*.cpp',
            'Include/Math/*.h', 'Include/Math/*.cpp',
            'Include/Meta/*.h', 'Include/Meta/*.cpp',
            'Include/Meta/Core/*.h', 'Include/Meta/Core/*.cpp',
            'Include/Meta/Core/Argument/*.h', 'Include/Meta/Core/Argument/*.cpp',
            'Include/Meta/Core/Array/*.h', 'Include/Meta/Core/Array/*.cpp',
            'Include/Meta/Core/Constructor/*.h', 'Include/Meta/Core/Constructor/*.cpp',
            'Include/Meta/Core/Destructor/*.h', 'Include/Meta/Core/Destructor/*.cpp',
            'Include/Meta/Core/Enum/*.h', 'Include/Meta/Core/Enum/*.cpp',
            'Include/Meta/Core/FieldInfo/*.h', 'Include/Meta/Core/FieldInfo/*.cpp',
            'Include/Meta/Core/MethodInfo/*.h', 'Include/Meta/Core/MethodInfo/*.cpp',
            'Include/Meta/Core/ParameterInfo/*.h', 'Include/Meta/Core/ParameterInfo/*.cpp',
            'Include/Meta/Core/Type/*.h', 'Include/Meta/Core/Type/*.cpp',
            'Include/Meta/Core/Variant/*.h', 'Include/Meta/Core/Variant/*.cpp',
            'Include/Meta/Deps/*.h', 'Include/Meta/Deps/*.cpp',
            'Include/Meta/Impl/*.hpp',
            'Include/Scene/*.h', 'Include/Stb/*.h', 'Include/Utils/*.h',
            'Include/VFS/*.h', 'Include/VFS/*.hpp',
            'Include/*.h', 'Include/*.cpp',
            '*.h'
        )
    },
    @{
        ModuleRoot = 'Engine/Source/Runtime/NNEngineLegacy'
        VariableName = 'NNEngineLegacy'
        RelativeGlobs = @(
            '*.cpp', '*.h',
            'Source/*.cpp',
            'Source/Animation/Interface/*.cpp', 'Source/Animation/Audio/*.cpp', 'Source/Animation/Core/*.cpp',
            'Source/Asset/*.cpp', 'Source/Asset/Accessor/*.cpp',
            'Source/Core/*.cpp', 'Source/Data/*.cpp', 'Source/Engine/*.cpp', 'Source/Engine/Manager/*.cpp',
            'Source/Event/*.cpp', 'Source/Game/*.cpp', 'Source/Interface/*.cpp', 'Source/Lua/*.cpp',
            'Source/Project/*.cpp', 'Source/Lua/Interface/*.cpp', 'Source/Render/*.cpp',
            'Source/Resource/*.cpp', 'Source/Resource/Texture/*.cpp', 'Source/Scene/*.cpp', 'Source/Utils/*.cpp',
            'Include/*.h', 'Interface/*.h',
            'Include/Animation/Interface/*.h', 'Include/Animation/Audio/*.h', 'Include/Animation/Core/*.h',
            'Include/Asset/*.h', 'Include/Asset/Accessor/*.h',
            'Include/Core/*.h', 'Include/Data/*.h', 'Include/Engine/*.h', 'Include/Engine/Manager/*.h',
            'Include/Game/*.h', 'Include/Interface/*.h', 'Include/Lua/*.h', 'Include/Project/*.h',
            'Include/Lua/Interface/*.h', 'Include/Render/*.h',
            'Include/Resource/*.h', 'Include/Resource/Texture/*.h', 'Include/Resource/Interface/*.h',
            'Include/Scene/*.h', 'Include/Utils/*.h'
        )
    },
    @{
        ModuleRoot = 'Engine/Source/Runtime/NNRuntimeRmlui'
        VariableName = 'NNRuntimeRmlui'
        RelativeGlobs = @(
            'Interface/*.h',
            'Include/*.h', 'Include/Lua/*.h', 'Include/Sol/*.h', 'Include/Rml/*.h', 'Include/Lua/Elements/*.h',
            'Source/*.cpp', 'Source/Lua/*.cpp', 'Source/Sol/*.cpp', 'Source/Rml/*.cpp',
            'Source/Sol/Elements/*.cpp', 'Source/Lua/Elements/*.cpp'
        )
    },
    @{
        ModuleRoot = 'Engine/Source/Runtime/NNRuntimeImGui'
        VariableName = 'NNRuntimeImGui'
        RelativeGlobs = @(
            '*.h', 'Include/*.h', 'Source/*.cpp',
            'Include/Imgui/*.h', 'Include/Imgui/*.cpp', 'Source/Imgui/*.cpp',
            'Include/ImGuiColorTextEdit/*.h', 'Source/ImGuiColorTextEdit/*.cpp',
            'Include/ImguiEx/*.h', 'Source/ImguiEx/*.cpp',
            'Include/ImguiLayer/*.h', 'Source/ImguiLayer/*.cpp',
            'Include/imGuizmo/*.h', 'Source/imGuizmo/*.cpp',
            'Include/ImNodeEditor/*.h', 'Source/ImNodeEditor/*.cpp',
            'Include/Imnodes/*.h', 'Source/Imnodes/*.cpp'
        )
    },
    @{
        ModuleRoot = 'Engine/Source/Editor/NNEditor'
        VariableName = 'NNEditor'
        RelativeGlobs = @('*.cpp', '*.h', 'Include/*.h', 'Source/*.cpp')
    },
    @{
        ModuleRoot = 'Engine/Source/Editor/NNEditorFramework'
        VariableName = 'NNEditorFramework'
        RelativeGlobs = @(
            'Include/*.h',
            'Include/Asset/*.h', 'Include/Asset/ImportStarter/*.h', 'Include/Asset/ImporterUITask/*.h',
            'Include/AssetEditor/*.h', 'Include/AssetImporter/*.h',
            'Include/DetailBrowser/*.h', 'Include/ContentBrowser/*.h', 'Include/CodeStudio/*.h',
            'Include/MainEditor/*.h', 'Include/Setting/*.h', 'Include/Preferences/*.h',
            'Include/UITask/*.h', 'Include/EditorCore/*.h',
            'Interface/*.h',
            'Source/*.cpp',
            'Source/Asset/*.cpp', 'Source/Asset/ImportStarter/*.cpp', 'Source/Asset/ImporterUITask/*.cpp',
            'Source/AssetEditor/*.cpp', 'Source/AssetImporter/*.cpp',
            'Source/DetailBrowser/*.cpp', 'Source/ContentBrowser/*.cpp', 'Source/CodeStudio/*.cpp',
            'Source/MainEditor/*.cpp', 'Source/Setting/*.cpp', 'Source/Preferences/*.cpp',
            'Source/UITask/*.cpp', 'Source/EditorCore/*.cpp', 'Source/Interface/*.cpp'
        )
    },
    @{
        ModuleRoot = 'Engine/Source/Runtime/NNRuntimeLua'
        VariableName = 'NNRuntimeLua'
        RelativeGlobs = @(
            'Source/Lua/lapi.c', 'Source/Lua/lauxlib.c', 'Source/Lua/lbaselib.c', 'Source/Lua/lcode.c',
            'Source/Lua/lcorolib.c', 'Source/Lua/lctype.c', 'Source/Lua/ldblib.c', 'Source/Lua/ldebug.c',
            'Source/Lua/ldo.c', 'Source/Lua/ldump.c', 'Source/Lua/lfunc.c', 'Source/Lua/lgc.c',
            'Source/Lua/linit.c', 'Source/Lua/liolib.c', 'Source/Lua/llex.c', 'Source/Lua/lmathlib.c',
            'Source/Lua/lmem.c', 'Source/Lua/loadlib.c', 'Source/Lua/lobject.c', 'Source/Lua/lopcodes.c',
            'Source/Lua/loslib.c', 'Source/Lua/lparser.c', 'Source/Lua/lstate.c', 'Source/Lua/lstring.c',
            'Source/Lua/lstrlib.c', 'Source/Lua/ltable.c', 'Source/Lua/ltablib.c', 'Source/Lua/ltm.c',
            'Source/Lua/lundump.c', 'Source/Lua/lutf8lib.c', 'Source/Lua/lvm.c', 'Source/Lua/lzio.c',
            'Source/VGLuaCore/LuaLocalizator.cpp', 'Source/VGLuaCore/LuaErrorManager.cpp',
            'Include/*.h', 'Include/*.hpp', 'Include/VGLuaCore/*.h'
        )
    }
)

foreach ($m in $modules) {
    & $gen @m
}
