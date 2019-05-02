Imports System.ComponentModel
Imports System.Text
Imports USOCKETLib
Imports SocketProAdapter
Imports SocketProAdapter.ClientSide

Namespace PPi
	Public Delegate Sub DUpdateProgress()
	Partial Public Class main
		Inherits Form
		Private m_spc As USocketPoolClass
		Private m_NPPi As CPiParallel
		Public UpdateProgress As DUpdateProgress
		Public Sub New()
			InitializeComponent()
			m_NPPi = New CPiParallel()
			m_NPPi.m_dlg = Me
			UpdateProgress = New DUpdateProgress(AddressOf OnUpdateControl)

			'use m_spc for your debug
			m_spc = m_NPPi.GetUSocketPool()
		End Sub

		Private Sub OnUpdateControl()
			System.Threading.Thread.Sleep(0)
			Dim strMsg As String = "Progress = " & m_NPPi.Progress.ToString() & "%"
			strMsg &= (", Parallels = " & m_NPPi.SocketsInParallel.ToString())
			strMsg &= (", Fails = " & m_NPPi.Fails.ToString())
			strMsg &= (", Paused = " & m_NPPi.Paused.ToString())
			strMsg &= (", Working = " & m_NPPi.Working.ToString())
			strMsg &= (", Pi = " & m_NPPi.GetPi().ToString())

			txtStatus.Text = strMsg

			If m_NPPi.SocketsInParallel = 0 Then
				btnStart.Text = "Start"
				btnPause.Text = "Pause"
			End If
		End Sub

		Private Sub main_FormClosed(ByVal sender As Object, ByVal e As FormClosedEventArgs) Handles MyBase.FormClosed
			m_NPPi.ShutdownPool()
		End Sub

		Private Sub main_Load(ByVal sender As Object, ByVal e As EventArgs) Handles MyBase.Load
			btnStart.Enabled = m_NPPi.BuildConnections()
		End Sub

		Private Sub btnStart_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnStart.Click
			If btnStart.Text = "Start" Then
				m_NPPi.PrepareAndExecuteJobs()
				If m_NPPi.Process() Then
					btnStart.Text = "Stop"
					btnPause.Enabled = True
				End If
			Else
				m_NPPi.Stop()
				btnStart.Text = "Start"
				btnPause.Text = "Pause"
				btnPause.Enabled = False
			End If
		End Sub

		Private Sub btnPause_Click(ByVal sender As Object, ByVal e As EventArgs) Handles btnPause.Click
			If btnPause.Text = "Pause" Then
				m_NPPi.Pause()
				btnPause.Text = "Resume"
			Else
				m_NPPi.Resume()
				btnPause.Text = "Pause"
			End If
		End Sub
	End Class

	Friend Class CPiParallel
		Inherits CSocketPoolEx(Of CPPi)
		Private Const m_nDivision As Integer = 100
		Public Function GetPi() As Double
			SyncLock m_cs
				Return m_dPi
			End SyncLock
		End Function
		Public ReadOnly Property Progress() As Integer
			Get
				SyncLock m_cs
					Return m_nDivision - JobManager.CountOfJobs
				End SyncLock
			End Get
		End Property

		Protected Overrides Function OnFailover(ByVal pHandler As CPPi, ByVal JobContext As IJobContext) As Boolean
			Dim str As String = "JobFail, JobId = " & JobContext.JobId
			str &= ", Progress = " & Progress
			Trace.WriteLine(str)

			'this is called within a worker thread
			m_dlg.BeginInvoke(m_dlg.UpdateProgress)
			Return True
		End Function

		Protected Overrides Sub OnJobDone(ByVal Handler As CPPi, ByVal JobContext As IJobContext)
			Dim str As String = "JobDone, JobId = " & JobContext.JobId
			str &= ", Progress = " & Progress
			Trace.WriteLine(str)

			m_dlg.BeginInvoke(m_dlg.UpdateProgress)
		End Sub

		Protected Overrides Sub OnReturnedResultProcessed(ByVal pHandler As CPPi, ByVal JobContext As IJobContext, ByVal sRequestId As Short)
			If sRequestId = piConst.idComputeCPPi Then
				SyncLock m_cs
					m_dPi += pHandler.m_ComputeRtn
				End SyncLock
			End If
		End Sub

		Public Sub PrepareAndExecuteJobs()
			Dim n As Integer
			Dim dStart As Double
			Dim nNum As Integer = 10000000
            Dim dStep As Double = 1.0 / nNum / m_nDivision

			SyncLock m_cs
				'initialize member
				m_dPi = 0.0
			End SyncLock

			'get an async handler
			Dim pi As CPPi = CType(JobManager.LockIdentity(), CPPi)

			If pi Is Nothing Then
				Return
			End If

			'a job containing one task only
			For n = 0 To m_nDivision - 1
				dStart = CDbl(n) / m_nDivision
                pi.ComputeAsync(dStart, dStep, nNum)
			Next n

			'a job containing two tasks
			'for (n = 0; n < m_nDivision; n++)
			'{
			'    pi.GetAttachedClientSocket().StartJob();
			'    dStart = (double)n / m_nDivision;
			'    pi.ComputeAsyn(dStart, dStep, nNum);
			'    n += 1;
			'    dStart = (double)n / m_nDivision;
			'    pi.ComputeAsyn(dStart, dStep, nNum);
			'    pi.GetAttachedClientSocket().EndJob();
			'}

			JobManager.UnlockIdentity(pi)

			'manually divide a large task into nDivision sub-tasks
			'int		nTaskId;
            'bool	ok;
            'short sRequestId = piConst.idComputeCPPi;
            'CUQueue	UQueue = new CUQueue();
            'for(n=0; n<m_nDivision; n++)
            '{
            '    dStart = (double)n/m_nDivision;

            '    UQueue.Push(dStart);
            '    UQueue.Push(dStep);
            '    UQueue.Push(nNum);

            '    IJobContext jc = JobManager.CreateJob(this);
            '    nTaskId = jc.AddTask(sRequestId, UQueue.GetBuffer(), UQueue.GetSize());
            '    ok = JobManager.EnqueueJob(jc);
			'    Process();

			'    UQueue.SetSize(0);
			'}
		End Sub

		Public Function BuildConnections() As Boolean
			Dim n As Integer
			Const Count As Integer = 5
			Dim pConnectionContext(Count - 1) As CConnectionContext
			For n = 0 To Count - 1
				pConnectionContext(n) = New CConnectionContext()
			Next n

			'set connection contexts
			pConnectionContext(0).m_strHost = "127.0.0.1"
			pConnectionContext(1).m_strHost = "localhost"
			pConnectionContext(2).m_strHost = "127.0.0.1"
			pConnectionContext(3).m_strHost = "localhost"
			pConnectionContext(4).m_strHost = "127.0.0.1"
			For n = 0 To Count - 1
				pConnectionContext(n).m_nPort = 20901
				pConnectionContext(n).m_strPassword = "SocketPro"
				pConnectionContext(n).m_strUID = "PassOne"
                pConnectionContext(n).m_EncrytionMethod = USOCKETLib.tagEncryptionMethod.NoEncryption
				pConnectionContext(n).m_bZip = False
			Next n

			'start socket pool with 2*3 USocket objects 
			Return StartSocketPool(pConnectionContext, 2, 3)
		End Function

		Private m_cs As New Object()
		Public m_dlg As PPi.main

		'protect the following member by monitor
		Private m_dPi As Double = 0.0
	End Class
End Namespace

