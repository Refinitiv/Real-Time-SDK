/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "limits.h"

#include "rtr/rsslTypes.h"

#include "rtr/rsslRmtes.h"
#include "rtr/rsslCharSet.h"
#include "rtr/rsslcnvtab.h"
#include "rtr/retmacros.h"

const char ESC_CHAR = 0x1B;
const char LBRKT_CHAR = 0x5B;
const char RHPA_CHAR = 0x60; 	/* Used for partial updates */
const char RREP_CHAR = 0x62;	/* Repeat command */
const char CSI_CHAR = 0x9B;
const char LS0_CHAR = 0x0F;
const char LS1_CHAR = 0x0E;
const char SS2_CHAR = 0x8E;
const char SS3_CHAR = 0x8F;

typedef enum {
	ERROR = 				-1,
	NORMAL = 				0,
	ESC = 					1,
	LBRKT = 				3,
	RHPA = 					4,
	RREP = 					5,
	ESC_21 = 				6,
	ESC_22 = 				7,
	ESC_24 = 				8,
	ESC_24_28 = 			9,
	ESC_24_29 = 			10,
	ESC_24_2A = 			11,
	ESC_24_2B = 			12,
	ESC_25 =				13,
	ESC_26 =				14,
	ESC_26_40 = 			15,
	ESC_26_40_ESC =			16,
	ESC_26_40_ESC_24 = 		17,
	ESC_26_40_ESC_24_29 = 	18,
	ESC_26_40_ESC_24_2A = 	19,
	ESC_26_40_ESC_24_2B =	20,
	ESC_28 = 				21,
	ESC_29 = 				22,
	ESC_2A = 				23,
	ESC_2B = 				24
} RMTESParseState;

typedef enum {
	TYPE_RMTES = 1,
	TYPE_UTF8 = 2,
} EncodeType;

int GLLowest(RsslRmtesCharSet* set)
{
	return (set->_shape == SHAPE_96) ? 0x20 : 0x21;
}
int GLHighest(RsslRmtesCharSet* set)
{
	return (set->_shape == SHAPE_96) ? 0x7F : 0x7E;
}

int GRLowest(RsslRmtesCharSet* set)
{
	return (set->_shape == SHAPE_96) ? 0xA0 : 0xA1;
}

int GRHighest(RsslRmtesCharSet* set)
{
	return (set->_shape == SHAPE_96) ? 0xFF : 0xFE;
}

int isGLChar(unsigned char inChar, RsslRmtesCharSet * set)
{
	return ((GLLowest(set) <= inChar) && 
			 (inChar <= GLHighest(set)) );
}

int isGRChar(unsigned char inChar, RsslRmtesCharSet * set)
{
	return ((GRLowest(set) <= inChar) && 
			 (inChar <= GRHighest(set)) );
}

/* Used to get the UCS2 encoded character from the first table.
  Precondition: set is a singleton character table(not a _stride of 2)
				inChar is a valid character(checked with isGRChar/isGLChar
*/
unsigned short GLConvertSingle1(unsigned char inChar, RsslRmtesCharSet * set)
{
	if(set->_table1)
		return set->_table1[inChar - GLLowest(set)];
	else
		return inChar;
}

/* Used to get the UCS2 encoded character from the second table.
  Precondition: set is a singleton character table(not a _stride of 2)
				inChar is a valid character(checked with isGRChar/isGLChar
				_table2 exists in the set
*/
unsigned short GLConvertSingle2(unsigned char inChar, RsslRmtesCharSet * set)
{
	return set->_table2[inChar - GLLowest(set)];
}

/* Used to get the UCS2 encoded character from the first table.
  Precondition: set is a singleton character table(not a _stride of 2)
				inChar is a valid character(checked with isGRChar/isGLChar
*/
unsigned short GRConvertSingle1(unsigned char inChar, RsslRmtesCharSet * set)
{
	if(set->_table1)
		return set->_table1[inChar - GRLowest(set)];
	else
		return inChar;
}

/* Used to get the UCS2 encoded character from the second table.
  Precondition: set is a singleton character table(not a _stride of 2)
				inChar is a valid character(checked with isGRChar/isGLChar
				_table2 exists in the set
*/
unsigned short GRConvertSingle2(unsigned char inChar, RsslRmtesCharSet * set)
{
	return set->_table2[inChar - GRLowest(set)];
}

unsigned short ConvertStride2GL(unsigned char inChar1, unsigned char inChar2, RsslRmtesCharSet* set)
{
	RsslUInt32 mapIndex;

	mapIndex = (inChar1 - GLLowest(set))*(94) + (inChar2 - GLLowest(set));

	if(mapIndex < set->_table1_length)
	{
		return set->_table1[mapIndex];
	}
	else if(set->_table2 && mapIndex >= set->_table2_start && mapIndex < (set->_table2_start + set->_table2_length))
	{
		return set->_table2[mapIndex - set->_table2_start];
	}
	else
		return 0xFFFD;
}

