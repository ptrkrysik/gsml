/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/****************************************************************************
** File: sbjcodecs.c
**
** Description:   
**   Contains functions that perform subjective encoding and decoding of
**   speech frames. The re-ordering is done in-place - i.e. the output is
**   stored in the input buffer.
**   Note: Old revision log is at the end of this file.
**
*****************************************************************************/
#include "gsmdata.h" 
#include "bbdata.h" 
#include "bbproto.h"  
#include "codec.h"
#include "dsprotyp.h"


#ifdef ETSI_FRAME_PACKING   
   
/*
* Subjective encoder index mapping table.
*/
const UShort encoder_tab[NUM_BITS_TCH_FRAME] =
{ 
   /*
   * class Ia
   */
   6-1,53-1,109-1,165-1,221-1,5-1,12-1,17-1,4-1,11-1,16-1,22-1,
   43-1,99-1,155-1,211-1,52-1,108-1,164-1,220-1,10-1,26-1,30-1,
   42-1,98-1,154-1,210-1,41-1,97-1,153-1,209-1,40-1,96-1,152-1,208-1,
   39-1,95-1,151-1,207-1,51-1,107-1,163-1,219-1,3-1,21-1,33-1,38-1,94-1,150-1,206-1,
   /*
   * class Ib
   */  
   25-1,29-1,45-1,101-1,157-1,213-1,37-1,93-1,149-1,205-1,47-1,103-1,159-1,215-1,
   2-1,9-1,15-1,36-1,20-1,24-1,32-1,44-1,100-1,156-1,212-1,50-1,106-1,162-1,218-1,
   56-1,59-1,62-1,65-1,68-1,71-1,74-1,77-1,80-1,83-1,86-1,89-1,                     
   92-1,112-1,115-1,118-1,121-1,124-1,127-1,130-1,133-1,136-1,139-1,142-1,145-1,      
   148-1,168-1,171-1,174-1,177-1,180-1,183-1,186-1,189-1,192-1,195-1,198-1,201-1,     
   204-1,224-1,227-1,230-1,233-1,236-1,239-1,242-1,245-1,248-1,251-1,254-1,257-1,     
   260-1,46-1,102-1,158-1,214-1,49-1,105-1,161-1,217-1,55-1,58-1,
   61-1,64-1,67-1,70-1,73-1,76-1,79-1,82-1,85-1,88-1,91-1,                        
   111-1,114-1,117-1,120-1,123-1,126-1,129-1,132-1,135-1,138-1,141-1,144-1,         
   147-1,167-1,170-1,173-1,176-1,179-1,182-1,185-1,188-1,191-1,194-1,197-1,200-1,     
   203-1,223-1,226-1,229-1,232-1,
   /*
   * class I
   */
   235-1,238-1,241-1,244-1,247-1,250-1,253-1,256-1,
   259-1,1-1,8-1,14-1,28-1,31-1,35-1,34-1,13-1,19-1,18-1,23-1,48-1,104-1,
   160-1,216-1,54-1,57-1,60-1,63-1,66-1,69-1,72-1,75-1,78-1,81-1,84-1,87-1,             
   90-1,110-1,113-1,116-1,119-1,122-1,125-1,128-1,131-1,134-1,137-1,140-1,143-1,      
   146-1,166-1,169-1,172-1,175-1,178-1,181-1,184-1,187-1,190-1,193-1,196-1,199-1,     
   202-1,222-1,225-1,228-1,231-1,234-1,237-1,240-1,243-1,246-1,249-1,252-1,255-1,     
   258-1,7-1,27-1
};
                                                     
#else
                                                     
