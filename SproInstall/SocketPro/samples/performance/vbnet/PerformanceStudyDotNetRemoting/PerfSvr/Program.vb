'#define USE_SQLCLIENT


Imports Microsoft.VisualBasic
Imports System
Imports System.Data
Imports DefMyCallsInterface
Imports System.Runtime.Remoting
Imports System.Runtime.Remoting.Channels
Imports System.Runtime.Remoting.Channels.Tcp

Namespace PerfSvr
	Friend Class Program
		Shared Sub Main(ByVal args As String())
			Dim demoSvr As CMyCallsImpl = New CMyCallsImpl()
			Dim tcp As TcpChannel = New TcpChannel(21910)
			System.Runtime.Remoting.Channels.ChannelServices.RegisterChannel(tcp, False)
			RemotingConfiguration.RegisterWellKnownServiceType(GetType(DefMyCallsInterface.CMyCallsImpl), "MyCalls", WellKnownObjectMode.SingleCall)
			Console.WriteLine("Server started at port 21910")
            Dim myTest As DataTable = demoSvr.OpenRowset("select * from shippers")
			If Not myTest Is Nothing Then
				Console.WriteLine("Database connection established")
			End If
			Console.WriteLine("Press the key <ENTER> to terminate the application ......")
			Console.ReadLine()
		End Sub
	End Class
End Namespace
