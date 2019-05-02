Imports SocketProAdapter

Namespace SampleThreeShared
	Public Class CTestItem
        Implements IUSerializer
		Public Sub New()
			m_dt = Date.Now
		End Sub
        Public Sub SaveTo(ByVal UQueue As CUQueue) Implements IUSerializer.SaveTo
            'make sure that both native and .NET codes are compatible to each of other
            Dim obj As Object = m_dt
            UQueue.Push(obj) 'save datetime as native VARIANT datetime
            UQueue.Push(m_lData)
            UQueue.Save(m_strUID)
        End Sub
        Public Function LoadFrom(ByVal UQueue As CUQueue) As Integer Implements IUSerializer.LoadFrom
            Dim nLen As Integer
            Dim obj As Object = Nothing

            'make sure that both native and .NET codes are compatible to each of other
            nLen = UQueue.Pop(obj)
            m_dt = CDate(obj)
            nLen += UQueue.Pop(m_lData)
            nLen += UQueue.Load(m_strUID)
            Return nLen
        End Function
		Public m_dt As Date
		Public m_lData As Long
		Public m_strUID As String
	End Class
End Namespace