#define	GSM_MAGIC	0xD	                             
/*
* Subjective encoder index mapping table.
*/
const UShort gsmFrToast[NUM_BITS_TCH_FRAME] =
{ 
   /*
   * class Ia
   */
   1-1,48-1,104-1,160-1,216-1,2-1,7-1,13-1,3-1,8-1,14-1,18-1,
   37-1,93-1,149-1,205-1,49-1,105-1,161-1,217-1,9-1,23-1,27-1,
   38-1,94-1,150-1,206-1,39-1,95-1,151-1,207-1,40-1,96-1,152-1,208-1,
   41-1,97-1,153-1,209-1,50-1,106-1,162-1,218-1,4-1,19-1,31-1,42-1,98-1,154-1,210-1,
   /*
   * class Ib
   */  
   24-1,28-1,44-1,100-1,156-1,212-1,43-1,99-1,155-1,211-1,46-1,102-1,158-1,214-1,
   5-1,10-1,15-1,34-1,20-1,25-1,32-1,45-1,101-1,157-1,213-1,51-1,107-1,163-1,219-1,
   54-1,57-1,60-1,63-1,66-1,69-1,72-1,75-1,78-1,81-1,84-1,87-1,                     
   90-1,110-1,113-1,116-1,119-1,122-1,125-1,128-1,131-1,134-1,137-1,140-1,143-1,      
   146-1,166-1,169-1,172-1,175-1,178-1,181-1,184-1,187-1,190-1,193-1,196-1,199-1,     
   202-1,222-1,225-1,228-1,231-1,234-1,237-1,240-1,243-1,246-1,249-1,252-1,255-1,     
   258-1,47-1,103-1,159-1,215-1,52-1,108-1,164-1,220-1,55-1,58-1,
   61-1,64-1,67-1,70-1,73-1,76-1,79-1,82-1,85-1,88-1,91-1,                        
   111-1,114-1,117-1,120-1,123-1,126-1,129-1,132-1,135-1,138-1,141-1,144-1,         
   147-1,167-1,170-1,173-1,176-1,179-1,182-1,185-1,188-1,191-1,194-1,197-1,200-1,     
   203-1,223-1,226-1,229-1,232-1,
   /*
   * class I
   */
   235-1,238-1,241-1,244-1,247-1,250-1,253-1,256-1,
   259-1,6-1,11-1,16-1,29-1,33-1,35-1,36-1,17-1,21-1,22-1,26-1,53-1,109-1,
   165-1,221-1,56-1,59-1,62-1,65-1,68-1,71-1,74-1,77-1,80-1,83-1,86-1,89-1,             
   92-1,112-1,115-1,118-1,121-1,124-1,127-1,130-1,133-1,136-1,139-1,142-1,145-1,      
   148-1,168-1,171-1,174-1,177-1,180-1,183-1,186-1,189-1,192-1,195-1,198-1,201-1,     
   204-1,224-1,227-1,230-1,233-1,236-1,239-1,242-1,245-1,248-1,251-1,254-1,257-1,     
   260-1,12-1,30-1
};

/*

The preliminary coded bits w(k) for k = 1 to 260 are hence defined by:
w(k) = s(k)	 for k = 1 to 71
w(k) = s(k?2) for k = 74 to 123
w(k) = s(k?4) for k = 126 to 178
w(k) = s(k?6) for k = 181 to s230
w(k) = s(k?8) for k = 233 to s252

Repetition bits:
w(k) = s(70)	  for k = 72 and 73
w(k) = s(120) for k = 124 and 125
w(k) = s(173) for k = 179 and 180 
w(k) = s(223) for k = 231 and 232

Parity bits:	
w(k = p(k?252) for k = 253 to 260
*/

