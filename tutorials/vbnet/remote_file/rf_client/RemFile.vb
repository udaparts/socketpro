Imports System
Imports SocketProAdapter
Imports SocketProAdapter.ClientSide
Imports System.IO

Public Class RemotingFile
    Inherits CAsyncServiceHandler

    Public Sub New()
        MyBase.New(RemFileConst.sidRemotingFile)
        m_sh = New CStreamHelper(Me)
    End Sub

    Public ReadOnly Property StreamHelper() As CStreamHelper
        Get
            Return m_sh
        End Get
    End Property
    Private m_sh As CStreamHelper
End Class
