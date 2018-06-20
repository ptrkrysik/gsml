/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/

/* linker command file (MAP 0) */

-heap  0x2000
-stack 0x1E00
         
-l rts6201.lib
-l dev6x.lib
/*-l drv6x.lib*/
         
     
MEMORY
{
  VEC_TABLE_LOAD(RWX)  : origin = 0x00000000 length = 0x00000300
  SBSRAM_PROG_MEM (RWX): origin = 0x00000A00 length = 0x0002b600
  SBSRAM_DATA_MEM (RW) : origin = 0x0002c000 length = 0x00014000 
  INT_PROG_MEM (RWX)   : origin = 0x01400000 length = 0x00010000
/*
* for PCI based T1 EVM board only      
*/   
  SDRAM0_DATA_MEM (RX) : origin = 0x02000000 length = 0x00400000
  SDRAM1_DATA_MEM (RX) : origin = 0x03000000 length = 0x00400000
 
  HPI_TX_MEM(RW)       : origin = 0x80000000 length = 0x00000204
  HPI_RX_MEM(RW)       : origin = 0x80000204 length = 0x00000204  
  INT_DATA_MEM (RW)  :   origin = 0x80000408 length = 0x0000fBF8
}

SECTIONS
{     
  .vec:          load = VEC_TABLE_LOAD
  
  .hpiRxMem     {  
                    bin\hpirx.obj(.far)   
                }
  		     	load = HPI_RX_MEM			fill = 0
                
  .hpiTxMem     {  
                    bin\hpitx.obj(.far)   
                }
  		     	load = HPI_TX_MEM			fill = 0
                
  .intText:		{  /* time critical code => internal memory */
  					demsubs.obj(.text)
  					viteq.obj(.text)
  					vitgsm.obj(.text)  					
  					chcodesubs.obj(.text)  					
  					Mod2asm.obj(.text)
  					intleav.obj(.text)
  					encryptsubs.obj(.text)
  					decryptopt.obj(.text)
  					bin\demtop.obj(.text) 
  					bin\demodc.obj(.text)
  					bin\rtscheck.obj(.text)   					
  					bin\Mod2spb.obj(.text)
  					bin\intleavs.obj(.text)  					
  					bin\burst.obj(.text)  					
  					bin\burstisr.obj(.text)
  					bin\miscfunc.obj(.text)
  					bin\codecs.obj(.text)  					
  					bin\topcodec.obj(.text)
  					bin\bfi.obj(.text)
  					bin\sbjcodecs.obj(.text)
  					bin\encrypt.obj(.text) 
  					bin\frameisr.obj(.text)  
  					bin\hpi.obj(.text)
  					bin\queue.obj(.text)
  					bin\dspmh.obj(.text) 
  					bin\tchmsg.obj(.text) 
  					bin\agc.obj(.text)   					     					
                    bin\siddetect.obj(.text)
                    
                    bin\inmem.obj(.text) 
  				}
                load =  INT_PROG_MEM  				
  				
  .text:        {     
    				bin\rrmsg.obj(.text)  
    				bin\syncmsg.obj(.text)    				
  					bin\tch.obj(.text)
  					bin\sdcch.obj(.text)  
   					bin\sacch.obj(.text)
					bin\accum.obj(.text)
   			        bin\bcch.obj(.text)    
  					bin\pagch.obj(.text)  
  					bin\pdch.obj(.text)  					   			    
                    bin\dspmain.obj(.text) 
                    bin\dspmaint.obj(.text) 
                    bin\msgstub.obj(.text) 
                    bin\dspinit.obj(.text) 
                    bin\frmsync.obj(.text) 
                    bin\groupini.obj(.text) 
                    bin\grouptbl.obj(.text) 
                    bin\init_hw2.obj(.text) 
                    bin\loopback.obj(.text) 
                    bin\dynpwrctrl.obj(.text) 
			        bin\report.obj(.text)     
                    bin\trxmsg.obj(.text) 				 
  				    bin\diagmsg.obj(.text) 				 
     	            bin\ccchmsg.obj(.text)
  					bin\dcchmsg.obj(.text)   					  					
                    bin\hoProc.obj(.text) 
                    bin\oammsg.obj(.text)
                    bin\drv6x.obj(.text)
                    bin\pdchMsg.obj(.text)

                    bin\exmem.obj(.text)   
                }
                load = SBSRAM_PROG_MEM  
                 
  .const:       load = INT_DATA_MEM
  .bss:		    load = INT_DATA_MEM		fill = 0  /* note: we do not use bss */
  .data:        load = INT_DATA_MEM
  .cinit        load = SBSRAM_DATA_MEM
  .pinit        load = SBSRAM_DATA_MEM
  .stack        load = INT_DATA_MEM      fill = 0xbadbeef
  
  .far:		    {  /* time critical far data => internal memory */
                    bin\inmem.obj(.far)   
                }
                load = INT_DATA_MEM		fill = 0
          
  .sysmem       load = SBSRAM_DATA_MEM
  .cio          load = SBSRAM_DATA_MEM
  
  .extData      {  /* non-time critical data => ext memory */
                    bin\exmem.obj(.far)   
                }
                load = SBSRAM_DATA_MEM  fill = 0
}
