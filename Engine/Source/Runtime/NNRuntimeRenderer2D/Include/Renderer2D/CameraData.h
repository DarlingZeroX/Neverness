#pragma once

/**
 * @file CameraData.h
 * @brief 相机渲染数据：由 SceneRenderer 从 CameraComponent + Transform 计算。
 *
 * 不依赖 Scene 头文件，保持解耦。
 */

namespace NN::Runtime::Renderer2D
{
    /// 相机渲染数据
    struct CameraData
    {
        float ViewMatrix[16];           ///< 逆 Transform WorldMatrix（4x4 列主序）
        float ProjectionMatrix[16];     ///< 来自 CameraComponent（4x4 列主序）
        float ViewProjectionMatrix[16]; ///< Proj * View（4x4 列主序）
        float OrthoWidth  = 10.0f;
        float OrthoHeight = 10.0f;
        float Near        = -1.0f;
        float Far         =  1.0f;
    };
}
