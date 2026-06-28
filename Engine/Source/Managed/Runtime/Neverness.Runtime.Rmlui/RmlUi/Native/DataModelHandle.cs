using System;
using System.Runtime.InteropServices;

namespace RmlUiNet.Native {
    internal static unsafe partial class DataModelHandle {
        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_DataModelHandle_DirtyAllVariables", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void DirtyAllVariables(IntPtr instance);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_DataModelHandle_DirtyVariable", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void DirtyVariable(IntPtr instance, string variable_name);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_DataModelHandle_IsVariableDirty", StringMarshalling = StringMarshalling.Utf8)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static partial bool IsVariableDirty(IntPtr instance, string variable_name);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_DataModelHandle_Free", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void Free(IntPtr instance);
    }
}
