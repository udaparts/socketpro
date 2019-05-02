Imports System
Imports System.Data
Imports System.Configuration
Imports System.Collections
Imports System.Web
Imports System.Web.Security
Imports System.Web.UI
Imports System.Web.UI.WebControls
Imports System.Web.UI.WebControls.WebParts
Imports System.Web.UI.HtmlControls
Imports SocketProAdapter
Imports SocketProAdapter.ClientSide

Public Partial Class _Default
    Inherits System.Web.UI.Page
    Protected Sub Page_Load(ByVal sender As Object, ByVal e As EventArgs)
        If AspNetPush.Comet.UrlToComet.Length = 0 Then
            'make sure you have already started the HTTP COMET server at
            'C:\Program Files\UDAParts\SocketPro\tutorial\VBNet\Chat\Server
            AspNetPush.Comet.UrlToComet = "http://localhost:20901/UCHAT"
        End If
    End Sub
    Protected Sub btnEnter_Click(ByVal sender As Object, ByVal e As EventArgs)
        Dim Groups() As Integer = {1, 2, 8}
        Dim strEnter As String = AspNetPush.Comet.Enter(Me, txtUserId.Text, Groups, "onMyMessage")
        btnEnter.Enabled = False
    End Sub
End Class