/* reordering EFR Bits */
const UShort gsmEfrSbjTable[NUM_BITS_TCH_FRAME] =
{   
39-1, 40-1, 41-1, 42-1, 43-1, 44-1,	
146-1, 147-1, 148-1, 149-1, 150-1, 151-1,
94-1, 95-1,
201-1, 202-1,
48-1,
89-1,
100-1,
141-1,
45-1,
152-1,
96-1,
203-1,
2-1, 3-1,
8-1,
10-1,
18-1, 19-1, 
24-1, 
46-1, 47-1, 
153-1, 154 -1,
97-1, 
204-1, 
4-1, 5-1, 
11-1, 12-1, 
16-1,
9-1,
6-1, 7-1,
13-1,	
17-1,	
20-1,
98-1,	
205-1,
	
1-1,	
14 -1, 15-1,
21-1,	
25 -1, 26	-1,
28-1,
155-1,        
207-1,	
196-1,	
248-1,	 
90-1,	
142-1,	
197-1,	
249-1,


253 -1, 254-1, 255-1, 256-1, 257-1, 258-1, 259-1,  260-1,
49  -1,
101 -1,
156 -1,
208-1,
22 -1, 23-1,
27 -1,
29 -1,
52 -1,
56 -1,
60 -1,
64 -1,
68 -1,
104-1,
108-1,
112-1,
116-1,
120-1,
159-1,
163-1,
167-1,
171-1,
175-1,
211-1,
215-1,
219-1,
223-1,
227-1,
91 -1,
143-1,
198-1,
250-1,
50 -1,
102-1,
157-1,
209-1,
30 -1, 31-1, 32-1,
33 -1, 34-1, 35-1, 36-1,
99-1,
206 -1,
53  -1,
57-1,

61	-1,
65	-1,
69	-1,
105	-1,
109	-1,
113	-1,
117	-1,
121	-1,
160	-1,
164	-1,
168	-1,
172	-1,
176	-1,
212	-1,
216	-1,
220	-1,
224	-1,
228	-1,
54	-1,
58	-1,
62	-1,
66	-1,
106	-1,
110-1,	
114	-1,
118	-1,
161	-1,
165	-1,
169	-1,
173	-1,
213	-1,
221	-1,
225	-1,
92	-1,
144	-1,
199	-1,
251	-1,
51	-1,
103	-1,
158	-1,
210	-1,
93	-1,
145	-1,
200	-1,


252	-1,
55	-1,
59	-1,
63	-1,
67	-1,
107	-1,
111	-1,
115	-1,
119	-1,
162	-1,
166	-1,
170	-1,
174	-1,
214	-1,
222	-1,
226	-1,
37 -1, 38-1,
	
70-1,
72 -1,73-1,
122-1,
124 -1,125-1,
177-1,
179 -1,180-1,
229	-1,
231 -1,232-1,
217 -1, 218-1,	
71	-1,
123	-1,
178	-1,
230	-1,
74	-1,
77	-1,
80	-1,
83	-1,
86	-1,
126	-1,
129	-1,
132	-1,
135	-1,
138	-1,
181	-1,
184	-1,
187	-1,
190	-1,
193	-1,
233	-1,
236	-1,
239	-1,
242	-1,
245	-1,
75	-1,
78	-1,
81	-1,
84	-1,
87	-1,
127	-1,
130	-1,
133	-1,
136	-1,
139	-1,
182	-1,
185	-1,
188	-1,
191	-1,
194	-1,
234	-1,
237	-1,
240	-1,
243	-1,
246	-1,
76	-1,
79	-1,
82	-1,
85	-1,
88	-1,
128	-1,
131	-1,
134	-1,
137	-1,
140	-1,
183	-1,
186	-1,
189	-1,
192	-1,
195	-1,
235	-1,
238	-1,
241	-1,
244	-1,
247	-1
 

};

#endif                 

  
#define EFR_NUM_TCH_PARITY_BITS         8
#define EFR_NUM_TCH_CRC_IN_BITS         65  
        