unsigned short ConvertStride2GR(unsigned char inChar1, unsigned char inChar2, RsslRmtesCharSet* set)
{
	RsslUInt32 mapIndex;

	mapIndex = (inChar1 - GRLowest(set))*(94) + (inChar2 - GRLowest(set));

	if(mapIndex < set->_table1_length)
	{
		return set->_table1[mapIndex];
	}
	else if(set->_table2 && mapIndex >= set->_table2_start && mapIndex < (set->_table2_start + set->_table2_length))
	{
		return set->_table2[mapIndex - set->_table2_start];
	}
	else
		return 0xFFFD;
}

int UCS2ToUTF8(unsigned short UCS_char, char* iter, char* endChar)
{
	if (UCS_char < 0x0080)
	{
		if ( iter < endChar )
		{
			*iter++ = (unsigned char)UCS_char;
			return 1;
		}
	}
	else if (UCS_char < 0x0800)
	{
		if ( (iter + 1) < endChar )
		{
			*iter++ = 0x0C0 | (UCS_char >> 6);
			*iter++ = 0x080 | (UCS_char & 0x3F);
			return 2;
		}
	}
	else	/* uni_char is 0x0800 - 0xFFFF */
	{
		if( ( iter+2) < endChar)
		{
			*iter++ = 0x0E0 | (UCS_char >> 12);
			*iter++ = 0x080 | ((UCS_char >> 6) & 0x3F);
			*iter++ = 0x080 | (UCS_char & 0x3F);

			return 3;
		}
	}

	return 0;
}

int UTF8ToUCS2(unsigned char *UTF_char, RsslUInt16* iter, unsigned char* endChar)
{
	if(UTF_char[0] < 0x80)
	{
		*iter = UTF_char[0];
		return 1;
	}
	else if(((UTF_char[0] & 0xE0) == 0xE0) )
	{
		if((UTF_char+2) < endChar)
		{
			if(UTF_char[1] == 0)
				return -1;
		
			*iter = ((UTF_char[0] & 0x0F) << 12 | 
					(UTF_char[1] & 0x3F) << 6 |
					(UTF_char[2] & 0x3F));
		
			return 3;
		}
	}
	else if(((UTF_char[0] & 0xC0) == 0xC0) )
	{
		if((UTF_char+1) < endChar)
		{
			if(UTF_char[1] == 0)
				return 0;
		
			*iter = ((UTF_char[0] & 0x1F) << 6 |
					(UTF_char[1] & 0x3F));
				
			return 2;
		}
	}
	
	return 0;
}

