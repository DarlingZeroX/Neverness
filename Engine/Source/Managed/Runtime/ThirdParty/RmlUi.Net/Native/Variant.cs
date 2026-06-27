using RmlUiNet;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace RmlUiNet.Native {
    internal static unsafe partial class Variant {
        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Variant_CreateInt", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr CreateInt(int value);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Variant_CreateString", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr CreateString(string value);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Variant_CreateUInt", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr CreateUInt(uint value);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Variant_CreateFloat", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr CreateFloat(float value);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Variant_CreateDouble", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr CreateDouble(double value);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Variant_CreateBool", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr CreateBool([MarshalAs(UnmanagedType.U1)] bool value);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Variant_GetType", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial int GetType(IntPtr variant);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Variant_GetAsDouble", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial double GetAsDouble(IntPtr variant, double defaultValue);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Variant_GetAsInt", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial int GetAsInt(IntPtr variant, int defaultValue);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Variant_GetAsString", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr GetAsString(IntPtr variant, string defaultValue);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Variant_GetAsUInt", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial uint GetAsUInt(IntPtr variant, uint defaultValue);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Variant_GetAsFloat", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial float GetAsFloat(IntPtr variant, float defaultValue);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Variant_GetAsBool", StringMarshalling = StringMarshalling.Utf8)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static partial bool GetAsBool(IntPtr variant, [MarshalAs(UnmanagedType.U1)] bool defaultValue);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Variant_Free", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void Free(IntPtr variant);
    }
}
