VERSION 5.00
Object = "{3B7C8863-D78F-101B-B9B5-04021C009402}#1.2#0"; "RICHTX32.OCX"
Object = "{5E9E78A0-531B-11CF-91F6-C2863C385E30}#1.0#0"; "MSFLXGRD.OCX"
Begin VB.Form frmDBA 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "My simple DBA from home"
   ClientHeight    =   9555
   ClientLeft      =   150
   ClientTop       =   435
   ClientWidth     =   13245
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   9555
   ScaleWidth      =   13245
   StartUpPosition =   3  'Windows Default
   Begin VB.TextBox txtPassword 
      Height          =   315
      IMEMode         =   3  'DISABLE
      Left            =   6600
      PasswordChar    =   "*"
      TabIndex        =   35
      Text            =   "PassOne"
      Top             =   480
      Width           =   1095
   End
   Begin VB.TextBox txtUserID 
      Height          =   315
      Left            =   6600
      TabIndex        =   32
      Text            =   "SocketPro"
      Top             =   60
      Width           =   1095
   End
   Begin VB.TextBox txtBytesSent 
      Height          =   375
      Left            =   9180
      TabIndex        =   30
      Text            =   "0"
      Top             =   3600
      Width           =   1095
   End
   Begin VB.TextBox txtBytesRcved 
      BeginProperty DataFormat 
         Type            =   1
         Format          =   "0"
         HaveTrueFalseNull=   0
         FirstDayOfWeek  =   0
         FirstWeekOfYear =   0
         LCID            =   1033
         SubFormatType   =   1
      EndProperty
      Height          =   375
      Left            =   11940
      TabIndex        =   29
      Text            =   "0"
      Top             =   3600
      Width           =   1095
   End
   Begin VB.CheckBox chkUseSSL 
      Caption         =   "Use SSL"
      Height          =   255
      Left            =   3840
      TabIndex        =   27
      Top             =   480
      Width           =   975
   End
   Begin VB.CommandButton cmdMoveLast 
      Caption         =   "MoveLast"
      Enabled         =   0   'False
      Height          =   375
      Left            =   6720
      TabIndex        =   26
      Top             =   3600
      Width           =   975
   End
   Begin VB.CommandButton cmdMoveFirst 
      Caption         =   "MoveFirst"
      Enabled         =   0   'False
      Height          =   375
      Left            =   3660
      TabIndex        =   25
      Top             =   3600
      Width           =   975
   End
   Begin VB.CommandButton cmdMovePrev 
      Caption         =   "MovePrev"
      Enabled         =   0   'False
      Height          =   375
      Left            =   5700
      TabIndex        =   24
      Top             =   3600
      Width           =   975
   End
   Begin VB.CommandButton cmdMoveNext 
      Caption         =   "MoveNext"
      Enabled         =   0   'False
      Height          =   375
      Left            =   4680
      TabIndex        =   23
      Top             =   3600
      Width           =   975
   End
   Begin VB.CommandButton cmdGetBatch 
      Caption         =   "Get the next batch of records"
      Enabled         =   0   'False
      Height          =   375
      Left            =   1200
      TabIndex        =   22
      Top             =   3600
      Width           =   2355
   End
   Begin MSFlexGridLib.MSFlexGrid fgRowset 
      Height          =   5295
      Left            =   180
      TabIndex        =   18
      Top             =   4020
      Width           =   12915
      _ExtentX        =   22781
      _ExtentY        =   9340
      _Version        =   393216
      Rows            =   0
      Cols            =   0
      FixedRows       =   0
      FixedCols       =   0
      AllowUserResizing=   1
   End
   Begin VB.CommandButton cmdToDS 
      Caption         =   "Connect to DB"
      Enabled         =   0   'False
      Height          =   375
      Left            =   7740
      TabIndex        =   17
      Top             =   840
      Width           =   1215
   End
   Begin VB.TextBox txtConnectionString 
      Height          =   375
      Left            =   1740
      TabIndex        =   16
      Text            =   "Provider=Microsoft.Jet.OLEDB.4.0;Data Source=NWIND3.mdb"
      Top             =   840
      Width           =   5955
   End
   Begin VB.TextBox txtErrorMsg 
      Height          =   375
      Left            =   300
      Locked          =   -1  'True
      TabIndex        =   11
      Top             =   3060
      Width           =   8595
   End
   Begin RichTextLib.RichTextBox txtMsgIn 
      Height          =   2655
      Left            =   9120
      TabIndex        =   8
      Top             =   360
      Width           =   3795
      _ExtentX        =   6694
      _ExtentY        =   4683
      _Version        =   393217
      Enabled         =   -1  'True
      ScrollBars      =   3
      TextRTF         =   $"frmDBA.frx":0000
   End
   Begin VB.Frame Frame2 
      Caption         =   "My message boad:"
      Height          =   3435
      Left            =   9000
      TabIndex        =   7
      Top             =   120
      Width           =   4035
      Begin VB.CommandButton cmdSend 
         Caption         =   "Send"
         Enabled         =   0   'False
         Height          =   375
         Left            =   3120
         TabIndex        =   10
         Top             =   3000
         Width           =   855
      End
      Begin VB.TextBox txtMsgOut 
         Height          =   375
         Left            =   120
         TabIndex        =   9
         Top             =   3000
         Width           =   2895
      End
   End
   Begin VB.CommandButton cmdShutDown 
      Caption         =   "Shut down"
      Height          =   375
      Left            =   7740
      TabIndex        =   6
      Top             =   420
      Width           =   1215
   End
   Begin VB.CommandButton cmdConnect 
      Caption         =   "Connect"
      Height          =   375
      Left            =   7740
      TabIndex        =   5
      Top             =   0
      Width           =   1215
   End
   Begin VB.TextBox txtPort 
      BeginProperty DataFormat 
         Type            =   1
         Format          =   "0"
         HaveTrueFalseNull=   0
         FirstDayOfWeek  =   0
         FirstWeekOfYear =   0
         LCID            =   1033
         SubFormatType   =   1
      EndProperty
      Height          =   375
      Left            =   4980
      TabIndex        =   4
      Text            =   "17001"
      Top             =   420
      Width           =   615
   End
   Begin VB.TextBox txtHost 
      Height          =   375
      Left            =   180
      TabIndex        =   2
      Text            =   "localhost"
      Top             =   420
      Width           =   3495
   End
   Begin VB.Frame Frame1 
      Caption         =   "Simple DBA:"
      Height          =   2295
      Left            =   180
      TabIndex        =   0
      Top             =   1260
      Width           =   8775
      Begin VB.CheckBox chkNeedRowset 
         Caption         =   "Need rowset ?"
         Height          =   315
         Left            =   5220
         TabIndex        =   20
         Top             =   1020
         Value           =   1  'Checked
         Width           =   1635
      End
      Begin VB.CommandButton cmdExecuteSQL 
         Caption         =   "ExecuteSQL"
         Enabled         =   0   'False
         Height          =   375
         Left            =   1920
         TabIndex        =   14
         Top             =   1020
         Width           =   2115
      End
      Begin VB.TextBox txtSQL 
         Height          =   375
         Left            =   120
         TabIndex        =   12
         Text            =   "Select * from orders"
         Top             =   600
         Width           =   8595
      End
      Begin VB.Label Label3 
         Caption         =   "Error Message:"
         Height          =   315
         Left            =   120
         TabIndex        =   21
         Top             =   1500
         Width           =   1935
      End
      Begin VB.Label Label4 
         Caption         =   "SQL Statement:"
         Height          =   255
         Left            =   120
         TabIndex        =   13
         Top             =   300
         Width           =   2715
      End
   End
   Begin VB.Label Label10 
      Alignment       =   1  'Right Justify
      Caption         =   "Password:"
      Height          =   255
      Left            =   5640
      TabIndex        =   34
      Top             =   480
      Width           =   855
   End
   Begin VB.Label Label9 
      Alignment       =   1  'Right Justify
      Caption         =   "User ID:"
      Height          =   255
      Left            =   5760
      TabIndex        =   33
      Top             =   60
      Width           =   735
   End
   Begin VB.Label Label8 
      Alignment       =   1  'Right Justify
      Caption         =   "Bytes sent -->"
      Height          =   255
      Left            =   7920
      TabIndex        =   31
      Top             =   3660
      Width           =   1215
   End
   Begin VB.Label Label7 
      Alignment       =   1  'Right Justify
      Caption         =   "Bytes received -->"
      Height          =   315
      Left            =   10380
      TabIndex        =   28
      Top             =   3660
      Width           =   1515
   End
   Begin VB.Label Label6 
      Caption         =   "Records:"
      Height          =   255
      Left            =   180
      TabIndex        =   19
      Top             =   3660
      Width           =   915
   End
   Begin VB.Label Label5 
      Caption         =   "Connection string:"
      Height          =   255
      Left            =   180
      TabIndex        =   15
      Top             =   900
      Width           =   1455
   End
   Begin VB.Label Label2 
      Caption         =   "Port:"
      Height          =   255
      Left            =   4980
      TabIndex        =   3
      Top             =   60
      Width           =   555
   End
   Begin VB.Label Label1 
      Caption         =   "Host: (111.222.212.121,  www.myserver.com or MyServerName)"
      Height          =   255
      Left            =   180
      TabIndex        =   1
      Top             =   60
      Width           =   4755
   End
