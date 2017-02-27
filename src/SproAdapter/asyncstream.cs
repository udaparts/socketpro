using System;
using System.IO;

namespace SocketProAdapter
{
    public static class CStreamSerializationHelper
    {
        public const int STREAM_CHUNK_SIZE = 10240;
        public const ushort idStartDownloading = 0x7F7F;
        public const ushort idDownloadCompleted = 0x7F7E;
        public const ushort idStartUploading = 0x7F7D;
        public const ushort idUploadCompleted = 0x7F7C;
        public const ushort idReadDataFromServerToClient = 0x7F7B;
        public const ushort idWriteDataFromClientToServer = 0x7F7A;

        internal static void Write(Stream s, CUQueue q)
        {
            if (q == null || q.GetSize() == 0)
                return;
            s.Write(q.m_bytes, (int)q.HeadPosition, (int)q.GetSize());
        }

        internal static uint Read(Stream s, CUQueue q)
        {
            uint res;
            if (q.MaxBufferSize < STREAM_CHUNK_SIZE + 16)
                q.Realloc(STREAM_CHUNK_SIZE + 16);
            res = (uint)s.Read(q.m_bytes, 0, (int)STREAM_CHUNK_SIZE);
            q.SetSize(res);
            return res;
        }
    }
}
