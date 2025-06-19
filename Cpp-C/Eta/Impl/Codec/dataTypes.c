/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2018-2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#define _RSSL_DATATOOLS_EXPORTS
#include "rtr/intDataTypes.h"
#include "rtr/rsslDataTypeEnums.h"
#include "rtr/rsslDataUtils.h"
#include "rtr/rsslQos.h"


RSSL_API RsslUInt32 rsslPrimitiveTypeSize(const RsslDataType dataType)
{
	return(_rsslPrimitiveTypeMaxEncSize(dataType));
}


RSSL_API const char* rsslDataTypeToString(RsslDataType type)
{
	if (type < RSSL_DT_LAST)
		return(_rsslDataTypeInfo[type].string);
	return 0;
}

RSSL_API const char* rsslDataTypeToOmmString(RsslDataType type)
{
	switch(type)
	{
		case RSSL_DT_UNKNOWN: return RSSL_OMMSTR_DT_UNKNOWN.data;
		case RSSL_DT_INT: return RSSL_OMMSTR_DT_INT.data;
		case RSSL_DT_UINT: return RSSL_OMMSTR_DT_UINT.data;
		case RSSL_DT_FLOAT: return RSSL_OMMSTR_DT_FLOAT.data;
		case RSSL_DT_DOUBLE: return RSSL_OMMSTR_DT_DOUBLE.data;
		case RSSL_DT_REAL: return RSSL_OMMSTR_DT_REAL.data;
		case RSSL_DT_DATE: return RSSL_OMMSTR_DT_DATE.data;
		case RSSL_DT_TIME: return RSSL_OMMSTR_DT_TIME.data;
		case RSSL_DT_DATETIME: return RSSL_OMMSTR_DT_DATETIME.data;
		case RSSL_DT_QOS: return RSSL_OMMSTR_DT_QOS.data;
		case RSSL_DT_STATE: return RSSL_OMMSTR_DT_STATE.data;
		case RSSL_DT_ENUM: return RSSL_OMMSTR_DT_ENUM.data;
		case RSSL_DT_ARRAY: return RSSL_OMMSTR_DT_ARRAY.data;
		case RSSL_DT_BUFFER: return RSSL_OMMSTR_DT_BUFFER.data;
		case RSSL_DT_ASCII_STRING: return RSSL_OMMSTR_DT_ASCII_STRING.data;
		case RSSL_DT_UTF8_STRING: return RSSL_OMMSTR_DT_UTF8_STRING.data;
		case RSSL_DT_RMTES_STRING: return RSSL_OMMSTR_DT_RMTES_STRING.data;
		case RSSL_DT_INT_1: return RSSL_OMMSTR_DT_INT_1.data;
		case RSSL_DT_UINT_1: return RSSL_OMMSTR_DT_UINT_1.data;
		case RSSL_DT_INT_2: return RSSL_OMMSTR_DT_INT_2.data;
		case RSSL_DT_UINT_2: return RSSL_OMMSTR_DT_UINT_2.data;
		case RSSL_DT_INT_4: return RSSL_OMMSTR_DT_INT_4.data;
		case RSSL_DT_UINT_4: return RSSL_OMMSTR_DT_UINT_4.data;
		case RSSL_DT_INT_8: return RSSL_OMMSTR_DT_INT_8.data;
		case RSSL_DT_UINT_8: return RSSL_OMMSTR_DT_UINT_8.data;
		case RSSL_DT_FLOAT_4: return RSSL_OMMSTR_DT_FLOAT_4.data;
		case RSSL_DT_DOUBLE_8: return RSSL_OMMSTR_DT_DOUBLE_8.data;
		case RSSL_DT_REAL_4RB: return RSSL_OMMSTR_DT_REAL_4RB.data;
		case RSSL_DT_REAL_8RB: return RSSL_OMMSTR_DT_REAL_8RB.data;
		case RSSL_DT_DATE_4: return RSSL_OMMSTR_DT_DATE_4.data;
		case RSSL_DT_TIME_3: return RSSL_OMMSTR_DT_TIME_3.data;
		case RSSL_DT_TIME_5: return RSSL_OMMSTR_DT_TIME_5.data;
		case RSSL_DT_DATETIME_7: return RSSL_OMMSTR_DT_DATETIME_7.data;
		case RSSL_DT_DATETIME_9: return RSSL_OMMSTR_DT_DATETIME_9.data;
		case RSSL_DT_DATETIME_11: return RSSL_OMMSTR_DT_DATETIME_11.data;
		case RSSL_DT_DATETIME_12: return RSSL_OMMSTR_DT_DATETIME_12.data;
		case RSSL_DT_TIME_7: return RSSL_OMMSTR_DT_TIME_7.data;
		case RSSL_DT_TIME_8: return RSSL_OMMSTR_DT_TIME_8.data;
		case RSSL_DT_NO_DATA: return RSSL_OMMSTR_DT_NO_DATA.data;
		case RSSL_DT_OPAQUE: return RSSL_OMMSTR_DT_OPAQUE.data;
		case RSSL_DT_XML: return RSSL_OMMSTR_DT_XML.data;
		case RSSL_DT_FIELD_LIST: return RSSL_OMMSTR_DT_FIELD_LIST.data;
		case RSSL_DT_ELEMENT_LIST: return RSSL_OMMSTR_DT_ELEMENT_LIST.data;
		case RSSL_DT_ANSI_PAGE: return RSSL_OMMSTR_DT_ANSI_PAGE.data;
		case RSSL_DT_FILTER_LIST: return RSSL_OMMSTR_DT_FILTER_LIST.data;
		case RSSL_DT_VECTOR: return RSSL_OMMSTR_DT_VECTOR.data;
		case RSSL_DT_MAP: return RSSL_OMMSTR_DT_MAP.data;
		case RSSL_DT_SERIES: return RSSL_OMMSTR_DT_SERIES.data;
		case RSSL_DT_MSG: return RSSL_OMMSTR_DT_MSG.data;
		case RSSL_DT_JSON: return RSSL_OMMSTR_DT_JSON.data;
		default: return NULL;
	}
}





