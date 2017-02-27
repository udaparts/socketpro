package SPA.ClientSide;

public class CStreamHelper {

    private final CStreamHelper m_this = this;
    private volatile CAsyncServiceHandler m_ash;

    public interface DProgress {

        void invoke(CStreamHelper sender, long pos);
    }
    public volatile DProgress Progress = null;

    public CStreamHelper(CAsyncServiceHandler ash) {
        if (ash == null) {
            throw new IllegalArgumentException("A valid service handler required");
        }
        m_ash = ash;
    }
    private volatile long m_nDownloadingFileSize = -1;

    public final long getDownloadingStreamSize() {
        return m_nDownloadingFileSize;
    }

    public final void Reset() {
        m_bw = null;
        synchronized (m_cs) {
            m_br = null;
        }
    }

    /**
     * Get a stream from remote server onto a receiving stream at client side
     *
     * @param receiver A stream at client side for receiving data from remote
     * server
     * @param RemotePath A string for finding an arbitrary file or other object
     * @return An empty string if successful. Otherwise, an error message if
     * failed
     * @throws java.io.IOException
     */
    public final String Download(java.io.FileOutputStream receiver, String RemotePath) throws java.io.IOException {
        synchronized (m_cs) {
            if (m_bw != null || m_br != null) {
                throw new UnsupportedOperationException("A stream during transaction");
            }
            m_bw = receiver;
        }
        m_ash.ResultReturned = new CAsyncServiceHandler.DOnResultReturned() {
            @Override
            public boolean invoke(CAsyncServiceHandler sender, short reqId, SPA.CUQueue qData) {
                boolean processed = false;
                switch (reqId) {
                    case SPA.CStreamSerializationHelper.idReadDataFromServerToClient:
                        if (qData.GetSize() > 0) {
                            synchronized (m_cs) {
                                try {
                                    SPA.CStreamSerializationHelper.Write(m_bw, qData);
                                    if (Progress != null) {
                                        Progress.invoke(m_this, (long) m_bw.getChannel().position());
                                    }
                                } catch (java.io.IOException err) {
                                }
                            }
                            qData.SetSize(0);
                        }
                        processed = true;
                        break;
                    default:
                        break;
                }
                return processed;
            }
        };
        final String[] res = {""};
        boolean ok = (m_ash.SendRequest(SPA.CStreamSerializationHelper.idStartDownloading, new SPA.CScopeUQueue().Save(RemotePath), new CAsyncServiceHandler.DAsyncResultHandler() {
            @Override
            public void invoke(CAsyncResult ar) {
                m_nDownloadingFileSize = ar.LoadLong();
                String s = ar.LoadString();
                res[0] = s;
                if (Progress != null && m_nDownloadingFileSize != -1 && s.length() == 0 && m_bw != null) {
                    Progress.invoke(m_this, 0);
                }
            }
        }) && m_ash.WaitAll());
        if (res[0] != null && res[0].length() > 0) {
            m_bw = null;
            m_ash.ResultReturned = null;
            return res[0];
        } else if (res[0] == null) {
            res[0] = "";
        }
        if (!ok && !m_ash.getAttachedClientSocket().getSendable()) {
            m_bw = null;
            m_ash.ResultReturned = null;
            return m_ash.getAttachedClientSocket().getErrorMsg();
        }
        if (Progress != null) {
            Progress.invoke(this, m_bw.getChannel().position());
        }
        if (!m_ash.SendRequest(SPA.CStreamSerializationHelper.idDownloadCompleted, new CAsyncServiceHandler.DAsyncResultHandler() {
            @Override
            public void invoke(CAsyncResult ar) {
                synchronized (m_cs) {
                    if (Progress != null) {
                        try {
                            Progress.invoke(m_this, m_bw.getChannel().position());
                        } catch (java.io.IOException err) {
                        }
                    }
                    m_bw = null;
                }
                m_ash.ResultReturned = null;
            }
        })) {
            m_bw = null;
            m_ash.ResultReturned = null;
            return m_ash.getAttachedClientSocket().getErrorMsg();
        }
        return res[0];
    }
    private final Object m_cs = new Object();
    private volatile java.io.FileInputStream m_br = null; //protected by m_cs
    private volatile java.io.FileOutputStream m_bw = null;

