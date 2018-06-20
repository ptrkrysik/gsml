# Microsoft Developer Studio Project File - Name="snmp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=snmp - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "snmp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "snmp.mak" CFG="snmp - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "snmp - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "snmp - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "snmp"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "snmp - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "snmp - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /X /I ".\include" /I "..\Common_To_Host\include" /I ".." /I "..\..\..\Common\include" /I "..\..\..\Common\Snmp\Include" /I "D:\Tornado86\target\h" /I "D:\Tornado86\target\h\snmp" /I "D:\snapshot\VxVmi860\vxVmi\target\h\snmp" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D CPU=I80486 /D "__VXWORKS__" /D "VXWORKS" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "snmp - Win32 Release"
# Name "snmp - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\mib_init.c
# End Source File
# Begin Source File

SOURCE=.\src\testMib.c
# End Source File
# Begin Source File

SOURCE=.\src\vipertbl.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "/Common/Snmp/Src"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\Common\snmp\src\api_init.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\snmp\src\link_list.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\snmp\src\oam_api.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\snmp\src\snmp_fileio.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\snmp\src\snmp_trap.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\snmp\src\snmpLocalIoLib.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\snmp\src\table.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\snmp\src\vipermib.c
# End Source File
# End Group
# Begin Group "/Common/Snmp/Include"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\Common\snmp\include\snmp_common.h
# End Source File
# End Group
# Begin Group "/Common/Include"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\Common\include\oam_api.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\include\oam_trap.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\include\table.h
# End Source File
# End Group
# Begin Group "/Common_to_host/Include"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Common_To_Host\include\mibhand.h
# End Source File
# Begin Source File

SOURCE=..\Common_To_Host\include\mibleaf.h
# End Source File
# Begin Source File

SOURCE=..\Common_To_Host\include\mibtags.h
# End Source File
# Begin Source File

SOURCE=..\Common_To_Host\include\vipermib.h
# End Source File
# Begin Source File

SOURCE=..\Common_To_Host\include\vipertbl.h
# End Source File
# End Group
# End Target
# End Project
