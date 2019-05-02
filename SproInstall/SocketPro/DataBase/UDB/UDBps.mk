
UDBps.dll: dlldata.obj UDB_p.obj UDB_i.obj
	link /dll /out:UDBps.dll /def:UDBps.def /entry:DllMain dlldata.obj UDB_p.obj UDB_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del UDBps.dll
	@del UDBps.lib
	@del UDBps.exp
	@del dlldata.obj
	@del UDB_p.obj
	@del UDB_i.obj
