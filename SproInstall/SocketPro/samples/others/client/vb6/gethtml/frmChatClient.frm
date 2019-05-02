VERSION 5.00
Object = "{3B7C8863-D78F-101B-B9B5-04021C009402}#1.2#0"; "RICHTX32.OCX"
Begin VB.Form frmChatVB 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "Get a html file"
   ClientHeight    =   6570
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   9255
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   6570
   ScaleWidth      =   9255
   StartUpPosition =   3  'Windows Default
   Begin VB.CheckBox chkSynName 
      Caption         =   "Syn ?"
      Height          =   315
      Left            =   7380
      TabIndex        =   21
      Top             =   6120
      Width           =   1455
   End
   Begin VB.CheckBox chkSynAddr 
      Caption         =   "Syn ?"
      Height          =   255
      Left            =   7380
      TabIndex        =   20
      Top             =   5700
      Width           =   1515
   End
   Begin VB.CommandButton cmdGetHostByName 
      Caption         =   "GetHostByName"
      Height          =   315
      Left            =   240
      TabIndex        =   19
      Top             =   6120
      Width           =   1455
   End
   Begin VB.CommandButton cmdGetHostByAddr 
      Caption         =   "GetHostByAddr"
      Height          =   375
      Left            =   240
      TabIndex        =   18
      Top             =   5640
      Width           =   1455
   End
   Begin VB.TextBox txtHostByName 
      Height          =   375
      Left            =   1740
      TabIndex        =   17
      Top             =   6060
      Width           =   5535
   End
   Begin VB.TextBox txtHostByAddr 
      Height          =   375
      Left            =   1740
      TabIndex        =   16
      Top             =   5640
      Width           =   5535
   End
   Begin VB.CheckBox chkUseSSL 
      Caption         =   "Use SSL"
      Height          =   255
      Left            =   4440
      TabIndex        =   15
      Top             =   360
      Value           =   1  'Checked
      Width           =   975
   End
   Begin RichTextLib.RichTextBox rtbRecv 
      Height          =   3615
      Left            =   240
      TabIndex        =   14
      Top             =   720
      Width           =   8775
      _ExtentX        =   15478
      _ExtentY        =   6376
      _Version        =   393217
      BorderStyle     =   0
      Enabled         =   -1  'True
      ScrollBars      =   3
      TextRTF         =   $"frmChatClient.frx":0000
   End
   Begin VB.TextBox txtIPAddr 
      Enabled         =   0   'False
      Height          =   315
      Left            =   6120
      TabIndex        =   12
      Top             =   300
      Width           =   1500
   End
   Begin VB.CommandButton cmdAddLF 
      Caption         =   "Add LF"
      Height          =   375
      Left            =   7860
      TabIndex        =   11
      Top             =   5160
      Width           =   1155
   End
   Begin VB.CommandButton cmdAddCR 
      Caption         =   "Add CR"
      Height          =   375
      Left            =   6420
      TabIndex        =   10
      Top             =   5160
      Width           =   1395
   End
   Begin VB.CommandButton cmdClear 
      Caption         =   "Clear messages"
      Height          =   375
      Left            =   7140
      TabIndex        =   9
      Top             =   4320
      Width           =   1875
   End
   Begin VB.CommandButton cmdSend 
      Caption         =   "Send to HTTP server"
      Enabled         =   0   'False
      Height          =   375
      Left            =   240
      TabIndex        =   8
      Top             =   5160
      Width           =   5955
   End
   Begin VB.TextBox txtMsg 
      Height          =   375
      Left            =   240
      TabIndex        =   6
      Text            =   "GET / HTTP/1.0"
      Top             =   4740
      Width           =   8775
   End
   Begin VB.CommandButton cmdDisconnect 
      Caption         =   "Disconnect"
      Height          =   315
      Left            =   7620
      TabIndex        =   5
      Top             =   360
      Width           =   1395
   End
   Begin VB.CommandButton cmdConnect 
      Caption         =   "Connect"
      Height          =   315
      Left            =   7620
      TabIndex        =   4
      Top             =   0
      Width           =   1395
   End
   Begin VB.TextBox lPort 
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
      Left            =   5460
      TabIndex        =   3
      Text            =   "443"
      Top             =   300
      Width           =   615
   End
   Begin VB.TextBox txtServer 
      Height          =   315
      Left            =   240
      TabIndex        =   0
      Text            =   "www.paypal.com"
      Top             =   300
      Width           =   4095
   End
   Begin VB.Label lblIPAddr 
      Caption         =   "IP Address:"
      Height          =   255
      Left            =   6120
      TabIndex        =   13
      Top             =   60
      Width           =   1095
   End
   Begin VB.Label Label1 
      Caption         =   "Command:"
      Height          =   255
      Left            =   240
      TabIndex        =   7
      Top             =   4440
      Width           =   4695
   End
   Begin VB.Label lblPort 
      Caption         =   "Port #:"
      Height          =   255
      Left            =   5460
      TabIndex        =   2
      Top             =   60
      Width           =   675
   End
   Begin VB.Label lblChatHost 
      Caption         =   "Host (www.microsoft.com or 207.46.230.220) :"
      Height          =   195
      Left            =   240
      TabIndex        =   1
      Top             =   60
      Width           =   4035
   End
