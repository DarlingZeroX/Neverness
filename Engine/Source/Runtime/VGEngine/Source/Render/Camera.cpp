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

#include "Render/Camera.h"
#include "HCore/include/Math/GLM/ext/matrix_clip_space.hpp"

namespace VisionGal
{
	//Letterbox2DCamera::Letterbox2DCamera()
	//{
	//	int width = VGBackend::Get()->GetWindow()->WindowWidth();
	//	int height = VGBackend::Get()->GetWindow()->WindowHeight();
    //
	//	OnScreenSizeChange(width, height);
	//}

	Letterbox2DCamera::Letterbox2DCamera(int width, int height)
	{
        OnViewportSizeChange(width, height);
	}

	const matrix4x4& Letterbox2DCamera::GetMatrix()
	{
		return m_Projection;
	}

    /**
	 * 适配策略枚举
	 */
    //enum class AspectRatioPolicy {
    //    FitVisible,       // 保持可见性（可能有黑边）
    //    FillVisible,      // 填充屏幕（可能裁剪边缘）
    //    Stretch,          // 拉伸（忽略宽高比）
    //    ExpandBackground  // 扩展背景（仅用于屏幕>设计）
    //};

	void Letterbox2DCamera::OnViewportSizeChange(int width, int height)
	{
		m_Width = width;
		m_Height = height;

		//m_Projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

        float designWidth = 1920;
        float designHeight = 1080;
        float actualWidth = m_Width;
        float actualHeight = m_Height;

        // 计算设计宽高比与实际宽高比
        float designAspect = designWidth / designHeight;
        float actualAspect = actualWidth / actualHeight;
        
        // 初始逻辑坐标范围（以设计分辨率为基准）
       m_Left = -designWidth / 2.0f;
       m_Right = designWidth / 2.0f;
       m_Bottom = -designHeight / 2.0f;
       m_Top = designHeight / 2.0f;

        bool screen_small = (designWidth >= actualWidth) && (designHeight >= actualHeight);

        if (actualAspect > designAspect) {
            // 实际屏幕更宽，固定垂直范围，缩放水平范围
            float scale = designAspect / actualAspect;

            // 显示的屏幕小于逻辑尺寸要换为倒数
            if (screen_small)
                scale = 1.0f / scale;

            m_Left *= scale;
            m_Right *= scale;
        }
        else {
            // 实际屏幕更高，固定水平范围，缩放垂直范围
            float scale = actualAspect / designAspect;

            // 显示的屏幕小于逻辑尺寸要换为倒数
            if (screen_small)
                scale = 1.0f / scale;

            m_Bottom *= scale;
            m_Top *= scale;
        }

        // 生成正交投影矩阵
        m_Projection = glm::ortho(m_Left, m_Right, m_Bottom, m_Top, -10000.0f, 10000.0f);
	}
}
