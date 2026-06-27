using System;
using System.Runtime.InteropServices;

namespace RmlUiNet.Native
{
    internal static unsafe partial class ElementDocument
    {
        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_ElementDocument_Show", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void Show(IntPtr document, ModalFlag modalFlag, FocusFlag focusFlag);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_ElementDocument_Hide", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void Hide(IntPtr document);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_ElementDocument_AddStyleSheetContainer", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void AddStyleSheetContainer(IntPtr document, IntPtr container);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_ElementDocument_Close", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void Close(IntPtr document);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_ElementDocument_PullToFront", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void PullToFront(IntPtr document);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_ElementDocument_New", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr Create(OnLoadInlineScript? onLoadInlineScript, OnLoadExternalScript? onLoadExternalScript);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_ElementDocument_CreateElement", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr CreateElement(IntPtr document, string name);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_ElementDocument_CreateTextNode", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr CreateTextNode(IntPtr document, string text);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_ElementDocument_SetTitle", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr SetTitle(IntPtr document, string title);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_ElementDocument_GetTitle", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr GetTitle(IntPtr document);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_ElementDocument_GetSourceURL", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr GetSourceURL(IntPtr document);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_ElementDocument_Free", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void Free(IntPtr document);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate void OnLoadInlineScript(string context, string source_path, int source_line);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate void OnLoadExternalScript(string source_path);
    }
}
