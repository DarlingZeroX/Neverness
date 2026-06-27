using System;
using System.Runtime.InteropServices;
using System.Text;

namespace RmlUiNet.Native
{
    internal static unsafe partial class StyleSheetContainer
    {
        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_StyleSheetContainer_New", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr Create();

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_StyleSheetContainer_Free", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void Free(IntPtr container);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_StyleSheetContainer_LoadStyleSheetContainer", StringMarshalling = StringMarshalling.Utf8)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static partial bool LoadStyleSheetContainer(IntPtr container, string path);
    }
}