    private long SendDataFromClientToServer() throws java.io.IOException {
        if (m_ash.getAttachedClientSocket().getBytesInSendingBuffer() > SPA.CStreamSerializationHelper.STREAM_CHUNK_SIZE) {
            return 0;
        }
        if (m_br == null) {
            return 0;
        }
        long send = 0;
        SPA.CScopeUQueue su = new SPA.CScopeUQueue();
        int read = SPA.CStreamSerializationHelper.Read(m_br, su.getUQueue());
        while (read > 0) {
            boolean ok = m_ash.SendRequest(SPA.CStreamSerializationHelper.idWriteDataFromClientToServer, su.getUQueue().getIntenalBuffer(), read, new CAsyncServiceHandler.DAsyncResultHandler() {
                @Override
                public void invoke(CAsyncResult ar) {
                    try {
                        SendDataFromClientToServer();
                    } catch (java.io.IOException err) {
                    }
                }
            });
            if (Progress != null) {
                Progress.invoke(this, (long) m_br.getChannel().position());
            }
            if (!ok) {
                m_br = null;
                break;
            }
            send += read;
            if (m_ash.getAttachedClientSocket().getBytesInSendingBuffer() > 10 * SPA.CStreamSerializationHelper.STREAM_CHUNK_SIZE) {
                break;
            }
            read = SPA.CStreamSerializationHelper.Read(m_br, su.getUQueue());
            if (read == 0) {
                if (!m_ash.SendRequest(SPA.CStreamSerializationHelper.idUploadCompleted, new CAsyncServiceHandler.DAsyncResultHandler() {
                    @Override
                    public void invoke(CAsyncResult ar) {
                        synchronized (m_cs) {
                            if (Progress != null) {
                                try {
                                    Progress.invoke(m_this, (long) m_br.getChannel().position());
                                } catch (java.io.IOException err) {
                                }
                            }
                            m_br = null;
                        }
                    }
                })) {
                    m_br = null;
                }
            }
        }
        return send;
    }

    /**
     * Send a stream from client to remote server
     *
     * @param source A source stream at client side
     * @param RemotePath A string sent to server for a file name or other object
     * which will receive this stream data
     * @return An empty string if successful. Otherwise, an error message if
     * failed
     * @throws java.io.IOException
     */
    public final String Upload(java.io.FileInputStream source, String RemotePath) throws java.io.IOException {
        m_ash.ResultReturned = null;
        synchronized (m_cs) {
            if (m_br != null) {
                throw new UnsupportedOperationException("A stream during transaction");
            }
        }
        if (source == null) {
            throw new UnsupportedOperationException("A readable source stream required");
        }
        final String[] res = {""};
        boolean ok = (m_ash.SendRequest(SPA.CStreamSerializationHelper.idStartUploading, (new SPA.CScopeUQueue()).Save(RemotePath), new CAsyncServiceHandler.DAsyncResultHandler() {
            @Override
            public void invoke(CAsyncResult ar) {
                res[0] = ar.LoadString();
            }
        }) && m_ash.WaitAll());
        if (res[0] != null && res[0].length() > 0) {
            return res[0];
        }
        if (!ok && !m_ash.getAttachedClientSocket().getSendable()) {
            return m_ash.getAttachedClientSocket().getErrorMsg();
        }
        synchronized (m_cs) {
            if (m_br != null) {
                throw new UnsupportedOperationException("A stream during transaction");
            }
            m_br = source;
            if (Progress != null) {
                Progress.invoke(this, m_br.getChannel().position());
            }
            if (SendDataFromClientToServer() == 0) {
                if (!m_ash.SendRequest(SPA.CStreamSerializationHelper.idUploadCompleted, new CAsyncServiceHandler.DAsyncResultHandler() {
                    @Override
                    public void invoke(CAsyncResult ar) {
                        synchronized (m_cs) {
                            if (Progress != null && m_br != null) {
                                try {
                                    Progress.invoke(m_this, m_br.getChannel().position());
                                } catch (java.io.IOException err) {
                                }
                            }
                            m_br = null;
                        }
                    }
                }) && !m_ash.getAttachedClientSocket().getSendable()) {
                    m_br = null;
                    return m_ash.getAttachedClientSocket().getErrorMsg();
                }
            }
        }
        return res[0];
    }

    public final CAsyncServiceHandler getAsyncServiceHandler() {
        return m_ash;
    }
}
