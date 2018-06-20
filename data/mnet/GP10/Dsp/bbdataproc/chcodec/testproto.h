/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/****************************************************************************
**
** File: testproto.h
**
*****************************************************************************/ 

/*
* Test function prototypes
*/
void TestCnvCodec(void);

void TestCCHInterleave(void);

void TestTCHInterleave(void);

void TestCrcCodec(void);

void TestTCHChanCodec(void);

void TestFACCHChanCodec(void);

void TestCCHChanCodec(void);

void TestSCHChanCodec(void);

void TestRACHChanCodec(void);

Int unpackFromFile(FILE *fin, Char *output, Int numBits);

void packToFile(FILE *f, UChar *input, Int numBits);

void crcEncoderTest(Char *inputPtr, Char *par_tab, Char *gen_poly, 
                UInt polyLength, UInt nbr);

void crcEncoderTest2(Char *inputPtr, Char *parityPtr, Char *polyPtr, 
                     UInt numParityBits, UInt numBits);

UShort reverseBits(UShort value);

void readBcchData(FILE *fin, UChar sysInfoType, UChar sigBufState, UInt TN);

void readCchData(FILE *fin, t_DLSigBufs *sigBufPtr);

void readSchData(FILE *fin, UChar sigBufState, UInt TN);

void writeCCHFrameData(FILE *fout, UInt TN, UInt numBits);
