/*
 * VGGalgameNodeGraph 模块导出宏
 *
 * 中文说明：
 * - 本库承载 Galgame 节点图在「运行时」侧的执行函数（如 DialogueList、Choice 等），
 *   与编辑器中的节点注册（VGEditorGalgame）配合使用。
 * - 仅在编译本动态库目标时定义 VG_GALGAME_NODEGRAPH_EXPORT，以便导出符号；
 *   其他模块链接本库时使用导入声明。
 */
#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#ifdef VG_GALGAME_NODEGRAPH_EXPORT
#define VG_GALGAME_NODEGRAPH_API __declspec(dllexport)
#else
#define VG_GALGAME_NODEGRAPH_API __declspec(dllimport)
#endif

#else

#ifdef VG_GALGAME_NODEGRAPH_EXPORT
#define VG_GALGAME_NODEGRAPH_API __attribute__((visibility("default")))
#else
#define VG_GALGAME_NODEGRAPH_API
#endif

#endif
