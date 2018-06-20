/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/****************************************************************************
* File: grouptbl.c
*
* Description:   
*
*****************************************************************************/
#include "stdlib.h"
#include "gsmdata.h"
#include "bbdata.h"
#include "dsprotyp.h"
            
void group0TableInit(void)
{
	dlGroupTable0[0] =  (Uint32 ) & dlIdleProc;
	/*dlGroupTable0[0] =  (Uint32 ) & dlFcchProc;	*/
	ulGroupTable0[0] =  (Uint32 ) & ulIdleProc;
}	
	     
void group4TableInit(void)
{
	ulGroupTableIV[0] =  (Uint32 ) & ulRachProc;
}	
	     
void ulGroupITableInit(void)
{                           
	Uint32 *ptr;
	ptr = ulGroupTableI;
	*ptr++ = (Uint32 ) & ulTchProcBurst_0;
	*ptr++ = (Uint32 ) & ulTchProcBurst_0;
	*ptr++ = (Uint32 ) & ulTchProcBurst_0;
	*ptr++ = (Uint32 ) & ulTchProcBurst_3;
	*ptr++ = (Uint32 ) & ulTchProcBurst_0;
	*ptr++ = (Uint32 ) & ulTchProcBurst_0;
	*ptr++ = (Uint32 ) & ulTchProcBurst_0;
	*ptr++ = (Uint32 ) & ulTchProcBurst_3;
	*ptr++ = (Uint32 ) & ulTchProcBurst_0;
	*ptr++ = (Uint32 ) & ulTchProcBurst_0;
	*ptr++ = (Uint32 ) & ulTchProcBurst_0;
	*ptr++ = (Uint32 ) & ulTchProcBurst_3;

	*ptr++ = (Uint32 ) & ulSachIdleProc; 
								
	*ptr++ = (Uint32 ) & ulTchProcBurst_0;
	*ptr++ = (Uint32 ) & ulTchProcBurst_0;
	*ptr++ = (Uint32 ) & ulTchProcBurst_0;
	*ptr++ = (Uint32 ) & ulTchProcBurst_3;
	*ptr++ = (Uint32 ) & ulTchProcBurst_0;
	*ptr++ = (Uint32 ) & ulTchProcBurst_0;
	*ptr++ = (Uint32 ) & ulTchProcBurst_0;
	*ptr++ = (Uint32 ) & ulTchProcBurst_3;
	*ptr++ = (Uint32 ) & ulTchProcBurst_0;
	*ptr++ = (Uint32 ) & ulTchProcBurst_0;
	*ptr++ = (Uint32 ) & ulTchProcBurst_0;
	*ptr++ = (Uint32 ) & ulTchProcBurst_3;

	*ptr++ = (Uint32 ) & ulSachIdleProc;
}

void dlGroupITableInit(void)
{                           
	Uint32 *ptr;
	ptr = dlGroupTableI;
	*ptr++ = (Uint32 ) & dlTchProcBurst_0;
	*ptr++ = (Uint32 ) & dlTchProcBurst_3;
	*ptr++ = (Uint32 ) & dlTchProcBurst_3;
	*ptr++ = (Uint32 ) & dlTchProcBurst_3;
	*ptr++ = (Uint32 ) & dlTchProcBurst_0;
	*ptr++ = (Uint32 ) & dlTchProcBurst_3;
	*ptr++ = (Uint32 ) & dlTchProcBurst_3;
	*ptr++ = (Uint32 ) & dlTchProcBurst_3;
	*ptr++ = (Uint32 ) & dlTchProcBurst_0;
	*ptr++ = (Uint32 ) & dlTchProcBurst_3;
	*ptr++ = (Uint32 ) & dlTchProcBurst_3;
	*ptr++ = (Uint32 ) & dlTchProcBurst_3;

	*ptr++ = (Uint32 ) & dlSachIdleProc; 
								
	*ptr++ = (Uint32 ) & dlTchProcBurst_0;
	*ptr++ = (Uint32 ) & dlTchProcBurst_3;
	*ptr++ = (Uint32 ) & dlTchProcBurst_3;
	*ptr++ = (Uint32 ) & dlTchProcBurst_3;
	*ptr++ = (Uint32 ) & dlTchProcBurst_0;
	*ptr++ = (Uint32 ) & dlTchProcBurst_3;
	*ptr++ = (Uint32 ) & dlTchProcBurst_3;
	*ptr++ = (Uint32 ) & dlTchProcBurst_3;
	*ptr++ = (Uint32 ) & dlTchProcBurst_0;
	*ptr++ = (Uint32 ) & dlTchProcBurst_3;
	*ptr++ = (Uint32 ) & dlTchProcBurst_3;
	*ptr++ = (Uint32 ) & dlTchProcBurst_3;

	*ptr++ = (Uint32 ) & dlSachIdleProc;
}

