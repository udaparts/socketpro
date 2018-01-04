package SPA;

import java.io.IOException;

@Deprecated
public final class CStreamSerializationHelper {

    public static final int STREAM_CHUNK_SIZE = 10240;
    public static final short idStartDownloading = 0x7F7F;
    public static final short idDownloadCompleted = 0x7F7E;
    public static final short idStartUploading = 0x7F7D;
    public static final short idUploadCompleted = 0x7F7C;
    public static final short idReadDataFromServerToClient = 0x7F7B;
    public static final short idWriteDataFromClientToServer = 0x7F7A;

    public static void Write(java.io.OutputStream s, CUQueue q) throws IOException {
        if (q == null || q.GetSize() == 0) {
            return;
        }
        s.write(q.getIntenalBuffer(), q.getHeadPosition(), q.GetSize());
    }

    public static int Read(java.io.InputStream s, CUQueue q) throws IOException {
        int res;
        if (q.getMaxBufferSize() < STREAM_CHUNK_SIZE + 16) {
            q.Realloc(STREAM_CHUNK_SIZE + 16);
        }
        res = (int) s.read(q.getIntenalBuffer(), 0, STREAM_CHUNK_SIZE);
        /*
         If no byte is available because the stream is at end of
         * file, the value -1 is returned; otherwise, at least one
         * byte is read and stored into buffer.
         */
        if (res == -1) {
            res = 0;
        }
        q.SetSize(res);
        return res;
    }
}
