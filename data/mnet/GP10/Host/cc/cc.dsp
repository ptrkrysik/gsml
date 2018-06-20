# Microsoft Developer Studio Project File - Name="cc" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=cc - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "cc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "cc.mak" CFG="cc - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "cc - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "cc - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "cc"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "cc - Win32 Release"

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

!ELSEIF  "$(CFG)" == "cc - Win32 Debug"

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
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "%WIND_BASE%\target\config\all" /I "%WIND_BASE%\target\src\config" /I "%WIND_BASE%\target\src\drv" /I "\ViperCell" /I "\vipercell\cc\include" /I "jcc\include" /I "cdc_bsp" /I "oam\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D CPU=PPC860 /D "VXWORKS" /D "H323_Exist" /D "__VXWORKS__" /D "VIPER_IP" /D "FAST_RTP_READ" /D "VX_IGNORE_GNU_LIBS" /D "__PROTOTYPE_5_0" /FR /YX /FD /GZ /c
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

# Name "cc - Win32 Release"
# Name "cc - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\CallConfig.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CallLeg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CallTask.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CCBldMSMsg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CCconfig.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CCH323Util.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CCHalfCall.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CCHandover.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CCHndUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CCHoSrcHnd.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CCHoTrgHnd.cpp
# End Source File
# Begin Source File

SOURCE=.\src_init\CCinit.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CCMobExtHnd.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CCMobIntHnd.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CCMobRemHnd.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CCMsgAnal.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CCOamHnd.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CCSessionHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CCTask.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CCUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CissCallLeg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\hortp.cpp
# End Source File
# Begin Source File

SOURCE=.\src\smqueue.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SmsCommon.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SmsFsm.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SmsHalfCall.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SmsHandlers.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SmsLeg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SmsMsgBuilder.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\Common_To_Host\cc\CallConfig.h
# End Source File
# Begin Source File

SOURCE=..\Common_To_Host\cc\CallLeg.h
# End Source File
# Begin Source File

SOURCE=..\Common_To_Host\cc\cc_util_ext.h
# End Source File
# Begin Source File

SOURCE=..\Common_To_Host\cc\CCconfig.h
# End Source File
# Begin Source File

SOURCE=..\Common_To_Host\cc\CCH323Util.h
# End Source File
# Begin Source File

SOURCE=..\Common_To_Host\cc\CCHalfCall.h
# End Source File
# Begin Source File

SOURCE=..\Common_To_Host\cc\CCHandover.h
# End Source File
# Begin Source File

SOURCE=..\Common_To_Host\cc\CCInt.h
# End Source File
# Begin Source File

SOURCE=..\Common_To_Host\cc\CCMsgAnal.h
# End Source File
# Begin Source File

SOURCE=..\Common_To_Host\cc\CCSessionHandler.h
# End Source File
# Begin Source File

SOURCE=..\Common_To_Host\cc\CCTypes.h
# End Source File
# Begin Source File

SOURCE=..\Common_To_Host\cc\CCUtil.h
# End Source File
# Begin Source File

SOURCE=..\Common_To_Host\cc\hortp.h
# End Source File
# Begin Source File

SOURCE=..\Common_To_Host\jcc\JCCComMsg.h
# End Source File
# Begin Source File

SOURCE=..\Common_To_Host\jcc\JCCL3Msg.h
# End Source File
# Begin Source File

SOURCE=..\Common_To_Host\cc\smqueue.h
# End Source File
# Begin Source File

SOURCE=..\Common_To_Host\cc\SmsCommonDef.h
# End Source File
# Begin Source File

SOURCE=..\Common_To_Host\cc\SmsHalfCall.h
# End Source File
# Begin Source File

SOURCE=..\Common_To_Host\cc\SmsHandler.h
# End Source File
# Begin Source File

SOURCE=..\Common_To_Host\cc\SMSLeg.h
# End Source File
# Begin Source File

SOURCE=..\Common_To_Host\cc\SmsMsgBuilder.h
# End Source File
# End Group
# Begin Group "Source API files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src_api\CCapiH323.cpp
# End Source File
# Begin Source File

SOURCE=.\src_api\CCapiViperLink.cpp
# End Source File
# Begin Source File

SOURCE=.\src_api\CCMobUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\src_api\ccutil2.cpp
# End Source File
# Begin Source File

SOURCE=.\src_api\SmsVblink.cpp
# End Source File
# End Group
# Begin Group "Docs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\encode cc.txt"

!IF  "$(CFG)" == "cc - Win32 Release"

!ELSEIF  "$(CFG)" == "cc - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=".\procs sms.txt"

!IF  "$(CFG)" == "cc - Win32 Release"

!ELSEIF  "$(CFG)" == "cc - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "Logging Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\logging\src\l3MsLog.cpp
# End Source File
# Begin Source File

SOURCE=..\logging\src\logging.cpp
# End Source File
# Begin Source File

SOURCE=..\logging\include\VCLOGGING.H
# End Source File
# Begin Source File

SOURCE=..\logging\include\vcmodules.h
# End Source File
# End Group
# Begin Group "L1Proxy Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\l1proxy\src\l1MsMsgProc.cpp
# End Source File
# End Group
# Begin Group "RM API Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;h"
# Begin Source File

SOURCE=..\rm\Include\rm_ccintf.h
# End Source File
# Begin Source File

SOURCE=..\rm\Src_api\rm_EntryIdVsTrxSlot.cpp
# End Source File
# End Group
# Begin Group "MM source files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;h"
# Begin Source File

SOURCE=..\mm\src\MMMdMsgProc.cpp
# End Source File
# End Group
# Begin Group "LUDB Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;h"
# Begin Source File

SOURCE=..\jcc\src\LUDBVoipMsgProc.cpp
# End Source File
# End Group
# End Target
# End Project
