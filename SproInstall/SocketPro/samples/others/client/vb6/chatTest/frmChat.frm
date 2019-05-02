VERSION 5.00
Begin VB.Form frmChat 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "Test Chat functionality"
   ClientHeight    =   7200
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   9060
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   7200
   ScaleWidth      =   9060
   StartUpPosition =   3  'Windows Default
   Begin VB.TextBox txtPassword 
      Height          =   375
      IMEMode         =   3  'DISABLE
      Left            =   5280
      PasswordChar    =   "#"
      TabIndex        =   29
      Text            =   "PassOne"
      Top             =   420
      Width           =   1095
   End
   Begin VB.TextBox txtUserID 
      Height          =   345
      Left            =   4080
      TabIndex        =   27
      Text            =   "SocketPro"
      Top             =   420
      Width           =   1095
   End
   Begin VB.Frame frameChat 
      Caption         =   "Chat:"
      Height          =   6075
      Left            =   240
      TabIndex        =   7
      Top             =   900
      Width           =   8595
      Begin VB.CommandButton cmdGetAllClients 
         Caption         =   "Get All Clients"
         Enabled         =   0   'False
         Height          =   375
         Left            =   6000
         TabIndex        =   32
         Top             =   4860
         Width           =   2415
      End
      Begin VB.ListBox lstAllClients 
         Height          =   1035
         Left            =   180
         TabIndex        =   31
         Top             =   4860
         Width           =   5715
      End
      Begin VB.CommandButton cmdExit 
         Caption         =   "Exit from group"
         Enabled         =   0   'False
         Height          =   435
         Left            =   4260
         TabIndex        =   25
         Top             =   300
         Width           =   1695
      End
      Begin VB.CommandButton cmdJoin 
         Caption         =   "Join a group"
         Enabled         =   0   'False
         Height          =   435
         Left            =   2700
         TabIndex        =   24
         Top             =   300
         Width           =   1455
      End
      Begin VB.TextBox txtGroupJoined 
         BeginProperty DataFormat 
            Type            =   1
            Format          =   "0"
            HaveTrueFalseNull=   0
            FirstDayOfWeek  =   0
            FirstWeekOfYear =   0
            LCID            =   1033
            SubFormatType   =   1
         EndProperty
         Height          =   435
         Left            =   1200
         TabIndex        =   23
         Top             =   300
         Width           =   1395
      End
      Begin VB.TextBox txtGroups 
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
         Left            =   4800
         TabIndex        =   21
         Text            =   "3"
         Top             =   4380
         Width           =   1095
      End
      Begin VB.CommandButton cmdSpeakGroups 
         Caption         =   "Speak to groups of listeners"
         Enabled         =   0   'False
         Height          =   375
         Left            =   6000
         TabIndex        =   20
         Top             =   3900
         Width           =   2415
      End
      Begin VB.TextBox txtMsgSpeak 
         Height          =   375
         Left            =   180
         TabIndex        =   19
         Top             =   3900
         Width           =   5715
      End
      Begin VB.CommandButton cmdSpeakTo 
         Caption         =   "Send a message to a client"
         Enabled         =   0   'False
         Height          =   375
         Left            =   5220
         TabIndex        =   18
         Top             =   2880
         Width           =   3195
      End
      Begin VB.TextBox txtListenerPort 
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
         Left            =   3960
         TabIndex        =   16
         Text            =   "1024"
         Top             =   3300
         Width           =   915
      End
      Begin VB.TextBox txtListenerAddress 
         Height          =   375
         Left            =   1560
         TabIndex        =   14
         Top             =   3300
         Width           =   1275
      End
      Begin VB.TextBox txtMsgOut 
         Height          =   375
         Left            =   180
         TabIndex        =   13
         Top             =   2880
         Width           =   4695
      End
      Begin VB.TextBox txtMsgIn 
         Height          =   1755
         Left            =   5040
         TabIndex        =   12
         Top             =   900
         Width           =   3375
      End
      Begin VB.CommandButton cmdGetListeners 
         Caption         =   "Get Listeners"
         Enabled         =   0   'False
         Height          =   375
         Left            =   180
         TabIndex        =   11
         Top             =   1680
         Width           =   1275
      End
      Begin VB.ListBox lstListeners 
         Height          =   645
         Left            =   180
         TabIndex        =   10
         Top             =   2100
         Width           =   4695
      End
      Begin VB.CommandButton cmdGetGroups 
         Caption         =   "Get Groups"
         Enabled         =   0   'False
         Height          =   315
         Left            =   180
         TabIndex        =   9
         Top             =   780
         Width           =   1275
      End
      Begin VB.ListBox lstGroups 
         Height          =   450
         Left            =   180
         TabIndex        =   8
         Top             =   1140
         Width           =   4695
      End
      Begin VB.Label Label6 
         Caption         =   "A Group ID:"
         Height          =   255
         Left            =   180
         TabIndex        =   26
         Top             =   420
         Width           =   915
      End
      Begin VB.Label Label5 
         Caption         =   "Groups Identified:"
         Height          =   315
         Left            =   3480
         TabIndex        =   22
         Top             =   4380
         Width           =   1335
      End
      Begin VB.Label Label4 
         Caption         =   "Port#:"
         Height          =   255
         Left            =   3300
         TabIndex        =   17
         Top             =   3360
         Width           =   615
      End
      Begin VB.Label Label3 
         Caption         =   "Listener Address:"
         Height          =   315
         Left            =   180
         TabIndex        =   15
         Top             =   3420
         Width           =   1335
      End
   End
   Begin VB.CommandButton cmdShutDown 
      Caption         =   "Shut Down"
      Height          =   375
      Left            =   7620
      TabIndex        =   6
      Top             =   420
      Width           =   1155
   End
   Begin VB.CommandButton cmdConnect 
      Caption         =   "Connect"
      Height          =   375
      Left            =   6480
      TabIndex        =   5
      Top             =   420
      Width           =   1095
   End
   Begin VB.CheckBox chkUseSSL 
      Caption         =   "Use SSL?"
      Height          =   375
      Left            =   2880
      TabIndex        =   4
      Top             =   360
      Width           =   1095
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
      Left            =   2040
      TabIndex        =   3
      Text            =   "17000"
      Top             =   360
      Width           =   675
   End
   Begin VB.TextBox txtHost 
      Height          =   375
      Left            =   240
      TabIndex        =   1
      Text            =   "localhost"
      Top             =   360
      Width           =   1695
   End
   Begin VB.Label Label8 
      Caption         =   "Password:"
      Height          =   255
      Left            =   5280
      TabIndex        =   30
      Top             =   120
      Width           =   1035
   End
   Begin VB.Label Label7 
      Caption         =   "User ID:"
      Height          =   255
      Left            =   4080
      TabIndex        =   28
      Top             =   120
      Width           =   855
   End
   Begin VB.Label Label2 
      Caption         =   "Port#:"
      Height          =   255
      Left            =   2100
      TabIndex        =   2
      Top             =   60
      Width           =   855
   End
   Begin VB.Label Label1 
      Caption         =   "Host:"
      Height          =   255
      Left            =   240
      TabIndex        =   0
      Top             =   60
      Width           =   1515
   End
