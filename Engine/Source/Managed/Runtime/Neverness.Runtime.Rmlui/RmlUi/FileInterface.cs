using System;
using System.Runtime.InteropServices;

namespace RmlUiNet
{
    public abstract class FileInterface : RmlBase<FileInterface>
    {
        private Native.FileInterface.OnOpen _onOpen;
        private Native.FileInterface.OnClose _onClose;
        private Native.FileInterface.OnRead _onRead;
        private Native.FileInterface.OnSeek _onSeek;
        private Native.FileInterface.OnTell _onTell;
        private Native.FileInterface.OnLength _onLength;
        private Native.FileInterface.OnLoadFile _onLoadFile;

        public unsafe FileInterface() : base(IntPtr.Zero)
        {
            _onOpen = Open;
            _onClose = Close;
            _onRead = _Read;
            _onSeek = Seek;
            _onTell = Tell;
            _onLength = Length;
            _onLoadFile = LoadFile;

            NativePtr = Native.FileInterface.Create(
                _onOpen,
                _onClose,
                _onLoadFile,
                _onRead,
                _onSeek,
                _onTell,
                _onLength
            );

            ManuallyRegisterCache(NativePtr, this);
        }

        internal unsafe ulong _Read(byte* buffer, uint size, IntPtr file)
        {
            var len = Read(size, file, out byte[] bytes);

            Marshal.Copy(bytes, 0, (IntPtr)buffer, (int)len);

            return len;
        }

        public abstract IntPtr Open(string path);
        public abstract void Close(IntPtr file);
        public abstract ulong Read(ulong size, IntPtr file, out byte[] bytes);
        public abstract bool Seek(IntPtr file, uint offset, int origin);
        public abstract ulong Tell(IntPtr file);
        public abstract ulong Length(IntPtr file);
        public abstract string LoadFile(string path);
    }
}
