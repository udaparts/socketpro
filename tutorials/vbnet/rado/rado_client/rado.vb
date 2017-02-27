Imports System
Imports SocketProAdapter
Imports SocketProAdapter.ClientSide
Imports System.Data

Public Class RAdo
    Inherits CAsyncAdohandler

    Public Sub New()
        MyBase.New(radoConst.sidRAdo)
    End Sub

    Public ReadOnly Property CurrentDataSet() As DataSet
        Get
            Return AdoSerializer.CurrentDataSet
        End Get
    End Property

    Public ReadOnly Property CurrentDataTable() As DataTable
        Get
            Return AdoSerializer.CurrentDataTable
        End Get
    End Property

    Public Function GetDataSet(ByVal sql0 As String, ByVal sql1 As String) As DataSet
        If ProcessR0(radoConst.idGetDataSetRAdo, sql0, sql1) Then
            Return AdoSerializer.CurrentDataSet
        End If
        Return New DataSet()
    End Function

    Public Function GetDataTable(ByVal sql As String) As DataTable
        If ProcessR0(radoConst.idGetDataTableRAdo, sql) Then
            Return AdoSerializer.CurrentDataTable
        End If
        Return New DataTable()
    End Function
End Class
