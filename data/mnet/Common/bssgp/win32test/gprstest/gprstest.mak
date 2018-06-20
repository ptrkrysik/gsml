# Microsoft Developer Studio Generated NMAKE File, Based on gprstest.dsp
!IF "$(CFG)" == ""
CFG=gprstest - Win32 Debug
!MESSAGE No configuration specified. Defaulting to gprstest - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "gprstest - Win32 Release" && "$(CFG)" != "gprstest - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "gprstest.mak" CFG="gprstest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "gprstest - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "gprstest - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "gprstest - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\gprstest.exe"


CLEAN :
	-@erase "$(INTDIR)\bssgp_decoder.obj"
	-@erase "$(INTDIR)\bssgp_encoder.obj"
	-@erase "$(INTDIR)\bssgp_ie_decoder.obj"
	-@erase "$(INTDIR)\bssgp_ie_encoder.obj"
	-@erase "$(INTDIR)\bssgp_unitdata.obj"
	-@erase "$(INTDIR)\bssgp_util.obj"
	-@erase "$(INTDIR)\bssgpwin32.obj"
	-@erase "$(INTDIR)\dbgout.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\gprstest.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\gprstest.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\gprstest.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\gprstest.pdb" /machine:I386 /out:"$(OUTDIR)\gprstest.exe" 
LINK32_OBJS= \
	"$(INTDIR)\bssgp_util.obj" \
	"$(INTDIR)\bssgp_encoder.obj" \
	"$(INTDIR)\bssgp_ie_decoder.obj" \
	"$(INTDIR)\bssgp_ie_encoder.obj" \
	"$(INTDIR)\bssgp_unitdata.obj" \
	"$(INTDIR)\bssgp_decoder.obj" \
	"$(INTDIR)\dbgout.obj" \
	"$(INTDIR)\bssgpwin32.obj"

"$(OUTDIR)\gprstest.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "gprstest - Win32 Debug"

OUTDIR=.
INTDIR=.
# Begin Custom Macros
OutDir=.
# End Custom Macros

ALL : "$(OUTDIR)\gprstest.exe"


CLEAN :
	-@erase "$(INTDIR)\bssgp_decoder.obj"
	-@erase "$(INTDIR)\bssgp_encoder.obj"
	-@erase "$(INTDIR)\bssgp_ie_decoder.obj"
	-@erase "$(INTDIR)\bssgp_ie_encoder.obj"
	-@erase "$(INTDIR)\bssgp_unitdata.obj"
	-@erase "$(INTDIR)\bssgp_util.obj"
	-@erase "$(INTDIR)\bssgpwin32.obj"
	-@erase "$(INTDIR)\dbgout.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\gprstest.exe"
	-@erase "$(OUTDIR)\gprstest.ilk"
	-@erase "$(OUTDIR)\gprstest.pdb"

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /I "\common\include" /I "\common\bssgp\win32test" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\gprstest.pch" /YX /FD /GZ  /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\gprstest.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib wsock32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\gprstest.pdb" /debug /machine:I386 /out:"$(OUTDIR)\gprstest.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\bssgp_util.obj" \
	"$(INTDIR)\bssgp_encoder.obj" \
	"$(INTDIR)\bssgp_ie_decoder.obj" \
	"$(INTDIR)\bssgp_ie_encoder.obj" \
	"$(INTDIR)\bssgp_unitdata.obj" \
	"$(INTDIR)\bssgp_decoder.obj" \
	"$(INTDIR)\dbgout.obj" \
	"$(INTDIR)\bssgpwin32.obj"

"$(OUTDIR)\gprstest.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("gprstest.dep")
!INCLUDE "gprstest.dep"
!ELSE 
!MESSAGE Warning: cannot find "gprstest.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "gprstest - Win32 Release" || "$(CFG)" == "gprstest - Win32 Debug"
SOURCE=..\..\src\bssgp_decoder.cpp

!IF  "$(CFG)" == "gprstest - Win32 Release"

CPP_SWITCHES=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\gprstest.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\bssgp_decoder.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "gprstest - Win32 Debug"

CPP_SWITCHES=/nologo /MLd /W3 /Gm /GX /ZI /Od /I "\common\include" /I "\common\bssgp\win32test" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\gprstest.pch" /YX /FD /GZ  /c 

"$(INTDIR)\bssgp_decoder.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\..\src\bssgp_encoder.cpp

"$(INTDIR)\bssgp_encoder.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\bssgp_ie_decoder.cpp

"$(INTDIR)\bssgp_ie_decoder.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\bssgp_ie_encoder.cpp

"$(INTDIR)\bssgp_ie_encoder.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\bssgp_unitdata.cpp

"$(INTDIR)\bssgp_unitdata.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\bssgp_util.cpp

"$(INTDIR)\bssgp_util.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\bssgpwin32.cpp

"$(INTDIR)\bssgpwin32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\dbgout.cpp

"$(INTDIR)\dbgout.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