void dlGroupVTableInit(void)
{
	Uint32 *ptr;
	ptr = dlGroupTableV;
	*ptr++ = (Uint32 ) & dlFcchProc;
	*ptr++ = (Uint32 ) & dlSchProc;

	*ptr++ = (Uint32 ) & dlBcchProc0;
	*ptr++ = (Uint32 ) & dlBcchProc1;
	*ptr++ = (Uint32 ) & dlBcchProc2;
	*ptr++ = (Uint32 ) & dlBcchProc3;

	*ptr++ = (Uint32 ) & dlPagchProc0;
	*ptr++ = (Uint32 ) & dlPagchProc1;
	*ptr++ = (Uint32 ) & dlPagchProc2;
	*ptr++ = (Uint32 ) & dlPagchProc3;

 	*ptr++ = (Uint32 ) & dlFcchProc;
	*ptr++ = (Uint32 ) & dlSchProc;
						
	*ptr++ = (Uint32 ) & dlPagchProc0;
	*ptr++ = (Uint32 ) & dlPagchProc1;
	*ptr++ = (Uint32 ) & dlPagchProc2;
	*ptr++ = (Uint32 ) & dlPagchProc3;

	*ptr++ = (Uint32 ) & dlPagchProc0;
	*ptr++ = (Uint32 ) & dlPagchProc1;
	*ptr++ = (Uint32 ) & dlPagchProc2;
	*ptr++ = (Uint32 ) & dlPagchProc3;

 	*ptr++ = (Uint32 ) & dlFcchProc;
	*ptr++ = (Uint32 ) & dlSchProc;

	*ptr++ = (Uint32 ) & dlSdcchProc0;
	*ptr++ = (Uint32 ) & dlSdcchProc3;
	*ptr++ = (Uint32 ) & dlSdcchProc3;
	*ptr++ = (Uint32 ) & dlSdcchProc3;

	*ptr++ = (Uint32 ) & dlSdcchProc0;
	*ptr++ = (Uint32 ) & dlSdcchProc3;
	*ptr++ = (Uint32 ) & dlSdcchProc3;
	*ptr++ = (Uint32 ) & dlSdcchProc3;

 	*ptr++ = (Uint32 ) & dlFcchProc;
	*ptr++ = (Uint32 ) & dlSchProc;

	*ptr++ = (Uint32 ) & dlSdcchProc0;
	*ptr++ = (Uint32 ) & dlSdcchProc3;
	*ptr++ = (Uint32 ) & dlSdcchProc3;
	*ptr++ = (Uint32 ) & dlSdcchProc3;

	*ptr++ = (Uint32 ) & dlSdcchProc0;
	*ptr++ = (Uint32 ) & dlSdcchProc3;
	*ptr++ = (Uint32 ) & dlSdcchProc3;
	*ptr++ = (Uint32 ) & dlSdcchProc3;

 	*ptr++ = (Uint32 ) & dlFcchProc;
	*ptr++ = (Uint32 ) & dlSchProc;

 	*ptr++ = (Uint32 ) & dlSacchProc0;
 	*ptr++ = (Uint32 ) & dlSacchProc1;
 	*ptr++ = (Uint32 ) & dlSacchProc1;
 	*ptr++ = (Uint32 ) & dlSacchProc3;

 	*ptr++ = (Uint32 ) & dlSacchProc0;
 	*ptr++ = (Uint32 ) & dlSacchProc1;
 	*ptr++ = (Uint32 ) & dlSacchProc1;
 	*ptr++ = (Uint32 ) & dlSacchProc3;

	*ptr++ = (Uint32 ) & dlIdleProc;								
}