End
Attribute VB_Name = "frmChat"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Dim WithEvents USocket As USOCKETLib.USocket
Attribute USocket.VB_VarHelpID = -1
Dim UChat As USOCKETLib.IUChat

Private Sub cmdConnect_Click()
On Error GoTo ccHandler
    Dim strError As String
    If chkUseSSL.Value > 0 Then
        USocket.EncryptionMethod = USOCKETLib.tagEncryptionMethod.MSSSL
    Else
        USocket.EncryptionMethod = USOCKETLib.tagEncryptionMethod.NoEncryption
    End If
    USocket.Connect txtHost.Text, CLng(txtPort.Text)
    strError = USocket.ErrorMsg
    Exit Sub
ccHandler:
    MsgBox "Error code = " & USocket.Rtn & ". " & USocket.ErrorMsg
End Sub

Private Sub cmdExit_Click()
    UChat.Exit
End Sub

Private Sub cmdGetAllClients_Click()
    UChat.GetAllClients
End Sub

Private Sub cmdGetGroups_Click()
    UChat.GetAllGroups
End Sub

Private Sub cmdGetListeners_Click()
    UChat.GetAllListeners -1 'Get all listeners for all of groups
End Sub

Private Sub cmdJoin_Click()
On Error GoTo jHandler
    UChat.Enter txtGroupJoined.Text
    Exit Sub
