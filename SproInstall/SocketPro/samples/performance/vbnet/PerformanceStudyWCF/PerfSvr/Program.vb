Imports Microsoft.VisualBasic
Imports System
Imports System.Collections.Generic
Imports System.Text
Imports System.ServiceModel
Imports DefMyCalls

Namespace PerfSvr
	Friend Class Program
		Shared Sub Main(ByVal args As String())
			' Create a ServiceHost for the CalculatorService type.
			Using serviceHost As ServiceHost = New ServiceHost(GetType(DefMyCalls.CMyCallsImpl))
				' Open the ServiceHost to create listeners and start listening for messages.
				serviceHost.Open()

				' The service can now be accessed.
				Console.WriteLine("The service is ready.")
				Console.WriteLine("Press <ENTER> to terminate service.")
				Console.ReadLine()
				serviceHost.Close()
			End Using

		End Sub
	End Class
End Namespace
