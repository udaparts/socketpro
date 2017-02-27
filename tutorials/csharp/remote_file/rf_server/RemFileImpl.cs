/* **** including all of defines, service id(s) and request id(s) ***** */
using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using System.IO;

//server implementation for service RemotingFile
public class RemotingFilePeer : CClientPeer
{
    protected override void OnReleaseResource(bool closing, uint nInfo)
    {
        CleanTarget();
        CleanSource();
    }

    private FileStream m_fsSource = null;
    [RequestAttr(CStreamSerializationHelper.idStartDownloading, true)]
    private string StartDownloadingFile(string RemoteFilePath, out ulong fileSize)
    {
        string errMsg;
        m_fsSource = CStreamHelper.DownloadFile(Handle, RemoteFilePath, out fileSize, out errMsg);
        return errMsg;
    }
    [RequestAttr(CStreamSerializationHelper.idReadDataFromServerToClient, true)]
    private void MoveDataFromServerToClient()
    {
        CStreamHelper.ReadDataFromServerToClient(Handle, m_fsSource);
    }
    [RequestAttr(CStreamSerializationHelper.idDownloadCompleted)]
    private void WaitDownloadCompleted()
    {
        CleanSource();
    }
    private void CleanSource()
    {
        if (m_fsSource != null)
        {
            m_fsSource.Close();
            m_fsSource = null;
        }
    }


    private FileStream m_fsReceiver = null;
    [RequestAttr(CStreamSerializationHelper.idStartUploading, true)]
    private string StartUploadingFile(string RemoteFilePath)
    {
        try
        {
            m_fsReceiver = new FileStream(RemoteFilePath, FileMode.Append);
        }
        catch (Exception err)
        {
            return err.Message;
        }
        return "";
    }
    [RequestAttr(CStreamSerializationHelper.idWriteDataFromClientToServer, true)]
    private void MoveDataFromClientToServer()
    {
        CStreamHelper.WriteDataFromClientToServer(UQueue, m_fsReceiver);
    }
    [RequestAttr(CStreamSerializationHelper.idUploadCompleted)]
    private void WaitUploadingCompleted()
    {
        CleanTarget();
    }
    private void CleanTarget()
    {
        if (m_fsReceiver != null)
        {
            m_fsReceiver.Close();
            m_fsReceiver = null;
        }
    }
}