jHandler:
    MsgBox "Error code = " & USocket.Rtn & ". " & USocket.ErrorMsg
End Sub

Private Sub cmdShutDown_Click()
    USocket.Shutdown
End Sub

Private Sub cmdSpeakGroups_Click()
    UChat.Speak txtMsgSpeak.Text, txtGroups.Text
End Sub

Private Sub cmdSpeakTo_Click()
On Error GoTo stHandler
    UChat.SpeakTo txtListenerAddress.Text, CLng(txtListenerPort.Text), txtMsgOut.Text
    Exit Sub
stHandler:
    MsgBox "Error code = " & USocket.Rtn & ". " & USocket.ErrorMsg
End Sub

Private Sub Form_Load()
    Set USocket = New USOCKETLib.USocket
    Set UChat = USocket
    USocket.RecvTimeout = 60000
    USocket.SendTimeout = 60000
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Set UChat = Nothing
    Set USocket = Nothing
End Sub

Private Sub USocket_OnOtherMessage(ByVal hSocket As Long, ByVal nMsg As Long, ByVal wParam As Long, ByVal lParam As Long)
    If nMsg = USOCKETLib.tagClientMessage.msgSSLEvent Then
        If wParam = USOCKETLib.tagSSLEvent.ssleDoSSLConnect Then
            Debug.Print "OpenSSL libraries loaded, and we are going to do SSL connect"
        End If
        If wParam = USOCKETLib.tagSSLEvent.ssleHandshakeDone Then
            USocket.SwitchTo USOCKETLib.tagServiceID.sidChat
            cmdJoin.Enabled = True
            cmdSpeakTo.Enabled = True
            cmdGetGroups.Enabled = True
            cmdGetListeners.Enabled = True
            cmdExit.Enabled = True
            cmdSpeakGroups.Enabled = True
            cmdGetAllClients.Enabled = True
        End If
    End If
End Sub