/*****************************************************************************
** Routine Name: sbjEncoder
**
** Description: 
**   Performs subjective encoding per GSM 06.10 table A2.1a for Full-rate
**   speech.   
**
** Method:
**
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**
** Returns: 
**
** Data Store I/O: 
**
*****************************************************************************/    
void sbjEncoder(Char* inputPtr, t_vocAlgo vocoAlgo)
{
   Int i;
   Char temp[NUM_BITS_TCH_FRAME+4];
   Char efrCrcInBits[EFR_NUM_TCH_CRC_IN_BITS];
   Char *tempBufPtr;

   switch(vocoAlgo)
   {              
#ifdef ETSI_FRAME_PACKING   

      case VOC_GSMF:
         /*
         * GSM full-rate vocoder is in use.
         * Re-order input array according to table storing result in temp.
         * Then move result from temp back to input buffer.
         */
         for(i=0; i<NUM_BITS_TCH_FRAME; i++)
         {
            temp[i] = inputPtr[encoder_tab[i]];
         }

         for(i=0; i<NUM_BITS_TCH_FRAME; i++)
         {
            inputPtr[i] = temp[i];
         }

         break;
#else      
     
      case VOC_GSMF:
                     
         for(i=0; i<NUM_BITS_TCH_FRAME; i++)
         {
            temp[i] = inputPtr[gsmFrToast[i]+4];
         }

         for(i=0; i<NUM_BITS_TCH_FRAME; i++)
         {
            inputPtr[i] = temp[i];
         }

         break;
                    
     case VOC_EFR:
/*
*  GSM 5.03 
*
The preliminary coded bits w(k) for k = 1 to 260 are hence defined by:
w(k) = s(k)	 for k = 1 to 71
w(k) = s(k-2) for k = 74 to 123
w(k) = s(k-4) for k = 126 to 178
w(k) = s(k-6) for k = 181 to s230
w(k) = s(k-8) for k = 233 to s252

Repetition bits:
w(k) = s(70)	  for k = 72 and 73
w(k) = s(120) for k = 124 and 125
w(k) = s(173) for k = 179 and 180 
w(k) = s(223) for k = 231 and 232

Parity bits:	
w(k = p(k?252) for k = 253 to 260
*/               

         /*
         *  skip first 4 bits 0xC
         */
         tempBufPtr = inputPtr+ 4;            
         /*
         *  CRC Encode to generate parity bits and store it to last 8 bit of a 260-bit frame
         */
         //crcEncoderEfrTCH(tempBufPtr, & temp[252]);

         
         /* bit mapping of 65 bits used for CRC calculation */  
         for(i=0; i<EFR_NUM_TCH_CRC_IN_BITS; i++)
         {
             efrCrcInBits[i] = tempBufPtr[tch_Efr_CRC_calc_LookUpTbl[i]];
         }      
                 
         crcEncoder16( efrCrcInBits,                 /* input data bits */
                            & temp[252],                 /* input parity bits */
                            EFR_CRC_POLY,
                            EFR_NUM_TCH_PARITY_BITS,
                            EFR_NUM_TCH_CRC_IN_BITS,                            
                            0); 
                              
         /*
         *  rearange input bits according to GSM 5.03
         */
         for(i=0; i<71; i++)
         {
            temp[i] = tempBufPtr[i];       
         }                  
         temp[i++] = tempBufPtr[69];
         temp[i++] = tempBufPtr[69];
                                                             
         for(i=73; i<123; i++)
         {
            temp[i] = tempBufPtr[i-2];
         }        
         temp[i++] = tempBufPtr[119];
         temp[i++] = tempBufPtr[119];
    
                    
         for(i=125; i<178; i++)
         {
            temp[i] = tempBufPtr[i-4];
         }        
         temp[i++] = tempBufPtr[172];
         temp[i++] = tempBufPtr[172];
                  
                    
         for(i=180; i<230; i++)
         {
            temp[i] = tempBufPtr[i-6];
         }        
         temp[i++] = tempBufPtr[222];
         temp[i++] = tempBufPtr[222];
                                     
         for(i=232; i<252; i++)
         {
            temp[i] = tempBufPtr[i-8];
         }        

                               
                     
         for(i=0; i<NUM_BITS_TCH_FRAME; i++)
         {
            inputPtr[i] = temp[gsmEfrSbjTable[i]];
         }

         break;
                          
#endif       
      
      default:
         /*
         * No others are currently supported
         */
         break;
   }
}

