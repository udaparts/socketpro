Imports Microsoft.VisualBasic
Imports System
Imports SocketProAdapter

Namespace SampleThreeShared
	Public Class CDefines
		'define your own service id starting from 0x10000001
		'udaparts reserves service ids from 0x00000000 through 0x10000000
		Public Const sidSThreeSvs As Integer = &H1F322311

		'define your request ids starting from 2001
		'udaparts reserves service ids from 0x0000 through 0x2000
		Public Const idSendOneItemToServer As Short = &H2001
		Public Const idSendAFewItemsToServer As Short = &H2002
		Public Const idSendALotItemsToServer As Short = &H2003
		Public Const idGetOneItemFromServer As Short = &H2004
		Public Const idGetAFewItemsFromServer As Short = &H2005
		'slow request
		Public Const idGetALotItemsFromServer As Short = &H2006

		Public Const idStartToSendALotItemsToServer As Short = &H2007
		Public Const idStartToGetALotItemsFromServer As Short = &H2008

		'slow request
		Public Const idSendingALotItemsToServer As Short = &H2009
		Public Const idGetingALotItemsFromServer As Short = &H200A

'		public const short idPushItemsIntoGlobalStack = 0x200B;
'		public const short idPopItemsFromGlobalStack = 0x200C;

		'slow request
		Public Const idSendItemsToServerByDotNetSerializer As Short = &H200D
		'slow request
		Public Const idGetItemsFromServerByDotNetSerializer As Short = &H200E
	End Class

    <Serializable()> _
    Public Class CTestItem : Implements IUSerializer
        Public Sub New()
            m_dt = System.DateTime.Now
        End Sub

        Public Sub SaveTo(ByVal UQueue As CUQueue) Implements IUSerializer.SaveTo
            Dim obj As Object = m_dt
            UQueue.Push(obj)
            UQueue.Push(m_lData)
            UQueue.Save(m_strUID)
        End Sub

        Public Function LoadFrom(ByVal UQueue As CUQueue) As Integer Implements IUSerializer.LoadFrom
            Dim nLen As Integer
            Dim nStrLen As Integer = 0
            Dim obj As Object = Nothing
            nLen = UQueue.Pop(obj)
            m_dt = CDate(obj)
            nLen += UQueue.Pop(m_lData)
            nLen += UQueue.Load(m_strUID)
            Return nLen
        End Function

        Public m_dt As DateTime
        Public m_lData As Long
        Public m_strUID As String
    End Class
End Namespace