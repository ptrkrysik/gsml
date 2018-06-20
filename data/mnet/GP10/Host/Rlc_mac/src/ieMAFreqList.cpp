// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieMAFreqList.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieMAFreqList.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

// *******************************************************************
// class IeMAFreqList
//
// Description:
//    MA Frequency List IE -- GSM04.60 12.8
// *******************************************************************


RlcMacResult IeMAFreqList::SetMAFreqList(unsigned char numEntries, unsigned short *arfcns)
{
    if ((numEntries <= MA_LIST_MAX_RF_FREQS) && (numEntries > 0))
    {
        numArfcns = numEntries;
        
        // Sort and store list 
        int duplicates = 0;
        for (int i = 0; i < numArfcns; i++)
        {
            int index = 0;
            for (int j = 0; j < numArfcns; j++)
            {
                if ((arfcns[i] <= arfcns[j]) && (i != j))
                {
                    index++;        
                }
                // Count how many duplicates (x2) in the list.
                if (arfcns[i] == arfcns[j])
                {
                    duplicates++;
                }
            }
            arfcnList[index] = arfcns[i];
        };
        
        // Reduce the total count by the number of duplicates.  Note that
        // duplicates were double counted above.
        numArfcns -= (duplicates >> 1);
        
        isValid = TRUE;
        return (RLC_MAC_SUCCESS);
    }
    else
    {
        return (RLC_MAC_PARAMETER_RANGE_ERROR);
    }
}



RlcMacResult IeMAFreqList::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeMAFreqList::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result;
    unsigned char MAListLen;
    unsigned char MAList[MAX_MA_FREQ_LIST_LEN];
    
    // Select the appropriate format.
    switch(SelectFrequencyListFormat())
    {
    case FREQ_LIST_FORMAT_BIT_MAP_0:
        result = EncodeFreqListBitMap0(MAListLen, MAList);  
        break;
    case FREQ_LIST_FORMAT_VARIABLE_BIT_MAP:
        result =  EncodeFreqListVariableBitMap(MAListLen, MAList);
        break;
    case FREQ_LIST_FORMAT_1024_RANGE:
        result =  EncodeFreqList1024Range(MAListLen, MAList);
        break;
    case FREQ_LIST_FORMAT_512_RANGE:
        result =  EncodeFreqList512Range(MAListLen, MAList);
        break;
    case FREQ_LIST_FORMAT_256_RANGE:
        result =  EncodeFreqList256Range(MAListLen, MAList);
        break;
    case FREQ_LIST_FORMAT_128_RANGE:
        result =  EncodeFreqList128Range(MAListLen, MAList);
        break;
    default:
        return RLC_MAC_ERROR;
    }
    
    
    // Pack Change Mark bits into the output bit stream.
/*    if ((result = dataStream.InsertBits8(changeMark, 2)) != RLC_MAC_SUCCESS)
    {
        DBG_ERROR("MAFreqList::EncodeIe ChangeMark failure %x\n", result);
    }       */
    
    DBG_LEAVE();
    return (result);
}


