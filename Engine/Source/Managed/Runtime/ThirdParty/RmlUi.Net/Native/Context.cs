using System;
using System.Runtime.InteropServices;

namespace RmlUiNet.Native
{
     internal static unsafe partial class Context
    {
        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Context_New", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr Create(string name, Vector2i dimensions, IntPtr renderInterface);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Context_Delete", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void Delete(IntPtr context);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Context_Update", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void Update(IntPtr context);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Context_SetDimensions", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void SetDimensions(IntPtr context, int x, int y);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Context_Render", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void Render(IntPtr context);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Context_LoadDocument", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr LoadDocument(IntPtr context, string documentPath);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Context_GetHoverElement", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr GetHoverElement(IntPtr context);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Context_LoadDocumentFromMemory", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr LoadDocumentFromMemory(IntPtr context, string documentRml, string sourceUrl);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Context_IsMouseInteracting", StringMarshalling = StringMarshalling.Utf8)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static partial bool IsMouseInteracting(IntPtr context);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Context_ProcessMouseLeave", StringMarshalling = StringMarshalling.Utf8)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static partial bool ProcessMouseLeave(IntPtr context);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Context_ProcessMouseMove", StringMarshalling = StringMarshalling.Utf8)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static partial bool ProcessMouseMove(IntPtr context, int x, int y, int keyModifierState);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Context_ProcessMouseButtonDown", StringMarshalling = StringMarshalling.Utf8)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static partial bool ProcessMouseButtonDown(IntPtr context, int buttonIndex, int keyModifierState);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Context_ProcessMouseButtonUp", StringMarshalling = StringMarshalling.Utf8)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static partial bool ProcessMouseButtonUp(IntPtr context, int buttonIndex, int keyModifierState);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Context_ProcessMouseWheel", StringMarshalling = StringMarshalling.Utf8)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static partial bool ProcessMouseWheel(IntPtr context, Vector2f wheelDelta, int keyModifierState);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Context_ProcessKeyDown", StringMarshalling = StringMarshalling.Utf8)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static partial bool ProcessKeyDown(IntPtr context, Input.KeyIdentifier identifier, int keyModifierState);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Context_ProcessKeyUp", StringMarshalling = StringMarshalling.Utf8)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static partial bool ProcessKeyUp(IntPtr context, Input.KeyIdentifier identifier, int keyModifierState);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Context_ProcessTextInput", StringMarshalling = StringMarshalling.Utf8)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static partial bool ProcessTextInput(IntPtr context, string input);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Context_AddEventListener", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void AddEventListener(IntPtr context, string name, IntPtr eventListener, [MarshalAs(UnmanagedType.U1)] bool inCapturePhase);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Context_RemoveEventListener", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void RemoveEventListener(IntPtr context, string name, IntPtr eventListener, [MarshalAs(UnmanagedType.U1)] bool inCapturePhase);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Context_GetName", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr GetName(IntPtr context);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Context_CreateDataModel", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr CreateDataModel(IntPtr context, string name);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Context_GetDataModel", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr GetDataModel(IntPtr context, string name);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Context_RemoveDataModel", StringMarshalling = StringMarshalling.Utf8)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static partial bool RemoveDataModel(IntPtr context, string name);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Context_GetFocusElement", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr GetFocusElement(IntPtr context);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Context_UnfocusDocument", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void UnfocusDocument(IntPtr context, IntPtr document);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Context_GetRootElement", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr GetRootElement(IntPtr context);
    }
}
