# Makefile for VC


all: delogo.dll


delogo.dll: delogo.obj delogo.res
	link /DLL /OUT:delogo.dll delogo.obj delogo.res

.cpp.obj:
	cl /c /O2 /EHsc /Fo$@ $<

.rc.res:
	rc $<

delogo.cpp: avisynth.h ../logo.h delogo.h yuy2.h yv12.h


clean:
	del *.obj *.res *.lib *.exp


