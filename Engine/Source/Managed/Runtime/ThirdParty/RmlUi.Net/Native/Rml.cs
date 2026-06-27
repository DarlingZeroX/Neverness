using System;
using System.Runtime.InteropServices;

namespace RmlUiNet.Native
{
    internal static unsafe partial class Rml
    {
        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Initialise", StringMarshalling = StringMarshalling.Utf8)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static partial bool Initialise();

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Shutdown", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void Shutdown();

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_SetRenderInterface", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void SetRenderInterface(IntPtr renderInterface);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_SetSystemInterface", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void SetSystemInterface(IntPtr systemInterface);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_SetFileInterface", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void SetFileInterface(IntPtr fileInterface);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_LoadFontFace", StringMarshalling = StringMarshalling.Utf8)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static partial bool LoadFontFace(string fileName, [MarshalAs(UnmanagedType.U1)] bool fallbackFace, FontWeight weight);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_CreateEventListener", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr CreateEventListener(OnProcessEvent onProcessEvent, OnAttachEvent onAttachEvent, OnDetachEvent onDetachEvent);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_ReleaseEventListener", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void ReleaseEventListener(IntPtr eventListener);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_RemoveContext", StringMarshalling = StringMarshalling.Utf8)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static partial bool RemoveContext(string name);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_RegisterPlugin", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void RegisterPlugin(IntPtr pluginPtr);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_GetRenderInterface", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr GetRenderInterface();

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_GetSystemInterface", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr GetSystemInterface();

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_CreateString", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr CreateString(string str);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Log", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr Log(LogType type, string str);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_UpdateString", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void UpdateString(IntPtr str, string newContent);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_FreeString", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void FreeString(IntPtr str);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_RegisterEventType", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial EventId RegisterEventType(string type, [MarshalAs(UnmanagedType.U1)] bool interruptible, [MarshalAs(UnmanagedType.U1)] bool bubbles, DefaultActionPhase default_action_phase);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate void OnProcessEvent(IntPtr eventPtr);
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate void OnAttachEvent(IntPtr elementPtr, string elementType);
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate void OnDetachEvent(IntPtr elementPtr, string elementType);
    }
}
