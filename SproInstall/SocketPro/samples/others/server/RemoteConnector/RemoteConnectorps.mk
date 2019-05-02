
RemoteConnectorps.dll: dlldata.obj RemoteConnector_p.obj RemoteConnector_i.obj
	link /dll /out:RemoteConnectorps.dll /def:RemoteConnectorps.def /entry:DllMain dlldata.obj RemoteConnector_p.obj RemoteConnector_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del RemoteConnectorps.dll
	@del RemoteConnectorps.lib
	@del RemoteConnectorps.exp
	@del dlldata.obj
	@del RemoteConnector_p.obj
	@del RemoteConnector_i.obj
