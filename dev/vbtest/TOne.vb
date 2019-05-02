Imports System
Imports SocketProAdapter
Imports SocketProAdapter.ClientSide

public class CTOne : Inherits CAsyncServiceHandler
	Public Sub New()
		MyBase.New(TOneConst.sidCTOne)
	End Sub

	Public Function QueryCount() As Integer
		Dim QueryCountRtn As Integer
		Dim bProcessRy as Boolean = ProcessR1(TOneConst.idQueryCountCTOne, QueryCountRtn)
		return QueryCountRtn
	End Function

	Public Function QueryGlobalCount() As Integer
		Dim QueryGlobalCountRtn As Integer
		Dim bProcessRy as Boolean = ProcessR1(TOneConst.idQueryGlobalCountCTOne, QueryGlobalCountRtn)
		return QueryGlobalCountRtn
	End Function

	Public Function QueryGlobalFastCount() As Integer
		Dim QueryGlobalFastCountRtn As Integer
		Dim bProcessRy as Boolean = ProcessR1(TOneConst.idQueryGlobalFastCountCTOne, QueryGlobalFastCountRtn)
		return QueryGlobalFastCountRtn
	End Function

	Public Sub Sleep(ByVal nTime As Integer) 
		Dim bProcessRy as Boolean = ProcessR0(TOneConst.idSleepCTOne, nTime)
	End Sub

	Public Function Echo(ByVal objInput As Object) As Object
		Dim EchoRtn As Object = Nothing
		Dim bProcessRy as Boolean = ProcessR1(TOneConst.idEchoCTOne, objInput, EchoRtn)
		return EchoRtn
	End Function

    Public Function EchoEx(ByVal str As SByte(), ByVal wstr As String, ByRef strOut As SByte(), ByRef wstrOut As String) As Boolean
        'Please implement IUSerializer for the class MyStruct
        Dim EchoExRtn As Boolean
        Dim bProcessRy As Boolean = ProcessR3(TOneConst.idEchoExCTOne, str, wstr, strOut, wstrOut, EchoExRtn)
        Return EchoExRtn
    End Function
End Class
