package remote_file.server;

import SPA.ServerSide.*;

public class RemotingFilePeer extends CClientPeer {

    @Override
    protected void OnReleaseResource(boolean closing, int nInfo) {
        CleanTarget();
        CleanSource();
    }

    private java.io.FileInputStream m_fsSource = null;

    @RequestAttr(RequestID = SPA.CStreamSerializationHelper.idStartDownloading, SlowRequest = true)
    private SPA.CUQueue StartDownloadingFile(String RemoteFilePath) {
        SPA.RefObject<String> errMsg = new SPA.RefObject<>("");
        SPA.RefObject<Long> fileSize = new SPA.RefObject<>(0L);
        m_fsSource = CStreamHelper.DownloadFile(getHandle(), RemoteFilePath, fileSize, errMsg);
        return new SPA.CUQueue().Save(fileSize.Value).Save(errMsg.Value);
    }

    @RequestAttr(RequestID = SPA.CStreamSerializationHelper.idReadDataFromServerToClient, SlowRequest = true)
    private void MoveDataFromServerToClient() {
        try {
            CStreamHelper.ReadDataFromServerToClient(getHandle(), m_fsSource);
        } catch (java.io.IOException err) {
        }
    }

    @RequestAttr(RequestID = SPA.CStreamSerializationHelper.idDownloadCompleted)
    private void WaitDownloadCompleted() {
        CleanSource();
    }

    private void CleanSource() {
        if (m_fsSource != null) {
            try {
                m_fsSource.close();
            } catch (java.io.IOException err) {
            }
            m_fsSource = null;
        }
    }

    private java.io.FileOutputStream m_fsReceiver = null;

    @RequestAttr(RequestID = SPA.CStreamSerializationHelper.idStartUploading, SlowRequest = true)
    private String StartUploadingFile(String RemoteFilePath) {
        try {
            m_fsReceiver = new java.io.FileOutputStream(RemoteFilePath, true);
        } catch (java.io.FileNotFoundException err) {
            return err.getMessage();
        }
        return "";
    }

    @RequestAttr(RequestID = SPA.CStreamSerializationHelper.idWriteDataFromClientToServer, SlowRequest = true)
    private void MoveDataFromClientToServer() {
        try {
            CStreamHelper.WriteDataFromClientToServer(getUQueue(), m_fsReceiver);
        } catch (java.io.IOException err) {
        }
    }

    @RequestAttr(RequestID = SPA.CStreamSerializationHelper.idUploadCompleted)
    private void WaitUploadingCompleted() {
        CleanTarget();
    }

    private void CleanTarget() {
        if (m_fsReceiver != null) {
            try {
                m_fsReceiver.flush();
                m_fsReceiver.close();
            } catch (java.io.IOException err) {
            }
            m_fsReceiver = null;
        }
    }
}
