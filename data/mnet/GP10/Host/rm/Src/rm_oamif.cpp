/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
    #ifndef __RM_OAMIF_CPP__
    #define __RM_OAMIF_CPP__
    
    #include "rm\rm_head.h"
    
    //MODNEED
    //rm_OamData_t rm_OamData; moved to sys_init.cpp
    //JcVipercellId_t  oam_AdjacentCellId[32]=
    //                 { {0,1},{0,2},
    //			 {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
    //			 {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
    //			 {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}
    //		     }; //Retrieve in terms of index co-referred by both DSP & RRM
    
    u8 oam_CellReselectOffset=0;
    u8 oam_TemporaryOffset=0;
    u8 oam_PenaltyTime=0;
  
    u8  oam_gsmCodec    = 1; //0x01-FR, 0x21-EFR, 0x41-superEFR 
    u8  oam_BcchTrx=0;
    u16 oam_TrxTuneSynthSetting[2][2];
    
    //AlarmAdded
    int rm_OpState;
    int rm_NoOfUsableTchf;
    int rm_nbrOfAvailableSDCCHs;  
	int rm_nbrOfAvailableSDCCH8s;
    #define TOTAL_AVAIL_SDCCHs     rm_nbrOfAvailableSDCCHs + rm_nbrOfAvailableSDCCH8s
    int rm_maxNbrOfBusyTCHs;
    int rm_maxNbrOfBusySDCCHs;
	int rm_maxNbrOfBusySDCCH8s;
    #define TOTAL_MAX_BUSY_SDCCHs  rm_maxNbrOfBusySDCCHs + rm_maxNbrOfBusySDCCH8s
    rm_TrxMgmt_t rm_TrxMgmt[OAMrm_MAX_TRXS];
    int rm_LiveSglCfg=RM_FALSE;
 
    //AMADDED
    T_CNI_RIL3_CELL_BARRED_ACCESS rm_CellBarred;
    AdministrativeState rm_AmState00;
    int rm_VcIsBarred;

    //PMADDED
    int rm_NoOfBusySdcch4;
	int rm_NoOfBusySdcch8;
    #define NUM_BUSY_SDCCHs     rm_NoOfBusySdcch4 + rm_NoOfBusySdcch8
    int rm_NoOfBusyTchf;
    int rm_NoOfIdleTchf;
    rm_AllChanAvailable_t rm_AllAvailableTchf;  
    rm_AllChanAvailable_t rm_AllAvailableSdcch4;
	rm_AllChanAvailable_t rm_AllAvailableSdcch8;
  
    //SMSCBAdded
    u16 oam_phTrxSelectBit[2]= {0x0000,0x4000};  	    //b14
    u16 oam_phTrxSynthSelectBit[2] = {0x0000,0x8000};     //b15
    u16 oam_phTrxHoppingSelectBit[2] = {0x0000,0x2000};   //b13
    u16 oam_phTrxPwrSelectBit[2] = {0x0000,0x1000};       //b12
    
    #endif //__RM_OAMIF_CPP__
