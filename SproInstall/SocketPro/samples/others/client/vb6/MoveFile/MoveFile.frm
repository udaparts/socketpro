VERSION 5.00
Begin VB.Form frmMoveFile 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "Exchange a file over internet"
   ClientHeight    =   3780
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   6420
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   3780
   ScaleWidth      =   6420
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton cmdCancel 
      Caption         =   "Cancel"
      Height          =   375
      Left            =   4260
      TabIndex        =   20
      Top             =   840
      Width           =   1935
   End
   Begin VB.TextBox txtPassword 
      Height          =   345
      IMEMode         =   3  'DISABLE
      Left            =   4620
      PasswordChar    =   "*"
      TabIndex        =   19
      Text            =   "PassOne"
      Top             =   480
      Width           =   1575
   End
   Begin VB.TextBox txtUserID 
      Height          =   375
      Left            =   4620
      TabIndex        =   16
      Text            =   "SocketPro"
      Top             =   60
      Width           =   1575
   End
   Begin VB.CheckBox chkZip 
      Caption         =   "Zip ?"
      Height          =   315
      Left            =   2880
      TabIndex        =   15
      Top             =   1260
      Width           =   975
   End
   Begin VB.CheckBox chkUseSSL 
      Caption         =   "Use SSL"
      Height          =   375
      Left            =   1800
      TabIndex        =   14
      Top             =   360
      Width           =   975
   End
   Begin VB.TextBox txtPercent 
      BeginProperty DataFormat 
         Type            =   1
         Format          =   "0"
         HaveTrueFalseNull=   0
         FirstDayOfWeek  =   0
         FirstWeekOfYear =   0
         LCID            =   1033
         SubFormatType   =   1
      EndProperty
      Height          =   315
      Left            =   5340
      Locked          =   -1  'True
      TabIndex        =   13
      Text            =   "0"
      Top             =   3240
      Width           =   795
   End
   Begin VB.CommandButton cmdDisconnect 
      Caption         =   "&Disconnect"
      Height          =   375
      Left            =   2220
      TabIndex        =   11
      Top             =   840
      Width           =   1935
   End
   Begin VB.CommandButton cmdSendFile 
      Caption         =   "Send a file to a remote host"
      Enabled         =   0   'False
      Height          =   375
      Left            =   3420
      TabIndex        =   10
      Top             =   2760
      Width           =   2775
   End
   Begin VB.CommandButton cmdGetFile 
      Caption         =   "Get a file from a remote host"
      Enabled         =   0   'False
      Height          =   375
      Left            =   240
      TabIndex        =   9
      Top             =   2760
      Width           =   3015
   End
   Begin VB.TextBox txtRemoteFile 
      Height          =   375
      Left            =   240
      TabIndex        =   8
      Text            =   "c:\charlieye\oledbpro.msi"
      Top             =   2280
      Width           =   5955
   End
   Begin VB.TextBox txtLocalFile 
      Height          =   375
      Left            =   240
      TabIndex        =   5
      Text            =   "c:\oledbpro.msi"
      Top             =   1620
      Width           =   5955
   End
   Begin VB.TextBox txtPort 
      Height          =   375
      Left            =   2820
      TabIndex        =   4
      Text            =   "17000"
      Top             =   360
      Width           =   795
   End
   Begin VB.TextBox txtServer 
      Height          =   375
      Left            =   240
      TabIndex        =   1
      Text            =   "localhost"
      Top             =   360
      Width           =   1515
   End
   Begin VB.CommandButton cmdConnect 
      Caption         =   "&Connect"
      Height          =   375
      Left            =   240
      TabIndex        =   0
      Top             =   840
      Width           =   1875
   End
   Begin VB.Label Label2 
      Alignment       =   1  'Right Justify
      Caption         =   "Password:"
      Height          =   255
      Left            =   3660
      TabIndex        =   18
      Top             =   480
      Width           =   855
   End
   Begin VB.Label Label1 
      Alignment       =   1  'Right Justify
      Caption         =   "User ID:"
      Height          =   315
      Left            =   3840
      TabIndex        =   17
      Top             =   60
      Width           =   675
   End
   Begin VB.Label lblPercent 
      Alignment       =   1  'Right Justify
      Caption         =   "File transferred %:"
      Height          =   255
      Left            =   3780
      TabIndex        =   12
      Top             =   3300
      Width           =   1515
   End
   Begin VB.Label lblRemoteFile 
      Caption         =   "Remote File:"
      Height          =   255
      Left            =   240
      TabIndex        =   7
      Top             =   2040
      Width           =   2055
   End
   Begin VB.Label lblLocalFile 
      Caption         =   "Local File:"
      Height          =   255
      Left            =   240
      TabIndex        =   6
      Top             =   1320
      Width           =   1935
   End
   Begin VB.Label lblPort 
      Caption         =   "Port #:"
      Height          =   255
      Left            =   2820
      TabIndex        =   3
      Top             =   60
      Width           =   675
   End
   Begin VB.Label lblServer 
      Caption         =   "Server:"
      Height          =   255
      Left            =   240
      TabIndex        =   2
      Top             =   120
      Width           =   1275
   End