RlcMacResult IeMAFreqList::EncodeFreqList1024Range(unsigned char &MAListLen, unsigned char *MAList)
{
    DBG_FUNC("IeMAFreqList::EncodeFreqList1024Range", RLC_MAC);
    DBG_ENTER();

    if (numArfcns > 17)
    {
        DBG_ERROR("EncodeFreqList1024Range: too many freqs %d\n, max=17", numArfcns);
        return RLC_MAC_PARAMETER_RANGE_ERROR;
    }
    
    int i;
    unsigned short W[64];           for(i=0; i < 64; W[i++]=0);
    unsigned short freqSet[64];     for(i=0; i < 64; freqSet[i++]=0);
    for(i=0; i < MAX_MA_FREQ_LIST_LEN; MAList[i++]=0);

    int numInSet= 0;
    // check if freq 0 is in the set, remove from the set if it is
    unsigned char f0 = 0;
    for (i=0; i < numArfcns; i++)
    {
        if (arfcnList[i]== 0)
        {
            f0 = 1;
        } else
        {
            freqSet[numInSet++] = arfcnList[i] - 1;
        }
    }

    // find the orig arfcn,
    unsigned short origArfcn = ReduceMinArfcn(numInSet, freqSet);

    // encode W[]
    EncodeFreqsSubTree(1, 1023, freqSet, numInSet, W);

    // generate the Wbits[]
    unsigned short Wbits[64];
    SetWbits(10, 16, Wbits);

    // now encode IE
    MAList[0] = 0x80 | (f0 << 2);
    int buffLen;
    StuffArray(numInSet, &W[1], &Wbits[1], buffLen, 2, MAList);

    MAListLen = (unsigned char) buffLen;
    // populate back to ARFCN array the modified frequency list
    // depending whether 0 is in or not, the rest is in W[] array starting from 1.
    if(f0)
    {
        // ARFCN 0 is in the list
        arfcnList[0]=0;
        GetFreqSetFromW(0,1023,W,numArfcns-1,arfcnList);
    } else {
        // ARFCN 0 is in NOT the list
        GetFreqSetFromW(0,1023,W,numArfcns,arfcnList);
        for(i=1;i<=numArfcns;i++) arfcnList[i-1]=arfcnList[i];
    }

    DBG_LEAVE();
    return RLC_MAC_SUCCESS;
}



RlcMacResult IeMAFreqList::EncodeFreqList512Range(unsigned char &MAListLen, unsigned char *MAList)
{
    DBG_FUNC("IeMAFreqList::EncodeFreqList512Range", RLC_MAC);
    DBG_ENTER();
    
    if (numArfcns > 18)
    {
        DBG_ERROR("EncodeFreqList512Range: too many freqs %d\n, max=18", numArfcns);
        return RLC_MAC_PARAMETER_RANGE_ERROR;
    }
    
    int i;
    unsigned short W[64];           for(i=0; i < 64; W[i++]=0);
    unsigned short freqSet[64];     for(i=0; i < 64; freqSet[i++]=0);
    for(i=0; i < MAX_MA_FREQ_LIST_LEN; MAList[i++]=0);

    // find the orig arfcn,
    unsigned short origArfcn = ReduceMinArfcn(numArfcns, freqSet);


    // encode W[]
    EncodeFreqsSubTree(1, 511, freqSet, numArfcns - 1, W);

    // generate the Wbits[]
    unsigned short Wbits[64];
    SetWbits(9, 17, Wbits);

    // Encode IE
    // IEI is encoded in msg encode function

    MAList[0] = 0x88 | ((unsigned char )(origArfcn >> 9));
    MAList[1] = ((unsigned char)(origArfcn >> 1));
    MAList[2] = ((unsigned char)(origArfcn << 7));

    int bufferLengthForW = MAX_MA_FREQ_LIST_LEN - 2;
    StuffArray(numArfcns - 1, &W[1], &Wbits[1], bufferLengthForW, 7, &MAList[2]);

    MAListLen = (unsigned char)(bufferLengthForW + 2);


    // populate back to ARFCN array the modified frequency list
    // origArfcn is the 1st one, the rest is in W[] array starting 1.
    arfcnList[0]=origArfcn;
    GetFreqSetFromW(origArfcn,511,W,numArfcns-1,arfcnList);

    DBG_LEAVE();
    return RLC_MAC_SUCCESS;

}


