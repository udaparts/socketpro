Imports System
Imports SocketProAdapter
Imports SocketProAdapter.ClientSide

Public Class PiWorker
    Inherits CAsyncServiceHandler

    Public Sub New()
        MyBase.New(piConst.sidPiWorker)
    End Sub

    Protected Overrides Sub OnResultReturned(ByVal sRequestId As UShort, ByVal UQueue As CUQueue)
        If RouteeRequest Then
            Select Case sRequestId
                Case piConst.idComputePi
                    Dim dStart As Double
                    Dim dStep As Double
                    Dim nNum As Integer
                    UQueue.Load(dStart).Load(dStep).Load(nNum)
                    Dim dX As Double = dStart + dStep / 2
                    Dim dd As Double = dStep * 4.0
                    Dim ComputeRtn As Double = 0.0
                    For n As Integer = 0 To nNum - 1
                        dX += dStep
                        ComputeRtn += dd / (1 + dX * dX)
                    Next n
                    SendRouteeResult(ComputeRtn)
                Case Else
            End Select
        End If
    End Sub
End Class