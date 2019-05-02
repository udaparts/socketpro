Imports SocketProAdapter
Imports SocketProAdapter.ClientSide

Public Class CMySocket
    Inherits CClientSocket
    Protected Overrides Sub OnSocketClosed(ByVal hSocket As Integer, ByVal nError As Integer)
        m_frmSOne.OnSocketClosed(hSocket, nError)
    End Sub

    Protected Overrides Sub OnSocketConnected(ByVal hSocket As Integer, ByVal nError As Integer)
        m_frmSOne.OnSocketConnected(hSocket, nError)
    End Sub

    Protected Overrides Sub OnBaseRequestProcessed(ByVal sRequestID As Short)
        m_frmSOne.OnBaseRequestProcessed(sRequestID)
    End Sub

    Protected Overrides Sub OnRequestProcessed(ByVal hSocket As Integer, ByVal sRequestID As Short, ByVal nLen As Integer, ByVal nLenInBuffer As Integer, ByVal sFlag As Short)
        If (sFlag = USOCKETLib.tagReturnFlag.rfCompleted) Then
            m_frmSOne.UpdateBytes()
        End If
        MyBase.OnRequestProcessed(hSocket, sRequestID, nLen, nLenInBuffer, sFlag)
    End Sub


    Public m_frmSOne As frmSOne

End Class