RlcMacResult IeMAFreqList::EncodeFreqList256Range(unsigned char &MAListLen, unsigned char *MAList)
{
    DBG_FUNC("IeMAFreqList::EncodeFreqList512Range", RLC_MAC);
    DBG_ENTER();
    
    if (numArfcns > 22)
    {
        DBG_ERROR("EncodeFreqList256Range: too many freqs %d\n, max=22", numArfcns);
        return RLC_MAC_PARAMETER_RANGE_ERROR;
    }

    int i;
    unsigned short W[64];           for(i=0; i < 64; W[i++]=0);
    unsigned short freqSet[64];     for(i=0; i < 64; freqSet[i++]=0);
    for(i=0; i < MAX_MA_FREQ_LIST_LEN; MAList[i++]=0);

    // find the orig arfcn,
    unsigned short origArfcn = ReduceMinArfcn(numArfcns, freqSet);

    // encode W[]
    EncodeFreqsSubTree(1, 255, freqSet, numArfcns - 1, W);

    // generate the Wbits[]
    unsigned short Wbits[64];
    SetWbits(8, 21, Wbits);

    // Encode IE
    MAList[0] = 0x8a | ((unsigned char )(origArfcn >> 9));
    MAList[1] = ((unsigned char)(origArfcn >> 1));
    MAList[2] = ((unsigned char)(origArfcn << 7));

    int bufferLengthForW = MAX_MA_FREQ_LIST_LEN - 2;
    StuffArray(numArfcns - 1,  &W[1], &Wbits[1], bufferLengthForW, 7, &MAList[2]);

    MAListLen = (unsigned char)(bufferLengthForW + 2);

    // populate back to ARFCN array the modified frequency list
    // origArfcn is the 1st one, the rest is in W[] array starting 1.
    arfcnList[0]=origArfcn;
    GetFreqSetFromW(origArfcn,255,W,numArfcns-1,arfcnList);

    DBG_LEAVE();
    return RLC_MAC_SUCCESS;
}



RlcMacResult IeMAFreqList::EncodeFreqList128Range(unsigned char &MAListLen, unsigned char *MAList)
{
    DBG_FUNC("IeMAFreqList::EncodeFreqList128Range", RLC_MAC);
    DBG_ENTER();
    
    if (numArfcns > 29)
    {
        DBG_ERROR("EncodeFreqList128Range: too many freqs %d, max=29\n", numArfcns);
        return RLC_MAC_PARAMETER_RANGE_ERROR;
    }
        
    int i;
    unsigned short W[64];           for(i=0; i < 64; W[i++]=0);
    unsigned short freqSet[64];     for(i=0; i < 64; freqSet[i++]=0);
    for(i=0; i < MAX_MA_FREQ_LIST_LEN; MAList[i++]=0);

    // find the orig arfcn,
    unsigned short origArfcn = ReduceMinArfcn(numArfcns, freqSet);

    // encode W[]
    EncodeFreqsSubTree(1, 127, freqSet, numArfcns - 1, W);

    // generate the Wbits[]
    unsigned short Wbits[64];
    SetWbits(7, 28, Wbits);

    // Encode IE
    MAList[0] = 0x8c | ((unsigned char )(origArfcn >> 9));
    MAList[1] = ((unsigned char)(origArfcn >> 1));
    MAList[2] = ((unsigned char)(origArfcn << 7));

    int bufferLengthForW = MAX_MA_FREQ_LIST_LEN - 2;
    StuffArray(numArfcns - 1,  &W[1], &Wbits[1], bufferLengthForW, 7, &MAList[2]);

    MAListLen = (unsigned char)(bufferLengthForW + 2);

    // populate back to ARFCN array the modified frequency list
    // origArfcn is the 1st one, the rest is in W[] array starting 1.
    arfcnList[0]=origArfcn;
    GetFreqSetFromW(origArfcn,127,W,numArfcns-1,arfcnList);

    DBG_LEAVE();
    return RLC_MAC_SUCCESS;
}




