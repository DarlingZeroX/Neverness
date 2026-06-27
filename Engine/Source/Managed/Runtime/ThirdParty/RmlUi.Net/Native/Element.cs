using System;
using System.Runtime.InteropServices;

namespace RmlUiNet.Native
{
    internal static unsafe partial class Element
    {
        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_GetTagName", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr GetTagName(IntPtr element);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_AddEventListener", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void AddEventListener(IntPtr element, string name, IntPtr eventListener, [MarshalAs(UnmanagedType.U1)] bool inCapturePhase);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_RemoveEventListener", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void RemoveEventListener(IntPtr element, string name, IntPtr eventListener, [MarshalAs(UnmanagedType.U1)] bool inCapturePhase);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_GetElementByID", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr GetElementById(IntPtr element, string id, out IntPtr foundElement);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_GetOwnerDocument", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr GetOwnerDocument(IntPtr element, out IntPtr foundElement);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_SetInnerRml", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void SetInnerRml(IntPtr element, string rml);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_GetInnerRml", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr GetInnerRml(IntPtr element);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_Focus", StringMarshalling = StringMarshalling.Utf8)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static partial bool Focus(IntPtr element);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_Blur", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void Blur(IntPtr element);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_Click", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void Click(IntPtr element);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_GetBox", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr GetBox(IntPtr element);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_GetOffsetTop", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial float GetOffsetTop(IntPtr element);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_GetAbsoluteLeft", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial float GetAbsoluteLeft(IntPtr element);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_GetAbsoluteTop", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial float GetAbsoluteTop(IntPtr element);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_GetClientHeight", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial float GetClientHeight(IntPtr element);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_GetClientWidth", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial float GetClientWidth(IntPtr element);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_GetAttributeString", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr GetAttributeString(IntPtr element, string attributeName, string defaultValue);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_SetAttributeString", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr SetAttributeString(IntPtr element, string attributeName, string value);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_GetPropertyString", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr GetPropertyString(IntPtr element, string propertyName);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_GetProperty", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr GetProperty(IntPtr element, string propertyName);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_GetPropertyById", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr GetPropertyById(IntPtr element, int propertyId);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_SetProperty", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr SetProperty(IntPtr element, string propertyName, string value);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_GetElementTypeName", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr GetElementTypeName(IntPtr element);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_AppendChild", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr AppendChild(IntPtr element, IntPtr elementToRemove, [MarshalAs(UnmanagedType.U1)] bool addToDom);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_AppendChildTag", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr AppendChildTag(IntPtr element, string tagName, [MarshalAs(UnmanagedType.U1)] bool addToDom);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_RemoveChild", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void RemoveChild(IntPtr element, IntPtr elementToAppend);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_ScrollTo", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr ScrollTo(IntPtr element, float x, float y, int behavior);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_SetClass", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr SetClass(IntPtr element, string className, [MarshalAs(UnmanagedType.U1)] bool enable);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_GetParentNode", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr GetParentNode(IntPtr element);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_ReplaceChild", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void ReplaceChild(IntPtr element, IntPtr elementToInsert, IntPtr elementToReplace);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_QuerySelector", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr QuerySelector(IntPtr element, string selector);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_HasClass", StringMarshalling = StringMarshalling.Utf8)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static partial bool HasClass(IntPtr element, string className);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_HasAttribute", StringMarshalling = StringMarshalling.Utf8)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static partial bool HasAttribute(IntPtr element, string attributeName);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_RemoveAttribute", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void RemoveAttribute(IntPtr element, string attributeName);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Element_DispatchEvent", StringMarshalling = StringMarshalling.Utf8)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static partial bool DispatchEvent(IntPtr element, string event_id, IntPtr parameters, [MarshalAs(UnmanagedType.U1)] bool interruptible, [MarshalAs(UnmanagedType.U1)] bool bubbles);
    }
}