End
Attribute VB_Name = "frmDBA"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Dim WithEvents ClientSocket As USocketLib.USocket
Attribute ClientSocket.VB_VarHelpID = -1
Dim WithEvents DataSource As UDBLib.UDataSource
Attribute DataSource.VB_VarHelpID = -1
Dim WithEvents Session As UDBLib.USession
Attribute Session.VB_VarHelpID = -1
Dim WithEvents Command As UDBLib.UCommand
Attribute Command.VB_VarHelpID = -1
Dim WithEvents Rowset As UDBLib.URowset
Attribute Rowset.VB_VarHelpID = -1


Private Sub ClientSocket_OnRequestProcessed(ByVal hSocket As Long, ByVal nRequestID As Integer, ByVal lLen As Long, ByVal lLenInBuffer As Long, ByVal sFlag As Integer)
    Dim Chat As USocketLib.IUChat
    If sFlag = USocketLib.tagReturnFlag.rfReceiving Or sFlag = USocketLib.tagReturnFlag.rfCompleted Then
        txtBytesRcved.Text = ClientSocket.GetBytesReceived
        txtBytesSent.Text = ClientSocket.GetBytesSent
    End If
    If sFlag <> USocketLib.tagReturnFlag.rfCompleted Then
        Exit Sub
    End If
    Select Case nRequestID
    Case USocketLib.tagChatRequestID.idSpeak, USocketLib.tagChatRequestID.idSpeakTo
        Set Chat = ClientSocket
        txtMsgIn.Text = Chat.Message
    Case Else
    End Select
