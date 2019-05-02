Imports System.Configuration
Imports System.Collections
Imports System.Web.Security
Imports System.Web.SessionState
Imports SocketProAdapter
Imports SocketProAdapter.ClientSide
Imports USOCKETLib
Imports SocketProAdapter.ClientSide.RemoteDB


Namespace AdoWebAsync
	Public Class [Global]
		Inherits HttpApplication
		'the pool for this sample AsyncADO service
		Public Shared m_AsyncADO As CSocketPool(Of CMyAdoHandler)

		'the pool for UDAParts generic remote database service using MS OLEDB technology
		Public Shared m_AsyncRdb As CSocketPool(Of CAsynDBLite)

        <MTAThread()> _
  Protected Sub Application_Start(ByVal sender As Object, ByVal e As EventArgs)
            'your own ADO.NET based service
            m_AsyncADO = New CSocketPool(Of CMyAdoHandler)()

            'UDAParts remote database service using OLEDB technology
            m_AsyncRdb = New CSocketPool(Of CAsynDBLite)()

            Dim n As Integer
            Const Count As Integer = 5
            Dim pConnectionContext(Count - 1) As CConnectionContext
            For n = 0 To Count - 1
                pConnectionContext(n) = New CConnectionContext()
            Next n

            'set connection contexts
            pConnectionContext(0).m_strHost = "127.0.0.1"
            pConnectionContext(1).m_strHost = "localhost"
            pConnectionContext(2).m_strHost = "localhost"
            pConnectionContext(3).m_strHost = "127.0.0.1"
            pConnectionContext(4).m_strHost = "localhost"
            For n = 0 To Count - 1
                pConnectionContext(n).m_nPort = 20905
                pConnectionContext(n).m_strPassword = "SocketPro"
                pConnectionContext(n).m_strUID = "PassOne"
                pConnectionContext(n).m_EncrytionMethod = USOCKETLib.tagEncryptionMethod.NoEncryption
                pConnectionContext(n).m_bZip = False
            Next n

            'error treatment ignored here
            Dim ok As Boolean = m_AsyncRdb.StartSocketPool(pConnectionContext, 2, 4)
            ok = m_AsyncADO.StartSocketPool(pConnectionContext, 1, 2)
        End Sub

		Protected Sub Application_End(ByVal sender As Object, ByVal e As EventArgs)
			If m_AsyncRdb IsNot Nothing Then
				m_AsyncRdb.ShutdownPool()
				m_AsyncRdb.Dispose()
			End If

			If m_AsyncADO IsNot Nothing Then
				m_AsyncADO.ShutdownPool()
				m_AsyncADO.Dispose()
			End If
		End Sub
	End Class
End Namespace