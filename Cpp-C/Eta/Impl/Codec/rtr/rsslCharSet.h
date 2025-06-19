/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2018-2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

/*
 *	This is used to store the character sets used to convert between 
 *	ISO2022(MTES) and UCS2
 */
 
 #ifndef __RSSL_CHAR_SET_H__
 #define __RSSL_CHAR_SET_H__
 
#ifdef __cplusplus
extern "C" {
#endif
 
 #include "rtr/rsslcnvtab.h"
 #include "rtr/rsslTypes.h"
 
extern const int SHAPE_94;
extern const int SHAPE_96;
 
typedef struct
{
	unsigned short* _table1;
	unsigned short* _table2;
	int _shape;
	RsslUInt32 _table1_length;
	RsslUInt32 _table2_start;
	RsslUInt32 _table2_length;
	int _stride;
} RsslRmtesCharSet;


typedef struct
{
	RsslRmtesCharSet * G0;
	RsslRmtesCharSet * G1;
	RsslRmtesCharSet * G2;
	RsslRmtesCharSet * G3;
	
	RsslRmtesCharSet ** GL;
	RsslRmtesCharSet ** GR;
} RsslRmtesWorkingSet;


extern const RsslRmtesCharSet _rsslReuterBasic1;
extern const RsslRmtesCharSet _rsslReuterBasic2;
extern const RsslRmtesCharSet _rsslJapaneseKatakana;
extern const RsslRmtesCharSet _rsslJapaneseLatin;
extern const RsslRmtesCharSet _rsslJapaneseKanji;
extern const RsslRmtesCharSet _rsslChinese1;
extern const RsslRmtesCharSet _rsslChinese2;

RTR_C_INLINE void initWorkingSet(RsslRmtesWorkingSet* ws)
{
	ws->G0 = (RsslRmtesCharSet *)&_rsslReuterBasic1;
	ws->G1 = (RsslRmtesCharSet *)&_rsslReuterBasic2;
	ws->G2 = (RsslRmtesCharSet *)&_rsslJapaneseKatakana;
	ws->G3 = (RsslRmtesCharSet *)&_rsslJapaneseKanji;
	
	ws->GL = &ws->G0;
	ws->GR = &ws->G1;
}

typedef enum {
	ESC_ERROR = -1,
	ESC_SUCCESS = 0,
	UTF_ENC = 1,
	RHPA_CMD = 2,
	RREP_CMD = 3,
	END_CHAR = 4,
} ESCReturnCode;

int controlParse(unsigned char* curPtr, unsigned char* endPtr, RsslRmtesWorkingSet* currentSet, ESCReturnCode *retCode);

#ifdef __cplusplus
}
#endif

#endif