void ulGroupVTableInit(void)
{
	ulGroupTableV[0] = (Uint32 ) & ulSdcchProc0;
	ulGroupTableV[1] = (Uint32 ) & ulSdcchProc0;			
	ulGroupTableV[2] = (Uint32 ) & ulSdcchProc0;
	ulGroupTableV[3] = (Uint32 ) & ulSdcchProc3;

	ulGroupTableV[4] = (Uint32 ) & ulRachProc;
	ulGroupTableV[5] = (Uint32 ) & ulRachProc;

	ulGroupTableV[6] = (Uint32 ) & ulSacchProc0;
	ulGroupTableV[7] = (Uint32 ) & ulSacchProc0;
	ulGroupTableV[8] = (Uint32 ) & ulSacchProc0;
	ulGroupTableV[9] = (Uint32 ) & ulSacchProc3;

	ulGroupTableV[10] = (Uint32 ) & ulSacchProc0;
	ulGroupTableV[11] = (Uint32 ) & ulSacchProc0;
	ulGroupTableV[12] = (Uint32 ) & ulSacchProc0;
	ulGroupTableV[13] = (Uint32 ) & ulSacchProc3;

	ulGroupTableV[14] = (Uint32 ) & ulRachProc;
	ulGroupTableV[15] = (Uint32 ) & ulRachProc;
	ulGroupTableV[16] = (Uint32 ) & ulRachProc;
	ulGroupTableV[17] = (Uint32 ) & ulRachProc;
	ulGroupTableV[18] = (Uint32 ) & ulRachProc;
	ulGroupTableV[19] = (Uint32 ) & ulRachProc;
	ulGroupTableV[20] = (Uint32 ) & ulRachProc;
	ulGroupTableV[21] = (Uint32 ) & ulRachProc;
	ulGroupTableV[22] = (Uint32 ) & ulRachProc;
	ulGroupTableV[23] = (Uint32 ) & ulRachProc;
	ulGroupTableV[24] = (Uint32 ) & ulRachProc;
	ulGroupTableV[25] = (Uint32 ) & ulRachProc;
	ulGroupTableV[26] = (Uint32 ) & ulRachProc;
	ulGroupTableV[27] = (Uint32 ) & ulRachProc;
	ulGroupTableV[28] = (Uint32 ) & ulRachProc;
	ulGroupTableV[29] = (Uint32 ) & ulRachProc;
	ulGroupTableV[30] = (Uint32 ) & ulRachProc;
	ulGroupTableV[31] = (Uint32 ) & ulRachProc;
	ulGroupTableV[32] = (Uint32 ) & ulRachProc;
	ulGroupTableV[33] = (Uint32 ) & ulRachProc;
	ulGroupTableV[34] = (Uint32 ) & ulRachProc;
	ulGroupTableV[35] = (Uint32 ) & ulRachProc;
	ulGroupTableV[36] = (Uint32 ) & ulRachProc;

	ulGroupTableV[37] = (Uint32 ) & ulSdcchProc0;
	ulGroupTableV[38] = (Uint32 ) & ulSdcchProc0;			
	ulGroupTableV[39] = (Uint32 ) & ulSdcchProc0;
	ulGroupTableV[40] = (Uint32 ) & ulSdcchProc3;

	ulGroupTableV[41] = (Uint32 ) & ulSdcchProc0;
	ulGroupTableV[42] = (Uint32 ) & ulSdcchProc0;			
	ulGroupTableV[43] = (Uint32 ) & ulSdcchProc0;
	ulGroupTableV[44] = (Uint32 ) & ulSdcchProc3;

	ulGroupTableV[45] = (Uint32 ) & ulRachProc;
	ulGroupTableV[46] = (Uint32 ) & ulRachProc;

	ulGroupTableV[47] = (Uint32 ) & ulSdcchProc0;
	ulGroupTableV[48] = (Uint32 ) & ulSdcchProc0;
	ulGroupTableV[49] = (Uint32 ) & ulSdcchProc0;
	ulGroupTableV[50] = (Uint32 ) & ulSdcchProc3;
}


