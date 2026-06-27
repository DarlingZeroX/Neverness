#pragma once

#include "../../VGUIConfig.h"
#include <cstdint>
#include <unordered_map>
#include <vector>

// 本地类型（替代 NNRuntimeScene 中的类型）
#include "../System/RmlUITypes.h"
#include "../System/RmlUIAssetResolver.h"

// RmlUI 核心头文件
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>

namespace Rml
{
	class ElementDocument;
}

// 前向声明渲染后端（Diligent）
namespace RmlDiligent { class RmlDiligentRenderInterface; }
class SystemInterface_SDL;
class UIFileInterfaceVFS;

// 前向声明 Diligent 渲染目标
namespace NN::Runtime::Render
{
	class INNRenderDevice;
	class INNRenderTarget;
}

// 包含完整定义（NNEntityHash 需要完整定义用于 unordered_map）
#include "../System/NNRmlUISystem.h"

namespace NN::Runtime::Renderer
{
	/**
	* @brief 文档运行时状态（简化，无 Loading）。
	*/
	enum class RmlDocState : std::uint8_t
	{
		Ready,      ///< 已加载，可渲染
		Hidden,     ///< 已加载但不可见
		Failed,     ///< 加载失败
	};

	/**
	* @brief 文档运行时实例（Renderer 内部管理）。
	*/
	struct RmlDocumentRuntime
	{
		Rml::ElementDocument* doc = nullptr;
		RmlDocState state = RmlDocState::Ready;
		NNGuid assetGuid{};
	};
}