End Sub

Private Sub ClientSocket_OnSocketClosed(ByVal hSocket As Long, ByVal lError As Long)
    cmdToDS.Enabled = False
    cmdSend.Enabled = False
    cmdExecuteSQL.Enabled = False
    cmdGetBatch.Enabled = False
    cmdMoveFirst.Enabled = False
    cmdMoveNext.Enabled = False
    cmdMovePrev.Enabled = False
    cmdMoveLast.Enabled = False
    txtBytesRcved.Text = ClientSocket.GetBytesReceived
    txtBytesSent.Text = ClientSocket.GetBytesSent
End Sub

Private Sub ClientSocket_OnSocketConnected(ByVal hSocket As Long, ByVal lError As Long)
    If lError = 0 Then 'no error
        Dim lMTU As Long
        Dim lMaxSpeed As Long
        Dim lType As Long
        Dim strMask As String
        Dim strDesc As String
        Dim Chat As USocketLib.IUChat
        ClientSocket.StartBatching
        ClientSocket.UserID = txtUserID.Text
        ClientSocket.Password = txtPassword.Text
        ClientSocket.SwitchTo USocketLib.tagServiceID.sidOleDB
        
        'clean password right after calling SwitchTo for the better security
        ClientSocket.Password = ""
        
        strDesc = ClientSocket.GetInterfaceAttributes(lMTU, lMaxSpeed, lType, strMask)
        If lMaxSpeed > 10000000 Or lType = USocketLib.tagInterfaceType.itLoopback Then '10 mbp
            ClientSocket.SetSockOptAtSvr USocketLib.tagSocketOption.soSndBuf, 1168000
            ClientSocket.SetSockOptAtSvr USocketLib.tagSocketOption.soRcvBuf, 1168000
            ClientSocket.TurnOnZipAtSvr False
            ClientSocket.ZipIsOn = False
        Else
            ClientSocket.TurnOnZipAtSvr True
            ClientSocket.ZipIsOn = True
        End If
        Set Chat = ClientSocket
        Chat.Enter 2
        ClientSocket.CommitBatching 'send all of requests in one batch
        cmdToDS.Enabled = True
        cmdSend.Enabled = True
    Else
        txtErrorMsg.Text = ClientSocket.ErrorMsg
    End If
