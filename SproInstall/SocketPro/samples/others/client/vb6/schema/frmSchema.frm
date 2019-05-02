VERSION 5.00
Object = "{5E9E78A0-531B-11CF-91F6-C2863C385E30}#1.0#0"; "MSFLXGRD.OCX"
Begin VB.Form frmSchema 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "Fast retrieving a schema information of a data source from anywhere"
   ClientHeight    =   9555
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   10965
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   9555
   ScaleWidth      =   10965
   StartUpPosition =   3  'Windows Default
   Begin VB.TextBox txtUserID 
      Height          =   315
      Left            =   8100
      TabIndex        =   36
      Text            =   "SocketPro"
      Top             =   60
      Width           =   1095
   End
   Begin VB.TextBox txtPassword 
      Height          =   315
      IMEMode         =   3  'DISABLE
      Left            =   8100
      PasswordChar    =   "*"
      TabIndex        =   35
      Text            =   "PassOne"
      Top             =   480
      Width           =   1095
   End
   Begin VB.CheckBox chkZip 
      Caption         =   "Zip ?"
      Height          =   315
      Left            =   3600
      TabIndex        =   34
      Top             =   480
      Width           =   915
   End
   Begin VB.CheckBox chkUseSSL 
      Caption         =   "Use SSL"
      Height          =   255
      Left            =   3600
      TabIndex        =   33
      Top             =   75
      Width           =   1215
   End
   Begin VB.TextBox txtErrorMsg 
      Height          =   375
      Left            =   1620
      Locked          =   -1  'True
      TabIndex        =   32
      Top             =   4320
      Width           =   9135
   End
   Begin VB.Frame Frame2 
      Caption         =   "Restrictions:"
      Height          =   2715
      Left            =   3780
      TabIndex        =   17
      Top             =   1500
      Width           =   6975
      Begin VB.TextBox txtType 
         Height          =   375
         Left            =   2100
         TabIndex        =   30
         Top             =   2040
         Width           =   4455
      End
      Begin VB.TextBox txtName 
         Height          =   375
         Left            =   2100
         TabIndex        =   29
         Top             =   1500
         Width           =   4455
      End
      Begin VB.TextBox txtSchema 
         Height          =   375
         Left            =   2100
         TabIndex        =   28
         Top             =   960
         Width           =   4455
      End
      Begin VB.TextBox txtCatalog 
         Height          =   375
         Left            =   2100
         TabIndex        =   27
         Top             =   420
         Width           =   4455
      End
      Begin VB.Label Label12 
         Alignment       =   1  'Right Justify
         Caption         =   "TABLE_TYPE:"
         Height          =   435
         Left            =   420
         TabIndex        =   26
         Top             =   2100
         Width           =   1575
      End
      Begin VB.Label Label11 
         Alignment       =   1  'Right Justify
         Caption         =   "TABLE_NAME:"
         Height          =   375
         Left            =   420
         TabIndex        =   25
         Top             =   1560
         Width           =   1575
      End
      Begin VB.Label Label10 
         Alignment       =   1  'Right Justify
         Caption         =   "TABLE_SCHEMA:"
         Height          =   375
         Left            =   300
         TabIndex        =   24
         Top             =   1020
         Width           =   1695
      End
      Begin VB.Label Label9 
         Alignment       =   1  'Right Justify
         Caption         =   "TABLE_CATALOG:"
         Height          =   375
         Left            =   180
         TabIndex        =   23
         Top             =   480
         Width           =   1815
      End
      Begin VB.Label Label8 
         Alignment       =   1  'Right Justify
         Caption         =   "TABLE_TYPE:"
         Height          =   315
         Left            =   4635
         TabIndex        =   22
         Top             =   5400
         Width           =   1755
      End
      Begin VB.Label Label7 
         Alignment       =   1  'Right Justify
         Caption         =   "TABLE_NAME:"
         Height          =   315
         Left            =   4755
         TabIndex        =   21
         Top             =   4800
         Width           =   1635
      End
      Begin VB.Label Label4 
         Alignment       =   1  'Right Justify
         Caption         =   "TABLE_SCHEMA:"
         Height          =   375
         Left            =   4575
         TabIndex        =   20
         Top             =   4260
         Width           =   1815
      End
      Begin VB.Label Label3 
         Alignment       =   1  'Right Justify
         Caption         =   "TABLE_CATALOG:"
         Height          =   315
         Left            =   4635
         TabIndex        =   19
         Top             =   3840
         Width           =   1755
      End
   End
   Begin VB.Frame Frame1 
      Caption         =   "Open a predefined schema rowset:"
      Height          =   2715
      Left            =   180
      TabIndex        =   11
      Top             =   1500
      Width           =   3555
      Begin VB.CommandButton cmdOpenSchema 
         Caption         =   "Open a schema rowset"
         Enabled         =   0   'False
         Height          =   375
         Left            =   480
         TabIndex        =   18
         Top             =   2220
         Width           =   2355
      End
      Begin VB.OptionButton optGUIDs 
         Caption         =   "DBSCHEMA_INDEXES"
         Height          =   315
         Index           =   1
         Left            =   360
         TabIndex        =   16
         Top             =   720
         Width           =   2655
      End
      Begin VB.OptionButton optGUIDs 
         Caption         =   "DBSCHEMA_VIEWS"
         Height          =   315
         Index           =   4
         Left            =   360
         TabIndex        =   15
         Top             =   1800
         Width           =   2655
      End
      Begin VB.OptionButton optGUIDs 
         Caption         =   "DBSCHEMA_TABLES"
         Height          =   315
         Index           =   3
         Left            =   360
         TabIndex        =   14
         Top             =   1440
         Width           =   2655
      End
      Begin VB.OptionButton optGUIDs 
         Caption         =   "DBSCHEMA_PRIMARY_KEYS"
         Height          =   315
         Index           =   2
         Left            =   360
         TabIndex        =   13
         Top             =   1080
         Width           =   2655
      End
      Begin VB.OptionButton optGUIDs 
         Caption         =   "DBSCHEMA_COLUMNS"
         Height          =   315
         Index           =   0
         Left            =   360
         TabIndex        =   12
         Top             =   360
         Value           =   -1  'True
         Width           =   2715
      End
   End
   Begin MSFlexGridLib.MSFlexGrid fgRowset 
      Height          =   4275
      Left            =   180
      TabIndex        =   9
      Top             =   5040
      Width           =   10635
      _ExtentX        =   18759
      _ExtentY        =   7541
      _Version        =   393216
      Rows            =   0
      Cols            =   0
      FixedRows       =   0
      FixedCols       =   0
      AllowUserResizing=   3
   End
   Begin VB.CommandButton cmdToDS 
      Caption         =   "Connect to DB"
      Enabled         =   0   'False
      Height          =   375
      Left            =   9300
      TabIndex        =   8
      Top             =   1140
      Width           =   1455
   End
   Begin VB.TextBox txtConnectionString 
      Height          =   375
      Left            =   1740
      TabIndex        =   7
      Text            =   "Provider=Microsoft.Jet.OLEDB.4.0;Data Source=c:\NWIND3.mdb"
      Top             =   1140
      Width           =   7515
   End
   Begin VB.CommandButton cmdShutDown 
      Caption         =   "Shut down"
      Height          =   375
      Left            =   9420
      TabIndex        =   5
      Top             =   480
      Width           =   1335
   End
   Begin VB.CommandButton cmdConnect 
      Caption         =   "Connect"
      Height          =   375
      Left            =   9420
      TabIndex        =   4
      Top             =   60
      Width           =   1335
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
      Left            =   2700
      TabIndex        =   3
      Text            =   "17000"
      Top             =   420
      Width           =   795
   End
   Begin VB.TextBox txtHost 
      Height          =   375
      Left            =   180
      TabIndex        =   1
      Text            =   "localhost"
      Top             =   420
      Width           =   2475
   End
   Begin VB.Label Label15 
      Alignment       =   1  'Right Justify
      Caption         =   "User ID:"
      Height          =   255
      Left            =   7260
      TabIndex        =   38
      Top             =   60
      Width           =   735
   End
   Begin VB.Label Label14 
      Alignment       =   1  'Right Justify
      Caption         =   "Password:"
      Height          =   255
      Left            =   7140
      TabIndex        =   37
      Top             =   480
      Width           =   855
   End
   Begin VB.Label Label13 
      Caption         =   "Error Message:"
      Height          =   315
      Left            =   180
      TabIndex        =   31
      Top             =   3840
      Width           =   1275
   End
   Begin VB.Label Label6 
      Caption         =   "Schema Rowset:"
      Height          =   255
      Left            =   180
      TabIndex        =   10
      Top             =   4740
      Width           =   1875
   End
   Begin VB.Label Label5 
      Alignment       =   1  'Right Justify
      Caption         =   "Connection string:"
      Height          =   255
      Left            =   180
      TabIndex        =   6
      Top             =   1200
      Width           =   1455
   End
   Begin VB.Label Label2 
      Caption         =   "Port:"
      Height          =   255
      Left            =   2700
      TabIndex        =   2
      Top             =   120
      Width           =   495
   End
   Begin VB.Label Label1 
      Caption         =   "Host:"
      Height          =   255
      Left            =   180
      TabIndex        =   0
      Top             =   60
      Width           =   2415
   End
