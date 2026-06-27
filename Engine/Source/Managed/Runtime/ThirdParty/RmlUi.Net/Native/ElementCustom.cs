using System;
using System.Runtime.InteropServices;
using System.Text;

namespace RmlUiNet.Native {
    internal static unsafe partial class ElementCustom {
        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_ElementCustom_New", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr Create(string tagName, OnRender? onRender);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_ElementCustom_Free", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr Free(IntPtr element);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate void OnRender();
    }
}
