using System;
using System.IO;

namespace SocketProAdapter.ServerSide
{
    public static class CStreamHelper
    {
        /// <summary>
        /// Read data from a source stream at server side and send its content onto a client
        /// </summary>
        /// <param name="PeerHandle">A peer socket handle to represent a client</param>
        /// <param name="source">A stream to a source file or other object</param>
        /// <returns>The number of data sent in bytes</returns>
        public static ulong ReadDataFromServerToClient(ulong PeerHandle, Stream source)
        {
            uint res;
            ulong sent = 0;
            using (CScopeUQueue su = new CScopeUQueue())
            {
                CUQueue q = su.UQueue;
                uint read = CStreamSerializationHelper.Read(source, q);
                while (read != 0)
                {
                    unsafe
                    {
                        fixed (byte* p = q.m_bytes)
                        {
                            res = ServerCoreLoader.SendReturnData(PeerHandle, CStreamSerializationHelper.idReadDataFromServerToClient, read, p);
                        }
                    }
                    if (res == CSocketPeer.REQUEST_CANCELED || res == CSocketPeer.SOCKET_NOT_FOUND)
                        break;
                    sent += res;
                    q.SetSize(0);
                    read = CStreamSerializationHelper.Read(source, q);
                }
            }
            return sent;
        }

        /// <summary>
        /// Write data from client to this server
        /// </summary>
        /// <param name="q">A memory queue containing data from a client</param>
        /// <param name="receiver">A stream at server side to receive data from a client</param>
        public static void WriteDataFromClientToServer(CUQueue q, Stream receiver)
        {
            CStreamSerializationHelper.Write(receiver, q);
        }

        /// <summary>
        /// Download a stream from server to a client. Internally, it will also fake an empty request (CStreamSerializationHelper.idReadDataFromServerToClient) on behalf on the client
        /// </summary>
        /// <param name="PeerHandle">A peer socket handle to represent a client</param>
        /// <param name="source">A valid stream at server side</param>
        /// <param name="fileSize">File size in bytes. It will be -1 if there is error</param>
        /// <param name="errMsg">An error message. It will be empty string with zero length if no error is found</param>
        public static void Download(ulong PeerHandle, Stream source, out ulong fileSize, out string errMsg)
        {
            if (source == null)
            {
                fileSize = ulong.MaxValue;
                errMsg = "Source stream not available";
                return;
            }
            else if (!source.CanRead)
            {
                fileSize = ulong.MaxValue;
                errMsg = "Source stream not readable";
                return;
            }
            errMsg = "";
            try
            {
                fileSize = (ulong)source.Length;
            }
            catch (Exception)
            {
                fileSize = ulong.MaxValue;
            }

            unsafe
            {
                byte[] temp = null;
                fixed (byte* p = temp)
                {
                    ServerCoreLoader.MakeRequest(PeerHandle, CStreamSerializationHelper.idReadDataFromServerToClient, p, (uint)0);
                }
            }
        }

        /// <summary>
        /// Download a file from server to a client. Internally, it will also fake an empty request (CStreamSerializationHelper.idReadDataFromServerToClient) on behalf on the client
        /// </summary>
        /// <param name="PeerHandle">A peer socket handle to represent a client</param>
        /// <param name="RemoteFilePath">A path to a file</param>
        /// <param name="fileSize">File size in bytes. It will be -1 if there is error</param>
        /// <param name="errMsg">An error message. It will be empty string with zero length if no error is found</param>
        /// <returns>A file stream</returns>
        public static FileStream DownloadFile(ulong PeerHandle, string RemoteFilePath, out ulong fileSize, out string errMsg)
        {
            FileStream fs = null;
            try
            {
                fs = new FileStream(RemoteFilePath, FileMode.Open);
                fileSize = (ulong)fs.Length;
                unsafe
                {
                    byte []temp = null;
                    fixed (byte* p = temp)
                    {
                        ServerCoreLoader.MakeRequest(PeerHandle, CStreamSerializationHelper.idReadDataFromServerToClient, p, (uint)0);
                    }
                }
                errMsg = "";
            }
            catch (Exception err)
            {
                fileSize = ulong.MaxValue;
                errMsg = err.Message;
            }
            return fs;
        }
    }
}