void dlGroupVIITableInit(void)
{
	Uint32 *ptr;
	ptr = dlGroupTableVII;

	*ptr++ = (Uint32 ) & dlSdcchProc0;
	*ptr++ = (Uint32 ) & dlSdcchProc3;
	*ptr++ = (Uint32 ) & dlSdcchProc3;
	*ptr++ = (Uint32 ) & dlSdcchProc3;


	*ptr++ = (Uint32 ) & dlSdcchProc0;
	*ptr++ = (Uint32 ) & dlSdcchProc3;
	*ptr++ = (Uint32 ) & dlSdcchProc3;
	*ptr++ = (Uint32 ) & dlSdcchProc3;

	*ptr++ = (Uint32 ) & dlSdcchProc0;
	*ptr++ = (Uint32 ) & dlSdcchProc3;
	*ptr++ = (Uint32 ) & dlSdcchProc3;
	*ptr++ = (Uint32 ) & dlSdcchProc3;

	*ptr++ = (Uint32 ) & dlSdcchProc0;
	*ptr++ = (Uint32 ) & dlSdcchProc3;
	*ptr++ = (Uint32 ) & dlSdcchProc3;
	*ptr++ = (Uint32 ) & dlSdcchProc3;

	*ptr++ = (Uint32 ) & dlSdcchProc0;
	*ptr++ = (Uint32 ) & dlSdcchProc3;
	*ptr++ = (Uint32 ) & dlSdcchProc3;
	*ptr++ = (Uint32 ) & dlSdcchProc3;

	*ptr++ = (Uint32 ) & dlSdcchProc0;
	*ptr++ = (Uint32 ) & dlSdcchProc3;
	*ptr++ = (Uint32 ) & dlSdcchProc3;
	*ptr++ = (Uint32 ) & dlSdcchProc3;

	*ptr++ = (Uint32 ) & dlSdcchProc0;
	*ptr++ = (Uint32 ) & dlSdcchProc3;
	*ptr++ = (Uint32 ) & dlSdcchProc3;
	*ptr++ = (Uint32 ) & dlSdcchProc3;

	*ptr++ = (Uint32 ) & dlSdcchProc0;
	*ptr++ = (Uint32 ) & dlSdcchProc3;
	*ptr++ = (Uint32 ) & dlSdcchProc3;
	*ptr++ = (Uint32 ) & dlSdcchProc3;


	*ptr++ = (Uint32 ) & dlSacchProc0;
	*ptr++ = (Uint32 ) & dlSacchProc1;
	*ptr++ = (Uint32 ) & dlSacchProc1;
	*ptr++ = (Uint32 ) & dlSacchProc3;

	*ptr++ = (Uint32 ) & dlSacchProc0;
	*ptr++ = (Uint32 ) & dlSacchProc1;
	*ptr++ = (Uint32 ) & dlSacchProc1;
	*ptr++ = (Uint32 ) & dlSacchProc3;

	
	*ptr++ = (Uint32 ) & dlSacchProc0;
	*ptr++ = (Uint32 ) & dlSacchProc1;
	*ptr++ = (Uint32 ) & dlSacchProc1;
	*ptr++ = (Uint32 ) & dlSacchProc3;

	*ptr++ = (Uint32 ) & dlSacchProc0;
	*ptr++ = (Uint32 ) & dlSacchProc1;
	*ptr++ = (Uint32 ) & dlSacchProc1;
	*ptr++ = (Uint32 ) & dlSacchProc3;




	*ptr++ = (Uint32 ) & dlIdleProc;		
	*ptr++ = (Uint32 ) & dlIdleProc;
	*ptr++ = (Uint32 ) & dlIdleProc;
}