End
Attribute VB_Name = "frmChatVB"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
'// This is a part of the SocketPro package.
'// Copyright (C) 2000-2004 UDAParts
'// All rights reserved.
'//
'// This source code is only intended as a supplement to the
'// SocketPro package and related electronic documentation provided with the package.
'// See these sources for detailed information regarding this
'// UDAParts product.
'
'// http://www.udaparts.com/index.htm
'// support@udaparts.com

Dim WithEvents ClientSocket As USocketLib.USocket
Attribute ClientSocket.VB_VarHelpID = -1
Dim strRecv As String

Private Sub ClientSocket_OnDataAvailable(ByVal hSocket As Long, ByVal lBytes As Long, ByVal lInfo As Long)
    Dim strData As String
    Dim vtData As Variant
    On Error GoTo averrHandler
    vtData = ClientSocket.Recv
    
    'convert bytes into wide string
    strData = StrConv(vtData, vbUnicode)
    strRecv = strRecv + strData
    Exit Sub
averrHandler:
    MsgBox "Error happens, and error code = " & CStr(ClientSocket.GetRtn())
End Sub


Private Sub ClientSocket_OnGetHostByAddr(ByVal hHandle As Long, ByVal bstrHostName As String, ByVal bstrHostAlias As String, ByVal lError As Long)
   If lError = 0 Then
    rtbRecv.Text = "Host Name = " & bstrHostName & ", Alias = " & bstrHostAlias
   Else
    rtbRecv.Text = "Error code = " & lError & ", Error message = " & ClientSocket.ErrorMsg
   End If
End Sub

Private Sub ClientSocket_OnGetHostByName(ByVal hHandle As Long, ByVal bstrHostName As String, ByVal bstrAlias As String, ByVal bstrIPAddr As String, ByVal lError As Long)
  If lError = 0 Then
    rtbRecv.Text = "Host Name = " & bstrHostName & ", Host Alias = " & bstrAlias & ", IP Addr = " & bstrIPAddr
   Else
    rtbRecv.Text = "Error code = " & lError & ", Error message = " & ClientSocket.ErrorMsg
   End If
End Sub

Private Sub ClientSocket_OnOtherMessage(ByVal hSocket As Long, ByVal nMsg As Long, ByVal wParam As Long, ByVal lParam As Long)
    If nMsg = USocketLib.msgSSLEvent Then
        If wParam = USocketLib.tagSSLEvent.ssleHandshakeStarted Then
            rtbRecv.Text = "Handshake started, lParam = " & lParam
        ElseIf wParam = USocketLib.tagSSLEvent.ssleHandshakeDone Then
            cmdSend.Enabled = True
            rtbRecv.Text = "Handshake done, lParam = " & lParam
        ElseIf wParam = USocketLib.tagSSLEvent.ssleHandshakeLoop Then
            rtbRecv.Text = "Handshake loop, lParam = " & lParam
        ElseIf wParam = USocketLib.tagSSLEvent.ssleHandshakeExit Then
            rtbRecv.Text = "Handshake exit, lParam = " & lParam
        ElseIf wParam = USocketLib.tagSSLEvent.ssleReadAlert Then
            rtbRecv.Text = "Read Alert, lParam = " & lParam
        ElseIf wParam = USocketLib.tagSSLEvent.ssleWriteAlert Then
            rtbRecv.Text = "Write Alert, lParam = " & lParam
        Else
            rtbRecv.Text = "Unknown event!"
        End If
    End If
End Sub

Private Sub ClientSocket_OnSocketClosed(ByVal hSocket As Long, ByVal lError As Long)
    Debug.Print "Socket closed"
    cmdSend.Enabled = False
    rtbRecv.Text = strRecv