End
Attribute VB_Name = "frmSchema"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Dim WithEvents ClientSocket As USOCKETLib.USocket
Attribute ClientSocket.VB_VarHelpID = -1
Dim WithEvents DataSource As UDBLib.UDataSource
Attribute DataSource.VB_VarHelpID = -1
Dim WithEvents Session As UDBLib.USession
Attribute Session.VB_VarHelpID = -1
Dim WithEvents Rowset As UDBLib.URowset
Attribute Rowset.VB_VarHelpID = -1
Dim iIndex As Long


Private Sub ClientSocket_OnSocketClosed(ByVal hSocket As Long, ByVal lError As Long)
    cmdToDS.Enabled = False
    cmdOpenSchema.Enabled = False
End Sub

Private Sub ClientSocket_OnSocketConnected(ByVal hSocket As Long, ByVal lError As Long)
    If lError = 0 Then 'no error
        Dim lMTU As Long
        Dim lMaxSpeed As Long
        Dim lType As Long
        Dim strMask As String
        Dim strDesc As String
        ClientSocket.StartBatching
        
        ClientSocket.UserID = txtUserID.Text
        ClientSocket.Password = txtPassword.Text
        
        ClientSocket.SwitchTo USOCKETLib.tagServiceID.sidOleDB
        
        'clean password right after calling SwitchTo for the better security
        ClientSocket.Password = ""
        
        strDesc = ClientSocket.GetInterfaceAttributes(lMTU, lMaxSpeed, lType, strMask)
        If lMaxSpeed > 10000000 Then
            ClientSocket.SetSockOptAtSvr USOCKETLib.tagSocketOption.soSndBuf, 1168000
            ClientSocket.SetSockOptAtSvr USOCKETLib.tagSocketOption.soRcvBuf, 1168000
        End If
        If chkZip.Value > 0 Then
            ClientSocket.TurnOnZipAtSvr True
            ClientSocket.ZipIsOn = True
        Else
            ClientSocket.TurnOnZipAtSvr False
            ClientSocket.ZipIsOn = False
        End If
        ClientSocket.CommitBatching 'send all of requests in one batch
        cmdToDS.Enabled = True
    Else
        txtErrorMsg.Text = ClientSocket.ErrorMsg
    End If