void ulGroupVIITableInit(void)
{
	ulGroupTableVII[0] = (Uint32 ) & ulSacchProc0;
	ulGroupTableVII[1] = (Uint32 ) & ulSacchProc0;			
	ulGroupTableVII[2] = (Uint32 ) & ulSacchProc0;
	ulGroupTableVII[3] = (Uint32 ) & ulSacchProc3;

	ulGroupTableVII[4] = (Uint32 ) & ulSacchProc0;
	ulGroupTableVII[5] = (Uint32 ) & ulSacchProc0;
	ulGroupTableVII[6] = (Uint32 ) & ulSacchProc0;
	ulGroupTableVII[7] = (Uint32 ) & ulSacchProc3;

	ulGroupTableVII[8] = (Uint32 ) &  ulSacchProc0;
    	ulGroupTableVII[9] = (Uint32 ) &  ulSacchProc0;
	ulGroupTableVII[10] = (Uint32 ) & ulSacchProc0;
	ulGroupTableVII[11] = (Uint32 ) & ulSacchProc3;


	ulGroupTableVII[12] = (Uint32 ) & ulIdleProc;
	ulGroupTableVII[13] = (Uint32 ) & ulIdleProc;
	ulGroupTableVII[14] = (Uint32 ) & ulIdleProc;

	ulGroupTableVII[15] = (Uint32 ) & ulSdcchProc0;
	ulGroupTableVII[16] = (Uint32 ) & ulSdcchProc0;
	ulGroupTableVII[17] = (Uint32 ) & ulSdcchProc0;
	ulGroupTableVII[18] = (Uint32 ) & ulSdcchProc3;

	ulGroupTableVII[19] = (Uint32 ) & ulSdcchProc0;
	ulGroupTableVII[20] = (Uint32 ) & ulSdcchProc0;
	ulGroupTableVII[21] = (Uint32 ) & ulSdcchProc0;
	ulGroupTableVII[22] = (Uint32 ) & ulSdcchProc3;

	ulGroupTableVII[23] = (Uint32 ) & ulSdcchProc0;
	ulGroupTableVII[24] = (Uint32 ) & ulSdcchProc0;
	ulGroupTableVII[25] = (Uint32 ) & ulSdcchProc0;
	ulGroupTableVII[26] = (Uint32 ) & ulSdcchProc3;

	ulGroupTableVII[27] = (Uint32 ) & ulSdcchProc0;
	ulGroupTableVII[28] = (Uint32 ) & ulSdcchProc0;
	ulGroupTableVII[29] = (Uint32 ) & ulSdcchProc0;
	ulGroupTableVII[30] = (Uint32 ) & ulSdcchProc3;

	ulGroupTableVII[31] = (Uint32 ) & ulSdcchProc0;
	ulGroupTableVII[32] = (Uint32 ) & ulSdcchProc0;
	ulGroupTableVII[33] = (Uint32 ) & ulSdcchProc0;
	ulGroupTableVII[34] = (Uint32 ) & ulSdcchProc3;

	ulGroupTableVII[35] = (Uint32 ) & ulSdcchProc0;
	ulGroupTableVII[36] = (Uint32 ) & ulSdcchProc0;			
	ulGroupTableVII[37] = (Uint32 ) & ulSdcchProc0;
	ulGroupTableVII[38] = (Uint32 ) & ulSdcchProc3;

	ulGroupTableVII[39] = (Uint32 ) & ulSdcchProc0;
	ulGroupTableVII[40] = (Uint32 ) & ulSdcchProc0;			
	ulGroupTableVII[41] = (Uint32 ) & ulSdcchProc0;
	ulGroupTableVII[42] = (Uint32 ) & ulSdcchProc3;

	ulGroupTableVII[43] = (Uint32 ) & ulSdcchProc0;
	ulGroupTableVII[44] = (Uint32 ) & ulSdcchProc0;
	ulGroupTableVII[45] = (Uint32 ) & ulSdcchProc0;
	ulGroupTableVII[46] = (Uint32 ) & ulSdcchProc3;

	ulGroupTableVII[47] = (Uint32 ) & ulSacchProc0;
	ulGroupTableVII[48] = (Uint32 ) & ulSacchProc0;
	ulGroupTableVII[49] = (Uint32 ) & ulSacchProc0;
	ulGroupTableVII[50] = (Uint32 ) & ulSacchProc3;

}

void sacchIdleTableInit(void)
{
 	dlSacchIdleTbl[0] = (Uint32 ) & dlSacchProc0;
	dlSacchIdleTbl[1] = (Uint32 ) & dlIdleProc;	

	dlSacchIdleTbl[2] = (Uint32 ) & dlSacchProc1;
	dlSacchIdleTbl[3] = (Uint32 ) & dlIdleProc;	

	dlSacchIdleTbl[4] = (Uint32 ) & dlSacchProc1;
	dlSacchIdleTbl[5] = (Uint32 ) & dlIdleProc;	

	dlSacchIdleTbl[6] = (Uint32 ) & dlSacchProc3;
	dlSacchIdleTbl[7] = (Uint32 ) & dlIdleProc;	
                                                
	/*
	* init uplink sacch idle function table
	*/                                                
	ulSacchIdleTbl[0] = (Uint32 ) & ulSacchProc0;
	ulSacchIdleTbl[1] = (Uint32 ) & ulIdleProc;	

	ulSacchIdleTbl[2] = (Uint32 ) & ulSacchProc0;
	ulSacchIdleTbl[3] = (Uint32 ) & ulIdleProc;	

	ulSacchIdleTbl[4] = (Uint32 ) & ulSacchProc0;
	ulSacchIdleTbl[5] = (Uint32 ) & ulIdleProc;	

	ulSacchIdleTbl[6] = (Uint32 ) & ulSacchProc3;
	ulSacchIdleTbl[7] = (Uint32 ) & handOverProc;	
} 

