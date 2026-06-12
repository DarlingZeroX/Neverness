// NNEngine.cs — C# P/Invoke bindings for NNRuntime Engine
// Drop this file into your C# project to call the native rendering API.

using System;
using System.Runtime.InteropServices;

namespace Neverness.Engine
{
    // ========================================================================
    //  Handle type (matches NNE_Handle in C++)
    // ========================================================================

    /// <summary>
    /// Opaque handle to a native engine resource.
    /// Do NOT interpret the value; pass it back to native functions.
    /// </summary>
    public readonly struct NNEHandle : IEquatable<NNEHandle>
    {
        public static readonly NNEHandle Invalid = new NNEHandle(0);

        private readonly ulong _value;

        public NNEHandle(ulong value) => _value = value;

        public bool IsValid => _value != 0;

        public bool Equals(NNEHandle other) => _value == other._value;
        public override bool Equals(object obj) => obj is NNEHandle h && Equals(h);
        public override int GetHashCode() => _value.GetHashCode();
        public override string ToString() => $"0x{_value:X16}";

        public static bool operator ==(NNEHandle a, NNEHandle b) => a._value == b._value;
        public static bool operator !=(NNEHandle a, NNEHandle b) => a._value != b._value;
    }

    // ========================================================================
    //  Data structures (must match C #pragma pack(push, 1))
    // ========================================================================

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct NNEVec3
    {
        public float X, Y, Z;

        public NNEVec3(float x, float y, float z) { X = x; Y = y; Z = z; }
    }

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct NNEMat4x4
    {
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
        public float[] M;

        public static NNEMat4x4 Identity => new NNEMat4x4
        {
            M = new float[] { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 }
        };
    }

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct NNECameraData
    {
        public NNEMat4x4 ViewMatrix;
        public NNEMat4x4 ProjMatrix;
        public NNEMat4x4 ViewProjMatrix;
        public NNEVec3 Position;
        public float NearPlane;
        public float FarPlane;
        public float FOV;
        public float AspectRatio;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct NNELightData
    {
        public int Type;           // 0=Directional, 1=Point, 2=Spot
        public NNEVec3 Direction;
        public NNEVec3 Position;
        public NNEVec3 Color;
        public float Intensity;
        public int CastShadows;
        public float ShadowBias;
        public NNEMat4x4 LightViewProj;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct NNERenderContextData
    {
        public NNECameraData Camera;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
        public NNELightData[] Lights;

        public int LightCount;
        public float DeltaTime;
        public float TotalTime;
        public ulong FrameNumber;
        public uint FrameWidth;
        public uint FrameHeight;
    }

    // ========================================================================
    //  Shader stage enum
    // ========================================================================

    public enum NNEShaderStage : int
    {
        Vertex = 0,
        Pixel = 1,
        Geometry = 2,
        Hull = 3,
        Domain = 4,
        Compute = 5
    }

    // ========================================================================
    //  Blend/Cull enums
    // ========================================================================

    public enum NNECullMode : int
    {
        None = 0,
        Front = 1,
        Back = 2
    }

    public enum NNEBlendFactor : int
    {
        Zero = 0,
        One = 1,
        SrcAlpha = 2,
        InvSrcAlpha = 3
    }

    // ========================================================================
    //  Native API — Device (existing)
    // ========================================================================

    public static class NNE_DeviceAPI
    {
        private const string DLL = "NNRuntimeNativeEngineAPI";

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr NNE_GetDevice();

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void NNE_SetDevice(IntPtr device);
    }

    // ========================================================================
    //  Native API — Resource Creation (existing)
    // ========================================================================

    public static class NNE_ResourceAPI
    {
        private const string DLL = "NNRuntimeNativeEngineAPI";

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern NNEHandle NNE_CreateBuffer(uint size, uint type, uint usage, IntPtr initialData);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern NNEHandle NNE_CreateTexture(uint width, uint height, uint format, uint usage,
            [MarshalAs(UnmanagedType.LPStr)] string debugName);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern NNEHandle NNE_CreateSampler(uint minFilter, uint magFilter, uint addressU, uint addressV);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern NNEHandle NNE_CreateShader(uint stage,
            [MarshalAs(UnmanagedType.LPStr)] string sourceCode,
            [MarshalAs(UnmanagedType.LPStr)] string entryPoint,
            [MarshalAs(UnmanagedType.LPStr)] string debugName);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern NNEHandle NNE_CreatePipelineState(NNEHandle vs, NNEHandle ps, IntPtr psoDesc);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern NNEHandle NNE_CreateRenderTarget(uint width, uint height, uint colorFormat, uint depthFormat);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void NNE_ReleaseHandle(NNEHandle handle);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern uint NNE_GetHandleType(NNEHandle handle);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern uint NNE_IsHandleValid(NNEHandle handle);
    }

    // ========================================================================
    //  Native API — Render Commands (existing)
    // ========================================================================

    public static class NNE_RenderAPI
    {
        private const string DLL = "NNRuntimeNativeEngineAPI";

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void NNE_ClearRenderTarget(float r, float g, float b, float a);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void NNE_SetPipelineState(NNEHandle pso);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void NNE_SetVertexBuffer(NNEHandle vb, uint slot);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void NNE_SetIndexBuffer(NNEHandle ib);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void NNE_SetViewport(float x, float y, float w, float h, float minD, float maxD);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void NNE_Draw(uint vertexCount, uint startVertex, uint instanceCount);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void NNE_DrawIndexed(uint indexCount, uint startIndex, int baseVertex, uint instanceCount);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void NNE_Present();
    }

    // ========================================================================
    //  Native API — Shader Asset (new)
    // ========================================================================

    public static class NNE_ShaderAPI
    {
        private const string DLL = "NNRuntimeNativeEngineAPI";

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern NNEHandle NNE_LoadShaderFromMemory(NNEHandle device,
            [MarshalAs(UnmanagedType.LPStr)] string source, uint length,
            int stage,
            [MarshalAs(UnmanagedType.LPStr)] string name,
            [MarshalAs(UnmanagedType.LPStr)] string entryPoint);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern NNEHandle NNE_LoadShaderFromFile(NNEHandle device,
            [MarshalAs(UnmanagedType.LPStr)] string path, int stage,
            [MarshalAs(UnmanagedType.LPStr)] string entryPoint);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void NNE_ReleaseShaderAsset(NNEHandle shaderAsset);
    }

    // ========================================================================
    //  Native API — Material (new)
    // ========================================================================

    public static class NNE_MaterialAPI
    {
        private const string DLL = "NNRuntimeNativeEngineAPI";

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern NNEHandle NNE_CreateMaterial();

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void NNE_MaterialSetVS(NNEHandle material, NNEHandle shaderAsset);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void NNE_MaterialSetPS(NNEHandle material, NNEHandle shaderAsset);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void NNE_MaterialSetFloat(NNEHandle material,
            [MarshalAs(UnmanagedType.LPStr)] string name, float value);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void NNE_MaterialSetInt(NNEHandle material,
            [MarshalAs(UnmanagedType.LPStr)] string name, int value);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void NNE_MaterialSetVec2(NNEHandle material,
            [MarshalAs(UnmanagedType.LPStr)] string name, float x, float y);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void NNE_MaterialSetVec3(NNEHandle material,
            [MarshalAs(UnmanagedType.LPStr)] string name, float x, float y, float z);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void NNE_MaterialSetVec4(NNEHandle material,
            [MarshalAs(UnmanagedType.LPStr)] string name, float x, float y, float z, float w);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void NNE_MaterialSetTexture(NNEHandle material,
            [MarshalAs(UnmanagedType.LPStr)] string name, NNEHandle texture);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void NNE_MaterialSetSampler(NNEHandle material,
            [MarshalAs(UnmanagedType.LPStr)] string name, NNEHandle sampler);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void NNE_MaterialSetCullMode(NNEHandle material, int cullMode);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void NNE_MaterialSetBlendEnable(NNEHandle material, int enable);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void NNE_MaterialSetDepthEnable(NNEHandle material, int depthEnable, int depthWrite);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern float NNE_MaterialGetFloat(NNEHandle material,
            [MarshalAs(UnmanagedType.LPStr)] string name);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern int NNE_MaterialGetInt(NNEHandle material,
            [MarshalAs(UnmanagedType.LPStr)] string name);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void NNE_ReleaseMaterial(NNEHandle material);
    }

    // ========================================================================
    //  Native API — Material Instance (new)
    // ========================================================================

    public static class NNE_MaterialInstanceAPI
    {
        private const string DLL = "NNRuntimeNativeEngineAPI";

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern NNEHandle NNE_CreateMaterialInstance(NNEHandle baseMaterial);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void NNE_MatInstSetFloat(NNEHandle instance,
            [MarshalAs(UnmanagedType.LPStr)] string name, float value);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void NNE_MatInstSetInt(NNEHandle instance,
            [MarshalAs(UnmanagedType.LPStr)] string name, int value);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void NNE_MatInstSetVec4(NNEHandle instance,
            [MarshalAs(UnmanagedType.LPStr)] string name, float x, float y, float z, float w);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void NNE_MatInstSetTexture(NNEHandle instance,
            [MarshalAs(UnmanagedType.LPStr)] string name, NNEHandle texture);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void NNE_MatInstSetSampler(NNEHandle instance,
            [MarshalAs(UnmanagedType.LPStr)] string name, NNEHandle sampler);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void NNE_ReleaseMaterialInstance(NNEHandle instance);
    }

    // ========================================================================
    //  Native API — SRP Pipeline (new)
    // ========================================================================

    public static class NNE_PipelineAPI
    {
        private const string DLL = "NNRuntimeNativeEngineAPI";

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern NNEHandle NNE_CreateForwardPipeline(NNEHandle device, uint width, uint height);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void NNE_DestroyPipeline(NNEHandle pipeline);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void NNE_ResizePipeline(NNEHandle pipeline, uint width, uint height);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern uint NNE_PipelineGetPassCount(NNEHandle pipeline);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void NNE_ExecutePipeline(NNEHandle pipeline, NNEHandle cmdList,
            ref NNERenderContextData ctx);
    }

    // ========================================================================
    //  High-level wrapper: Material
    // ========================================================================

    /// <summary>
    /// Managed wrapper around a native material handle.
    /// </summary>
    public class NNEMaterial : IDisposable
    {
        public NNEHandle Handle { get; private set; }

        public NNEMaterial()
        {
            Handle = NNE_MaterialAPI.NNE_CreateMaterial();
        }

        public void SetShader(NNEHandle vs, NNEHandle ps)
        {
            NNE_MaterialAPI.NNE_MaterialSetVS(Handle, vs);
            NNE_MaterialAPI.NNE_MaterialSetPS(Handle, ps);
        }

        public void SetFloat(string name, float value) =>
            NNE_MaterialAPI.NNE_MaterialSetFloat(Handle, name, value);

        public void SetInt(string name, int value) =>
            NNE_MaterialAPI.NNE_MaterialSetInt(Handle, name, value);

        public void SetVec4(string name, float x, float y, float z, float w) =>
            NNE_MaterialAPI.NNE_MaterialSetVec4(Handle, name, x, y, z, w);

        public void SetTexture(string name, NNEHandle tex) =>
            NNE_MaterialAPI.NNE_MaterialSetTexture(Handle, name, tex);

        public void SetSampler(string name, NNEHandle sampler) =>
            NNE_MaterialAPI.NNE_MaterialSetSampler(Handle, name, sampler);

        public void SetCullMode(NNECullMode mode) =>
            NNE_MaterialAPI.NNE_MaterialSetCullMode(Handle, (int)mode);

        public void SetBlend(bool enable) =>
            NNE_MaterialAPI.NNE_MaterialSetBlendEnable(Handle, enable ? 1 : 0);

        public void SetDepth(bool enable, bool write) =>
            NNE_MaterialAPI.NNE_MaterialSetDepthEnable(Handle, enable ? 1 : 0, write ? 1 : 0);

        public float GetFloat(string name) =>
            NNE_MaterialAPI.NNE_MaterialGetFloat(Handle, name);

        public int GetInt(string name) =>
            NNE_MaterialAPI.NNE_MaterialGetInt(Handle, name);

        public void Dispose()
        {
            if (Handle.IsValid)
            {
                NNE_MaterialAPI.NNE_ReleaseMaterial(Handle);
                Handle = NNEHandle.Invalid;
            }
        }
    }

    // ========================================================================
    //  High-level wrapper: Material Instance
    // ========================================================================

    public class NNEMaterialInstance : IDisposable
    {
        public NNEHandle Handle { get; private set; }

        public NNEMaterialInstance(NNEMaterial baseMaterial)
        {
            Handle = NNE_MaterialInstanceAPI.NNE_CreateMaterialInstance(baseMaterial.Handle);
        }

        public void SetFloat(string name, float value) =>
            NNE_MaterialInstanceAPI.NNE_MatInstSetFloat(Handle, name, value);

        public void SetVec4(string name, float x, float y, float z, float w) =>
            NNE_MaterialInstanceAPI.NNE_MatInstSetVec4(Handle, name, x, y, z, w);

        public void SetTexture(string name, NNEHandle tex) =>
            NNE_MaterialInstanceAPI.NNE_MatInstSetTexture(Handle, name, tex);

        public void Dispose()
        {
            if (Handle.IsValid)
            {
                NNE_MaterialInstanceAPI.NNE_ReleaseMaterialInstance(Handle);
                Handle = NNEHandle.Invalid;
            }
        }
    }

    // ========================================================================
    //  High-level wrapper: Forward Pipeline
    // ========================================================================

    public class NNEForwardPipeline : IDisposable
    {
        public NNEHandle Handle { get; private set; }

        public NNEForwardPipeline(NNEHandle device, uint width, uint height)
        {
            Handle = NNE_PipelineAPI.NNE_CreateForwardPipeline(device, width, height);
        }

        public uint PassCount => NNE_PipelineAPI.NNE_PipelineGetPassCount(Handle);

        public void Resize(uint width, uint height) =>
            NNE_PipelineAPI.NNE_ResizePipeline(Handle, width, height);

        public void Execute(NNEHandle cmdList, ref NNERenderContextData ctx) =>
            NNE_PipelineAPI.NNE_ExecutePipeline(Handle, cmdList, ref ctx);

        public void Dispose()
        {
            if (Handle.IsValid)
            {
                NNE_PipelineAPI.NNE_DestroyPipeline(Handle);
                Handle = NNEHandle.Invalid;
            }
        }
    }

} // namespace Neverness.Engine