Private Sub USocket_OnRequestProcessed(ByVal hSocket As Long, ByVal nRequestID As Integer, ByVal lLen As Long, ByVal lLenInBuffer As Long, ByVal sFlag As Integer)
    Dim lCount As Long
    Dim lIndex As Long
    Dim lGroupID As Long
    Dim lSvsID As Long
    Dim strItem As String
    Dim strDescription As String
    Dim strListenerAddr As String
    Dim strUserID As String
    Dim lPort As Long
    If sFlag <> USOCKETLib.tagReturnFlag.rfCompleted Then
        Exit Sub
    End If
    Select Case nRequestID
    Case USOCKETLib.tagChatRequestID.idGetAllGroups
        lstGroups.Clear
        lCount = UChat.Groups
        For lIndex = 0 To lCount - 1
            strDescription = UChat.GetGroupInfo(lIndex, lGroupID)
            strItem = "Group ID = " & lGroupID & " Group Description = " & strDescription
            lstGroups.AddItem strItem
        Next
    Case USOCKETLib.tagChatRequestID.idGetAllClients
        lstAllClients.Clear
        lCount = UChat.Listeners
        For lIndex = 0 To lCount - 1
            strListenerAddr = UChat.GetInfo(lIndex, lGroupID, strUserID, lSvsID, lPort)
            strItem = "User ID = " & strUserID & "@" & strListenerAddr & ":" & lPort & " within GroupID = " & lGroupID
            lstAllClients.AddItem strItem
        Next
    Case USOCKETLib.tagChatRequestID.idGetAllListeners
        lstListeners.Clear
        lCount = UChat.Listeners
        For lIndex = 0 To lCount - 1
            strListenerAddr = UChat.GetInfo(lIndex, lGroupID, strUserID, lSvsID, lPort)
            strItem = "User ID = " & strUserID & "@" & strListenerAddr & ":" & lPort & " within GroupID = " & lGroupID
            lstListeners.AddItem strItem
        Next
    Case USOCKETLib.tagChatRequestID.idSpeak, USOCKETLib.tagChatRequestID.idSendUserMessage, USOCKETLib.tagChatRequestID.idXSpeak
        lstListeners.Clear
        lCount = UChat.Listeners
        For lIndex = 0 To lCount - 1
            strListenerAddr = UChat.GetInfo(lIndex, lGroupID, strUserID, lSvsID, lPort)
            strItem = "User ID = " & strUserID & "@" & strListenerAddr & ":" & lPort & " within GroupID = " & lGroupID
            lstListeners.AddItem strItem
        Next
        txtMsgIn.Text = UChat.Message
    Case USOCKETLib.tagChatRequestID.idSpeakTo
        lstListeners.Clear
        lCount = UChat.Listeners
        For lIndex = 0 To lCount - 1
            strListenerAddr = UChat.GetInfo(lIndex, lGroupID, strUserID, lSvsID, lPort)
            strItem = "User ID = " & strUserID & "@" & strListenerAddr & ":" & lPort & " within GroupID = " & lGroupID
            lstListeners.AddItem strItem
        Next
        txtMsgIn.Text = UChat.Message
    Case USOCKETLib.tagChatRequestID.idEnter, USOCKETLib.tagChatRequestID.idXEnter
        lstListeners.Clear
        lCount = UChat.Listeners
        For lIndex = 0 To lCount - 1
            strListenerAddr = UChat.GetInfo(lIndex, lGroupID, strUserID, lSvsID, lPort)
            strItem = "User ID = " & strUserID & "@" & strListenerAddr & ":" & lPort & " within GroupID = " & lGroupID
            lstListeners.AddItem strItem
        Next
        txtMsgIn.Text = "Join"
    Case USOCKETLib.tagChatRequestID.idExit
        lstListeners.Clear
        lCount = UChat.Listeners
        For lIndex = 0 To lCount - 1
            strListenerAddr = UChat.GetInfo(lIndex, lGroupID, strUserID, lSvsID, lPort)
            strItem = "User ID = " & strUserID & "@" & strListenerAddr & ":" & lPort & " within GroupID = " & lGroupID
            lstListeners.AddItem strItem
        Next
        txtMsgIn.Text = "Exit"
    Case Else
    End Select
End Sub

Private Sub USocket_OnSocketClosed(ByVal hSocket As Long, ByVal lError As Long)
    cmdJoin.Enabled = False
    cmdSpeakTo.Enabled = False
    cmdGetGroups.Enabled = False
    cmdGetListeners.Enabled = False
    cmdExit.Enabled = False
    cmdSpeakGroups.Enabled = False
    cmdGetAllClients.Enabled = False
End Sub

Private Sub USocket_OnSocketConnected(ByVal hSocket As Long, ByVal lError As Long)
    If lError = 0 Then
        
        USocket.UserID = txtUserID.Text
        USocket.Password = txtPassword.Text
        USocket.SwitchTo USOCKETLib.tagServiceID.sidChat
        
        'clean password right after calling SwitchTo for the better security
        USocket.Password = ""
        
        cmdJoin.Enabled = True
        cmdSpeakTo.Enabled = True
        cmdGetGroups.Enabled = True
        cmdGetListeners.Enabled = True
        cmdExit.Enabled = True
        cmdSpeakGroups.Enabled = True
        cmdGetAllClients.Enabled = True
        
    Else
        MsgBox "Having problem in connecting, and error code = " & lError
    End If
End Sub