RlcMacResult IeMAFreqList::EncodeFreqListVariableBitMap(unsigned char &MAListLen, unsigned char *MAList)
{
    DBG_FUNC("IeMAFreqList::EncodeFreqListVariableBitMap", RLC_MAC);
    DBG_ENTER();

    if (numArfcns > 64)
    {
        DBG_ERROR("EncodeFreqListVariableBitMap: too many freqs %d\n",
                    numArfcns);
        return RLC_MAC_PARAMETER_RANGE_ERROR;
    }

    // find the orig arfcn
    unsigned short origArfcn = arfcnList[0];

    int i;
    for (i=1; i < numArfcns; i++)
    {
        if (arfcnList[i] < origArfcn)
        {
            origArfcn = arfcnList[i];
        }
    }

    for (i=0; i < MAX_MA_FREQ_LIST_LEN; i++)
    {
        MAList[i] = 0;
    }

    // Encode IE
    MAList[0] = 0x8e | ((unsigned char )(origArfcn >> 9));
    MAList[1] = ((unsigned char)(origArfcn >> 1));
    MAList[2] = ((unsigned char)(origArfcn << 7));
    
    int octet, lshift, diffmod, maxOctet = 3;
    for (i=0; i < numArfcns; i++)
    {
        if (arfcnList[i] == origArfcn) continue;
        short diff = arfcnList[i] - origArfcn;

        octet = 2 + (diff / 8);
        if (octet >= MAX_MA_FREQ_LIST_LEN)
        {
            DBG_ERROR("EncodeFreqListVariableBitMap: arfcn out of range %d, origArfcn %d\n",
                        arfcnList[i], origArfcn);
            return RLC_MAC_PARAMETER_RANGE_ERROR;
        } 
        if (octet > maxOctet)
        {
            maxOctet = octet;
        } 

        diffmod = diff % 8;
        lshift = 7 - diffmod;
        MAList[octet] |= 1 << lshift;
    }

    MAListLen = (unsigned char)(maxOctet + 1);

    // populate back to ARFCN array the modified frequency list
    // origArfcn is the 1st one

    int j,idx=0;
    arfcnList[idx++]=origArfcn;
    for(i=2;i<=maxOctet;i++)
    {
        for(j=7;j>=0;j--)
        {
            if(i==2 && j==7) continue;  // skip the ORIG-ARFCN bit
            if( (MAList[i] & (0x01<<j)) != 0)
            {
                arfcnList[idx++]=origArfcn+((i-2)*8+(8-j)-1) ;
            }
        }
    }

    DBG_LEAVE();
    return RLC_MAC_SUCCESS;
}



RlcMacResult IeMAFreqList::EncodeFreqListBitMap0(unsigned char &MAListLen, unsigned char *MAList)
{
    DBG_FUNC("IeMAFreqList::EncodeFreqListBitMap0", RLC_MAC);
    DBG_ENTER();
    // this format is used for arfcn range is 1 to 124

    int i;
    for (i=0; i < 16; i++)
    {
        MAList[i] = 0;
    }
    for (i=0; i < numArfcns; i++)
    {
        if (arfcnList[i] == 0 || arfcnList[i] > 124)
        {
            DBG_ERROR("EncodeFreqListBitMap0: arfcn out of range %d\n", arfcnList[i]);
            return RLC_MAC_PARAMETER_RANGE_ERROR;
        }
        int octet, bit;
        octet = 15 - ((arfcnList[i]-1) / 8);
        bit = (arfcnList[i]-1) % 8;

        if (bit > 0) 
        {
            MAList[octet] |= 1 << bit;
        } else
        {
            MAList[octet] |= 0x01;
        }
    }
    // format ID and spare bits are 0s, no need to encode
    DBG_LEAVE();
    return RLC_MAC_SUCCESS;
}


FREQ_LIST_FORMAT_ID IeMAFreqList::SelectFrequencyListFormat()
{
    DBG_FUNC("IeMAFreqList::SelectFrequencyListFormat", RLC_MAC);
    DBG_ENTER();
    
    // find the min and max freq,
    unsigned short minArfcn = arfcnList[0];
    unsigned short maxArfcn = arfcnList[0];

    // find the min and max frequencies
    for (int i=1; i < numArfcns; i++)
    {
        if (arfcnList[i] < minArfcn) minArfcn = arfcnList[i];
        if (arfcnList[i] > maxArfcn) maxArfcn = arfcnList[i];
    }

    if( (maxArfcn - minArfcn) > 512) return FREQ_LIST_FORMAT_1024_RANGE;
    if( (maxArfcn - minArfcn) > 256) return FREQ_LIST_FORMAT_512_RANGE;
    if( (maxArfcn - minArfcn) > 128) return FREQ_LIST_FORMAT_256_RANGE;
    if( (maxArfcn - minArfcn) > 111) return FREQ_LIST_FORMAT_128_RANGE;
    DBG_LEAVE();
    return FREQ_LIST_FORMAT_VARIABLE_BIT_MAP;
}


// helper function to get the GREATEST_POWER_OF_2_LESSER_OR_EQUAL_TO(index)
int IeMAFreqList::GetGreatestPowerOf2LessorOrEqualTo(int index)
{
    DBG_FUNC("IeMAFreqList::GetGreatestPowerOf2LessorOrEqualTo", RLC_MAC);
    DBG_ENTER();
    
    int answer;
    for(answer=1;answer<=index;answer*=2);
    DBG_LEAVE();
    return answer/2;
}


