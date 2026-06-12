#pragma once

#include "NNSRPMath.h"
#include <cstdint>

namespace NN::Runtime::SRP
{
    // ========================================================================
    //  NNCameraData â€?Camera parameters for rendering
    // ========================================================================

    struct NNCameraData
    {
        Matrix4x4 ViewMatrix;
        Matrix4x4 ProjMatrix;
        Matrix4x4 ViewProjMatrix;

        Vector3 Position;
        float NearPlane = 0.1f;
        float FarPlane = 1000.0f;
        float FOV = 60.0f * 3.14159265f / 180.0f; // radians
        float AspectRatio = 16.0f / 9.0f;

        // Update ViewProj = Proj * View
        void UpdateViewProj()
        {
            ViewProjMatrix = ProjMatrix * ViewMatrix;
        }

        // Setup from look-at parameters
        void SetupPerspective(const Vector3& eye, const Vector3& target, const Vector3& up,
                              float fov, float aspect, float nearZ, float farZ)
        {
            Position = eye;
            FOV = fov;
            AspectRatio = aspect;
            NearPlane = nearZ;
            FarPlane = farZ;

            ViewMatrix = Matrix4x4::LookAt(eye, target, up);
            ProjMatrix = Matrix4x4::Perspective(fov, aspect, nearZ, farZ);
            UpdateViewProj();
        }

        // Setup orthographic (for shadow maps, 2D)
        void SetupOrtho(const Vector3& eye, const Vector3& target, const Vector3& up,
                        float left, float right, float bottom, float top, float nearZ, float farZ)
        {
            Position = eye;
            NearPlane = nearZ;
            FarPlane = farZ;

            ViewMatrix = Matrix4x4::LookAt(eye, target, up);
            ProjMatrix = Matrix4x4::Ortho(left, right, bottom, top, nearZ, farZ);
            UpdateViewProj();
        }
    };

} // namespace NN::Runtime::SRP
