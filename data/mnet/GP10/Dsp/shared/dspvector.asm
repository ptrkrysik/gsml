*****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************

* Vector Table for JetCell C6201 DSP 
        
	.global _c_int00        
             
	.text             
	.sect .vec
_Reset:
	b	_c_int00
	nop
	nop
	nop
	nop
	nop
	nop
	nop					
_NMI				
	b	_NMI
	nop
	nop
	nop
	nop
	nop
	nop
	nop	
	
ReservedInt:
	nop
	nop
	nop
	nop
	nop
	nop
	nop	
	nop
	
	nop
	nop
	nop
	nop
	nop
	nop	
	nop
	nop
	