/*	Parses the characters for a control group sequence for conversion ( first character is between 0x00 and 0x1F(CL) or between 0x70 and 0x8F(CR) ) 
	If error returned, this means that the sequence is either invalid, or contains a partial update/repeat character sequence (these should be removed from the buffer with the applyToCache function)
	Otherwise, returns codes for success if a working set change is correctly appiled, or that the next character is a shift value.
*/
int controlParse(unsigned char* curPtr, unsigned char* endPtr, RsslRmtesWorkingSet* currentSet, ESCReturnCode *retCode)
{
	RMTESParseState state = NORMAL;
	RMTESParseState newState = NORMAL;
	int length = 0;
	unsigned char* iIter = curPtr;
	
	if(*iIter == 0x00)/* ignore NULL characters for this parse */
	{
		*retCode = ESC_SUCCESS;
		return 1;
	}
	else if(*iIter == 0x0F)
	{
		currentSet->GL = &(currentSet->G0);
		*retCode = ESC_SUCCESS;
		return 1;
	}
	else if(*iIter == 0x0E)
	{
		currentSet->GL = &(currentSet->G1);
		*retCode = ESC_SUCCESS;
		return 1;
	}
	else if(*iIter == 0x1B)
	{
		state = ESC;
		length++;
	}
	else if(*iIter == 0x1C)
	{
		*retCode = END_CHAR;
		return 0;
	}
	else
	{
		*retCode = ESC_SUCCESS;
		return 0;
	}
	
	do
	{
		iIter++;
		length++;
		
		newState = NORMAL;
		switch(state)
		{
			case ESC:
				if(*iIter == 0x21)
					newState = ESC_21;
				else if(*iIter == 0x22)
					newState = ESC_22;
				else if(*iIter == 0x24)
					newState = ESC_24;
				else if(*iIter == 0x25)
					newState = ESC_25;
				else if(*iIter == 0x26)
					newState = ESC_26;
				else if(*iIter == 0x28)
					newState = ESC_28;
				else if(*iIter == 0x29)
					newState = ESC_29;
				else if(*iIter == 0x2A)
					newState = ESC_2A;
				else if(*iIter == 0x2B)
					newState = ESC_2B;
				else if(*iIter == LBRKT_CHAR)
					newState = LBRKT;
				else if(*iIter == 0x6E)
					currentSet->GL = &currentSet->G2;
				else if(*iIter == 0x6F)
					currentSet->GL = &currentSet->G3;
				else if(*iIter == 0x7E)
					currentSet->GR = &currentSet->G1;
				else if(*iIter == 0x7D)
					currentSet->GR = &currentSet->G2;
				else if(*iIter == 0x7C)
					currentSet->GR = &currentSet->G3;
				else
				{
					*retCode = ESC_ERROR;
					return 0;
				}
				break;
			case ESC_21:
				if(*iIter != 0x40)  /* Refinitiv Ctrl 1 to CL */
				{
					*retCode = ESC_ERROR;
					return 0;
				}
				break;
			case ESC_22:
				if(*iIter != 0x30)	/* Refinitiv Ctrl 2 to CR */
				{
					*retCode = ESC_ERROR;
					return 0;
				}
				break;
			case ESC_24:
				if(*iIter == 0x28)
					newState = ESC_24_28;
				else if(*iIter == 0x29)
					newState = ESC_24_29;
				else if(*iIter == 0x2A)
					newState = ESC_24_2A;
				else if(*iIter == 0x2B)
					newState = ESC_24_2B;
				else
				{
					*retCode = ESC_ERROR;
					return 0;
				}
				break;
			case ESC_24_28:
				if(*iIter == 0x47)
					currentSet->G0 = (RsslRmtesCharSet *)&_rsslChinese1;
				else if(*iIter == 0x48)
					currentSet->G0 = (RsslRmtesCharSet *)&_rsslChinese2;
				else
				{
					*retCode = ESC_ERROR;
					return 0;
				}
				break;
			case ESC_24_29:
				if(*iIter == 0x47)
					currentSet->G1 = (RsslRmtesCharSet *)&_rsslChinese1;
				else if(*iIter == 0x48)
					currentSet->G1 = (RsslRmtesCharSet *)&_rsslChinese2;
				else
				{
					*retCode = ESC_ERROR;
					return 0;
				}
				break;
			case ESC_24_2A:
				if(*iIter == 0x47)
					currentSet->G2 = (RsslRmtesCharSet *)&_rsslChinese1;
				else if(*iIter == 0x35)
					currentSet->G2 = (RsslRmtesCharSet *)&_rsslChinese1;
				else if(*iIter == 0x48)
					currentSet->G2 = (RsslRmtesCharSet *)&_rsslChinese2;
				else
				{
					*retCode = ESC_ERROR;
					return 0;
				}
				break;
			case ESC_24_2B:
				if(*iIter == 0x47)
					currentSet->G3 = (RsslRmtesCharSet *)&_rsslChinese1;
				else if(*iIter == 0x48)
					currentSet->G3 = (RsslRmtesCharSet *)&_rsslChinese2;
				else if(*iIter == 0x36)
					currentSet->G3 = (RsslRmtesCharSet *)&_rsslChinese2;
				else if(*iIter == 0x34)
					currentSet->G3 = (RsslRmtesCharSet *)&_rsslJapaneseKanji;
				else
				{
					*retCode = ESC_ERROR;
					return 0;
				}
				break;
			case ESC_25:
				if(*iIter == 0x30)
				{
					*retCode = UTF_ENC;
					return length;
				}
				else
				{
					*retCode = ESC_ERROR;
					return 0;
				}
				break;
			case ESC_26:
				if(*iIter == 0x40)
					newState = ESC_26_40;
				else
				{
					*retCode = ESC_ERROR;
					return 0;
				}
				break;
			case ESC_26_40:
				if(*iIter == ESC_CHAR)
					newState = ESC_26_40_ESC;
				else
				{
					*retCode = ESC_ERROR;
					return 0;
				}
				break;
			case ESC_26_40_ESC:
				if(*iIter == 0x24)
					newState = ESC_26_40_ESC_24;
				else
				{
					*retCode = ESC_ERROR;
					return 0;
				}
				break;
			case ESC_26_40_ESC_24:
				if(*iIter == 0x42)
					currentSet->G0 = (RsslRmtesCharSet *)&_rsslJapaneseKanji;
				else if(*iIter == 0x29)
					newState = ESC_26_40_ESC_24_29;
				else if(*iIter == 0x2A)
					newState = ESC_26_40_ESC_24_2A;
				else if(*iIter == 0x2B)
					newState = ESC_26_40_ESC_24_2B;
				else
				{
					*retCode = ESC_ERROR;
					return 0;
				}
				break;
			case ESC_26_40_ESC_24_29:
				if(*iIter == 0x42)
					currentSet->G1 = (RsslRmtesCharSet *)&_rsslJapaneseKanji;
				else
				{
					*retCode = ESC_ERROR;
					return 0;
				}
				break;
			case ESC_26_40_ESC_24_2A:
				if(*iIter == 0x42)
					currentSet->G2 = (RsslRmtesCharSet *)&_rsslJapaneseKanji;
				else
				{
					*retCode = ESC_ERROR;
					return 0;
				}
				break;
			case ESC_26_40_ESC_24_2B:
				if(*iIter == 0x42)
					currentSet->G3 = (RsslRmtesCharSet *)&_rsslJapaneseKanji;
				else
				{
					*retCode = ESC_ERROR;
					return 0;
				}
				break;
			case ESC_28:
				if(*iIter == 0x42)
					currentSet->G0 = (RsslRmtesCharSet *)&_rsslReuterBasic1;
				else if(*iIter == 0x49)
					currentSet->G0 = (RsslRmtesCharSet *)&_rsslJapaneseKatakana;
				else if(*iIter == 0x4A)
					currentSet->G0 = (RsslRmtesCharSet *)&_rsslJapaneseLatin;
				else
				{
					*retCode = ESC_ERROR;
					return 0;
				}
				break;
			case ESC_29:
				if(*iIter == 0x31)
					currentSet->G1 = (RsslRmtesCharSet *)&_rsslReuterBasic2;
				else if(*iIter == 0x42)
					currentSet->G1 = (RsslRmtesCharSet *)&_rsslReuterBasic1;
				else if(*iIter == 0x49)
					currentSet->G1 = (RsslRmtesCharSet *)&_rsslJapaneseKatakana;
				else if(*iIter == 0x4A)
					currentSet->G1 = (RsslRmtesCharSet *)&_rsslJapaneseLatin;
				else
				{
					*retCode = ESC_ERROR;
					return 0;
				}
				break;
			case ESC_2A:
				if(*iIter == 0x32)
					currentSet->G2 = (RsslRmtesCharSet *)&_rsslJapaneseKatakana;
				else
				{
					*retCode = ESC_ERROR;
					return 0;
				}
				break;
			case ESC_2B:
				if(*iIter == 0x33)
					currentSet->G3 = (RsslRmtesCharSet *)&_rsslJapaneseLatin;
				else
				{
					*retCode = ESC_ERROR;
					return 0;
				}
				break;
			case ERROR:
			case NORMAL:
			case LBRKT:
			case RHPA:
			case RREP:
			  break;
		}
		state = newState;
		
	} while(iIter <= endPtr && newState != NORMAL);
	
	if(state == NORMAL)
	{
		*retCode = ESC_SUCCESS;
		return length;
	}
	else
	{
		*retCode = ESC_ERROR;
		return 0;
	}
}
	
