using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace RmlUiNet.Native
{
    internal static unsafe partial class FileInterface
    {
        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_FileInterface_New")]
        internal static partial IntPtr Create(
            OnOpen onOpen,
            OnClose onClose,
            OnLoadFile onLoadFile,
            OnRead onRead,
            OnSeek onSeek,
            OnTell onTell,
            OnLength onLength
        );

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate IntPtr OnOpen(string path);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate void OnClose(IntPtr file);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal unsafe delegate ulong OnRead(byte* buffer, uint size, IntPtr file);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate bool OnSeek(IntPtr file, uint offset, int origin);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate ulong OnTell(IntPtr file);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate ulong OnLength(IntPtr file);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate string OnLoadFile(string path);
    }
}