End
Attribute VB_Name = "frmMoveFile"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Dim WithEvents USocket As USocketLib.USocket
Attribute USocket.VB_VarHelpID = -1
Dim WithEvents UFile As UFILELib.UFile
Attribute UFile.VB_VarHelpID = -1
Dim lFileSize As Long

Private Sub cmdCancel_Click()
    UFile.Cancel
End Sub

Private Sub cmdConnect_Click()
On Error GoTo ccHandler
    Dim strError As String
    If chkUseSSL.Value > 0 Then
        USocket.EncryptionMethod = USocketLib.tagEncryptionMethod.MSTLSv1
    Else
        USocket.EncryptionMethod = USocketLib.tagEncryptionMethod.NoEncryption
    End If
    USocket.Connect txtServer.Text, CLng(txtPort.Text)
    Exit Sub
ccHandler:
    MsgBox "Error code = " & USocket.Rtn & ". " & USocket.ErrorMsg
End Sub

Private Sub cmdDisconnect_Click()
    USocket.Disconnect 'abort socket connection
End Sub

Private Sub cmdGetFile_Click()
On Error GoTo gfHandler
    USocket.StartBatching
    If chkZip.Value > 0 Then
        USocket.TurnOnZipAtSvr True
    Else
        USocket.TurnOnZipAtSvr False
    End If
    UFile.GetFile txtRemoteFile.Text, txtLocalFile.Text, False
    USocket.CommitBatching 'batching two requests at client only
    Exit Sub
gfHandler:
    USocket.AbortBatching
    MsgBox UFile.ErrorMsg
End Sub

Private Sub cmdSendFile_Click()
On Error GoTo sfHandler
    If chkZip.Value > 0 Then
        USocket.ZipIsOn = True
    Else
        USocket.ZipIsOn = False
    End If
    UFile.SendFile txtLocalFile.Text, txtRemoteFile.Text, False
    Exit Sub
sfHandler:
    MsgBox UFile.ErrorMsg
End Sub

Private Sub Form_Load()
    Set USocket = New USocketLib.USocket
    Set UFile = New UFILELib.UFile
    USocket.RecvTimeout = 1200000
    USocket.SendTimeout = 1200000
    UFile.AttachSocket USocket
End Sub

Private Sub UFile_OnRequestProcessed(ByVal hSocket As Long, ByVal nRequestID As Integer, ByVal lLen As Long, ByVal lLenInBuffer As Long, ByVal sFlag As Integer)
    Dim dFileSize As Double
    Dim dRemain As Double
    Dim nPercent As Integer
    If UFile.Rtn <> USocketLib.tagUErrorCode.uecOK Then
        MsgBox UFile.ErrorMsg
        Exit Sub
    End If
    If nRequestID = UFILELib.tagFileRequestID.idGetFile Or nRequestID = UFILELib.tagFileRequestID.idSendFile Then
        Select Case sFlag
        Case USocketLib.tagReturnFlag.rfComing
            txtPercent.Text = "0"
            lFileSize = lLen
            Debug.Print "lFileSize = " & lFileSize
        Case USocketLib.tagReturnFlag.rfReceiving
            dFileSize = lFileSize
            dRemain = lLen
            nPercent = ((dFileSize - dRemain) / dFileSize + 0.005) * 100
            txtPercent.Text = nPercent
            Debug.Print "Remain = " & lLen
        Case USocketLib.tagReturnFlag.rfCompleted
            txtPercent.Text = "100"
            Debug.Print "Completed"
        Case Else
        End Select
    End If
End Sub

Private Sub USocket_OnSocketClosed(ByVal hSocket As Long, ByVal lError As Long)
    cmdSendFile.Enabled = False
    cmdGetFile.Enabled = False
End Sub

Private Sub USocket_OnSocketConnected(ByVal hSocket As Long, ByVal lError As Long)
    If lError = 0 Then
        Debug.Print "Socket opened"
        USocket.StartBatching
        USocket.UserID = txtUserID.Text
        USocket.Password = txtPassword
        USocket.SwitchTo USocketLib.tagServiceID.sidWinFile
        
        'clean password right after calling SwitchTo for the better security
        USocket.Password = ""
        
        USocket.SetSockOpt USocketLib.tagSocketOption.soSndBuf, 116800
        USocket.SetSockOpt USocketLib.tagSocketOption.soRcvBuf, 116800
        USocket.SetSockOptAtSvr USocketLib.tagSocketOption.soSndBuf, 116800
        USocket.SetSockOptAtSvr USocketLib.tagSocketOption.soRcvBuf, 116800
        USocket.CommitBatching
        cmdSendFile.Enabled = True
        cmdGetFile.Enabled = True
    Else
        MsgBox "Having problem in connecting, and error code = " & lError & ", error msg = " & USocket.ErrorMsg
    End If
End Sub