void dlGroupXITableInit(void)
/*---------------------------------------------------
  Function:  dlGroupXITableInit
  Inputs:
             None
  Outputs:
           dlGrouTableXI
  Description:
    dlGroupXITableInit  initializes the 52-multiframe
    downlink table pointer for GroupXI.   
-- ---------------------------------------------------*/    
{
   

        dlGroupTableXI[0] = (Uint32) & dlPbcchProc0;
        dlGroupTableXI[1] = (Uint32) & dlBcchProc1;
        dlGroupTableXI[2] = (Uint32) & dlBcchProc2;
        dlGroupTableXI[3] = (Uint32) & dlBcchProc3;

		
        
        dlGroupTableXI[4] = (Uint32) dlPpchProc0;
        dlGroupTableXI[5] = (Uint32) dlPpchProc1;
        dlGroupTableXI[6] = (Uint32) dlPpchProc2;
        dlGroupTableXI[7] = (Uint32) dlPpchProc3;

        dlGroupTableXI[8]  = (Uint32) dlPpchProc0;
        dlGroupTableXI[9]  = (Uint32) dlPpchProc1;
        dlGroupTableXI[10] = (Uint32) dlPpchProc2;
        dlGroupTableXI[11] = (Uint32) dlPpchProc3;


        dlGroupTableXI[12] = (Uint32) &dlPtcchProc0;
        
        dlGroupTableXI[13] = (Uint32) &dlPdchProc0;
        dlGroupTableXI[14] = (Uint32) &dlPdchProc3;
        dlGroupTableXI[15] = (Uint32) &dlPdchProc3;
        dlGroupTableXI[16] = (Uint32) &dlPdchProc3;
        
        dlGroupTableXI[17] = (Uint32) &dlPdchProc0;
        dlGroupTableXI[18]=  (Uint32) &dlPdchProc3;
        dlGroupTableXI[19] = (Uint32) &dlPdchProc3;
        dlGroupTableXI[20] = (Uint32) &dlPdchProc3;
        
        dlGroupTableXI[21] = (Uint32) &dlPdchProc0;
        dlGroupTableXI[22] = (Uint32) &dlPdchProc3;
        dlGroupTableXI[23] = (Uint32) &dlPdchProc3;
        dlGroupTableXI[24] = (Uint32) &dlPdchProc3;
        
        dlGroupTableXI[25] = (Uint32)&dlPtcchIdleProc;
        
        dlGroupTableXI[26] = (Uint32) &dlPdchProc0;
        dlGroupTableXI[27] = (Uint32) &dlPdchProc3;
        dlGroupTableXI[28] = (Uint32) &dlPdchProc3;
        dlGroupTableXI[29] = (Uint32) &dlPdchProc3;
        
        dlGroupTableXI[30] = (Uint32) &dlPdchProc0;
        dlGroupTableXI[31] = (Uint32) &dlPdchProc3;
        dlGroupTableXI[32] = (Uint32) &dlPdchProc3;
        dlGroupTableXI[33] = (Uint32) &dlPdchProc3;
        

        dlGroupTableXI[34] = (Uint32) &dlPdchProc0;
        dlGroupTableXI[35] = (Uint32) &dlPdchProc3;
        dlGroupTableXI[36] = (Uint32) &dlPdchProc3;
        dlGroupTableXI[37] = (Uint32) &dlPdchProc3;
        
        dlGroupTableXI[38]=  (Uint32)& dlPtcchProc0;;
        
        dlGroupTableXI[39] = (Uint32) &dlPdchProc0;
        dlGroupTableXI[40] = (Uint32) &dlPdchProc3;
        dlGroupTableXI[41] = (Uint32) &dlPdchProc3;
        dlGroupTableXI[42] = (Uint32) &dlPdchProc3;
        
        dlGroupTableXI[43] = (Uint32) &dlPdchProc0;  
        dlGroupTableXI[44] = (Uint32) &dlPdchProc3;
        dlGroupTableXI[45] = (Uint32) &dlPdchProc3;
        dlGroupTableXI[46] = (Uint32) &dlPdchProc3;
        
        dlGroupTableXI[47] = (Uint32) &dlPdchProc0;
        dlGroupTableXI[48] = (Uint32) &dlPdchProc3;
        dlGroupTableXI[49] = (Uint32) &dlPdchProc3;
        dlGroupTableXI[50] = (Uint32) &dlPdchProc3;
        
        dlGroupTableXI[51] = (Uint32) &dlPtcchIdleProc;

  

     
	


}

