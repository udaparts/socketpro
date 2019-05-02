
UFileps.dll: dlldata.obj UFile_p.obj UFile_i.obj
	link /dll /out:UFileps.dll /def:UFileps.def /entry:DllMain dlldata.obj UFile_p.obj UFile_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del UFileps.dll
	@del UFileps.lib
	@del UFileps.exp
	@del dlldata.obj
	@del UFile_p.obj
	@del UFile_i.obj
