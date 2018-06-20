# Microsoft Developer Studio Generated NMAKE File, Based on AlarmCode.dsp


!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

OUTDIR=.\.
INTDIR=.\.
# Begin Custom Macros

ALARMCODE= ..\AlarmCode.English

# End Custom Macros

ALL : $(ALARMCODE)


CLEAN :
	-@erase "$(INTDIR)\AlarmCode.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\AlarmCode.exe"
	-@erase "$(OUTDIR)\AlarmCode.ilk"
	-@erase "$(OUTDIR)\AlarmCode.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ= /MLd /W3 /Gm /GX /ZI /Od /I "..\..\Common_To_Host\include" /I "..\..\Common_To_Host" /I "..\..\..\..\Common\include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\AlarmCode.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\AlarmCode.pdb" /debug /machine:I386 /out:"$(OUTDIR)\AlarmCode.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\AlarmCode.obj"

$(ALARMCODE) : "$(OUTDIR)\AlarmCode.exe"
	"$(OUTDIR)\AlarmCode.exe" > "$@"


"$(OUTDIR)\AlarmCode.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

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



SOURCE=.\AlarmCode.cpp

"$(INTDIR)\AlarmCode.obj" : $(SOURCE) "$(INTDIR)"