End Sub

Private Sub ClientSocket_OnSocketConnected(ByVal hSocket As Long, ByVal lError As Long)
    Dim lPort As Long
    Dim strAddr As String
    If lError Then
        MsgBox "Failed in establishing a socket connection, error code = " & lError
    Else
        txtIPAddr.Text = ClientSocket.GetPeerName(lPort)
        Debug.Print "Peer = " & txtIPAddr & ", Port = " & lPort
        If chkUseSSL.Value > 0 Then Exit Sub
        cmdSend.Enabled = True
    End If
    
End Sub

Private Sub cmdAddCR_Click()
    txtMsg.Text = txtMsg.Text & Chr$(13)
End Sub

Private Sub cmdAddLF_Click()
    txtMsg.Text = txtMsg.Text & Chr$(10)
End Sub

Private Sub cmdClear_Click()
    rtbRecv.Text = ""
End Sub

Private Sub cmdConnect_Click()
    On Error GoTo cerrHandler
    If chkUseSSL.Value = 1 Then
        ClientSocket.EncryptionMethod = USocketLib.tagEncryptionMethod.MSTLSv1
    Else
        ClientSocket.EncryptionMethod = USocketLib.tagEncryptionMethod.NoEncryption
    End If
    ClientSocket.Connect txtServer, lPort
    Exit Sub
cerrHandler:
    MsgBox "Error happens, and error code = " & CStr(ClientSocket.GetRtn())
End Sub

Private Sub cmdDisconnect_Click()
    strRecv = ""
    ClientSocket.Shutdown
End Sub


Private Sub cmdGetHostByAddr_Click()
    Dim bSyn As Boolean
    Dim strHostName As String
    Dim lHandle As Long
    Dim strAlias As String
    If chkSynAddr.Value > 0 Then
        bSyn = True
    Else
        bSyn = False
    End If
    strHostName = ClientSocket.GetHostByAddr(txtHostByAddr.Text, 2, bSyn, lHandle, strAlias)
    rtbRecv.Text = "Host Name = " & strHostName & ", Alias = " & strAlias
End Sub

Private Sub cmdGetHostByName_Click()
    Dim bSyn As Boolean
    Dim strIPAddr As String
    Dim strHost As String
    Dim strAlias As String
    Dim lHandle As Long
    If chkSynName.Value > 0 Then
        bSyn = True
    Else
        bSyn = False
    End If
    strHost = ClientSocket.GetHostByName(txtHostByName.Text, bSyn, lHandle, strIPAddr, strAlias)
    rtbRecv.Text = "Host = " & strHost & ", IP Addr = " & strIPAddr & ", Alias = " & strAlias
End Sub

Private Sub cmdSend_Click()
    On Error GoTo serrHandler
    Dim byteData() As Byte
    Dim asciiStr() As Byte
    Dim vtData As Variant
    Dim lLen As Long
    Dim lIndex As Long
    strRecv = ""
    lLen = Len(txtMsg.Text)
    byteData = txtMsg.Text
    ReDim asciiStr(lLen) As Byte
    
    'convert wide string into ASCII string in bytes
    For lIndex = 0 To lLen - 1
        asciiStr(lIndex) = byteData(lIndex * 2)
    Next
    
    lLen = ClientSocket.Send(asciiStr) 'send an array of bytes to a remote host
    
    Debug.Print lLen & " bytes of data are sent to a remote server"
    
    Exit Sub
serrHandler:
    MsgBox "Error happens, and error code = " & CStr(ClientSocket.Rtn)
End Sub

Private Sub Form_Load()
    Set ClientSocket = New USocketLib.USocket
    
    'SocketPro client is written for sending a request to a remote SocketPro server by default
    'In case you use the client software to send data to a non-SocketPro server,
    'you have to set the following property to true.
    ClientSocket.UseBaseSocketOnly = True
    
    txtMsg.Text = "GET / HTTP/1.0" & Chr$(13) & Chr$(10) & Chr$(13) & Chr$(10)
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Set ClientSocket = Nothing
End Sub

'// This is a part of the SocketPro package.
'// Copyright (C) 2000-2004 UDAParts
'// All rights reserved.
'//
'// This source code is only intended as a supplement to the
'// SocketPro package and related electronic documentation provided with the package.
'// See these sources for detailed information regarding this
'// UDAParts product.
'
'// http://www.udaparts.com/index.htm
'// support@udaparts.com

