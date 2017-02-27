Imports System
Imports SocketProAdapter
Imports SocketProAdapter.ClientSide

Public Class Pi
    Inherits CAsyncServiceHandler

    Public Sub New()
        MyBase.New(piConst.sidPi)
    End Sub
End Class