End Sub


Private Sub cmdConnect_Click()
On Error GoTo cHandler
    If chkUseSSL.Value > 0 Then
        ClientSocket.EncryptionMethod = USocketLib.tagEncryptionMethod.MSTLSv1
    Else
        ClientSocket.EncryptionMethod = USocketLib.tagEncryptionMethod.NoEncryption
    End If
    ClientSocket.Connect txtHost.Text, txtPort.Text
    Exit Sub
cHandler:
    txtErrorMsg.Text = ClientSocket.ErrorMsg
End Sub

Private Sub cmdExecuteSQL_Click()
    Dim sCreatedObject As Integer
    Dim lHint As Long
    txtErrorMsg.Text = ""
    If chkNeedRowset.Value Then
        sCreatedObject = UDBLib.tagCreatedObject.coRowset
    Else
        sCreatedObject = UDBLib.tagCreatedObject.coNothing
    End If
    lHint = UDBLib.tagRowsetHint.rhScrollable
    ClientSocket.StartBatching
    Command.ExecuteSQL txtSQL.Text, sCreatedObject, ctStatic, lHint
    Rowset.Open
    'DBPROP_CANSCROLLBACKWARDS   = 0x15L,
    Rowset.GetProperty 21 'Query if DBPROP_CANSCROLLBACKWARDS is set
    Rowset.GetBatchRecords 0, True
    ClientSocket.CommitBatching True
    cmdGetBatch.Enabled = False
    cmdMoveFirst.Enabled = False
    cmdMoveNext.Enabled = False
    cmdMovePrev.Enabled = False
    cmdMoveLast.Enabled = False
End Sub

Private Sub cmdGetBatch_Click()
    txtErrorMsg.Text = ""
    Rowset.GetBatchRecords
End Sub

Private Sub cmdMoveFirst_Click()
    txtErrorMsg.Text = ""
    Rowset.MoveFirst
End Sub

Private Sub cmdMoveLast_Click()
    txtErrorMsg.Text = ""
    Rowset.MoveLast
End Sub

Private Sub cmdMoveNext_Click()
    txtErrorMsg.Text = ""
    Rowset.MoveNext
End Sub

Private Sub cmdMovePrev_Click()
    txtErrorMsg.Text = ""
    Rowset.MovePrev
End Sub

Private Sub cmdSend_Click()
    Dim Chat As USocketLib.IUChat
    Set Chat = ClientSocket
    Chat.Speak txtMsgOut.Text
End Sub

Private Sub cmdShutDown_Click()
    ClientSocket.Shutdown
End Sub

Private Sub cmdToDS_Click()
    ClientSocket.StartBatching
    DataSource.Open txtConnectionString.Text
    Session.Open
    Command.Open
    ClientSocket.CommitBatching True
    cmdExecuteSQL.Enabled = False
    cmdGetBatch.Enabled = False
    cmdMoveFirst.Enabled = False
    cmdMoveNext.Enabled = False
    cmdMovePrev.Enabled = False
    cmdMoveLast.Enabled = False
End Sub

