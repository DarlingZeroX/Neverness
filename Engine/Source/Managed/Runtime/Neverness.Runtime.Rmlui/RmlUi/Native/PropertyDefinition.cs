using System;
using System.Runtime.InteropServices;

namespace RmlUiNet.Native
{
    internal static unsafe partial class PropertyDefinition
    {
        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_PropertyDefinition_AddParser", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr AddParser(IntPtr propertyDefinition, string parserName, string parserParams);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_PropertyDefinition_GetId", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial int GetId(IntPtr propertyDefinition);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_PropertyDefinition_GetValueString", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial string GetValueString(IntPtr propertyDefinition, IntPtr property, string defaultValue);
    }
}
