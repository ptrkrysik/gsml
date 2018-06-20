#ifndef __HTTP_H__
#define __HTTP_H__

#define kMaxStringLength  64
#define kQSEventString    "Event Number "
#define kQSMchResponseString "You will see the response of your command in this window\n"
#define kQSEventStringLen (STRLEN(kQSEventString))

typedef struct environment Environment;

extern void
http_registerPostHandler(sbyte       *pDocName, 
                            RLSTATUS    (*pHandler)(struct environment *pEnv, 
                                                    sbyte       *pPostData, 
                                                    unsigned int totalBytesRead));
extern void	http_feedbackInit();
extern void	http_oamOperationInit();

void http_msgPrint(char *format, ...);
#ifdef	_DEBUG
#define http_dbgMsg(XX) fflush(stdout); printf("[HTTPD] ");  http_msgPrint XX
#else
#define http_dbgMsg(XX)
#endif

extern char Http_serverRoot[];

/*-------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------------------

typedef void (StrGetFuncPtr) (struct environment *, void *, void *, sbyte *);

extern StrGetFuncPtr http_vbGetViperCells;
extern StrGetFuncPtr http_vbGetIpAddress;
extern StrGetFuncPtr getHelpTopic;
extern StrGetFuncPtr getHelpDetail;

extern StrGetFuncPtr getAlarmList;
extern StrGetFuncPtr getAlarmDetail;
extern StrGetFuncPtr viperCellTime;

---------------------------------------------------------------------------------------------------*/

extern void http_vbGetViperCells(struct environment *, void *, void *, sbyte *);
extern void http_vbGetIpAddress(struct environment *, void *, void *, sbyte *);
extern void getHelpTopic(struct environment *, void *, void *, sbyte *);
extern void getHelpDetail(struct environment *, void *, void *, sbyte *);
extern void getAlarmList(struct environment *, void *, void *, sbyte *);
extern void getAlarmDetail(struct environment *, void *, void *, sbyte *);
extern void viperCellTime(struct environment *, void *, void *, sbyte *);
extern void getCdcPowerSupplyStatus(struct environment *, void *, void *, sbyte *);
extern void getCdcTemp(struct environment *, void *, void *, sbyte *);
extern void getIpAddress(struct environment *, void *, void *, sbyte *);
extern RLSTATUS setIpAddress(struct environment *envVar, void *pDest, void *pString);

/* User Mgmt */
extern sbyte4 
UserMgmtCmdSet(struct environment *envVar, void *pInputBuffer, void *pSource, sbyte *pArgs);
/* Smscb Command Get*/ 
extern void 
UserMgmtCmdGet(struct environment *envVar, void *pHtmlOutputBuf, void *pSource, sbyte *pArgs);

/* SSL password */
extern  void 
http_vbGetSslPassword(struct environment *envVar, void *pHtmlOutputBuf, void *pSource, sbyte *pArgs);
extern sbyte4 
http_vbSetSslPassword(struct environment *envVar, void *pInputBuffer, void *pSource, sbyte *pArgs);

/* SSL capability */
extern void
http_vbGetSslCapability(struct environment *envVar, void *pHtmlOutputBuf, void *pSource, sbyte *pArgs);

/* CLI status and session termination */
extern  void 
http_vbGetCliStatus(struct environment *envVar, void *pHtmlOutputBuf, void *pSource, sbyte *pArgs);
extern  void 
http_vbTerminateCliSession(struct environment *envVar, void *pHtmlOutputBuf, void *pSource, sbyte *pArgs);

/* The followings routines are defined  in cdcUtil/src/CPU_Mon.c */
extern float DS1780_get1_8_V();
extern float DS1780_get3_3_V();
extern float DS1780_get5_0_V();
extern int DS1780_getCdcTemp();

/* yichoi for reload top_frame to display changed user */
extern  void 
reload_topframeGet(struct environment *envVar, void *pHtmlOutputBuf, void *pSource, sbyte *pArgs);

#endif

