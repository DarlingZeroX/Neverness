#pragma once

#if defined(_WIN32) || defined(_WIN64)
#ifdef HNG_EDITOR_CORE_API_EXPORT
#define HNG_EDITOR_CORE_API __declspec(dllexport)
#else
#define HNG_EDITOR_CORE_API __declspec(dllimport)
#endif

#else
#ifdef HNG_EDITOR_CORE_API_EXPORT
#define HNG_EDITOR_CORE_API __attribute__((visibility("default")))
#else
#define HNG_EDITOR_CORE_API
#endif
#endif

//#include <NNRuntimeImGui/Include/ImNodeEditor/imgui_node_editor.h>
//
//namespace IMNE = ax::NodeEditor;

