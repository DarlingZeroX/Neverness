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
#include "../Interface/GameInterface.h"

namespace VisionGal
{
	class Letterbox2DCamera: public IOrthoCamera
	{
	public:
		//Letterbox2DCamera();
		Letterbox2DCamera(int width, int height);
		Letterbox2DCamera(const Letterbox2DCamera&) = delete;
		Letterbox2DCamera& operator=(const Letterbox2DCamera&) = delete;
		Letterbox2DCamera(Letterbox2DCamera&&) noexcept = default;
		Letterbox2DCamera& operator=(Letterbox2DCamera&&) noexcept = default;
		~Letterbox2DCamera() override = default;

		const matrix4x4& GetMatrix() override;
		void OnViewportSizeChange(int width, int height) override;
		std::string GetCameraType() override { return "Letterbox2D"; }

		float GetLeft() override { return m_Left; }
		float GetRight() override { return m_Right; }
		float GetTop() override { return m_Top; }
		float GetBottom() override { return m_Bottom; }
		float GetWidth() override { return m_Right * 2.0f; };
		float GetHeight() override { return m_Top * 2.0f; };
	private:
		float m_Left;
		float m_Right;
		float m_Bottom;
		float m_Top;
		int m_Width, m_Height;
		matrix4x4 m_Projection;
	};
}