/* Assumptions: all updates have already been applied to the RMTES buffer... there should be no repeat or cursor move commands */
RSSL_API RsslRet rsslRMTESToUTF8(RsslRmtesCacheBuffer *pRmtesBuffer, RsslBuffer *pStringBuffer)
{
	RMTESParseState state = NORMAL;
	ESCReturnCode retCode;
	int ret;
	int tempRet;
	unsigned char* inIter;
	char* outIter;
	unsigned char* endInput;
	char* endOutput;
	EncodeType encType = TYPE_RMTES;

	unsigned char* tempChar;
	unsigned short tempShort;
	
	RsslRmtesWorkingSet curWorkingSet;
	RsslRmtesCharSet * shiftGL = NULL;
	RsslRmtesCharSet * tmpGL;

	RSSL_ASSERT(pRmtesBuffer != NULL, Invalid parameter passed as NULL);
	RSSL_ASSERT(pStringBuffer != NULL, Invalid parameter passed as NULL);
	
	if (pRmtesBuffer->data == NULL || pRmtesBuffer->allocatedLength == 0)
		return RSSL_RET_INVALID_DATA;

	if (pStringBuffer->data == NULL || pStringBuffer->length == 0)
		return RSSL_RET_BUFFER_TOO_SMALL;

	inIter = (unsigned char*)pRmtesBuffer->data;
	outIter = pStringBuffer->data;
	endInput = (unsigned char*)pRmtesBuffer->data + pRmtesBuffer->length;
	endOutput = pStringBuffer->data + pStringBuffer->length;
	
	initWorkingSet(&curWorkingSet);
	
	retCode = ESC_SUCCESS;
	
	while(inIter < endInput && state != ERROR)
	{
		if(encType == TYPE_RMTES)
		{
			if(*inIter < 0x20)  /* CL character */
			{
				if(shiftGL != NULL)
					return RSSL_RET_FAILURE;
				else
				if((ret = controlParse(inIter, endInput, &curWorkingSet, &retCode)) == 0)
				{
					if(retCode == ESC_ERROR)
						return RSSL_RET_FAILURE;
					else if(retCode == END_CHAR) /* Parse is done, return success after triming data length */
					{
						pStringBuffer->length = (rtrUInt32)(outIter - pStringBuffer->data);
						return RSSL_RET_SUCCESS;
					}
					else if(retCode == ESC_SUCCESS)
					{
						tempRet = UCS2ToUTF8((unsigned short)*inIter, (char*)outIter, (char*)endOutput);
						if(tempRet == 0)
							return RSSL_RET_BUFFER_TOO_SMALL;
						else
							outIter += tempRet;

						inIter++;
					}
				}
				else
				{
					inIter += ret;
					if(retCode == UTF_ENC)
						encType = TYPE_UTF8;
				}
			}
			else if ((*curWorkingSet.GL)->_shape == SHAPE_94 && *inIter == 0x20)
			/* Space character, if 94 character set */
			{
				tempRet = UCS2ToUTF8((unsigned short)*inIter, outIter, endOutput);
				if(tempRet == 0)
					return RSSL_RET_BUFFER_TOO_SMALL;
				else
					outIter += tempRet;

				inIter++;
			}
			else if ((*curWorkingSet.GL)->_shape == SHAPE_94 && *inIter == 0x7F) /* delete character, if 94 character set */
			{
				tempRet = UCS2ToUTF8((unsigned short)0xFFFD, outIter, endOutput);
				
				if (tempRet == 0)
					return RSSL_RET_BUFFER_TOO_SMALL;
				else
					outIter += tempRet;

				inIter++;
			}
			else if(*inIter < 0x80) /* GL character */
			{
				if(shiftGL)
					tmpGL = shiftGL;
				else
					tmpGL = *curWorkingSet.GL;
				
				if(isGLChar(*inIter, tmpGL))
				{
					if(tmpGL->_stride == 2)
					{
						tempChar = inIter;
						if(inIter < endInput)
						{
							inIter++;
							if(isGLChar(*inIter, tmpGL) )
							{
								tempRet = UCS2ToUTF8(ConvertStride2GL(*tempChar, *inIter, tmpGL), outIter, endOutput);
								if(tempRet == 0)
									return RSSL_RET_BUFFER_TOO_SMALL;
								else
									outIter += tempRet;
							}
							else
							{
								return RSSL_RET_FAILURE;
							}
						}
						else
							return RSSL_RET_FAILURE;
					}
					else
					{
						tempRet = UCS2ToUTF8(GLConvertSingle1(*inIter, tmpGL), outIter, endOutput);
						if(tempRet == 0)
							return RSSL_RET_BUFFER_TOO_SMALL;
						else
							outIter += tempRet;
						
						if(tmpGL->_table2)
						{
							tempShort = GLConvertSingle2(*inIter, tmpGL);
							if(tempShort != 0)
							{
								tempRet = UCS2ToUTF8(tempShort, outIter, endOutput);
								if(tempRet == 0)
									return RSSL_RET_BUFFER_TOO_SMALL;
								else
									outIter += tempRet;
							}
						}
					}
				}
				else
				{
					return RSSL_RET_FAILURE;
				}

				shiftGL = NULL;
				inIter++;
			}
			else if(*inIter < 0xA0)  /* CR Character set */
			{
				if(shiftGL)
					return RSSL_RET_FAILURE;
				else if(*inIter == 0x8E)
					shiftGL = curWorkingSet.G2;
				else if(*inIter == 0x8F)
					shiftGL = curWorkingSet.G3;
				else
				{
					tempRet = UCS2ToUTF8(0xFFFD, outIter, endOutput);
					if(tempRet == 0)
						return RSSL_RET_BUFFER_TOO_SMALL;
					else
						outIter += tempRet;
				}

				inIter++;
			}
			else
			{
				if(shiftGL)
					return RSSL_RET_FAILURE;
				else if(isGRChar(*inIter, *curWorkingSet.GR))
				{
					if((*curWorkingSet.GR)->_stride == 2)
					{
						tempChar = inIter;
						if(inIter < endInput)
						{
							inIter++;
							if(isGRChar(*inIter, (*curWorkingSet.GR)) )
							{
								tempRet = UCS2ToUTF8(ConvertStride2GR(*tempChar, *inIter, *curWorkingSet.GR), outIter, endOutput);
								if(tempRet == 0)
									return RSSL_RET_BUFFER_TOO_SMALL;
								else
									outIter += tempRet;
							}
							else
							{
								tempRet = UCS2ToUTF8(0xFFFD, outIter, endOutput);
								if(tempRet == 0)
									return RSSL_RET_BUFFER_TOO_SMALL;
								else
									outIter += tempRet;
							}
						}
						else
							return RSSL_RET_FAILURE;
					}
					else
					{
						tempRet = UCS2ToUTF8(GRConvertSingle1(*inIter, *curWorkingSet.GR), outIter, endOutput);
						if(tempRet == 0)
							return RSSL_RET_BUFFER_TOO_SMALL;
						else
							outIter += tempRet;
						
						if((*curWorkingSet.GR)->_table2)
						{
							tempShort = GRConvertSingle2(*inIter, *curWorkingSet.GR);
							if(tempShort != 0)
							{
								tempRet = UCS2ToUTF8(tempShort, outIter, endOutput);
								if(tempRet == 0)
									return RSSL_RET_BUFFER_TOO_SMALL;
								else
									outIter += tempRet;
							}
						}
					}
				}
				else
				{
					return RSSL_RET_FAILURE;
				}

				inIter++;
			}
		}
		else /* UTF8 Encode */
		{
			if(*inIter == 0x1B) /* Escape control character */
			{
				if((ret = controlParse(inIter, endInput, &curWorkingSet, &retCode)) == 0)
				{
					return RSSL_RET_FAILURE;
				}
				else
					inIter += ret;
			}
			else /* Just copy the data, since it's already encoded in UTF8 */
			{
				*outIter = *inIter;
				outIter++;
				inIter++;
			}
		}
	}

	/* Trim output buffer length */
	
	pStringBuffer->length = (rtrUInt32)(outIter - pStringBuffer->data);
	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslRMTESToUCS2(RsslRmtesCacheBuffer *pRmtesBuffer, RsslU16Buffer *pShortBuffer)
{
	RMTESParseState state = NORMAL;
	ESCReturnCode retCode;
	int ret;
	unsigned char* inIter;
	unsigned short* outIter;
	unsigned char* endInput;
	unsigned short* endOutput;
	EncodeType encType = TYPE_RMTES;

	unsigned char* tempChar;
	unsigned short tempShort;
	
	RsslRmtesWorkingSet curWorkingSet;
	RsslRmtesCharSet * shiftGL = NULL;
	RsslRmtesCharSet * tmpGL = NULL;

	RSSL_ASSERT(pRmtesBuffer != NULL, Invalid parameter passed as NULL);
	RSSL_ASSERT(pShortBuffer != NULL, Invalid parameter passed as NULL);
	
	if (pRmtesBuffer->data == NULL || pRmtesBuffer->allocatedLength == 0)
		return RSSL_RET_INVALID_DATA;

	if (pShortBuffer->data == NULL || pShortBuffer->length == 0)
		return RSSL_RET_BUFFER_TOO_SMALL;

	inIter = (unsigned char*)pRmtesBuffer->data;
	outIter = (unsigned short*)pShortBuffer->data;
	endInput = (unsigned char*)(pRmtesBuffer->data + pRmtesBuffer->length);
	endOutput = (unsigned short*)pShortBuffer->data + pShortBuffer->length;

	initWorkingSet(&curWorkingSet);
	
	retCode = ESC_SUCCESS;
	
	while(inIter < endInput && outIter < endOutput && state != ERROR)
	{
		if(encType == TYPE_RMTES)
		{
			if(*inIter < 0x20)  /* CL character */
			{
				if(shiftGL != NULL)
					return RSSL_RET_FAILURE;
				else if((ret = controlParse(inIter, endInput, &curWorkingSet, &retCode)) == 0)
				{
					if(retCode == ESC_ERROR)
					{
						return RSSL_RET_FAILURE;
					}
					else if(retCode == END_CHAR) /* Parse is done, return success after triming data length */
					{
						pShortBuffer->length = (rtrUInt32)(outIter - pShortBuffer->data);
						return RSSL_RET_SUCCESS;
					}
					else if(retCode == ESC_SUCCESS)
					{
						*outIter = (unsigned short)*inIter;
						outIter++;
						inIter++;
					}
				}
				else
				{
					inIter += ret;
					if(retCode == UTF_ENC)
						encType = TYPE_UTF8;
				}
			}
			else if((*curWorkingSet.GL)->_shape == SHAPE_94 && *inIter == 0x20) /* Space character, if 94 character set */
			{
				*outIter = 0x20;
				outIter++;
				inIter++;
			}
			else if((*curWorkingSet.GL)->_shape == SHAPE_94 && *inIter == 0x7F) /* delete character, if 94 character set */
			{
				*outIter = 0xFFFD;
				outIter++;
				inIter++;
			}
			else if(*inIter < 0x80) /* GL character */
			{
				if(shiftGL)
					tmpGL = shiftGL;
				else
					tmpGL = *curWorkingSet.GL;
				
				if(isGLChar(*inIter, tmpGL))
				{
					if(tmpGL->_stride == 2)
					{
						tempChar = inIter;
						if(inIter < endInput)
						{
							inIter++;
							if(isGLChar(*inIter, tmpGL) )
							{
								*outIter = ConvertStride2GL(*tempChar, *inIter, tmpGL);
								
								outIter++;
								
								if(outIter > endOutput)
									return RSSL_RET_BUFFER_TOO_SMALL;
							}
							else
							{
								return RSSL_RET_FAILURE;
							}
								
						}
						else
							return RSSL_RET_FAILURE;
					}
					else
					{
						*outIter = GLConvertSingle1(*inIter, tmpGL);
						outIter++;
						
						if(tmpGL->_table2)
						{
							if(outIter > endOutput)
								return RSSL_RET_BUFFER_TOO_SMALL;
							
							tempShort = GLConvertSingle2(*inIter, tmpGL);

							if(tempShort != 0)
							{
								*outIter = tempShort;
								outIter++;
							}
						}
					}
				}
				else
				{
					return RSSL_RET_FAILURE;
				}

				inIter++;
				shiftGL = NULL;
			}
			else if(*inIter < 0xA0)  /* CR Character set */
			{
				if(shiftGL)
					return RSSL_RET_FAILURE;
				
				if(*inIter == 0x8E)
					shiftGL = curWorkingSet.G2;
				else if(*inIter == 0x8F)
					shiftGL = curWorkingSet.G3;
				else
				{
					*outIter = 0xFFFD;
					outIter++;
				}

				inIter++;
			}
			else
			{
				if(shiftGL)
					return RSSL_RET_FAILURE;
				else if(isGRChar(*inIter, *curWorkingSet.GR))
				{
					if((*curWorkingSet.GR)->_stride == 2)
					{
						tempChar = inIter;
						if(inIter < endInput)
						{
							inIter++;
							if(isGRChar(*inIter, (*curWorkingSet.GR)) )
							{
								*outIter = ConvertStride2GR(*tempChar, *inIter, *curWorkingSet.GR);
								outIter++;

								if(outIter > endOutput)
									return RSSL_RET_BUFFER_TOO_SMALL;
							}
						}
						else
							return RSSL_RET_FAILURE;
					}
					else
					{
						*outIter = GRConvertSingle1(*inIter, *curWorkingSet.GR);
						outIter++;
						if(outIter > endOutput)
								return RSSL_RET_BUFFER_TOO_SMALL;
						
						if((*curWorkingSet.GR)->_table2)
						{
							tempShort = GRConvertSingle2(*inIter, *curWorkingSet.GR);
							if(tempShort != 0)
							{
								*outIter = tempShort;
								outIter++;

								if(outIter > endOutput)
									return RSSL_RET_BUFFER_TOO_SMALL;
							}
						}
					}
				}
				else
					return RSSL_RET_FAILURE;

				inIter++;
			}
		}
		else /* UTF8 Encode */
		{
			if(*inIter == 0x1B) /* Escape control character */
			{
				if((ret = controlParse(inIter, endInput, &curWorkingSet, &retCode)) < 0)
				{
					return RSSL_RET_FAILURE;
				}
				else
				{
					if(ret == 0)
						inIter++;
					else
						inIter += ret;
				}
			}
			else /* Just copy the data, since it's already encoded in UTF8 */
			{
				ret = UTF8ToUCS2(inIter, outIter, endInput);
				outIter++;
				if(ret == 0)
					inIter++;
				else
					inIter += ret;
			}
		}
	}

	/* Trim output buffer length */
	
	pShortBuffer->length = (rtrUInt32)(outIter - pShortBuffer->data);
	return RSSL_RET_SUCCESS;
}

/*	Handles the following cases:
 *		1) ESC LBRKT NumericNumber(base 10) RHPA - Cursor command
 *		2) ESC LBRKT NumericNumber(base 10) RREP - Repeat command for previous character
 */
RSSL_API RsslBool rsslHasPartialRMTESUpdate(RsslBuffer *pBuffer)
{
	rtrUInt32 i;
	RMTESParseState state = NORMAL;

	RSSL_ASSERT(pBuffer != NULL, Invalid parameter passed as NULL);

	if (pBuffer->data == NULL || pBuffer->length == 0)
		return RSSL_FALSE;
	
	for(i = 0; i < pBuffer->length; i++)
	{
		switch(state)
		{
			case NORMAL:
				if(pBuffer->data[i] == ESC_CHAR)
					state = ESC;
				/* This appears to break in Korean encodings... not sure if it's still valid for current feeds */
				//else if(pBuffer->data[i] == CSI_CHAR)
				//	state = LBRKT;
				break;
			case ESC:
				if(pBuffer->data[i] == LBRKT_CHAR)
					state = LBRKT;
				else if(pBuffer->data[i] == ESC_25)
					return RSSL_FALSE; /* Error.  UTF8 sequence should not be before command */
				else
					state = NORMAL;
				break;
			case LBRKT:
			    if (pBuffer->data[i] >= '0' && pBuffer->data[i] <= '9')
				  break;
				if (pBuffer->data[i] == RHPA_CHAR || pBuffer->data[i] == RREP_CHAR)
				  return RSSL_TRUE;
				return RSSL_FALSE; /* Error */
				break;
			default:
				return RSSL_FALSE;
		}
	}
	
	return RSSL_FALSE;
}
	
/* Applies the inBuffer's partial update data to the outBuffer.
 * Preconditions: outBuffer is large enough to handle the additional data
 *                outBuffer has already been populated with data
 * Result: inBuffer's partial update(s) are applied to outBuffer
 */
RSSL_API RsslRet rsslRMTESApplyToCache(RsslBuffer *inBuffer, RsslRmtesCacheBuffer *cacheBuf)
{
	RsslUInt32 inBufPos = 0;
	RsslUInt32 cacheBufPos = 0;
	RsslUInt32 numCount = 0;
	char prevChar;
	RsslUInt32 escStart;
	RsslBool escFirst = RSSL_FALSE;
	RMTESParseState state = NORMAL;
	rtrUInt32 maxLen = 0;

	RSSL_ASSERT(inBuffer != NULL, Invalid paramter passed as NULL);
	RSSL_ASSERT(cacheBuf != NULL, Invalid paramter passed as NULL);

	if (inBuffer->data == NULL || inBuffer->length == 0)
		return RSSL_RET_INVALID_ARGUMENT;

	if (cacheBuf->data == NULL || cacheBuf->allocatedLength == 0)
		return RSSL_RET_BUFFER_TOO_SMALL;
	
	if(inBuffer->data[0] == ESC_CHAR)
	{
		escFirst = RSSL_TRUE;
		escStart = 0;
	}

	while(inBufPos < inBuffer->length)
	{
		switch(state)
		{
			case NORMAL:
				if(inBuffer->data[inBufPos] == ESC_CHAR)
				{
					state = ESC;
					escStart = inBufPos;
					numCount = 0;
				}
				/* Breaks korean language display */
				/*else if(inBuffer->data[inBufPos] == CSI_CHAR)
				{
					state = LBRKT;
					numCount = 0;
				}*/
				else
				{
					if(cacheBufPos >= cacheBuf->allocatedLength)
					{
						/*Out of space */
						return RSSL_RET_BUFFER_TOO_SMALL;
					}
					prevChar = inBuffer->data[inBufPos];
					cacheBuf->data[cacheBufPos] = inBuffer->data[inBufPos];
					cacheBufPos++;
				}
				break;
			case ESC:
				if(inBuffer->data[inBufPos] == LBRKT_CHAR)
					state = LBRKT;
				else if(inBuffer->data[inBufPos] == 0x25)
				{
					inBufPos++;
					if(inBuffer->data[inBufPos] == 0x30)
					{
						if(cacheBufPos + 3 >= cacheBuf->allocatedLength)
						{
						/* Error: Out of space */
							return RSSL_RET_BUFFER_TOO_SMALL;
						}
						cacheBuf->data[cacheBufPos++] = ESC_CHAR;
						cacheBuf->data[cacheBufPos++] = 0x25;
						cacheBuf->data[cacheBufPos++] = 0x30;
						state = NORMAL;
					}
					else
					{
						/*Error */
						return RSSL_RET_FAILURE;
					}
				}
				else
				{
					/* normal escape code, print ESC character to buffer and continue */
					state = NORMAL;
					if(cacheBufPos + 2 >= cacheBuf->allocatedLength)
					{
						return RSSL_RET_BUFFER_TOO_SMALL;
					}
					cacheBuf->data[cacheBufPos++] = ESC_CHAR;
					cacheBuf->data[cacheBufPos++] = inBuffer->data[inBufPos];
				}					
				break;
			case LBRKT:
				if(inBuffer->data[inBufPos] >= '0' && inBuffer->data[inBufPos] <= '9')
				{
				  RsslUInt64 tmp = (RsslUInt64)(numCount)*10 + inBuffer->data[inBufPos] - '0';
				  if (tmp > UINT_MAX)
					return RSSL_RET_FAILURE;
				  numCount = (RsslUInt32)(tmp);
				}
				else 
				if(inBuffer->data[inBufPos] == RHPA_CHAR)
				/* Move cursor command */
				{
					if(escStart == 0)
					{
						escFirst = RSSL_TRUE;
					}
					if(numCount >= 0)
					{
						cacheBufPos = numCount;
						numCount = 0;
						state = NORMAL;
						
						if(cacheBufPos > cacheBuf->allocatedLength)
						/* Out of memory */
							return RSSL_RET_BUFFER_TOO_SMALL;
					}
					else
					{
						return RSSL_RET_FAILURE;
					}
				}
				else if(inBuffer->data[inBufPos] == RREP_CHAR)
				/* Repeat character command.  This is always 1 char */
				{
				    RsslUInt32 i;
					/* Check for overrun first */
					if(escStart == 0)
					{
						escFirst = RSSL_TRUE;
					}
					if(cacheBuf->allocatedLength < cacheBufPos+numCount)
						return RSSL_RET_BUFFER_TOO_SMALL;
					for(i = 0; i < numCount; i++)
					{
						cacheBuf->data[cacheBufPos++] = prevChar;
					}
					state = NORMAL;
				}
				else
					return RSSL_RET_FAILURE;
				break;
			default:
			/* Error, should not happen*/
				return RSSL_RET_FAILURE;
		}
		
		++inBufPos;
		if(cacheBufPos > maxLen)
			maxLen = cacheBufPos;
	}
	
	/* Check that we're not in a weird state */
	if(state != NORMAL)
		return RSSL_RET_FAILURE;
	
	if(escFirst == RSSL_TRUE)
	{
		if(maxLen > cacheBuf->length)
		{
			cacheBuf->length = maxLen;
		}
		else
			return RSSL_RET_SUCCESS;
	}
	else
	{
		cacheBuf->length = maxLen;
	}
	
	return RSSL_RET_SUCCESS;
}

#ifdef __cplusplus
}
#endif