End Sub

Private Sub cmdConnect_Click()
On Error GoTo cHandler
    If chkUseSSL.Value > 0 Then
        ClientSocket.EncryptionMethod = USOCKETLib.tagEncryptionMethod.MSTLSv1
    Else
        ClientSocket.EncryptionMethod = USOCKETLib.tagEncryptionMethod.NoEncryption
    End If
    ClientSocket.Connect txtHost.Text, txtPort.Text
    Exit Sub
cHandler:
    txtErrorMsg.Text = ClientSocket.ErrorMsg
End Sub

Private Sub cmdOpenSchema_Click()
    txtErrorMsg.Text = ""
    Dim vtRestrictions() As Variant
    
    'you can get the following GUID data from the file oledb.h
    
    'extern const OLEDBDECLSPEC GUID DBSCHEMA_COLUMNS = {0xc8b52214,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
    'extern const OLEDBDECLSPEC GUID DBSCHEMA_INDEXES = {0xc8b5221e,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
    'extern const OLEDBDECLSPEC GUID DBSCHEMA_PRIMARY_KEYS = {0xc8b522c5,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
    'extern const OLEDBDECLSPEC GUID DBSCHEMA_TABLES = {0xc8b52229,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
    'extern const OLEDBDECLSPEC GUID DBSCHEMA_VIEWS = {0xc8b5222d,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
    
    fgRowset.Rows = 0
    fgRowset.Cols = 0
    
