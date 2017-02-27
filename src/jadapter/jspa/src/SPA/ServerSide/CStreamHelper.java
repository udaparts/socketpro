package SPA.ServerSide;

public final class CStreamHelper {

    /**
     * Read data from a source stream at server side and send its content onto a
     * client
     *
     * @param PeerHandle A peer socket handle to represent a client
     * @param source A stream to a source file or other object
     * @return The number of data sent in bytes
     * @throws java.io.IOException
     */
    public static long ReadDataFromServerToClient(long PeerHandle, java.io.InputStream source) throws java.io.IOException {
        int res;
        int read;
        long sent = 0;
        SPA.CScopeUQueue su = new SPA.CScopeUQueue();
        SPA.CUQueue q = su.getUQueue();
        read = SPA.CStreamSerializationHelper.Read(source, q);
        while (read != 0) {
            res = ServerCoreLoader.SendReturnData(PeerHandle, SPA.CStreamSerializationHelper.idReadDataFromServerToClient, read, q.getIntenalBuffer());
            if (res == CSocketPeer.REQUEST_CANCELED || res == CSocketPeer.SOCKET_NOT_FOUND) {
                break;
            }
            sent += res;
            q.SetSize(0);
            read = SPA.CStreamSerializationHelper.Read(source, q);
        }
        return sent;
    }

    /**
     * Write data from client to this server
     *
     * @param q A memory queue containing data from a client
     * @param receiver A stream at server side to receive data from a client
     * @throws java.io.IOException
     */
    public static void WriteDataFromClientToServer(SPA.CUQueue q, java.io.OutputStream receiver) throws java.io.IOException {
        SPA.CStreamSerializationHelper.Write(receiver, q);
    }

    /**
     * Download a stream from server to a client. Internally, it will also fake
     * an empty request
     * (CStreamSerializationHelper.idReadDataFromServerToClient) on behalf on
     * the client
     *
     * @param PeerHandle A peer socket handle to represent a client
     * @param source A valid stream at server side
     * @param fileSize File size in bytes. It will be -1 if there is error
     * @param errMsg An error message. It will be empty string with zero length
     * if no error is found
     */
    public static void Download(long PeerHandle, java.io.FileInputStream source, SPA.RefObject<Long> fileSize, SPA.RefObject<String> errMsg) {
        if (source == null) {
            fileSize.Value = -1L;
            errMsg.Value = "Source stream not available";
            return;
        }
        errMsg.Value = "";
        try {
            fileSize.Value = source.getChannel().size();
        } catch (java.io.IOException e) {
            fileSize.Value = -1L;
            errMsg.Value = e.getMessage();
        }
        ServerCoreLoader.MakeRequest(PeerHandle, SPA.CStreamSerializationHelper.idReadDataFromServerToClient, (byte[]) null, (int) 0);
    }

    /**
     * Download a file from server to a client. Internally, it will also fake an
     * empty request (CStreamSerializationHelper.idReadDataFromServerToClient)
     * on behalf on the client
     *
     * @param PeerHandle A peer socket handle to represent a client
     * @param RemoteFilePath A path to a file
     * @param fileSize File size in bytes. It will be -1 if there is error
     * @param errMsg An error message. It will be empty string with zero length
     * if no error is found
     * @return A file stream
     */
    public static java.io.FileInputStream DownloadFile(long PeerHandle, String RemoteFilePath, SPA.RefObject<Long> fileSize, SPA.RefObject<String> errMsg) {
        java.io.FileInputStream fs = null;
        try {
            fs = new java.io.FileInputStream(RemoteFilePath);
            fileSize.Value = fs.getChannel().size();
            ServerCoreLoader.MakeRequest(PeerHandle, SPA.CStreamSerializationHelper.idReadDataFromServerToClient, (byte[]) null, (int) 0);
            errMsg.Value = "";
        } catch (java.io.IOException err) {
            fileSize.Value = -1L;
            errMsg.Value = err.getMessage();
        }
        return fs;
    }
}
