using System;
using System.Numerics;
using System.Runtime.InteropServices;

namespace RmlUiNet.Native
{
     internal static unsafe partial class RenderInterface
    {
        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_RenderInterface_New")]
        internal static partial IntPtr Create(
            OnCompileGeometry onCompileGeometry,
            OnRenderGeometry onRenderGeometry,
            OnReleaseGeometry onReleaseGeometry,
            OnLoadTexture onLoadTexture,
            OnGenerateTexture onGenerateTexture,
            OnReleaseTexture onReleaseTexture,
            OnEnableScissorRegion onEnableScissorRegion,
            OnSetScissorRegion onSetScissorRegion,
            OnSetTransform onSetTransform
        );

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate IntPtr OnCompileGeometry(Vertex* vertices, int vertexCount, int* indices, int indexCount);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate void OnRenderGeometry(IntPtr geometry, Vector2f translation, IntPtr texture);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate void OnReleaseGeometry(IntPtr geometry);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate IntPtr OnLoadTexture(ref Vector2i dimensions, string source);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate IntPtr OnGenerateTexture(byte* source, int numBytes, Vector2i dimensions);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate void OnReleaseTexture(IntPtr textureHandle);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate void OnEnableScissorRegion(bool enable);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate void OnSetScissorRegion(int x, int y, int width, int height);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate void OnSetTransform(IntPtr transform);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_RenderInterface_Test")]
        internal static partial IntPtr Test(IntPtr renderInterface);
    }
}