/*****************************************************************************
** Routine Name: sbjDecoder
**
** Description: 
**   Performs subjective encoding per GSM 06.10 table A2.1a for Full-rate
**   speech.   
**
** Method:
**
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**
** Returns: 
**
** Data Store I/O: 
**
*****************************************************************************/
Bool sbjDecoder(Char* inputPtr, t_vocAlgo vocoAlgo, Uint8 timeSlot)
{
   Int i;
   Char temp[NUM_BITS_TCH_FRAME+4], *tempPtr;
   Char efrCrcInBits[EFR_NUM_TCH_CRC_IN_BITS];   
   
   Bool speechParityError = 0;
   
   Uint32 repeatBitError = 0;
   Uint8 repeatBits;
/*   
  Uchar efrMsFrame[50] = { 0x2e, 00, 00, 00, 04, 0x30, 00, 0x00,
    0x0e, 00, 01, 01, 0x6b, 0xd7, 0x83, 0x4b, 
    0x31, 0x86, 0x99, 0x07, 0x63, 0x36, 0x3c, 0x4e, 0xa8, 0xae,0xa1,
    0x87, 0x2d,  0x4d,  0xa7,  0x08, 
    0xc3, 0xbe, 0xac, 0x70, 0x08, 0x01, 
    0x6c, 0x9c, 0xda, 0xb7, 0xf2, 0x8a,
    0xac, 0xbe, 0x40, 0x01 ,
    00 ,00  };  
*/    
    
   Uchar efrMsFrame[50] = { 0x2e, 00, 00, 00, 04, 0x30, 00, 0x00,
    0x0e, 00, 01, 01,  0x92, 0x76, 0x0a , 0x15 ,
    0x4e, 0x0b, 0x16, 0x82, 0x63, 0x0a, 0x22, 0x52, 0x55, 0xf0, 0xa0, 0xc9, 0xff, 0x46, 0x1b, 0x3f, 
    0x6d, 0xc8, 0x7f, 0xfe, 0x3a, 0x94, 0x09, 0xba, 0xd6, 0x05, 0xd2, 0x6c, 0xec, 0x01, 0x00, 0x01, 
   00, 01 
   };

   switch(vocoAlgo)
   {   
#ifdef ETSI_FRAME_PACKING   
   
      case VOC_GSMF:
         /*
         * GSM full-rate vocoder is in use.
         * Re-order input array according to table storing result in temp.
         * Then move result from temp back to input buffer.
         */
         for(i=0; i<NUM_BITS_TCH_FRAME; i++)
         {
            temp[encoder_tab[i]] = inputPtr[i];
         }

         for(i=0; i<NUM_BITS_TCH_FRAME; i++)
         {
            inputPtr[i] = temp[i];
         }

         break;
#else      
                
      case VOC_GSMF:   
      
         /*
         *  bypass first four bits - GSM_MAGIC 0x1101
         */  
         /*
         *  first 4 bits are GSM_MAGIC 1101
         */
         temp[0] =1;
         temp[1] =1;
         temp[2] =0;
         temp[3] =1;
                        
         for(i=0; i<NUM_BITS_TCH_FRAME; i++)
         {
            temp[gsmFrToast[i]+4] = inputPtr[i];
         }

         for(i=0; i<NUM_BITS_TCH_FRAME+4; i++)
         {
            inputPtr[i] = temp[i];
         }
                            
         /*
         *   Speech Parity Error is always 0 for GSM FR
         */                            
         speechParityError = 0;            
         break;
                         
                
      case VOC_EFR:   
        
        
     /*   unpackByteMSB( & efrMsFrame[12],
                      inputPtr, 
                      NUM_BITS_TCH_FRAME+4);      
     */    
         
         /*
         *  undo the transmit reordering
         */
                
         for(i=0; i<NUM_BITS_TCH_FRAME; i++)
         {
            temp[gsmEfrSbjTable[i]] = inputPtr[i];
         }            
         
         /*
         *  first 4 bits are GSM_MAGIC_EFR 1100
         */
         inputPtr[0] =1;
         inputPtr[1] =1;
         inputPtr[2] =0;
         inputPtr[3] =0;
         
         /*
         *  Get 244 Bits per EFR speech frame from un-reordered bits
         */          
         tempPtr =  inputPtr + 4;
                   
         for(i=0; i<71; i++)
         {
            tempPtr[i] = temp[i];
         }         
         /*
         *  majority rule for repeated bits
         */                                                       
         repeatBits = tempPtr[69] + temp[71] +  temp[72];
         if( repeatBits == 1 || repeatBits == 2)
         {
           repeatBitError =  tempPtr[69] | temp[71]<<1 +  temp[72]<<2;
         }         
         tempPtr[69] =  repeatBits >> 1;
         
                       
         for(i=73; i<123; i++)
         {
            tempPtr[i-2] = temp[i];
         }   
         /*
         *  majority rule for repeated bits
         */           
         repeatBits = tempPtr[119] + temp[123] +  temp[124];
         if( repeatBits == 1 || repeatBits == 2)
         {
           repeatBitError |=  tempPtr[69]<<8 | temp[123]<<9 +  temp[124]<<10;
         }  
         tempPtr[119] =  repeatBits >> 1;
 
                    
         for(i=125; i<178; i++)
         {
            tempPtr[i-4] = temp[i];
         }        
         /*
         *  majority rule for repeated bits
         */           
         repeatBits = tempPtr[172] + temp[178] +  temp[179];
         if( repeatBits == 1 || repeatBits == 2)
         {
           repeatBitError |=  tempPtr[172]<<16 | temp[178]<<17 +  temp[179]<<18;
         }           
         tempPtr[172] =  repeatBits >> 1;
                     
         for(i=180; i<230; i++)
         {
            tempPtr[i-6] = temp[i];
         }   
         /*
         *  majority rule for repeated bits
         */                
         repeatBits = tempPtr[222] + temp[230] +  temp[231];
         if( repeatBits == 1 || repeatBits == 2)
         {
           repeatBitError |=  tempPtr[222]<<24 | temp[230]<<25 +  temp[231]<<26;
         }           
         tempPtr[222] =  repeatBits >> 1;
                                     
         for(i=232; i<252; i++)
         {
            tempPtr[i-8] = temp[i];
         }        

         /*
         *  send a diag report
         if( repeatBitError ) sendDiagMsg(0x3f,0,timeSlot,4, & repeatBitError);
         */  
                        
         /*
         *  get bits required to generate parity bits.
         */      
         //speechParityError = crcDecoderEfrTCH(tempPtr, &temp[252]);
         
         
         /* bit mapping of 65 bits used for CRC calculation */  
         for(i=0; i<EFR_NUM_TCH_CRC_IN_BITS; i++)
         {
             efrCrcInBits[i] = tempPtr[tch_Efr_CRC_calc_LookUpTbl[i]];
         }      
                 
         speechParityError = crcDecoder16( efrCrcInBits,                /* input data bits */
                                           & temp[252],                 /* input parity bits */
                                           EFR_CRC_POLY,
                                           EFR_NUM_TCH_PARITY_BITS,
                                           EFR_NUM_TCH_CRC_IN_BITS,                            
                                           0); 
         break;                         
#endif      
                      
      default:
         /*
         * No others are currently supported
         */               
         speechParityError = 0;
         break;
   }                           
   return (speechParityError);
}

/********************** Old revision log **********************************
**
** Revision 1.4  2000-01-12 10:17:04-08  whuang
** Added CRC encoder/Decoder, subjective encoder/decoder and VOIP format for EFR.
** No uplink frame is sent out, if CRC error is found for EFR also. Created a common
** CRC encoder/decoder for SCH, TCH, RACH and EFR
**
** Revision 1.3  1999-04-06 18:09:28-07  whuang
** added GSM FR Toast frame packing; added silence frame on DL and UL when there is
** no valid voice frame.
**
** Revision 1.2  1999-03-11 15:55:58-08  whuang
** <>
**
** Revision 1.1  1999-01-07 14:16:06-08  randyt
** Initial revision
** PVCS Version 1.0
**
 * 
 *    Rev 1.0   Oct 27 1998 16:19:30   Administrator
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.0   Oct 23 1998 17:56:04   Administrator
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
** Revision 1.3  1998/10/15 20:12:05  dlanza
** Added the TCH path and compiled successfully.
**
** Revision 1.2  1998/10/15 16:18:21  dlanza
** Creation.
**
** Revision 1.1  1998/10/15 16:16:08  dlanza
** Initial revision
**/

