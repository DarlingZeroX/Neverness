using System;
using System.Runtime.InteropServices;

namespace RmlUiNet.Native {
     internal static unsafe partial class DataModelConstructor {
        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_DataModelConstructor_BindFloat", StringMarshalling = StringMarshalling.Utf8)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static partial bool BindFloat(IntPtr instance, string name, IntPtr data);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_DataModelConstructor_BindUInt", StringMarshalling = StringMarshalling.Utf8)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static partial bool BindUInt(IntPtr instance, string name, IntPtr data);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_DataModelConstructor_BindInt", StringMarshalling = StringMarshalling.Utf8)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static partial bool BindInt(IntPtr instance, string name, IntPtr data);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_DataModelConstructor_BindBool", StringMarshalling = StringMarshalling.Utf8)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static partial bool BindBool(IntPtr instance, string name, IntPtr data);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_DataModelConstructor_BindString", StringMarshalling = StringMarshalling.Utf8)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static partial bool BindString(IntPtr instance, string name, IntPtr data);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_DataModelConstructor_BindStringArray", StringMarshalling = StringMarshalling.Utf8)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static partial bool BindStringArray(IntPtr instance, string name, IntPtr data, int count);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_DataModelConstructor_UnbindStringArray", StringMarshalling = StringMarshalling.Utf8)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static partial bool UnbindStringArray(IntPtr instance, string name);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_DataModelConstructor_GetModelHandle", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr GetModelHandle(IntPtr instance);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_DataModelConstructor_RegisterStruct", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr RegisterStruct(IntPtr instance, string name);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_DataModelConstructor_BindVariable", StringMarshalling = StringMarshalling.Utf8)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static partial bool BindVariable(IntPtr instance, string name, IntPtr variable);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_DataModelHandle_Free", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void Free(IntPtr instance);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_DataModelConstructor_BindEventCallback", StringMarshalling = StringMarshalling.Utf8)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static partial bool BindEventCallback(IntPtr instance, string name, DataEventFunc func);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate void DataEventFunc(IntPtr dataModel, IntPtr evt, nint* variants, int numVariants);
    }
}