void ulGroupXITableInit(void)
/*----------------------------------------------------
  Function:  ulGroupXITableInit
  Inputs:
             None
  Outputs:
           ulGrouTableXI
  Description:
    ulGroupXITableInit initializes the 52-multiframe
    uplink table pointer for GroupXI.   
-- ---------------------------------------------------*/    
{

      
        ulGroupTableXI[0] = (Uint32 ) & ulPdchProc0;
        ulGroupTableXI[1] = (Uint32 ) & ulPdchProc0;
        ulGroupTableXI[2] = (Uint32 ) & ulPdchProc0;
        ulGroupTableXI[3] = (Uint32 ) & ulPdchProc3;

        ulGroupTableXI[4] = (Uint32 ) & ulPdchProc0;
        ulGroupTableXI[5] = (Uint32 ) & ulPdchProc0;
        ulGroupTableXI[6] = (Uint32 ) & ulPdchProc0;
        ulGroupTableXI[7] = (Uint32 ) & ulPdchProc3;

        ulGroupTableXI[8] = (Uint32 ) & ulPdchProc0;
        ulGroupTableXI[9] = (Uint32 ) & ulPdchProc0;
        ulGroupTableXI[10] =(Uint32 ) & ulPdchProc0;
        ulGroupTableXI[11] =(Uint32 ) & ulPdchProc3;

        ulGroupTableXI[12] = (Uint32) &ulPtcchProc;
        
        ulGroupTableXI[13] = (Uint32) &ulPdchProc0;
        ulGroupTableXI[14] = (Uint32) &ulPdchProc0;
        ulGroupTableXI[15] = (Uint32) &ulPdchProc0;    
        ulGroupTableXI[16] = (Uint32) &ulPdchProc3;
        
        ulGroupTableXI[17] = (Uint32) &ulPdchProc0;
        ulGroupTableXI[18]=  (Uint32) &ulPdchProc0;
        ulGroupTableXI[19] = (Uint32) &ulPdchProc0;
        ulGroupTableXI[20] = (Uint32) &ulPdchProc3;
        
        ulGroupTableXI[21] = (Uint32) &ulPdchProc0;
        ulGroupTableXI[22] = (Uint32) &ulPdchProc0;
        ulGroupTableXI[23] = (Uint32) &ulPdchProc0;     
        ulGroupTableXI[24] = (Uint32) &ulPdchProc3;
        
        ulGroupTableXI[25] = (Uint32) & ulPtcchIdleProc;;
        
        ulGroupTableXI[26] = (Uint32) &ulPdchProc0;
        ulGroupTableXI[27] = (Uint32) &ulPdchProc0;
        ulGroupTableXI[28] = (Uint32) &ulPdchProc0;
        ulGroupTableXI[29] = (Uint32) &ulPdchProc3;
        
        
        ulGroupTableXI[30] = (Uint32) &ulPdchProc0;
        ulGroupTableXI[31] = (Uint32) &ulPdchProc0; 
        ulGroupTableXI[32] = (Uint32) &ulPdchProc0;
        ulGroupTableXI[33] = (Uint32) &ulPdchProc3;
        
        ulGroupTableXI[34] = (Uint32) &ulPdchProc0;
        ulGroupTableXI[35] = (Uint32) &ulPdchProc0;
        ulGroupTableXI[36] = (Uint32) &ulPdchProc0;
        ulGroupTableXI[37] = (Uint32) &ulPdchProc3;
        
        ulGroupTableXI[38]=  (Uint32) &ulPtcchProc;
        ulGroupTableXI[39] = (Uint32 ) & ulPdchProc0;
        ulGroupTableXI[40] = (Uint32 ) & ulPdchProc0;
        ulGroupTableXI[41] = (Uint32 ) & ulPdchProc0;
        ulGroupTableXI[42] = (Uint32 ) & ulPdchProc3;
        
        ulGroupTableXI[43] = (Uint32 ) & ulPdchProc0;
        ulGroupTableXI[44] = (Uint32 ) & ulPdchProc0;
        ulGroupTableXI[45] = (Uint32 ) & ulPdchProc0;
        ulGroupTableXI[46] = (Uint32 ) & ulPdchProc3;
        
        ulGroupTableXI[47] = (Uint32 ) & ulPdchProc0;
        ulGroupTableXI[48] = (Uint32 ) & ulPdchProc0;
        ulGroupTableXI[49] = (Uint32 ) & ulPdchProc0;
        ulGroupTableXI[50] = (Uint32 ) & ulPdchProc3;
        
        ulGroupTableXI[51] = (Uint32) & ulPtcchIdleProc;

 


  	   



}

