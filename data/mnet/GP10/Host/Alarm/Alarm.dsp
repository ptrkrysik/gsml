# Microsoft Developer Studio Project File - Name="Alarm" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Alarm - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Alarm.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Alarm.mak" CFG="Alarm - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Alarm - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Alarm - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Alarm - Win32 Release"

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

!ELSEIF  "$(CFG)" == "Alarm - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Alarm___Win32_Debug"
# PROP BASE Intermediate_Dir "Alarm___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Alarm___Win32_Debug"
# PROP Intermediate_Dir "Alarm___Win32_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ  /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /X /I ".\include" /I "..\Common_To_Host\include" /I "..\..\..\Common\Alarm\include" /I "..\..\..\Common\include" /I "..\Common_to_host" /I "D:\Tornado86\target\h" /I "D:\Tornado86\target\h\snmp" /I "D:\snapshot\VxVmi860\vxVmi\target\h\snmp" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D CPU=I80486 /D "__VXWORKS__" /FR"Debug/" /Fo"Debug/" /Fd"Debug/" /FD /GZ /c
# SUBTRACT CPP /YX
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

# Name "Alarm - Win32 Release"
# Name "Alarm - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\Common\Alarm\src\alarm_diskFile.cpp

!IF  "$(CFG)" == "Alarm - Win32 Release"

!ELSEIF  "$(CFG)" == "Alarm - Win32 Debug"

# ADD CPP /nologo /Gm

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\Common\Alarm\src\alarm_event.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\Alarm\src\alarm_main.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\Alarm\src\alarm_message.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\Alarm\src\alarm_modules.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\Alarm\src\alarm_socket.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\Alarm\src\alarm_sysCmd.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\Common\Alarm\include\alarm.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\Alarm\include\alarm_interface.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\Alarm\include\AlarmTask.h
# End Source File
# End Group
# Begin Group "Common To Host Header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Common_To_Host\include\AlarmCode.h
# End Source File
# Begin Source File

SOURCE=..\Common_To_Host\include\mibtags.h
# End Source File
# Begin Source File

SOURCE=..\Common_To_Host\include\MnetModuleId.h
# End Source File
# End Group
# End Target
# End Project
