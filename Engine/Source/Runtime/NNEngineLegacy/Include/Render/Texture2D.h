/*
* This source file is part of VisionGal, the Visual Novel Engine
*
* For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
* GitHub page: https://github.com/DarlingZeroX/VisionGal
*
* Copyright (c) 2025-present 梦旅缘心
*
* See the LICENSE file in the project root for details.
*/

#pragma once
#include "../EngineConfig.h"
#include "NNRuntimeCore/Include/Core/Core.h"
#include <NNRuntimeRHI/Interface/Texture.h>
#include "NNKernel/Interface/HCoreTypes.h"

namespace VisionGal {

	//struct RenderResource : public VGObject
	//{
	//	~RenderResource() override = default;
	//
	//	const String& GetResourcePath() { return m_ResourcePath; }
	//	void SetResourcePath(const String& path) { m_ResourcePath = path; }
	//
	//private:
	//	String m_ResourcePath;
	//};

	struct VG_ENGINE_API Texture2D : public VGEngineResource
	{
		Texture2D();
		Texture2D(Ref<VGFX::ITexture> texture);

		Texture2D(const Texture2D&) = delete;
		Texture2D& operator=(const Texture2D&) = delete;
		Texture2D(Texture2D&&) noexcept = default;
		Texture2D& operator=(Texture2D&&) noexcept = default;

		~Texture2D() override = default;

		int Width();
		int Height();
		float2 Size();
		const VGFX::TextureDesc* GetTextureDesc();

		Ref<VGFX::ITexture>& GetTexture();
		void SetTexture(const Ref<VGFX::ITexture>& texture);

	private:
		Ref<VGFX::ITexture> m_Texture;
	};
}