// NOTE, freq-list encoding is an expensive operation,
// application user should preformat it once, and 
// store, and use the preformat msg to send 
//
//
// IMPORTANT!!! The index must start from 1 instead of 0. Effectively,
//              W[0] is NOT used. The result starts from W[1], W[2], ...
//              and so on.

void IeMAFreqList::EncodeFreqsSubTree(int index, int range,unsigned short *set,
    short numberInSet, unsigned short *W)
{
    DBG_FUNC("IeMAFreqList::EncodeFreqsSubTree", RLC_MAC);
    DBG_ENTER();
    
    if(numberInSet == 0)
    {
        W[index] = 0;
        DBG_LEAVE();
        return;
    }
    if(numberInSet == 1)
    {
        W[index] = 1 + set[0];
        DBG_LEAVE();
        return;
    }

    int half = range/2;
    int halfminus = ((range-1)/2);
    int halfSet = (numberInSet -1)/2;
    int n, j, i ;
    int pivotIndex=(-1);
    for(i=0; i < numberInSet; i++)
    {
        n = 0;
        for(j=0; j < numberInSet; j++)
        {
            if (((range + set[j] - set[i]) % range) <= halfminus)
                    n++;
        }
        if ((n-1) == halfSet)
        {
            // found the pivot point
            pivotIndex = i;
            break;
        }
    }
    if(pivotIndex<0)
    {
        // impossible. error in algorithm
        DBG_ERROR("IeMAFreqList: CNI_RIL3_EncodeFreqsSubTree() algorithm detect data set error. ARFCN list potential duplication\n");
    }
    W[index] = set[pivotIndex] + 1;


    // get the GREATEST_POWER_OF_2_LESSER_OR_EQUAL_TO(index)
    int power2s = GetGreatestPowerOf2LessorOrEqualTo(index);

    // encode the left side
    unsigned short subSet[18];
    unsigned short origValue = (set[pivotIndex] + halfminus + 1) % range;
    short subSetIndex = 0;
    for(i=0; i < numberInSet; i++)
    {
        if(((range + set[i] - origValue) % range) < half)
        {
            subSet[subSetIndex++] = (range + set[i]- origValue) % range;
        }
    }

    EncodeFreqsSubTree(index + power2s, range/2, subSet, subSetIndex, W);

    // encode the right side
    origValue = (set[pivotIndex] + 1) % range;
    subSetIndex = 0;
    for(i=0; i < numberInSet; i++)
    {
        if(((range + set[i] - origValue) % range) < half)
        {
            subSet[subSetIndex++] = (range + set[i]- origValue) % range;
        }
    }

    EncodeFreqsSubTree((index + 2 * power2s), (range -1)/2, subSet, subSetIndex, W);
    DBG_LEAVE();
}

void IeMAFreqList::GetFreqSetFromW(unsigned short origArfcn, int original_range, 
    unsigned short *W, short numberInW, unsigned short *set)
{
    DBG_FUNC("IeMAFreqList::GetFreqSetFromW", RLC_MAC);
    DBG_ENTER();
    
    int index, K;
    int range;
    unsigned short N;

    for(K=1;K<=numberInW; K++)
    {
        index = K;
        range = original_range /  GetGreatestPowerOf2LessorOrEqualTo(index);
        N = W[index] - 1;
        while(index>1)
        {
            range = 2*range+1;
            if(2*index<3*GetGreatestPowerOf2LessorOrEqualTo(index))
            {
                // left child
                index = index - GetGreatestPowerOf2LessorOrEqualTo(index)/2;
                N = (N+W[index]-1+(range-1)/2+1) % range;
            } else {
                // right child
                index = index - GetGreatestPowerOf2LessorOrEqualTo(index);
                N = (N+W[index]-1+1) % range;
            }
        }
        set[K]=origArfcn+N+1;
    }
    DBG_LEAVE();
}