'    iIndex = optGUIDs.
    'see the doc of OLEDB for how to set up restrictions
    Select Case iIndex
    Case 0  'DBSCHEMA_COLUMNS
        ReDim vtRestrictions(2)
        If Len(txtCatalog.Text) > 0 Then
            vtRestrictions(0) = txtCatalog.Text
        End If
        If Len(txtSchema.Text) > 0 Then
            vtRestrictions(1) = txtSchema.Text
        End If
        If Len(txtName.Text) Then
            vtRestrictions(2) = txtName.Text
        End If
        Rowset.GetSchemaRowset "{c8b52214-5cf3-11ce-ade5-00aa0044773d}", vtRestrictions, 50
    Case 1  'DBSCHEMA_INDEXES
        ReDim vtRestrictions(4)
        If Len(txtCatalog.Text) > 0 Then
            vtRestrictions(0) = txtCatalog.Text
        End If
        If Len(txtSchema.Text) > 0 Then
            vtRestrictions(1) = txtSchema.Text
        End If
        If Len(txtName.Text) Then
            vtRestrictions(4) = txtName.Text
        End If
        Rowset.GetSchemaRowset "{c8b5221e-5cf3-11ce-ade5-00aa0044773d}", vtRestrictions, 50
    Case 2 'DBSCHEMA_PRIMARY_KEYS
        ReDim vtRestrictions(2)
        If Len(txtCatalog.Text) > 0 Then
            vtRestrictions(0) = txtCatalog.Text
        End If
        If Len(txtSchema.Text) > 0 Then
            vtRestrictions(1) = txtSchema.Text
        End If
        If Len(txtName.Text) Then
            vtRestrictions(2) = txtName.Text
        End If
        Rowset.GetSchemaRowset "{c8b522c5-5cf3-11ce-ade5-00aa0044773d}", vtRestrictions, 50
    Case 3 'DBSCHEMA_TABLES
        ReDim vtRestrictions(3)
        If Len(txtCatalog.Text) > 0 Then
            vtRestrictions(0) = txtCatalog.Text
        End If
        If Len(txtSchema.Text) > 0 Then
            vtRestrictions(1) = txtSchema.Text
        End If
        If Len(txtName.Text) Then
            vtRestrictions(2) = txtName.Text
        End If
        If Len(txtType.Text) > 0 Then
            vtRestrictions(3) = txtType.Text
        End If
        Rowset.GetSchemaRowset "{c8b52229-5cf3-11ce-ade5-00aa0044773d}", vtRestrictions, 50
    Case 4 'DBSCHEMA_VIEWS
        ReDim vtRestrictions(2)
        If Len(txtCatalog.Text) > 0 Then
            vtRestrictions(0) = txtCatalog.Text
        End If
        If Len(txtSchema.Text) > 0 Then
            vtRestrictions(1) = txtSchema.Text
        End If
        If Len(txtName.Text) Then
            vtRestrictions(2) = txtName.Text
        End If
        Rowset.GetSchemaRowset "{c8b5222d-5cf3-11ce-ade5-00aa0044773d}", vtRestrictions, 50
    Case Else
    End Select
    Rowset.AsynFetch