RsslDataTypeInfo	_rsslDataTypeInfo[RSSL_DT_LAST+1] =
	/* data type, maxEncodedSize, flags, assertions */
{
	{ RSSL_DT_UNKNOWN, 0, __RSZVAR, 0, 0, 0, 0, 0, 0, 0, 0, "RSSL_DT_UNKNOWN" },									/* (0) */

	{ 1/*RSSL_DT_INT32*/, RSSL_DT_INT, 0/*__RSZI32+1*/, 0/*__RDTFVLEN*/, 0, 0/*_rsslEncInt32*/, _rsslDecBuf8, 0/*_rsslInt32AsString*/, 0, 0/*_rsslEncodeInt32_4*/, 0/*_rsslDecInt32*/, 0/*"RSSL_DT_INT32"*/ },		/* (1) */
	{ 2/*RSSL_DT_UINT32*/, RSSL_DT_UINT, 0/*__RSZUI32+1*/, 0/*__RDTFVLEN*/, 0, 0/*_rsslEncUInt32*/, _rsslDecBuf8, 0/*_rsslUInt32AsString*/, 0, 0/*_rsslEncodeUInt32_4*/,  0/*_rsslDecUInt32*/, 0/*"RSSL_DT_UINT32"*/ },	/* (2) */
	{ RSSL_DT_INT, RSSL_DT_INT, __RSZI64+1, __RDTFVLEN, 0, _rsslEncInt, _rsslDecBuf8, _rsslIntAsString, _rsslIntToString, _rsslEncodeInt_8, _rsslDecInt, "RSSL_DT_INT" },		/* (3) */
	{ RSSL_DT_UINT, RSSL_DT_UINT, __RSZUI64+1, __RDTFVLEN, 0, _rsslEncUInt, _rsslDecBuf8, _rsslUIntAsString, _rsslUIntToString, _rsslEncodeUInt_8, _rsslDecUInt, "RSSL_DT_UINT" },	/* (4) */
	{ RSSL_DT_FLOAT, RSSL_DT_FLOAT, __RSZFLT+1, __RDTFFLEN, 0, _rsslEncFloat, _rsslDecBuf8, _rsslFloatAsString, _rsslFloatToString, _rsslEncodeFloat, _rsslDecFloat, "RSSL_DT_FLOAT"},		/* (5) */
	{ RSSL_DT_DOUBLE, RSSL_DT_DOUBLE, __RSZDBL+1, __RDTFFLEN, 0, _rsslEncDouble, _rsslDecBuf8, _rsslDoubleAsString, _rsslDoubleToString, _rsslEncodeDouble, _rsslDecDouble, "RSSL_DT_DOUBLE" },	/* (6) */
	{ 7/*RSSL_DT_REAL32*/, RSSL_DT_REAL, 0/*__RSZRL32+1*/, 0/*__RDTFVLEN*/, 0, 0/*_rsslEncReal32*/, _rsslDecBuf8, 0/*_rsslReal32AsString*/, 0, 0/*_rsslEncodeReal32*/, 0/*_rsslDecReal32*/, 0/*"RSSL_DT_REAL32"*/ },	/* (7) */
	{ RSSL_DT_REAL, RSSL_DT_REAL, __RSZRL64+1, __RDTFVLEN, 0, _rsslEncReal, _rsslDecBuf8, _rsslRealAsString, _rsslRealToString, _rsslEncodeReal, _rsslDecReal, "RSSL_DT_REAL" },	/* (8) */
	{ RSSL_DT_DATE, RSSL_DT_DATE, __RSZDT+1, __RDTFFLEN, 0, _rsslEncDate, _rsslDecBuf8, _rsslDateAsString, _rsslDateToString, _rsslEncodeDate, _rsslDecDate, "RSSL_DT_DATE" },					/* (9) */
	{ RSSL_DT_TIME, RSSL_DT_TIME, __RSZTM+1, __RDTFFLEN, 0, _rsslEncTime, _rsslDecBuf8, _rsslTimeAsString, _rsslTimeToString, _rsslEncodeTime, _rsslDecTime, "RSSL_DT_TIME" },					/* (10) */
	{ RSSL_DT_DATETIME, RSSL_DT_DATETIME, __RSZDTM+1, __RDTFFLEN, 0, _rsslEncDateTime, _rsslDecBuf8, _rsslDateTimeAsString, _rsslDateTimeToString, _rsslEncodeDateTime, _rsslDecDateTime, "RSSL_DT_DATETIME" },		/* (11) */
	{ RSSL_DT_QOS, RSSL_DT_QOS, __RSZQOS+1, __RDTFVLEN, 0, _rsslEncQos, _rsslDecBuf8, _rsslQosAsString, _rsslQosToString, _rsslEncodeQos, _rsslDecQos, "RSSL_DT_QOS" },						/* (12) */
	{ RSSL_DT_STATE, RSSL_DT_STATE, __RSZVAR, __RDTFLSPE, 0, _rsslEncState, _rsslDecBuf16, _rsslStateAsString, _rsslStateToString, _rsslEncodeState, _rsslDecState, "RSSL_DT_STATE" },					/* (13) */
	{ RSSL_DT_ENUM, RSSL_DT_ENUM, __RSZUI16+1, __RDTFFLEN, 0, _rsslEncEnum, _rsslDecBuf16, _rsslEnumAsString, _rsslEnumToString, _rsslEncodeEnum2, _rsslDecEnum, "RSSL_DT_ENUM" }, /* (14) */
	{ RSSL_DT_ARRAY, RSSL_DT_ARRAY, __RSZVAR, __RDTFLSPE, 0, _rsslEncBuffer, _rsslDecBuf16, 0, 0, 0, 0, "RSSL_DT_ARRAY" },					/* (15) */
	{ RSSL_DT_BUFFER, RSSL_DT_BUFFER, __RSZVAR, __RDTFLSPE, 0, _rsslEncBuffer, _rsslDecBuf16, _rsslBufferAsString, _rsslBufferToString, _rsslEncodeBuffer, _rsslDecBuffer16, "RSSL_DT_BUFFER" },				/* (16) */
	{ RSSL_DT_ASCII_STRING, RSSL_DT_ASCII_STRING, __RSZVAR, __RDTFLSPE, 0, _rsslEncBuffer, _rsslDecBuf16, _rsslBufferAsString, _rsslBufferToString, _rsslEncodeBuffer, _rsslDecBuffer16, "RSSL_DT_ASCII_STRING" },	/* (17) */
	{ RSSL_DT_UTF8_STRING, RSSL_DT_UTF8_STRING, __RSZVAR, __RDTFLSPE, 0, _rsslEncBuffer, _rsslDecBuf16, _rsslBufferAsString, _rsslBufferToString, _rsslEncodeBuffer, _rsslDecBuffer16, "RSSL_DT_UTF8_STRING" },		/* (18) */
	{ RSSL_DT_RMTES_STRING, RSSL_DT_RMTES_STRING, __RSZVAR, __RDTFLSPE, 0, _rsslEncBuffer, _rsslDecBuf16, _rsslBufferAsString, _rsslBufferToString, _rsslEncodeBuffer, _rsslDecBuffer16, "RSSL_DT_RMTES_STRING" },	/* (19) */
	{ 20, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (20) */
	{ 21, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (21) */
	{ 22, 22, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (22) */
	{ 23, 23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (23) */
	{ 24, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (24) */
	{ 25, 25, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (25) */
	{ 26, 26, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (26) */
	{ 27, 27, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (27) */
	{ 28, 28, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (28) */
	{ 29, 29, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (29) */
	{ 30, 30, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (30) */
	{ 31, 31, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (31) */
	{ 32, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (32) */
	{ 33, 33, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (33) */
	{ 34, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (34) */
	{ 35, 35, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (35) */
	{ 36, 36, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (36) */
	{ 37, 37, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (37) */
	{ 38, 38, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (38) */
	{ 39, 39, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (39) */
	{ 40, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (40) */
	{ 41, 41, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (41) */
	{ 42, 42, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (42) */
	{ 43, 43, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (43) */
	{ 44, 44, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (44) */
	{ 45, 45, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (45) */
	{ 46, 46, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (46) */
	{ 47, 47, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (47) */
	{ 48, 48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (48) */
	{ 49, 49, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (49) */
	{ 50, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (50) */
	{ 51, 51, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (51) */
	{ 52, 52, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (52) */
	{ 53, 53, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (53) */
	{ 54, 54, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (54) */
	{ 55, 55, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (55) */
	{ 56, 56, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (56) */
	{ 57, 57, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (57) */
	{ 58, 58, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (58) */
	{ 59, 59, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (59) */
	{ 60, 60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (60) */
	{ 61, 61, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (61) */
	{ 62, 62, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (62) */
	{ 63, 63, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (63) */
	{ RSSL_DT_INT_1, RSSL_DT_INT, __RSZI8, __RDTFFLEN, 0, _rsslEncodeInt1, _rsslDec8, _rsslIntAsString, _rsslIntToString, _rsslEncodeInt_1, _rsslDecInt, "RSSL_DT_INT_1" },				/* (64) */
	{ RSSL_DT_UINT_1, RSSL_DT_UINT, __RSZUI8, __RDTFFLEN, 0, _rsslEncodeUInt1, _rsslDec8, _rsslUIntAsString, _rsslUIntToString, _rsslEncodeUInt_1, _rsslDecUInt, "RSSL_DT_UINT_1" },			/* (65) */
	{ RSSL_DT_INT_2, RSSL_DT_INT, __RSZI16, __RDTFFLEN, 0, _rsslEncodeInt2, _rsslDec16, _rsslIntAsString, _rsslIntToString, _rsslEncodeInt_2, _rsslDecInt, "RSSL_DT_INT_2" },			/* (66) */
	{ RSSL_DT_UINT_2, RSSL_DT_UINT, __RSZUI16, __RDTFFLEN, 0, _rsslEncodeUInt2, _rsslDec16, _rsslUIntAsString, _rsslUIntToString, _rsslEncodeUInt_2, _rsslDecUInt, "RSSL_DT_UINT_2" },		/* (67) */
	{ RSSL_DT_INT_4, RSSL_DT_INT, __RSZI32, __RDTFFLEN, 0, _rsslEncodeInt4, _rsslDec32, _rsslIntAsString, _rsslIntToString, _rsslEncodeInt_4, _rsslDecInt, "RSSL_DT_INT_4" },			/* (68) */
	{ RSSL_DT_UINT_4, RSSL_DT_UINT, __RSZUI32, __RDTFFLEN, 0, _rsslEncodeUInt4, _rsslDec32, _rsslUIntAsString, _rsslUIntToString, _rsslEncodeUInt_4, _rsslDecUInt, "RSSL_DT_UINT_4" },		/* (69) */
	{ RSSL_DT_INT_8, RSSL_DT_INT, __RSZI64, __RDTFFLEN, 0, _rsslEncodeInt8, _rsslDec64, _rsslIntAsString, _rsslIntToString, _rsslEncodeInt_8, _rsslDecInt, "RSSL_DT_INT_8" },			/* (70) */
	{ RSSL_DT_UINT_8, RSSL_DT_UINT, __RSZUI64, __RDTFFLEN, 0, _rsslEncodeUInt8, _rsslDec64, _rsslUIntAsString, _rsslUIntToString, _rsslEncodeUInt_8, _rsslDecUInt, "RSSL_DT_UINT_8" },		/* (71) */
	{ RSSL_DT_FLOAT_4, RSSL_DT_FLOAT, __RSZFLT, __RDTFFLEN, 0, _rsslEncFloat_4, _rsslDec32, _rsslFloatAsString, _rsslFloatToString, _rsslEncodeFloat, _rsslDecFloat, "RSSL_DT_FLOAT_4" },			/* (72) */
	{ RSSL_DT_DOUBLE_8, RSSL_DT_DOUBLE, __RSZDBL, __RDTFFLEN, 0, _rsslEncDouble_8, _rsslDec64, _rsslDoubleAsString,  _rsslDoubleToString,  _rsslEncodeDouble, _rsslDecDouble, "RSSL_DT_DOUBLE_8" },			/* (73) */
	{ RSSL_DT_REAL_4RB, RSSL_DT_REAL, __RSZRL32, __RDTFVLEN, 0, _rsslEncReal_4rb, _rsslDecReal_4rb, _rsslRealAsString, _rsslRealToString, _rsslEncodeReal, _rsslDecReal, "RSSL_DT_REAL_4RB"},		/* (74) */
	{ RSSL_DT_REAL_8RB, RSSL_DT_REAL, __RSZRL64, __RDTFVLEN, 0, _rsslEncReal_8rb, _rsslDecReal_8rb, _rsslRealAsString, _rsslRealToString, _rsslEncodeReal, _rsslDecReal, "RSSL_DT_REAL_8RB" },		/* (75) */
	{ RSSL_DT_DATE_4, RSSL_DT_DATE, __RSZDT, __RDTFFLEN, 0, _rsslEncDate_4, _rsslDec32, _rsslDateAsString, _rsslDateToString, _rsslEncodeDate, _rsslDecDate, "RSSL_DT_DATE_4" },				/* (76) */
	{ RSSL_DT_TIME_3, RSSL_DT_TIME, __RSZTM3, __RDTFFLEN, 0, _rsslEncTime_3, _rsslDec24, _rsslTimeAsString, _rsslTimeToString, _rsslEncodeTime, _rsslDecTime, "RSSL_DT_TIME_3" },				/* (77) */
	{ RSSL_DT_TIME_5, RSSL_DT_TIME, __RSZTM5, __RDTFFLEN, 0, _rsslEncTime_5, _rsslDec40, _rsslTimeAsString, _rsslTimeToString, _rsslEncodeTime, _rsslDecTime, "RSSL_DT_TIME_5" },				/* (78) */
	{ RSSL_DT_DATETIME_7, RSSL_DT_DATETIME, __RSZDTM7, __RDTFFLEN, 0, _rsslEncDateTime_7, _rsslDec56, _rsslDateTimeAsString, _rsslDateTimeToString, _rsslEncodeDateTime, _rsslDecDateTime, "RSSL_DT_DATETIME_7" },	/* (79) */	
	{ RSSL_DT_DATETIME_9, RSSL_DT_DATETIME, __RSZDTM9, __RDTFFLEN, 0, _rsslEncDateTime_9, _rsslDec72, _rsslDateTimeAsString, _rsslDateTimeToString, _rsslEncodeDateTime, _rsslDecDateTime, "RSSL_DT_DATETIME_9"  },			/* (80) */
	{ RSSL_DT_DATETIME_11, RSSL_DT_DATETIME, __RSZDTM11, __RDTFFLEN, 0, _rsslEncDateTime_11, _rsslDec88, _rsslDateTimeAsString, _rsslDateTimeToString, _rsslEncodeDateTime, _rsslDecDateTime, "RSSL_DT_DATETIME_11" }, /* (81) */
	{ RSSL_DT_DATETIME_12, RSSL_DT_DATETIME, __RSZDTM, __RDTFFLEN, 0, _rsslEncDateTime_12, _rsslDec96, _rsslDateTimeAsString, _rsslDateTimeToString, _rsslEncodeDateTime, _rsslDecDateTime, "RSSL_DT_DATETIME_12" }, /* (82) */
	{ RSSL_DT_TIME_7, RSSL_DT_TIME, __RSZTM7, __RDTFFLEN, 0, _rsslEncTime_7, _rsslDec56, _rsslTimeAsString, _rsslTimeToString, _rsslEncodeTime, _rsslDecTime, "RSSL_DT_TIME_7" },	/* (83) */
	{ RSSL_DT_TIME_8, RSSL_DT_TIME, __RSZTM, __RDTFFLEN, 0, _rsslEncTime_8, _rsslDec64, _rsslTimeAsString, _rsslTimeToString, _rsslEncodeTime, _rsslDecTime, "RSSL_DT_TIME_8" },													/* (84) */
	{ 85, 85, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (85) */
	{ 86, 86, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (86) */
	{ 87, 87, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (87) */
	{ 88, 88, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (88) */
	{ 89, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (89) */
	{ 90, 90, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (90) */
	{ 91, 91, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (91) */
	{ 92, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (92) */
	{ 93, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (93) */
	{ 94, 94, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (94) */
	{ 95, 95, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (95) */
	{ 96, 96, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (96) */
	{ 97, 97, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (97) */
	{ 98, 98, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (98) */
	{ 99, 99, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },													/* (99) */
	{ 100, 100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (100) */
	{ 101, 101, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (101) */
	{ 102, 102, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (102) */
	{ 103, 103, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (103) */
	{ 104, 104, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (104) */
	{ 105, 105, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (105) */
	{ 106, 106, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (106) */
	{ 107, 107, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (107) */
	{ 108, 108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (108) */
	{ 109, 109, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (109) */
	{ 110, 110, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (110) */
	{ 111, 111, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (111) */
	{ 112, 112, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (112) */
	{ 113, 113, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (113) */
	{ 114, 114, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (114) */
	{ 115, 115, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (115) */
	{ 116, 116, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (116) */
	{ 117, 117, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (117) */
	{ 118, 118, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (118) */
	{ 119, 119, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (119) */
	{ 120, 120, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (120) */
	{ 121, 121, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (121) */
	{ 122, 122, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (122) */
	{ 123, 123, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (123) */
	{ 124, 124, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (124) */
	{ 125, 125, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (125) */
	{ 126, 126, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (126) */
	{ 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (127) */
	{ RSSL_DT_NO_DATA, RSSL_DT_NO_DATA, __RSZVAR, RsslDTFValid, 0, 0, 0, 0, 0, 0, 0, "RSSL_DT_NO_DATA" },			/* (128) */
	{ 129, 129, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	/* (129) */
	{ RSSL_DT_OPAQUE, RSSL_DT_OPAQUE, __RSZVAR, RsslDTFValid, 0, 0, _rsslDecBuf16, 0, 0, 0, 0, "RSSL_DT_OPAQUE" },				/* (130) */
	{ RSSL_DT_XML, RSSL_DT_XML, __RSZVAR, RsslDTFValid, 0, 0, _rsslDecBuf16, 0, 0, 0, 0, "RSSL_DT_XML" },					/* (131) */
	{ RSSL_DT_FIELD_LIST, RSSL_DT_FIELD_LIST, __RSZVAR, RsslDTFValid, 0, 0, _rsslDecBuf16, 0, 0, 0, 0, "RSSL_DT_FIELD_LIST" },		/* (132) */
	{ RSSL_DT_ELEMENT_LIST, RSSL_DT_ELEMENT_LIST, __RSZVAR, RsslDTFValid, 0, 0,_rsslDecBuf16, 0, 0, 0, 0, "RSSL_DT_ELEMENT_LIST" },	/* (133) */
	{ RSSL_DT_ANSI_PAGE, RSSL_DT_ANSI_PAGE, __RSZVAR, RsslDTFValid, 0, 0, _rsslDecBuf16, 0, 0, 0, 0, "RSSL_DT_ANSI_PAGE" },		/* (134) */
	{ RSSL_DT_FILTER_LIST, RSSL_DT_FILTER_LIST, __RSZVAR, RsslDTFValid, 0, 0, _rsslDecBuf16, 0, 0, 0, 0, "RSSL_DT_FILTER_LIST" },	/* (135) */
	{ RSSL_DT_VECTOR, RSSL_DT_VECTOR, __RSZVAR, RsslDTFValid, 0, 0, _rsslDecBuf16, 0, 0, 0, 0, "RSSL_DT_VECTOR" },				/* (136) */
	{ RSSL_DT_MAP, RSSL_DT_MAP, __RSZVAR, RsslDTFValid, 0, 0, _rsslDecBuf16, 0, 0, 0, 0, "RSSL_DT_MAP" },					/* (137) */
	{ RSSL_DT_SERIES, RSSL_DT_SERIES, __RSZVAR, RsslDTFValid, 0, 0, _rsslDecBuf16, 0, 0, 0, 0, "RSSL_DT_SERIES" },				/* (138) */
	{ 139, 139, __RSZVAR, RsslDTFValid, 0, 0, _rsslDecBuf16, 0, 0, 0, 0, 0 },				/* (139) */ /* formerly RSSL_DT_MARKETFEED */
	{ 140, 140, __RSZVAR, RsslDTFValid, 0, 0, _rsslDecBuf16, 0, 0, 0, 0, 0 },				/* (140) */ /* formerly RSSL_DT_TIBMSG */
	{ RSSL_DT_MSG, RSSL_DT_MSG, __RSZVAR, RsslDTFValid, 0, 0, _rsslDecBuf16, 0, 0, 0, 0, "RSSL_DT_MSG" },				/* (141) */
	{ RSSL_DT_JSON, RSSL_DT_JSON, __RSZVAR, RsslDTFValid, 0, 0, _rsslDecBuf16, 0, 0, 0, 0, "RSSL_DT_JSON" },												/* (142) */
	{ 143, 143, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (143) */
	{ 144, 144, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (144) */
	{ 145, 145, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (145) */
	{ 146, 146, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (146) */
	{ 147, 147, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (147) */
	{ 148, 148, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (148) */
	{ 149, 149, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (149) */
	{ 150, 150, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (150) */
	{ 151, 151, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (151) */
	{ 152, 152, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (152) */
	{ 153, 153, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (153) */
	{ 154, 154, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (154) */
	{ 155, 155, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (155) */
	{ 156, 156, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (156) */
	{ 157, 157, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (157) */
	{ 158, 158, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (158) */
	{ 159, 159, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (159) */
	{ 160, 160, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (160) */
	{ 161, 161, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (161) */
	{ 162, 162, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (162) */
	{ 163, 163, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (163) */
	{ 164, 164, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (164) */
	{ 165, 165, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (165) */
	{ 166, 166, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (166) */
	{ 167, 167, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (167) */
	{ 168, 168, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (168) */
	{ 169, 169, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (169) */
	{ 170, 170, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (170) */
	{ 171, 171, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (171) */
	{ 172, 172, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (172) */
	{ 173, 173, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (173) */
	{ 174, 174, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (174) */
	{ 175, 175, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (175) */
	{ 176, 176, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (176) */
	{ 177, 177, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (177) */
	{ 178, 178, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (178) */
	{ 179, 179, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (179) */
	{ 180, 180, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (180) */
	{ 181, 181, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (181) */
	{ 182, 182, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (182) */
	{ 183, 183, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (183) */
	{ 184, 184, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (184) */
	{ 185, 185, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (185) */
	{ 186, 186, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (186) */
	{ 187, 187, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (187) */
	{ 188, 188, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (188) */
	{ 189, 189, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (189) */
	{ 190, 190, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (190) */
	{ 191, 191, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (191) */
	{ 192, 192, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (192) */
	{ 193, 193, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (193) */
	{ 194, 194, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (194) */
	{ 195, 195, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (195) */
	{ 196, 196, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (196) */
	{ 197, 197, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (197) */
	{ 198, 198, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (198) */
	{ 199, 199, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (199) */
	{ 200, 200, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (200) */
	{ 201, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (201) */
	{ 202, 202, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (202) */
	{ 203, 203, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (203) */
	{ 204, 204, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (204) */
	{ 205, 205, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (205) */
	{ 206, 206, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (206) */
	{ 207, 207, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (207) */
	{ 208, 208, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (208) */
	{ 209, 209, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (209) */
	{ 210, 210, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (210) */
	{ 211, 211, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (211) */
	{ 212, 212, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (212) */
	{ 213, 213, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (213) */
	{ 214, 214, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (214) */
	{ 215, 215, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (215) */
	{ 216, 216, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (216) */
	{ 217, 217, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (217) */
	{ 218, 218, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (218) */
	{ 219, 219, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (219) */
	{ 220, 220, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (220) */
	{ 221, 221, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (221) */
	{ 222, 222, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (222) */
	{ 223, 223, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (223) */
	{ 224, 224, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (224) */
	{ 225, 225, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (225) */
	{ 226, 226, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (226) */
	{ 227, 227, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (227) */
	{ 228, 228, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (228) */
	{ 229, 229, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (229) */
	{ 230, 230, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (230) */
	{ 231, 231, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (231) */
	{ 232, 232, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (232) */
	{ 233, 233, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (233) */
	{ 234, 234, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (234) */
	{ 235, 235, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (235) */
	{ 236, 236, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (236) */
	{ 237, 237, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (237) */
	{ 238, 238, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (238) */
	{ 239, 239, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (239) */
	{ 240, 240, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (240) */
	{ 241, 241, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (241) */
	{ 242, 242, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (242) */
	{ 243, 243, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (243) */
	{ 244, 244, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (244) */
	{ 245, 245, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (245) */
	{ 246, 246, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (246) */
	{ 247, 247, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (247) */
	{ 248, 248, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (248) */
	{ 249, 249, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (249) */
	{ 250, 250, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (250) */
	{ 251, 251, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (251) */
	{ 252, 252, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (252) */
	{ 253, 253, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (253) */
	{ 254, 254, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },												/* (254) */
	{ RSSL_DT_LAST, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }											/* RSSL_DT_LAST */
};

static int __rsslDataToolsInitialized=0;
void _rsslDataToolsInit()
{
	if (!__rsslDataToolsInitialized)
	{
		int i;
		RsslDataTypeInfo	*typeInfo;
		for (i=0;i<RSSL_DT_LAST;i++)
		{
				/* Verify the static table to make sure correct */
			typeInfo = &_rsslDataTypeInfo[i];
			RSSL_ASSERT(typeInfo != 0, Invalid);
			RSSL_ASSERT(typeInfo->dataType == i, Invalid);
		}
		__rsslDataToolsInitialized = 1;
	}
}

RsslInt32 _rsslAdjustRateQos(const RsslQos *qos, int unspecifiedValue)
{
	if (qos->rate == RSSL_QOS_RATE_TICK_BY_TICK)
		return 0;
	if (qos->rate == RSSL_QOS_RATE_JIT_CONFLATED)
		return 65535 * 2;
	if (qos->rate == RSSL_QOS_RATE_TIME_CONFLATED)
	{
		if (qos->rateInfo == 65535)
			return 65535 * 2 + 1;  //max time conflated is worse than JIT conflated
		return qos->rateInfo * 2 - 1;
	}

	return unspecifiedValue;
}

RsslInt32 _rsslAdjustTimeQos(const RsslQos *qos, int unspecifiedValue)
{
	if (qos->timeliness == RSSL_QOS_TIME_REALTIME)
		return 0;
	if (qos->timeliness == RSSL_QOS_TIME_DELAYED_UNKNOWN)
		return 65536;
	if (qos->timeliness == RSSL_QOS_TIME_DELAYED)
		return qos->timeInfo;
	return unspecifiedValue;
}

RSSL_API RsslBool rsslQosIsBetter(const RsslQos *newQos, const RsslQos *oldQos)
{
	RsslInt32 newAdjustedRate = _rsslAdjustRateQos(newQos, 65535 * 2 + 3);
	RsslInt32 oldAdjustedRate = _rsslAdjustRateQos(oldQos, 65535 * 2 + 3);

	RsslInt32 newAdjustedTimeliness = _rsslAdjustTimeQos(newQos, 65535 * 2 + 3);
	RsslInt32 oldAdjustedTimeliness = _rsslAdjustTimeQos(oldQos, 65535 * 2 + 3);

	if (newAdjustedTimeliness < oldAdjustedTimeliness) 
		return RSSL_TRUE;
	if (newAdjustedTimeliness > oldAdjustedTimeliness)
		return RSSL_FALSE;

	if (newAdjustedRate < oldAdjustedRate)
		return RSSL_TRUE;

	return RSSL_FALSE;
}

RSSL_API RsslBool rsslQosIsInRange(const RsslQos *bestQos, const RsslQos *worstQos, const RsslQos *Qos)
{
	RsslInt32 bestAdjustedRate = _rsslAdjustRateQos(bestQos, -1);
	RsslInt32 worstAdjustedRate = _rsslAdjustRateQos(worstQos, 65535 * 2 + 3);
	RsslInt32 qosAdjustedRate = _rsslAdjustRateQos(Qos, -1);
	RsslInt32 bestAdjustedTimeliness = _rsslAdjustTimeQos(bestQos, -1);
	RsslInt32 worstAdjustedTimeliness = _rsslAdjustTimeQos(worstQos, 65535 * 2 + 3);
	RsslInt32 qosAdjustedTimeliness = _rsslAdjustTimeQos(Qos, -1);

	if (bestQos->rate == RSSL_QOS_RATE_UNSPECIFIED && bestQos->timeliness == RSSL_QOS_TIME_UNSPECIFIED &&
		worstQos->rate == RSSL_QOS_RATE_UNSPECIFIED && worstQos->timeliness == RSSL_QOS_TIME_UNSPECIFIED)
		return RSSL_TRUE;

	if (rsslQosIsEqual(bestQos, worstQos))
		return rsslQosIsEqual(bestQos, Qos);

	if (Qos->rate == RSSL_QOS_RATE_UNSPECIFIED)
	{
		if ((bestQos->rate != RSSL_QOS_RATE_UNSPECIFIED) ||
			(worstQos->rate != RSSL_QOS_RATE_UNSPECIFIED))
			return RSSL_FALSE;
	}

	if (Qos->timeliness == RSSL_QOS_TIME_UNSPECIFIED)
	{
		if ((bestQos->timeliness != RSSL_QOS_TIME_UNSPECIFIED) ||
			(worstQos->timeliness != RSSL_QOS_TIME_UNSPECIFIED))
			return RSSL_FALSE;
	}

	return ((bestAdjustedRate <= qosAdjustedRate) && (worstAdjustedRate >= qosAdjustedRate) &&
			(bestAdjustedTimeliness <= qosAdjustedTimeliness) && (worstAdjustedTimeliness >= qosAdjustedTimeliness));
}
