# Microsoft Developer Studio Project File - Name="cdr" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=cdr - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "cdr.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "cdr.mak" CFG="cdr - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "cdr - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "cdr - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "cdr"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "cdr - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /O2 /X /I "D:\vxworks\VxVmi860\vxVmi\target\h" /I "D:\vxworks\VxVmi860\vxVmi\target\config\all" /I "D:\vxworks\VxVmi860\vxVmi\target\src\config" /I "D:\vxworks\VxVmi860\vxVmi\target\src\drv" /I "\ViperCell" /I "\ViperCell\Cdr\include" /I "\vipercell\cc\include" /I "jcc\include" /I "\Vipercell\oam\include" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D CPU=PPC860 /D "VXWORKS" /D "H323_Exist" /D "__VXWORKS__" /D "VIPER_IP" /D "FAST_RTP_READ" /D "VX_IGNORE_GNU_LIBS" /D "__PROTOTYPE_5_0" /FR /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "cdr - Win32 Debug"

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
# ADD CPP /nologo /Za /W3 /GX /ZI /Od /X /I "D:\vxworks\VxVmi860\vxVmi\target\h" /I "D:\vxworks\VxVmi860\vxVmi\target\config\all" /I "D:\vxworks\VxVmi860\vxVmi\target\src\config" /I "D:\vxworks\VxVmi860\vxVmi\target\src\drv" /I "\ViperCell" /I "\ViperCell\Cdr\include" /I "\vipercell\cc\include" /I "jcc\include" /I "\Vipercell\oam\include" /D CPU=PPC860 /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "VXWORKS" /D "H323_Exist" /D "__VXWORKS__" /D "VIPER_IP" /D "FAST_RTP_READ" /D "VX_IGNORE_GNU_LIBS" /D "__PROTOTYPE_5_0" /FR /YX /FD /GZ /c
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

# Name "cdr - Win32 Release"
# Name "cdr - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\Alarm\AlarmTool\AlarmCode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\cdr\src_client\CdrApi.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\cdr\src_client\CdrBase.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CdrInit.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\cdr\src_client\CdrMain.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\cdr\src_client\CdrModule.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\cdr\src\CdrRef.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\cdr\src_client\CdrRefSem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\cdr\src_client\CdrRemClientSocket.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\cdr\src_client\CdrSystem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\socket\src_dbg\debug.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\Common_To_Host\include\AlarmCode.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\cdr\include\CdrBase.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\cdr\include\CdrCommon.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\include\CdrDbg.h
# End Source File
# Begin Source File

SOURCE=.\include\cdrdefs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\cdr\include\CdrInterface.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\cdr\include\CdrModule.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\cdr\include\CdrRef.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\cdr\include\CdrRefSem.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\cdr\include\CdrRemClientSocket.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\cdr\include\CdrShellIntf.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\include\cdr\CdrSSA.h
# End Source File
# Begin Source File

SOURCE=..\Common_To_Host\cdr\CdrVoiceCall.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\include\dbgfunc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\include\defs.h
# End Source File
# Begin Source File

SOURCE=..\Common_To_Host\include\mibtags.h
# End Source File
# Begin Source File

SOURCE=..\Common_To_Host\include\MnetModuleId.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\cdr\include\msgProc.h
# End Source File
# Begin Source File

SOURCE=.\include\StdAfx.h
# End Source File
# End Group
# Begin Group "Source API files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\Common\cdr\src_client\CdrSSA.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CdrVoiceCall.cpp
# End Source File
# End Group
# Begin Group "Source Init Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# End Group
# Begin Group "Socket Files"

# PROP Default_Filter ""
# Begin Group "Stub"

# PROP Default_Filter ""
# End Group
# Begin Source File

SOURCE=..\..\..\Common\socket\include\CallBackSSL.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\socket\src\GenClientSocket.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\include\socket\GenClientSocket.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\socket\src\GenSocket.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\include\socket\GenSocket.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\socket\src\GenSocketSSL.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\include\socket\GenSocketSSL.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\socket\src_ssl\GenSocketSSL_lib.cpp
# End Source File
# Begin Source File

SOURCE=..\Common_To_Host\include\socdefs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\socket\src_ssl\ssl_intf.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\include\socket\SslIntf.h
# End Source File
# End Group
# End Target
# End Project