void IeMAFreqList::SetWbits(int firstNumBits, int arrayLength, unsigned short *Wbits)
{
    DBG_FUNC("IeMAFreqList::SetWbits", RLC_MAC);
    DBG_ENTER();
    
    // generate the Wbits[]
    memset(&Wbits[1], 0, (sizeof(short)*arrayLength));
    Wbits[1] = firstNumBits;
    int i, k;
    int powerk = 1;
    for (k = 1, i=2; (i<=arrayLength) && (k < firstNumBits); k++)
    {
        powerk *=2;
        for(i= powerk; 
            (i <= arrayLength) && (i <= (powerk*2)-1); 
            i++
            )
        {
            Wbits[i] = firstNumBits-k;
        }
    }
    DBG_LEAVE();
}


// Set the reducedArfcn array equal to arfcn array of all freq except the
// F0, minus F0 + 1
// return F0
unsigned short IeMAFreqList::ReduceMinArfcn(int numFreq, unsigned short *reducedArfcn)
{
    DBG_FUNC("IeMAFreqList::ReduceMinArfcn", RLC_MAC);
    DBG_ENTER();
    
    // find the min freq,
    unsigned short minArfcn = arfcnList[0];
    int i;
    for (i=1; i < numFreq; i++)
    {
        if (arfcnList[i] < minArfcn)
        {
            minArfcn = arfcnList[i];
        }
    }
    
    int numInSet = 0;
    for (i=0; i < numFreq; i++)
    {
        if (arfcnList[i] > minArfcn)
        {
            reducedArfcn[numInSet++] = arfcnList[i] - minArfcn - 1;
        }
    }
    DBG_LEAVE();
    return minArfcn;
}

void IeMAFreqList::StuffArray(
    int arrayLength, 
    unsigned short *W, 
    unsigned short *Wbits,
    int &bufferLength,
    int startBit,   // 1-8; 1:lsb, 8:msb, msbs before startBit have to be preserved
                    // startBit is advanced to the next available bit before return
    unsigned char *buffer)
{
    DBG_FUNC("IeMAFreqList::StuffArray", RLC_MAC);
    DBG_ENTER();
    
    unsigned long pipe;
    int pipebits;
    pipebits = 8 - startBit;

    if(pipebits > 0)
    {
        // set up the initial value in the pipe which is stored in buffer[0] msbs
        pipe = buffer[0] >> startBit;
    }

    int maxBufferLength = bufferLength;
    int bufferIndex = 0;
    unsigned char value;
    int i = 0;
    while ((i < arrayLength) &&  (bufferIndex < maxBufferLength))
    {
        
        // Wbits[] can NOT be greater than 24. If this is not hold true
        // the algorithm will not pipe.
        
        if((pipebits + Wbits[i])<=32)
        {
            pipe = (pipe << Wbits[i]) + W[i];
            pipebits +=Wbits[i];
            i++;
            continue;   // while loop
        }
        
        // need to output byte because not enough to hold the next entry
        if(pipebits < 8)
        {
            DBG_ERROR("IeMAFreqList::StuffArray error, pipebits < 8\n");
            return;
        }
        
        value = (unsigned char )(pipe >> (pipebits-8));
        buffer[bufferIndex++] = value;
        pipe = pipe - (value << (pipebits-8));
        pipebits = pipebits - 8;
    }
    
    // at this point, we shall have no more W[] to encode. We shall start to empty
    // the data in the pipe 
    
    while(pipebits>=8)  // empty the remaining whole bytes in the pipe
    {
        value = (unsigned char)(pipe >> (pipebits-8));
        buffer[bufferIndex++] = value;
        pipe = pipe - (value << (pipebits-8));
        pipebits = pipebits -8;
    }
    
    if(pipebits>0)  // clean up the last fractional byte
    {
        buffer[bufferIndex++] = (unsigned char)(pipe << (8-pipebits));
    }

    bufferLength = bufferIndex; 
    DBG_LEAVE();
    return;
}


void IeMAFreqList::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tMA Freq List");
        for (int i = 0; i < numArfcns; i++)
        {
            if ((i % 5) == 0) outObj->Trace("\n\t\t");
            outObj->Trace("%4d", arfcnList[i]);
        }
        outObj->Trace("\n");    
    }  
}