End Sub

Private Sub cmdShutDown_Click()
    ClientSocket.Shutdown  'gracefully diconnect from a remote server
End Sub

Private Sub cmdToDS_Click()
    txtErrorMsg.Text = ""
    ClientSocket.StartBatching
    DataSource.Open txtConnectionString
    Session.Open
    ClientSocket.CommitBatching True
End Sub

Private Sub Form_Load()
    Set ClientSocket = New USOCKETLib.USocket
    Set DataSource = New UDBLib.UDataSource
    Set Session = New UDBLib.USession
    Set Rowset = New UDBLib.URowset
    ClientSocket.RecvTimeout = 5000
    DataSource.AttachSocket ClientSocket
    Session.AttachSocket ClientSocket
    Rowset.AttachSocket ClientSocket
    
    ClientSocket.ReturnEvents = USOCKETLib.tagReturnFlag.rfCompleted 'we just want compeleted event only
    
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Set Rowset = Nothing
    Set Session = Nothing
    Set DataSource = Nothing
    Set ClientSocket = Nothing
End Sub

Private Sub optGUIDs_Click(Index As Integer)
    iIndex = Index
End Sub

Private Sub Rowset_OnRequestProcessed(ByVal hSocket As Long, ByVal nRequestID As Integer, ByVal lLen As Long, ByVal lLenInBuffer As Long, ByVal sFlag As Integer)
    Dim lCols As Long
    Dim lRows As Long
    Dim lCol As Long
    Dim lRow As Long
    Dim vtData As Variant
    Dim lPrevRows As Long
    If Rowset.Rtn = 0 Then
        Select Case nRequestID
        Case UDBLib.tagDBRequestID.idRowsetAsynFetch
            lCols = Rowset.GetCols
            lRows = Rowset.GetRowsFetched
            If fgRowset.Rows = 0 Then
                fgRowset.Rows = lRows + 1
                fgRowset.Cols = lCols + 1
                If lRows = 0 Then
                    fgRowset.Rows = 2
                End If
                fgRowset.FixedRows = 1
                fgRowset.FixedCols = 1
                For lCol = 1 To lCols
                     fgRowset.TextMatrix(0, lCol) = Rowset.GetColName(lCol)
                Next
            Else
                fgRowset.Rows = fgRowset.Rows + lRows
            End If
            For lRow = 0 To lRows - 1
                For lCol = 1 To lCols
                    'Like column index, data indexed is started from 1
                    vtData = Rowset.GetData(lRow, lCol)
                    fgRowset.TextMatrix(fgRowset.Rows - lRows + lRow, lCol) = vtData
                Next
            Next
        Case UDBLib.tagDBRequestID.idRowsetSendBLOB
            Debug.Print "BLOB count = " & lLen & " BLOB index = " & lLenInBuffer
        Case Else
        End Select
    Else
        txtErrorMsg.Text = Rowset.ErrorMsg
    End If
End Sub

Private Sub Session_OnRequestProcessed(ByVal hSocket As Long, ByVal nRequestID As Integer, ByVal lLen As Long, ByVal lLenInBuffer As Long, ByVal sFlag As Integer)
    If nRequestID = UDBLib.tagDBRequestID.idSessionOpen Then
        If Session.Rtn = 0 Then
            cmdOpenSchema.Enabled = True
        Else
            cmdOpenSchema.Enabled = False
            txtErrorMsg.Text = Session.ErrorMsg
        End If
    End If
End Sub