Private Sub Command_OnRequestProcessed(ByVal hSocket As Long, ByVal nRequestID As Integer, ByVal lLen As Long, ByVal lLenInBuffer As Long, ByVal sFlag As Integer)
    If nRequestID = UDBLib.tagDBRequestID.idCmndOpen Then
        If Command.Rtn = 0 Then
            cmdExecuteSQL.Enabled = True
        Else
            cmdExecuteSQL.Enabled = False
            txtErrorMsg.Text = Command.ErrorMsg 'You may need to check error messages with DataSource and Session
        End If
    End If
End Sub

Private Sub Form_Load()
    Set ClientSocket = New USocketLib.USocket
    Set DataSource = New UDBLib.UDataSource
    Set Session = New UDBLib.USession
    Set Command = New UDBLib.UCommand
    Set Rowset = New UDBLib.URowset
    ClientSocket.RecvTimeout = 1000000
    DataSource.AttachSocket ClientSocket
    Session.AttachSocket ClientSocket
    Command.AttachSocket ClientSocket
    Rowset.AttachSocket ClientSocket
    ClientSocket.ReturnEvents = USocketLib.tagReturnFlag.rfCompleted + USocketLib.tagReturnFlag.rfComing
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Set Rowset = Nothing
    Set Session = Nothing
    Set Command = Nothing
    Set DataSource = Nothing
    Set ClientSocket = Nothing
End Sub


Private Sub Rowset_OnRequestProcessed(ByVal hSocket As Long, ByVal nRequestID As Integer, ByVal lLen As Long, ByVal lLenInBuffer As Long, ByVal sFlag As Integer)
    Dim lCols As Long
    Dim lRows As Long
    Dim lCol As Long
    Dim lRow As Long
    Dim vtData As Variant
    Dim lPrevRows As Long
    If Rowset.Rtn >= 0 Then
        Select Case nRequestID
        Case UDBLib.tagDBRequestID.idRowsetGetProperty
            vtData = Rowset.Property
            If vtData Then
                cmdMovePrev.Enabled = True
                cmdMoveLast.Enabled = True
            Else
                cmdMovePrev.Enabled = False
                cmdMoveLast.Enabled = False
            End If
        Case idRowsetGetBatchRecords, idRowsetMoveFirst, idRowsetMoveNext, idRowsetMovePrev, idRowsetMoveLast, idRowsetGetRowsAt
            lRows = Rowset.GetRowsFetched
            If lRows > 0 Then
                fgRowset.Rows = lRows + 1
                lCols = Rowset.GetCols
                For lRow = 0 To lRows - 1
                    For lCol = 1 To lCols
                        vtData = Rowset.GetData(lRow, lCol)
                        fgRowset.TextMatrix(lRow + 1, lCol) = vtData
                    Next
                Next
            Else
                lCols = Rowset.GetCols
                fgRowset.Rows = 2
                For lCol = 1 To lCols
                    fgRowset.TextMatrix(1, lCol) = vtData
                Next
            End If
        Case UDBLib.tagDBRequestID.idRowsetOpen
            cmdGetBatch.Enabled = True
            cmdMoveFirst.Enabled = True
            cmdMoveNext.Enabled = True
            lCols = Rowset.GetCols
            fgRowset.Cols = lCols + 1
            fgRowset.Rows = 2
            fgRowset.FixedRows = 1
            fgRowset.FixedCols = 1
            For lCol = 1 To lCols
                 fgRowset.TextMatrix(0, lCol) = Rowset.GetColName(lCol)
                 If Rowset.GetDataType(lCol) = sdVT_STR Or Rowset.GetDataType(lCol) = sdVT_WSTR Then
                    fgRowset.ColAlignment(lCol) = flexAlignLeftCenter
                 Else
                    fgRowset.ColAlignment(lCol) = flexAlignRightCenter
                 End If
            Next
        Case UDBLib.tagDBRequestID.idRowsetSendBLOB
            Debug.Print "BLOB count = " & lLen & " BLOB index = " & lLenInBuffer
        Case Else
        End Select
    End If
    If Rowset.Rtn <> 0 Then
        txtErrorMsg.Text = Rowset.ErrorMsg + " Error code = " & Rowset.Rtn
    End If
End Sub

