#ifndef __CDRSHELLINTF_H__
#define __CDRSHELLINTF_H__
// *******************************************************************
//
// (c) Copyright Cisco Systems Inc, 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : CdrShellIntf.h
// Author(s)   : Igal Gutkin
// Create Date : 11/21/00
// Description : CDR module shell interface definitions
//
// *******************************************************************

#ifdef __cplusplus
 extern "C" 
 {
    void CdrDataPrint        ();
    void CdrEnableExtraInfo  ();
    void CdrDisableExtraInfo ();
    int  SavePwdCdr          (LPCSTR lpPasswd);
    void _CdrSendDebug       ();
 }
#else
    void CdrDataPrint        (void);
    void CdrEnableExtraInfo  (void);
    void CdrDisableExtraInfo (void);
    int  SavePwdCdr          (LPCSTR lpPasswd);
    void _CdrSendDebug       (void);
#endif //__cplusplus


#endif //__CDRSHELLINTF_H__