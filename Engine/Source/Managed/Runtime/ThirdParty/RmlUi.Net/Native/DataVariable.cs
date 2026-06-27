using System;
using System.Runtime.InteropServices;

namespace RmlUiNet.Native {
    internal static unsafe partial class DataVariable {
        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_DataVariable_Create")]
        internal static partial IntPtr Create(IntPtr definition, IntPtr data);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_DataVariable_Free")]
        internal static partial void Free(IntPtr instance);
    }
}