void dlGroupXIIITableInit(void)
{                           
	Uint32 *ptr;
	ptr = dlGroupTableXIII;
	*ptr++ = (Uint32 ) & dlPdchProc0;
	*ptr++ = (Uint32 ) & dlPdchProc3;
	*ptr++ = (Uint32 ) & dlPdchProc3;
	*ptr++ = (Uint32 ) & dlPdchProc3;
	*ptr++ = (Uint32 ) & dlPdchProc0;
	*ptr++ = (Uint32 ) & dlPdchProc3;
	*ptr++ = (Uint32 ) & dlPdchProc3;
	*ptr++ = (Uint32 ) & dlPdchProc3;
	*ptr++ = (Uint32 ) & dlPdchProc0;
	*ptr++ = (Uint32 ) & dlPdchProc3;
	*ptr++ = (Uint32 ) & dlPdchProc3;
	*ptr++ = (Uint32 ) & dlPdchProc3;
	*ptr++ = (Uint32 ) & dlPtcchIdleProc;
} 
								
void ulGroupXIIITableInit(void)
{                           
	Uint32 *ptr;
	ptr = ulGroupTableXIII;
	*ptr++ = (Uint32 ) & ulPdchProc0;
	*ptr++ = (Uint32 ) & ulPdchProc0;
	*ptr++ = (Uint32 ) & ulPdchProc0;
	*ptr++ = (Uint32 ) & ulPdchProc3;
	*ptr++ = (Uint32 ) & ulPdchProc0;
	*ptr++ = (Uint32 ) & ulPdchProc0;
	*ptr++ = (Uint32 ) & ulPdchProc0;
	*ptr++ = (Uint32 ) & ulPdchProc3;
	*ptr++ = (Uint32 ) & ulPdchProc0;
	*ptr++ = (Uint32 ) & ulPdchProc0;
	*ptr++ = (Uint32 ) & ulPdchProc0;
	*ptr++ = (Uint32 ) & ulPdchProc3;
	*ptr++ = (Uint32 ) & ulPtcchIdleProc;
}

void dlPtcchTableInit(void)
{                           
	Uint32 *ptr;
	ptr = dlPtcchTable;
	*ptr++ = (Uint32 ) & dlPtcchProc0;
	*ptr++ = (Uint32 ) & dlPtcchIdleProc0;
	*ptr++ = (Uint32 ) & dlPtcchProc3;
	*ptr++ = (Uint32 ) & dlPtcchIdleProc0;
	*ptr++ = (Uint32 ) & dlPtcchProc3;
	*ptr++ = (Uint32 ) & dlPtcchIdleProc0;
	*ptr++ = (Uint32 ) & dlPtcchProc3;
	*ptr++ = (Uint32 ) & dlPtcchIdleProc3;
}

void ulPtcchTableInit(void)
{                           
	Uint32 *ptr;
	ptr = ulPtcchTable;
	*ptr++ = (Uint32 ) & ulPtcchProc;
	*ptr++ = (Uint32 ) & ulPtcchIdleProc0;
}
 

void dlPbcchTableInit(void)
{
        Uint32 *ptr;


        ptr = dlPbcchTable;

        *ptr++ = (Uint32) & dlPbcchProc0;
        *ptr++ = (Uint32) & dlBcchProc1;
        *ptr++ = (Uint32) & dlBcchProc2;
        *ptr++ = (Uint32) & dlBcchProc3;
        
}


void dlPpchTableInit(void)
{
        Uint32 *ptr;

        ptr    = dlPpchTable;
        
        *ptr++ = (Uint32) & dlPpchProc0;
        *ptr++ = (Uint32) & dlPpchProc1;
        *ptr++ = (Uint32) & dlPpchProc2;
        *ptr++ = (Uint32) & dlPpchProc3;
}

        
        










