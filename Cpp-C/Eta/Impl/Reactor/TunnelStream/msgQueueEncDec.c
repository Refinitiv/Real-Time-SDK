/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/

#include "rtr/msgQueueEncDec.h"
#include "rtr/rsslRDMQueueMsg.h"
#include <assert.h>

RTR_C_ALWAYS_INLINE RsslInt16 swap16(RsslUInt16 value)
{
	return ((((value) & 0xFF00) >> 8) | 
			(((value) & 0x00FF) << 8));
}

RTR_C_ALWAYS_INLINE RsslUInt32 swap32(RsslUInt32 value)
{
	return ((((value) & 0xFF000000) >> 24) |
			(((value) & 0x00FF0000) >> 8)  | 
			(((value) & 0x0000FF00) << 8)  |
			(((value) & 0x000000FF) << 24) );
}

RTR_C_ALWAYS_INLINE RsslUInt64 swap64(RsslUInt64 value)
{
	return ((((value) & 0xFF00000000000000LL) >> 56) | 
			(((value) & 0x00FF000000000000LL) >> 40) |
			(((value) & 0x0000FF0000000000LL) >> 24) |
			(((value) & 0x000000FF00000000LL) >> 8)  | 
			(((value) & 0x00000000FF000000LL) << 8)  |
			(((value) & 0x0000000000FF0000LL) << 24) |
			(((value) & 0x000000000000FF00LL) << 40) |
			(((value) & 0x00000000000000FFLL) << 56) );

}

RTR_C_ALWAYS_INLINE RsslUInt32 bufPut8(char *out, RsslUInt8 value)
{
	char *in = (char*)&value;
	out[0] = in[0];
	return 1;
}

RTR_C_ALWAYS_INLINE RsslUInt32 bufPut16(char *out, RsslUInt16 value)
{
	char *in = (char*)&value;
	out[0] = in[1];
	out[1] = in[0];
	return 2;
}

RTR_C_ALWAYS_INLINE RsslUInt32 bufPut24(char *out, RsslUInt32 value)
{
	char *in = (char*)&value;
	out[0] = in[2];
	out[1] = in[1];
	out[2] = in[0];
	return 3;
}

RTR_C_ALWAYS_INLINE RsslUInt32 bufPut32(char *out, RsslUInt32 value)
{
	char *in = (char*)&value;
	out[0] = in[3];
	out[1] = in[2];
	out[2] = in[1];
	out[3] = in[0];
	return 4;
}

RTR_C_ALWAYS_INLINE RsslUInt32 bufPut40(char *out, RsslUInt64 value)
{
	char *in = (char*)&value;
	out[0] = in[4];
	out[1] = in[3];
	out[2] = in[2];
	out[3] = in[1];
	out[4] = in[0];
	return 5;
}

RTR_C_ALWAYS_INLINE RsslUInt32 bufPut48(char *out, RsslUInt64 value)
{
	char *in = (char*)&value;
	out[0] = in[5];
	out[1] = in[4];
	out[2] = in[3];
	out[3] = in[2];
	out[4] = in[1];
	out[5] = in[0];
	return 6;
}

RTR_C_ALWAYS_INLINE RsslUInt32 bufPut56(char *out, RsslUInt64 value)
{
	char *in = (char*)&value;
	out[0] = in[6];
	out[1] = in[5];
	out[2] = in[4];
	out[3] = in[3];
	out[4] = in[2];
	out[5] = in[1];
	out[6] = in[0];
	return 7;
}

RTR_C_ALWAYS_INLINE RsslUInt32 bufPut64(char *out, RsslUInt64 value)
{
	char *in = (char*)&value;
	out[0] = in[7];
	out[1] = in[6];
	out[2] = in[5];
	out[3] = in[4];
	out[4] = in[3];
	out[5] = in[2];
	out[6] = in[1];
	out[7] = in[0];
	return 8;
}

RTR_C_ALWAYS_INLINE RsslUInt32 bufGet16(char *in, RsslUInt16 *pValue)
{
	char *out = (char*)pValue;
	out[0] = in[1];
	out[1] = in[0];
	return 2;
}

RTR_C_ALWAYS_INLINE RsslUInt32 bufGet32(char *in, RsslUInt32 *pValue)
{
	char *out = (char*)pValue;
	out[0] = in[3];
	out[1] = in[2];
	out[2] = in[1];
	out[3] = in[0];
	return 4;
}

RTR_C_ALWAYS_INLINE RsslUInt32 bufGetI32(char *in, RsslInt32 *pValue)
{
	char *out = (char*)pValue;
	out[0] = in[3];
	out[1] = in[2];
	out[2] = in[1];
	out[3] = in[0];
	return 4;
}


RTR_C_ALWAYS_INLINE RsslUInt32 bufPutLenSpecI64(char *out, RsslInt64 value)
{
	register RsslUInt64 ckval = (value >= 0) ? (value << 1) : (((-value) - 1) << 1); // shift once here instead of each if/else if

	if (ckval & RTR_ULL(0xFF00000000000000) ) {
		*((RsslUInt8*)out) = 0x08;
		bufPut64(out + 1, (RsslUInt64)value);
		return 9;
	} else if (ckval & RTR_ULL(0x00FF000000000000) ) {
		*((RsslUInt8*)out) = 0x07;
		bufPut56(out + 1, (RsslUInt64)value);
		return 8;
	} else if (ckval & RTR_ULL(0x0000FF0000000000) ) {
		*((RsslUInt8*)out) = 0x06;
		bufPut48(out + 1, (RsslUInt64)value);
		return 7;
	} else if (ckval & RTR_ULL(0x000000FF00000000) ) {
		*((RsslUInt8*)out) = 0x05;
		bufPut40(out + 1, (RsslUInt64)value);
		return 6;
	} else if (ckval & RTR_ULL(0x00000000FF000000) ) {
		*((RsslUInt8*)out) = 0x04;
		bufPut32(out + 1, (RsslUInt32)value);
		return 5;
	} else if (ckval & RTR_ULL(0x0000000000FF0000) ) {
		*((RsslUInt8*)out) = 0x03;
		bufPut24(out + 1, (RsslUInt32)value);
		return 4;
	} else if (ckval & RTR_ULL(0x000000000000FF00) ) {
		*((RsslUInt8*)out) = 0x02;
		bufPut16(out + 1, (RsslUInt16)value);
		return 3;
	} else {
		*((RsslUInt8*)out) = 0x01;
		bufPut8(out + 1, (RsslUInt8)value);
		return 2;
	}
}

RTR_C_ALWAYS_INLINE RsslUInt32 bufGetLenSpecI64(RsslInt64 *svalptr, char *sptr)
{
	register RsslUInt8 len = (*(RsslUInt8*)(sptr));
		
	switch (len)
	{
		case 0:
			*svalptr = 0;
			return 1;
		case 1:
			*svalptr = (RsslInt64)((*(RsslUInt8*)(sptr+1)));
			if (*svalptr & 0x80)
				*svalptr |= RTR_LL(0xFFFFFFFFFFFFFF00);
			return 2;
		case 2:
			*svalptr = (RsslInt64)swap16( *((RsslUInt16*)(sptr+1) ));
			if (*svalptr & 0x8000)
				*svalptr |= RTR_LL(0xFFFFFFFFFFFF0000);
			return 3;
		case 3:
			// there are actually 4 bytes on wire.  mask out length and swap
			*svalptr = (RsslInt64)swap32( (*(RsslUInt32*)(sptr)) & 0xFFFFFF00);
			if (*svalptr & 0x800000)
				*svalptr |= RTR_LL(0xFFFFFFFFFF000000);
			return 4;
		case 4:
			*svalptr = (RsslInt64)swap32( *((RsslUInt32*)(sptr+1)));
			if (*svalptr & 0x80000000)
				*svalptr |= RTR_LL(0xFFFFFFFF00000000);
			return 5;
		case 5:
			// only 6 bytes on wire including length.  swap 5 byte value and shift over three bytes before swap
			*svalptr = (RsslInt64)swap64( ((*(RsslUInt64*)(sptr+1)) << 24) );
			if (*svalptr & RTR_LL(0x8000000000))
				*svalptr |= RTR_LL(0xFFFFFF0000000000);
			return 6;
		case 6:
			// byteswap 8 byte value (2 bytes arent ours), but we need to shift over 16 before we swap
			*svalptr = (RsslInt64)swap64( ((*(RsslUInt64*)(sptr+1)) << 16 ) );
			if (*svalptr & RTR_LL(0x800000000000))
				*svalptr |= RTR_LL(0xFFFF000000000000);
			return 7;
		case 7:
			// there are a full 8 bytes on wire, byte swap with length, but mask it off
			*svalptr = (RsslInt64)swap64( (*(RsslUInt64*)(sptr)) & RTR_LL(0xFFFFFFFFFFFFFF00) );
			if (*svalptr & RTR_LL(0x80000000000000))
				*svalptr |= RTR_LL(0xFF00000000000000);
			return 8;
		case 8:
			// full 8 bytes on wire for value
			*svalptr = (RsslInt64)swap64( *((RsslUInt64*)(sptr+1)) );
			return 9;
	}
	return -1;
}

RTR_C_ALWAYS_INLINE void bufReplaceLenSpecI64(char *sptr, RsslInt64 newValue)
{
	switch (sptr[0])
	{
		case 1:
			bufPut8(sptr + 1, (RsslUInt8)newValue);
			return;
		case 2:
			bufPut16(sptr + 1, (RsslUInt16)newValue);
			return;
		case 3:
			bufPut24(sptr + 1, (RsslUInt32)newValue);
			return;
		case 4:
			bufPut32(sptr + 1, (RsslUInt32)newValue);
			return;
		case 5:
			bufPut40(sptr + 1, newValue);
			return;
		case 6:
			bufPut48(sptr + 1, newValue);
			return;
		case 7:
			bufPut56(sptr + 1, newValue);
			return;
		case 8:
			bufPut64(sptr + 1, newValue);
			return;
		default:
			return;
	}
}

RTR_C_ALWAYS_INLINE RsslInt32 bufPutResBitU15(char *dptr, RsslUInt16 sval)
{
	if (sval < (RsslUInt16)0x80) {
		*(RsslUInt8*)dptr = (RsslUInt8)sval;
		return 1;
	}
	sval |= 0x8000;
	bufPut16(dptr,sval);
	return 2;
}

RTR_C_ALWAYS_INLINE RsslUInt32 bufGetResBitU15(RsslUInt16 *dvalptr, char *sptr)
{
	register RsslUInt8 temp = (*(RsslUInt8*)sptr);
	if (temp & 0x80) {
		*dvalptr = swap16(*(RsslUInt16*)sptr) & 0x7FFF;
		return 2;
	}
	*dvalptr = (RsslUInt16)temp;
	return 1;
}


/* Max RsslUInt8 */
static const RsslUInt COS_MAXU_8 = 0xffULL;

/* Encodes a QueueData's extended header from the given info. */
static RsslRet _msgQueueSubstreamDataEncodeExtendedHeader(RsslBuffer *pTmpBuffer, 
	RsslUInt8 opcode, RsslBuffer *pFromQueue, RsslInt64 identifier, RsslInt64 timeout);

/* Encodes a QueueAck's extended header from the given info. */
static RsslRet _msgQueueSubstreamAckEncodeExtendedHeader(RsslBuffer *pTmpBuffer, 
	RsslBuffer *pFromQueue, RsslInt64 identifier);

RTR_C_ALWAYS_INLINE void tunnelStreamMsgKeyApplyCoSFilter(RsslMsgKey *pMsgKey, RsslClassOfService *pCos, RsslBool isProvider)
{
	pMsgKey->flags |= RSSL_MKF_HAS_FILTER;
	pMsgKey->filter = RDM_COS_COMMON_PROPERTIES_FLAG;

	if (pCos->authentication.type != RDM_COS_AU_NOT_REQUIRED)
		pMsgKey->filter |= RDM_COS_AUTHENTICATION_FLAG;

	if (pCos->flowControl.type != RDM_COS_FC_NONE)
		pMsgKey->filter |= RDM_COS_FLOW_CONTROL_FLAG;

	if (pCos->dataIntegrity.type != RDM_COS_DI_BEST_EFFORT)
		pMsgKey->filter |= RDM_COS_DATA_INTEGRITY_FLAG;

	if (pCos->guarantee.type != RDM_COS_GU_NONE)
		pMsgKey->filter |= RDM_COS_GUARANTEE_FLAG;
}


RsslRet rsslEncodeClassOfService(RsslEncodeIterator *pIter, RsslClassOfService *pCos, RsslUInt32 filter,
		RsslBool isProvider, RsslUInt streamVersion, RsslErrorInfo *pErrorInfo)
{
	RsslFilterList		filterList;
	RsslFilterEntry		filterEntry;
	RsslElementList		elemList;
	RsslElementEntry	elemEntry;
	RsslRet				ret;
	RsslUInt			tempUInt;
	RsslInt				tempInt;

	rsslClearFilterList(&filterList);
	filterList.containerType = RSSL_DT_ELEMENT_LIST;

	if ((ret = rsslEncodeFilterListInit(pIter, &filterList)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
			"TunnelStream ClassOfService rsslEncodeFilterListInit failed: %d", ret);
		return RSSL_RET_FAILURE;
	}

	/* Common properties */
	rsslClearFilterEntry(&filterEntry);
	filterEntry.action = RSSL_FTEA_SET_ENTRY;
	filterEntry.id = RDM_COS_COMMON_PROPERTIES_ID;
	if ((ret = rsslEncodeFilterEntryInit(pIter, &filterEntry, 0)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
			"TunnelStream ClassOfService common rsslEncodeFilterEntryInit failed: %d", ret);
		return RSSL_RET_FAILURE;
	}

	rsslClearElementList(&elemList);
	rsslElementListApplyHasStandardData(&elemList);
	if ((ret = rsslEncodeElementListInit(pIter, &elemList, NULL, 0)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
			"TunnelStream ClassOfService common rsslEncodeElementListInit failed: %d", ret);
		return RSSL_RET_FAILURE;
	}

	if (isProvider)
	{
		/* MaxMsgSize */
		rsslClearElementEntry(&elemEntry);
		elemEntry.name = RSSL_ENAME_COS_MAX_MSG_SIZE;
		elemEntry.dataType = RSSL_DT_UINT;
		if (streamVersion > 1)
		{
			tempUInt = pCos->common.maxMsgSize;
		}
		else
		{
			tempUInt = pCos->common.maxFragmentSize;
		}
		if ((ret = rsslEncodeElementEntry(pIter, &elemEntry, &tempUInt)) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
				"TunnelStream ClassOfService common.maxMsgSize rsslEncodeElementEntry failed: %d", ret);
			return RSSL_RET_FAILURE;
		}
		if (streamVersion > 1)
		{
			/* MaxFragmentSize */
			rsslClearElementEntry(&elemEntry);
			elemEntry.name = RSSL_ENAME_COS_MAX_FRAGMENT_SIZE;
			elemEntry.dataType = RSSL_DT_UINT;
			tempUInt = pCos->common.maxFragmentSize;
			if ((ret = rsslEncodeElementEntry(pIter, &elemEntry, &tempUInt)) != RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
					"TunnelStream ClassOfService common.maxFragmentSize rsslEncodeElementEntry failed: %d", ret);
				return RSSL_RET_FAILURE;
			}
			/* Support Fragmentation */
			rsslClearElementEntry(&elemEntry);
			elemEntry.name = RSSL_ENAME_COS_SUPPS_FRAGMENTATION;
			elemEntry.dataType = RSSL_DT_UINT;
			tempUInt = 1;
			if ((ret = rsslEncodeElementEntry(pIter, &elemEntry, &tempUInt)) != RSSL_RET_SUCCESS)
			{
				rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
					"TunnelStream ClassOfService common.supportFragmentation rsslEncodeElementEntry failed: %d", ret);
				return RSSL_RET_FAILURE;
			}
		}
	}

	/* ProtocolType */
	rsslClearElementEntry(&elemEntry);
	elemEntry.name = RSSL_ENAME_COS_PROT_TYPE;
	elemEntry.dataType = RSSL_DT_UINT;
	tempUInt = pCos->common.protocolType;
	if ((ret = rsslEncodeElementEntry(pIter, &elemEntry, &tempUInt)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
			"TunnelStream ClassOfService common.protocolType rsslEncodeElementEntry failed: %d", ret);
		return RSSL_RET_FAILURE;
	}

	/* ProtocolMajorVersion */
	rsslClearElementEntry(&elemEntry);
	elemEntry.name = RSSL_ENAME_COS_PROT_MAJOR_VERSION;
	elemEntry.dataType = RSSL_DT_UINT;
	tempUInt = pCos->common.protocolMajorVersion;
	if ((ret = rsslEncodeElementEntry(pIter, &elemEntry, &tempUInt)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
			"TunnelStream ClassOfService common.protcolMajorVersion rsslEncodeElementEntry failed: %d", ret);
		return RSSL_RET_FAILURE;
	}

	/* ProtocolMinorVersion */
	rsslClearElementEntry(&elemEntry);
	elemEntry.name = RSSL_ENAME_COS_PROT_MINOR_VERSION;
	elemEntry.dataType = RSSL_DT_UINT;
	tempUInt = pCos->common.protocolMinorVersion;
	if ((ret = rsslEncodeElementEntry(pIter, &elemEntry, &tempUInt)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
			"TunnelStream ClassOfService common.protocolMinorVersion rsslEncodeElementEntry failed: %d", ret);
		return RSSL_RET_FAILURE;
	}

	/* StreamVersion */
	rsslClearElementEntry(&elemEntry);
	elemEntry.name = RSSL_ENAME_COS_STREAM_VERSION;
	elemEntry.dataType = RSSL_DT_UINT;

	tempUInt = streamVersion;
	if ((ret = rsslEncodeElementEntry(pIter, &elemEntry, &tempUInt)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
		"TunnelStream ClassOfService common.streamVersion rsslEncodeElementEntry failed: %d", ret);
		return RSSL_RET_FAILURE;
	}

	if ((ret = rsslEncodeElementListComplete(pIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
			"TunnelStream ClassOfService common rsslEncodeElementListComplete failed: %d", ret);
		return RSSL_RET_FAILURE;
	}

	if ((ret = rsslEncodeFilterEntryComplete(pIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
			"TunnelStream ClassOfService common rsslEncodeFilterEntryComplete failed: %d", ret);
		return RSSL_RET_FAILURE;
	}

	/* Authentication properties */
	rsslClearFilterEntry(&filterEntry);
	filterEntry.action = RSSL_FTEA_SET_ENTRY;
	filterEntry.id = RDM_COS_AUTHENTICATION_ID;
	if ((ret = rsslEncodeFilterEntryInit(pIter, &filterEntry, 0)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
				"TunnelStream ClassOfService authentication rsslEncodeFilterEntryInit failed: %d", ret);
		return RSSL_RET_FAILURE;
	}

	rsslClearElementList(&elemList);
	rsslElementListApplyHasStandardData(&elemList);
	if ((ret = rsslEncodeElementListInit(pIter, &elemList, NULL, 0)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
				"TunnelStream ClassOfService authentication rsslEncodeElementListInit failed: %d", ret);
		return RSSL_RET_FAILURE;
	}

	/* Type */
	rsslClearElementEntry(&elemEntry);
	elemEntry.name = RSSL_ENAME_COS_TYPE;
	elemEntry.dataType = RSSL_DT_UINT;
	tempUInt = pCos->authentication.type;
	if ((ret = rsslEncodeElementEntry(pIter, &elemEntry, &tempUInt)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
				"TunnelStream ClassOfService authentication.type rsslEncodeElementEntry failed: %d", ret);
		return RSSL_RET_FAILURE;
	}

	if ((ret = rsslEncodeElementListComplete(pIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
				"TunnelStream ClassOfService authentication rsslEncodeElementListComplete failed: %d", ret);
		return RSSL_RET_FAILURE;
	}

	if ((ret = rsslEncodeFilterEntryComplete(pIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
				"TunnelStream ClassOfService authentication rsslEncodeFilterEntryComplete failed: %d", ret);
		return RSSL_RET_FAILURE;
	}

	/* Flow control properties */
	rsslClearFilterEntry(&filterEntry);
	filterEntry.action = RSSL_FTEA_SET_ENTRY;
	filterEntry.id = RDM_COS_FLOW_CONTROL_ID;
	if ((ret = rsslEncodeFilterEntryInit(pIter, &filterEntry, 0)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
				"TunnelStream ClassOfService flowControl rsslEncodeFilterEntryInit failed: %d", ret);
		return RSSL_RET_FAILURE;
	}

	rsslClearElementList(&elemList);
	rsslElementListApplyHasStandardData(&elemList);
	if ((ret = rsslEncodeElementListInit(pIter, &elemList, NULL, 0)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
				"TunnelStream ClassOfService flowControl rsslEncodeElementListInit failed: %d", ret);
		return RSSL_RET_FAILURE;
	}

	/* Type */
	rsslClearElementEntry(&elemEntry);
	elemEntry.name = RSSL_ENAME_COS_TYPE;
	elemEntry.dataType = RSSL_DT_UINT;
	tempUInt = pCos->flowControl.type;
	if ((ret = rsslEncodeElementEntry(pIter, &elemEntry, &tempUInt)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
				"TunnelStream ClassOfService flowControl.type rsslEncodeElementEntry failed: %d", ret);
		return RSSL_RET_FAILURE;
	}

	if (pCos->flowControl.type != RDM_COS_FC_NONE && pCos->flowControl.recvWindowSize >= 0)
	{
		/* RecvWindowSize */
		rsslClearElementEntry(&elemEntry);
		elemEntry.name = RSSL_ENAME_COS_RECV_WINDOW_SIZE;
		elemEntry.dataType = RSSL_DT_INT;
		tempInt = pCos->flowControl.recvWindowSize;
		if ((ret = rsslEncodeElementEntry(pIter, &elemEntry, &tempInt)) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
					"TunnelStream ClassOfService flowControl.recvWindowSize rsslEncodeElementEntry failed: %d", ret);
			return RSSL_RET_FAILURE;
		}
	}

	if ((ret = rsslEncodeElementListComplete(pIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
			"TunnelStream ClassOfService flowControl rsslEncodeElementListComplete failed: %d", ret);
		return RSSL_RET_FAILURE;
	}

	if ((ret = rsslEncodeFilterEntryComplete(pIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
			"TunnelStream ClassOfService flowControl rsslEncodeFilterEntryComplete failed: %d", ret);
		return RSSL_RET_FAILURE;
	}


	/* Data integrity properties */
	rsslClearFilterEntry(&filterEntry);
	filterEntry.action = RSSL_FTEA_SET_ENTRY;
	filterEntry.id = RDM_COS_DATA_INTEGRITY_ID;
	if ((ret = rsslEncodeFilterEntryInit(pIter, &filterEntry, 0)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
				"TunnelStream ClassOfService dataIntegrity rsslEncodeFilterEntryInit failed: %d", ret);
		return RSSL_RET_FAILURE;
	}

	rsslClearElementList(&elemList);
	rsslElementListApplyHasStandardData(&elemList);
	if ((ret = rsslEncodeElementListInit(pIter, &elemList, NULL, 0)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
				"TunnelStream ClassOfService dataIntegrity rsslEncodeElementListInit failed: %d", ret);
		return RSSL_RET_FAILURE;
	}

	rsslClearElementEntry(&elemEntry);
	elemEntry.name = RSSL_ENAME_COS_TYPE;
	elemEntry.dataType = RSSL_DT_UINT;
	tempUInt = pCos->dataIntegrity.type;
	if ((ret = rsslEncodeElementEntry(pIter, &elemEntry, &tempUInt)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
				"TunnelStream ClassOfService dataIntegrity.type rsslEncodeElementEntry failed: %d", ret);
		return RSSL_RET_FAILURE;
	}

	if ((ret = rsslEncodeElementListComplete(pIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
				"TunnelStream ClassOfService dataIntegrity rsslEncodeElementListComplete failed: %d", ret);
		return RSSL_RET_FAILURE;
	}

	if ((ret = rsslEncodeFilterEntryComplete(pIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
				"TunnelStream ClassOfService dataIntegrity rsslEncodeFilterEntryComplete failed: %d", ret);
		return RSSL_RET_FAILURE;
	}

	if (filter & RDM_COS_GUARANTEE_FLAG)
	{
		/* Data guarantee properties */
		rsslClearFilterEntry(&filterEntry);
		filterEntry.action = RSSL_FTEA_SET_ENTRY;
		filterEntry.id = RDM_COS_GUARANTEE_ID;
		if ((ret = rsslEncodeFilterEntryInit(pIter, &filterEntry, 0)) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
					"TunnelStream ClassOfService guarantee rsslEncodeFilterEntryInit failed: %d", ret);
			return RSSL_RET_FAILURE;
		}

		rsslClearElementList(&elemList);
		rsslElementListApplyHasStandardData(&elemList);
		if ((ret = rsslEncodeElementListInit(pIter, &elemList, NULL, 0)) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
					"TunnelStream ClassOfService guarantee rsslEncodeElementListInit failed: %d", ret);
			return RSSL_RET_FAILURE;
		}

		rsslClearElementEntry(&elemEntry);
		elemEntry.name = RSSL_ENAME_COS_TYPE;
		elemEntry.dataType = RSSL_DT_UINT;
		tempUInt = pCos->guarantee.type;
		if ((ret = rsslEncodeElementEntry(pIter, &elemEntry, &tempUInt)) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
					"TunnelStream ClassOfService guarantee.type rsslEncodeElementEntry failed: %d", ret);
			return RSSL_RET_FAILURE;
		}

		if ((ret = rsslEncodeElementListComplete(pIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
					"TunnelStream ClassOfService guarantee rsslEncodeElementListComplete failed: %d", ret);
			return RSSL_RET_FAILURE;
		}

		if ((ret = rsslEncodeFilterEntryComplete(pIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
					"TunnelStream ClassOfService guarantee rsslEncodeFilterEntryComplete failed: %d", ret);
			return RSSL_RET_FAILURE;
		}
	}

	if ((ret = rsslEncodeFilterListComplete(pIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
			"TunnelStream ClassOfService rsslEncodeFilterListComplete failed: %d", ret);
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

RsslRet rsslGetClassOfServiceStreamVersion(RsslDecodeIterator *pIter, RsslUInt *pStreamVersion,
		RsslErrorInfo *pErrorInfo)
{
	RsslFilterList		filterList;
	RsslFilterEntry		filterEntry;
	RsslElementList		elemList;
	RsslElementEntry	elemEntry;
	RsslRet				ret;
	RsslUInt			tempUInt;

	*pStreamVersion = COS_CURRENT_STREAM_VERSION;

	if ((ret = rsslDecodeFilterList(pIter, &filterList)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
				"ClassOfService rsslDecodeFilterList failed: %d", ret);
		return ret;
	}

	while ((ret = rsslDecodeFilterEntry(pIter, &filterEntry)) != RSSL_RET_END_OF_CONTAINER)
	{
		if (ret != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
					"ClassOfService rsslDecodeFilterEntry failed: %d", ret);
			return ret;
		}

		switch(filterEntry.id)
		{
			case RDM_COS_COMMON_PROPERTIES_ID:

				if (filterEntry.containerType != RSSL_DT_ELEMENT_LIST)
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
							"Decoded ClassOfService common FilterEntry has wrong containerType: %u", filterEntry.containerType);
					return RSSL_RET_FAILURE;
				}

				if ((ret = rsslDecodeElementList(pIter, &elemList, NULL)) != RSSL_RET_SUCCESS)
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
							"ClassOfService common rsslDecodeElementList failed: %d", ret);
					return RSSL_RET_FAILURE;
				}

				while ((ret = rsslDecodeElementEntry(pIter, &elemEntry)) != RSSL_RET_END_OF_CONTAINER)
				{
					if (ret != RSSL_RET_SUCCESS)
					{
						rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
								"ClassOfService common rsslDecodeElementEntry failed: %d", ret);
						return RSSL_RET_FAILURE;
					}

					if (rsslBufferIsEqual(&elemEntry.name, &RSSL_ENAME_COS_STREAM_VERSION))
					{
						if (elemEntry.dataType != RSSL_DT_UINT)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
									"Decoded ClassOfService common.streamVersion has wrong dataType: %u", elemEntry.dataType);
							return RSSL_RET_FAILURE;
						}

						if ((ret = rsslDecodeUInt(pIter, &tempUInt)) != RSSL_RET_SUCCESS)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
									"ClassOfService common.streamVersion decode failed: %d", ret);
							return RSSL_RET_FAILURE;
						}

						if (tempUInt > COS_MAXU_8)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
									"Decoded ClassOfService common.streamVersion %llu is out of range", tempUInt);
							return RSSL_RET_FAILURE;
						}

						*pStreamVersion = tempUInt;
						return RSSL_RET_SUCCESS;
					}
				}

				break;

			default:
				/* Ignore */
				break;
		}
	}

	return RSSL_RET_SUCCESS;

}

RsslRet rsslDecodeClassOfService(RsslDecodeIterator *pIter, RsslClassOfService *pCos,
		RsslUInt *pStreamVersion, RsslErrorInfo *pErrorInfo)
{
	RsslFilterList		filterList;
	RsslFilterEntry		filterEntry;
	RsslElementList		elemList;
	RsslElementEntry	elemEntry;
	RsslRet				ret;
	RsslUInt			tempUInt;
	RsslInt				tempInt;

	rsslClearClassOfService(pCos);
	if (pStreamVersion != NULL)
		*pStreamVersion = COS_CURRENT_STREAM_VERSION;

	if ((ret = rsslDecodeFilterList(pIter, &filterList)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
				"ClassOfService rsslDecodeFilterList failed: %d", ret);
		return ret;
	}

	while ((ret = rsslDecodeFilterEntry(pIter, &filterEntry)) != RSSL_RET_END_OF_CONTAINER)
	{
		if (ret != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
					"ClassOfService rsslDecodeFilterEntry failed: %d", ret);
			return ret;
		}

		switch(filterEntry.id)
		{
			case RDM_COS_COMMON_PROPERTIES_ID:

				if (filterEntry.containerType != RSSL_DT_ELEMENT_LIST)
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
							"Decoded ClassOfService common FilterEntry has wrong containerType: %u", filterEntry.containerType);
					return RSSL_RET_FAILURE;
				}

				if ((ret = rsslDecodeElementList(pIter, &elemList, NULL)) != RSSL_RET_SUCCESS)
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
							"ClassOfService common rsslDecodeElementList failed: %d", ret);
					return RSSL_RET_FAILURE;
				}

				while ((ret = rsslDecodeElementEntry(pIter, &elemEntry)) != RSSL_RET_END_OF_CONTAINER)
				{
					if (ret != RSSL_RET_SUCCESS)
					{
						rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
								"ClassOfService common rsslDecodeElementEntry failed: %d", ret);
						return RSSL_RET_FAILURE;
					}

					if (rsslBufferIsEqual(&elemEntry.name, &RSSL_ENAME_COS_MAX_MSG_SIZE))
					{
						if (elemEntry.dataType != RSSL_DT_UINT)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
									"Decoded ClassOfService common.maxMsgSize has wrong dataType: %u", elemEntry.dataType);
							return RSSL_RET_FAILURE;
						}

						if ((ret = rsslDecodeUInt(pIter, &tempUInt)) != RSSL_RET_SUCCESS)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
									"ClassOfService common.maxMsgSize decode failed: %d", ret);
							return RSSL_RET_FAILURE;
						}

						pCos->common.maxMsgSize = tempUInt;
					}
					else if (rsslBufferIsEqual(&elemEntry.name, &RSSL_ENAME_COS_MAX_FRAGMENT_SIZE))
					{
						if (elemEntry.dataType != RSSL_DT_UINT)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
								"Decoded ClassOfService common.maxFragmentSize has wrong dataType: %u", elemEntry.dataType);
							return RSSL_RET_FAILURE;
						}

						if ((ret = rsslDecodeUInt(pIter, &tempUInt)) != RSSL_RET_SUCCESS)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
								"ClassOfService common.maxMsgSize decode failed: %d", ret);
							return RSSL_RET_FAILURE;
						}

						pCos->common.maxFragmentSize = tempUInt;
					}
					else if (rsslBufferIsEqual(&elemEntry.name, &RSSL_ENAME_COS_SUPPS_FRAGMENTATION))
					{
						if (elemEntry.dataType != RSSL_DT_UINT)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
								"Decoded ClassOfService common.supportsFragmentation has wrong dataType: %u", elemEntry.dataType);
							return RSSL_RET_FAILURE;
						}

						if ((ret = rsslDecodeUInt(pIter, &tempUInt)) != RSSL_RET_SUCCESS)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
								"ClassOfService common.supportsFragmentation decode failed: %d", ret);
							return RSSL_RET_FAILURE;
						}

						pCos->common.supportsFragmentation = tempUInt;
					}
					else if (rsslBufferIsEqual(&elemEntry.name, &RSSL_ENAME_COS_PROT_TYPE))
					{
						if (elemEntry.dataType != RSSL_DT_UINT)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
									"Decoded ClassOfService common.protocolType has wrong dataType: %u", elemEntry.dataType);
							return RSSL_RET_FAILURE;
						}

						if ((ret = rsslDecodeUInt(pIter, &tempUInt)) != RSSL_RET_SUCCESS)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
									"ClassOfService common.protocolType decode failed: %d", ret);
							return RSSL_RET_FAILURE;
						}

						if (tempUInt > COS_MAXU_8)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
									"Decoded ClassOfService common.protocolType %llu is out of range", tempUInt);
							return RSSL_RET_FAILURE;
						}

						pCos->common.protocolType = (RsslUInt8)tempUInt;
					}
					else if (rsslBufferIsEqual(&elemEntry.name, &RSSL_ENAME_COS_PROT_MAJOR_VERSION))
					{
						if (elemEntry.dataType != RSSL_DT_UINT)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
									"Decoded ClassOfService common.protocolMajorVersion has wrong dataType: %u", elemEntry.dataType);
							return RSSL_RET_FAILURE;
						}

						if ((ret = rsslDecodeUInt(pIter, &tempUInt)) != RSSL_RET_SUCCESS)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
									"ClassOfService common.protocolMajorVersion decode failed: %d", ret);
							return RSSL_RET_FAILURE;
						}

						if (tempUInt > COS_MAXU_8)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
									"Decoded ClassOfService common.protocolMajorVersion %llu is out of range", tempUInt);
							return RSSL_RET_FAILURE;
						}

						pCos->common.protocolMajorVersion = (RsslUInt8)tempUInt;
					}
					else if (rsslBufferIsEqual(&elemEntry.name, &RSSL_ENAME_COS_PROT_MINOR_VERSION))
					{
						if (elemEntry.dataType != RSSL_DT_UINT)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
									"Decoded ClassOfService common.protocolMinorVersion has wrong dataType: %u", elemEntry.dataType);
							return RSSL_RET_FAILURE;
						}

						if ((ret = rsslDecodeUInt(pIter, &tempUInt)) != RSSL_RET_SUCCESS)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
									"ClassOfService common.protocolMinorVersion decode failed: %d", ret);
							return RSSL_RET_FAILURE;
						}

						if (tempUInt > COS_MAXU_8)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
									"Decoded ClassOfService common.protocolMinorVersion %llu is out of range", tempUInt);
							return RSSL_RET_FAILURE;
						}

						pCos->common.protocolMinorVersion = (RsslUInt8)tempUInt;
					}
					else if (pStreamVersion != NULL && rsslBufferIsEqual(&elemEntry.name, &RSSL_ENAME_COS_STREAM_VERSION))
					{
						if (elemEntry.dataType != RSSL_DT_UINT)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
									"Decoded ClassOfService common.streamVersion has wrong dataType: %u", elemEntry.dataType);
							return RSSL_RET_FAILURE;
						}

						if ((ret = rsslDecodeUInt(pIter, &tempUInt)) != RSSL_RET_SUCCESS)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
									"ClassOfService common.streamVersion decode failed: %d", ret);
							return RSSL_RET_FAILURE;
						}

						if (tempUInt > COS_MAXU_8)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
									"Decoded ClassOfService common.streamVersion %llu is out of range", tempUInt);
							return RSSL_RET_FAILURE;
						}

						*pStreamVersion = tempUInt;
					}
				}

				break;

			case RDM_COS_AUTHENTICATION_ID:
				if (filterEntry.containerType != RSSL_DT_ELEMENT_LIST)
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
							"Decoded ClassOfService authentication FilterEntry has wrong containerType: %u", filterEntry.containerType);
					return RSSL_RET_FAILURE;
				}

				if ((ret = rsslDecodeElementList(pIter, &elemList, NULL)) != RSSL_RET_SUCCESS)
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
							"ClassOfService authentication rsslDecodeElementList failed: %d", ret);
					return RSSL_RET_FAILURE;
				}

				while ((ret = rsslDecodeElementEntry(pIter, &elemEntry)) != RSSL_RET_END_OF_CONTAINER)
				{
					if (ret != RSSL_RET_SUCCESS)
					{
						rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
								"ClassOfService authentication rsslDecodeElementEntry failed: %d", ret);
						return RSSL_RET_FAILURE;
					}

					if (rsslBufferIsEqual(&elemEntry.name, &RSSL_ENAME_COS_TYPE))
					{
						if (elemEntry.dataType != RSSL_DT_UINT)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
									"Decoded ClassOfService authentication.type has wrong dataType: %u", elemEntry.dataType);
							return RSSL_RET_FAILURE;
						}

						if ((ret = rsslDecodeUInt(pIter, &tempUInt)) != RSSL_RET_SUCCESS)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
									"ClassOfService authentication.type decode failed: %d", ret);
							return RSSL_RET_FAILURE;
						}

						if (tempUInt > COS_MAXU_8)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
									"Decoded ClassOfService authentication.type %llu is out of range", tempUInt);
							return RSSL_RET_FAILURE;
						}

						pCos->authentication.type = (RDMClassOfServiceAuthenticationType)tempUInt;
					}
				}
				break;

			case RDM_COS_FLOW_CONTROL_ID:
				if (filterEntry.containerType != RSSL_DT_ELEMENT_LIST)
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
							"Decoded ClassOfService flowControl FilterEntry has wrong containerType: %u", filterEntry.containerType);
					return RSSL_RET_FAILURE;
				}

				if ((ret = rsslDecodeElementList(pIter, &elemList, NULL)) != RSSL_RET_SUCCESS)
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
							"ClassOfService flowControl rsslDecodeElementList failed: %d", ret);
					return RSSL_RET_FAILURE;
				}

				while ((ret = rsslDecodeElementEntry(pIter, &elemEntry)) != RSSL_RET_END_OF_CONTAINER)
				{
					if (ret != RSSL_RET_SUCCESS)
					{
						rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
								"ClassOfService flowControl rsslDecodeElementEntry failed: %d", ret);
						return RSSL_RET_FAILURE;
					}

					if (rsslBufferIsEqual(&elemEntry.name, &RSSL_ENAME_COS_TYPE))
					{
						if (elemEntry.dataType != RSSL_DT_UINT)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
									"Decoded ClassOfService flowControl.type has wrong dataType: %u", elemEntry.dataType);
							return RSSL_RET_FAILURE;
						}

						if ((ret = rsslDecodeUInt(pIter, &tempUInt)) != RSSL_RET_SUCCESS)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
									"ClassOfService flowControl.type decode failed: %d", ret);
							return RSSL_RET_FAILURE;
						}

						if (tempUInt > COS_MAXU_8)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
								"Decoded ClassOfService flowControl.type %llu is out of range", tempUInt);
							return RSSL_RET_FAILURE;
						}


						pCos->flowControl.type = tempUInt;
					}
					else if (rsslBufferIsEqual(&elemEntry.name, &RSSL_ENAME_COS_RECV_WINDOW_SIZE))
					{
						if (elemEntry.dataType != RSSL_DT_INT)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
									"Decoded ClassOfService flowControl.recvWindowSize has wrong dataType: %u", elemEntry.dataType);
							return RSSL_RET_FAILURE;
						}

						if ((ret = rsslDecodeInt(pIter, &tempInt)) != RSSL_RET_SUCCESS)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
									"ClassOfService flowControl.recvWindowSize decode failed: %d", ret);
							return RSSL_RET_FAILURE;
						}

						pCos->flowControl.recvWindowSize = tempInt;
					}
				}
				break;

			case RDM_COS_DATA_INTEGRITY_ID:
				if (filterEntry.containerType != RSSL_DT_ELEMENT_LIST)
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
							"Decoded ClassOfService dataIntegrity FilterEntry has wrong containerType: %u", filterEntry.containerType);
					return RSSL_RET_FAILURE;
				}

				if ((ret = rsslDecodeElementList(pIter, &elemList, NULL)) != RSSL_RET_SUCCESS)
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
							"ClassOfService dataIntegrity rsslDecodeElementList failed: %d", ret);
					return RSSL_RET_FAILURE;
				}

				while ((ret = rsslDecodeElementEntry(pIter, &elemEntry)) != RSSL_RET_END_OF_CONTAINER)
				{
					if (ret != RSSL_RET_SUCCESS)
					{
						rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
								"ClassOfService dataIntegrity rsslDecodeElementEntry failed: %d", ret);
						return RSSL_RET_FAILURE;
					}

					if (rsslBufferIsEqual(&elemEntry.name, &RSSL_ENAME_COS_TYPE))
					{
						if (elemEntry.dataType != RSSL_DT_UINT)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
									"Decoded ClassOfService dataIntegrity.type has wrong dataType: %u", elemEntry.dataType);
							return RSSL_RET_FAILURE;
						}

						if ((ret = rsslDecodeUInt(pIter, &tempUInt)) != RSSL_RET_SUCCESS)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
									"ClassOfService dataIntegrity.type decode failed: %d", ret);
							return RSSL_RET_FAILURE;
						}

						if (tempUInt > COS_MAXU_8)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
								"Decoded ClassOfService dataIntegrity.type %llu is out of range", tempUInt);
							return RSSL_RET_FAILURE;
						}

						pCos->dataIntegrity.type = tempUInt;
					}
				}
				break;

			case RDM_COS_GUARANTEE_ID:
				if (filterEntry.containerType != RSSL_DT_ELEMENT_LIST)
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
							"Decoded ClassOfService guarantee FilterEntry has wrong containerType: %u", filterEntry.containerType);
					return RSSL_RET_FAILURE;
				}

				if ((ret = rsslDecodeElementList(pIter, &elemList, NULL)) != RSSL_RET_SUCCESS)
				{
					rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
							"ClassOfService guarantee rsslDecodeElementList failed: %d", ret);
					return RSSL_RET_FAILURE;
				}

				while ((ret = rsslDecodeElementEntry(pIter, &elemEntry)) != RSSL_RET_END_OF_CONTAINER)
				{
					if (ret != RSSL_RET_SUCCESS)
					{
						rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
								"ClassOfService guarantee rsslDecodeElementEntry failed: %d", ret);
						return RSSL_RET_FAILURE;
					}

					if (rsslBufferIsEqual(&elemEntry.name, &RSSL_ENAME_COS_TYPE))
					{
						if (elemEntry.dataType != RSSL_DT_UINT)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
									"Decoded ClassOfService guarantee.type has wrong dataType: %u", elemEntry.dataType);
							return RSSL_RET_FAILURE;
						}

						if ((ret = rsslDecodeUInt(pIter, &tempUInt)) != RSSL_RET_SUCCESS)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
									"ClassOfService guarantee.type decode failed: %d", ret);
							return RSSL_RET_FAILURE;
						}

						if (tempUInt > COS_MAXU_8)
						{
							rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
								"Decoded ClassOfService guarantee.type %llu is out of range", tempUInt);
							return RSSL_RET_FAILURE;
						}

						pCos->guarantee.type = tempUInt;
					}
				}
				break;

			default:
				/* Ignore */
				break;
		}
	}

	return RSSL_RET_SUCCESS;

}


RsslRet tunnelStreamRequestEncode(RsslEncodeIterator *pIter, TunnelStreamRequest *requestHeader, RsslClassOfService *pCos, RsslUInt streamVersion, RsslErrorInfo *pErrorInfo)
{
	/* Send a request message to open the initial tunnel stream. */
	RsslRequestMsg	requestMsg;
	RsslRet			ret;
	tunnelStreamRequestSetRsslMsg(requestHeader, &requestMsg, pCos);

	if ((ret = rsslEncodeMsgInit(pIter, (RsslMsg *)&requestMsg, 0)) != RSSL_RET_ENCODE_CONTAINER)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
			"TunnelStreamRequest ClassOfService rsslEncodeMsgInit failed: %d", ret);
		return RSSL_RET_FAILURE;
	}

	if ((ret = rsslEncodeClassOfService(pIter, pCos, requestMsg.msgBase.msgKey.filter, RSSL_FALSE, streamVersion, pErrorInfo)) != RSSL_RET_SUCCESS)
		return ret;

	if ((ret = rsslEncodeMsgComplete(pIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
			"TunnelStreamRequest ClassOfService rsslEncodeMsgComplete failed: %d", ret);
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

void tunnelStreamRequestSetRsslMsg(TunnelStreamRequest *requestHeader,
		RsslRequestMsg *oRsslMsg, RsslClassOfService *pCos)
{
	rsslClearRequestMsg(oRsslMsg);
	oRsslMsg->msgBase.msgClass = RSSL_MC_REQUEST;
	oRsslMsg->msgBase.streamId = requestHeader->base.streamId;
	oRsslMsg->msgBase.domainType = requestHeader->base.domainType;
	oRsslMsg->msgBase.containerType = RSSL_DT_FILTER_LIST;

	rsslRequestMsgApplyStreaming(oRsslMsg);
	rsslRequestMsgApplyPrivateStream(oRsslMsg);
	rsslRequestMsgApplyQualifiedStream(oRsslMsg);

	rsslMsgKeyApplyHasName(&oRsslMsg->msgBase.msgKey);
	oRsslMsg->msgBase.msgKey.name = requestHeader->name;
	rsslMsgKeyApplyHasServiceId(&oRsslMsg->msgBase.msgKey);
	oRsslMsg->msgBase.msgKey.serviceId = requestHeader->serviceId;

	tunnelStreamMsgKeyApplyCoSFilter(&oRsslMsg->msgBase.msgKey, pCos, RSSL_FALSE);
}

RsslRet tunnelStreamRefreshEncode(RsslEncodeIterator *pIter, TunnelStreamRefresh *pTunnelRefresh, RsslClassOfService *pCos, RsslUInt streamVersion, RsslErrorInfo *pErrorInfo)
{
	RsslRefreshMsg refreshMsg;
	RsslRet ret; 

	assert(pTunnelRefresh->containerType == RSSL_DT_FILTER_LIST);

	tunnelStreamRefreshSetRsslMsg(pTunnelRefresh, &refreshMsg, pCos);

	if ((ret = rsslEncodeMsgInit(pIter, (RsslMsg *)&refreshMsg, 0)) != RSSL_RET_ENCODE_CONTAINER)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
			"TunnelStreamRefresh ClassOfService rsslEncodeMsgInit failed: %d", ret);
		return RSSL_RET_FAILURE;
	}

	if ((ret = rsslEncodeClassOfService(pIter, pCos, refreshMsg.msgBase.msgKey.filter, RSSL_TRUE, streamVersion, pErrorInfo)) != RSSL_RET_SUCCESS)
		return ret;

	if ((ret = rsslEncodeMsgComplete(pIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
			"TunnelStreamRefresh ClassOfService rsslEncodeMsgComplete failed: %d", ret);
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

void tunnelStreamRefreshSetRsslMsg(TunnelStreamRefresh *pTunnelRefresh, RsslRefreshMsg *oRsslMsg, RsslClassOfService *pCos)
{
	assert(pCos != NULL);

	rsslClearRefreshMsg(oRsslMsg);
	oRsslMsg->msgBase.streamId = pTunnelRefresh->base.streamId;
	oRsslMsg->msgBase.domainType = pTunnelRefresh->base.domainType;
	oRsslMsg->msgBase.containerType = RSSL_DT_FILTER_LIST;

	rsslRefreshMsgApplyPrivateStream(oRsslMsg);
	rsslRefreshMsgApplyQualifiedStream(oRsslMsg);
	rsslRefreshMsgApplyRefreshComplete(oRsslMsg);
	rsslRefreshMsgApplySolicited(oRsslMsg);
	rsslRefreshMsgApplyClearCache(oRsslMsg);

	oRsslMsg->msgBase.msgKey.flags = RSSL_MKF_HAS_FILTER;

	if (pTunnelRefresh->flags & TS_RFMF_HAS_NAME)
	{
		oRsslMsg->flags |= RSSL_RFMF_HAS_MSG_KEY;
		oRsslMsg->msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
		oRsslMsg->msgBase.msgKey.name = pTunnelRefresh->name;
	}

	if (pTunnelRefresh->flags & TS_RFMF_HAS_SERVICE_ID)
	{
		oRsslMsg->flags |= RSSL_RFMF_HAS_MSG_KEY;
		oRsslMsg->msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
		oRsslMsg->msgBase.msgKey.serviceId = pTunnelRefresh->serviceId;
	}

	tunnelStreamMsgKeyApplyCoSFilter(&oRsslMsg->msgBase.msgKey, pCos, RSSL_TRUE);

	oRsslMsg->state.streamState = RSSL_STREAM_OPEN;
	oRsslMsg->state.dataState = RSSL_DATA_OK;
}

/* Encodes a tunnel stream status. */
RsslRet tunnelStreamStatusEncode(RsslEncodeIterator *pIter, TunnelStreamStatus *pTunnelStatus, RsslClassOfService *pCos, RsslUInt streamVersion, RsslErrorInfo *pErrorInfo)
{
	RsslStatusMsg statusMsg;
	RsslRet ret; 

	rsslClearStatusMsg(&statusMsg);
	tunnelStreamStatusSetRsslMsg(pTunnelStatus, &statusMsg, pCos);

	if(pCos != NULL)
	{
		assert(statusMsg.msgBase.containerType == RSSL_DT_FILTER_LIST);
		assert(statusMsg.flags & RSSL_STMF_HAS_MSG_KEY);
		assert(statusMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_FILTER);

		if ((ret = rsslEncodeMsgInit(pIter, (RsslMsg *)&statusMsg, 0)) != RSSL_RET_ENCODE_CONTAINER)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
					"TunnelStreamStatus rsslEncodeMsgInit failed: %d", ret);
			return RSSL_RET_FAILURE;
		}

		if ((ret = rsslEncodeClassOfService(pIter, pCos, statusMsg.msgBase.msgKey.filter, RSSL_TRUE, streamVersion, pErrorInfo)) != RSSL_RET_SUCCESS)
			return ret;

		if ((ret = rsslEncodeMsgComplete(pIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
					"TunnelStreamStatus rsslEncodeMsgComplete failed: %d", ret);
			return RSSL_RET_FAILURE;
		}
	}
	else
	{
		assert(statusMsg.msgBase.containerType == RSSL_DT_NO_DATA);

		if ((ret = rsslEncodeMsg(pIter, (RsslMsg*)&statusMsg)) != RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, ret, 
					__FILE__, __LINE__, "TunnelStreamStatus rsslEncodeMsg failed: %d(%s).", ret, rsslRetCodeToString(ret));
			return RSSL_RET_FAILURE;
		}
	}

	return RSSL_RET_SUCCESS;
}

/* Populates the RsslStatusMsg to send a tunnel stream status. */
void tunnelStreamStatusSetRsslMsg(TunnelStreamStatus *pTunnelStatus, RsslStatusMsg *oRsslMsg, RsslClassOfService *pCos)
{
	rsslClearStatusMsg(oRsslMsg);
	oRsslMsg->msgBase.streamId = pTunnelStatus->base.streamId;
	oRsslMsg->msgBase.domainType = pTunnelStatus->base.domainType;
	oRsslMsg->state = pTunnelStatus->state;

	rsslStatusMsgApplyPrivateStream(oRsslMsg);
	rsslStatusMsgApplyQualifiedStream(oRsslMsg);
	rsslStatusMsgApplyClearCache(oRsslMsg);
	rsslStatusMsgApplyHasState(oRsslMsg);

	if (pCos != NULL)
	{
		oRsslMsg->msgBase.containerType = RSSL_DT_FILTER_LIST;
		rsslStatusMsgApplyHasMsgKey(oRsslMsg);
		rsslMsgKeyApplyHasFilter(&oRsslMsg->msgBase.msgKey);
		oRsslMsg->msgBase.msgKey.flags = RSSL_MKF_HAS_FILTER | RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_SERVICE_ID;
		oRsslMsg->msgBase.msgKey.name = pTunnelStatus->name;
		oRsslMsg->msgBase.msgKey.serviceId = pTunnelStatus->serviceId;

		tunnelStreamMsgKeyApplyCoSFilter(&oRsslMsg->msgBase.msgKey, pCos, RSSL_TRUE);
	}
	else
		oRsslMsg->msgBase.containerType = RSSL_DT_NO_DATA;
}

RsslRet tunnelStreamDataEncodeInit(RsslEncodeIterator *pIter, TunnelStreamData *dataHeader, RsslUInt streamVersion)
{
	RsslGenericMsg genericMsg;
	RsslBuffer tmpBuffer;
	int ret, encodedLen = 0;
	char *pData = NULL;

	rsslClearGenericMsg(&genericMsg);
	genericMsg.msgBase.msgClass = RSSL_MC_GENERIC;
	genericMsg.msgBase.streamId = dataHeader->base.streamId;
	genericMsg.msgBase.domainType = dataHeader->base.domainType;
	if ((dataHeader->flags & TS_DF_FRAGMENTED) == 0) // non-fragmented
	{
		// set container type to MSG if not already set, otherwise set to container type in tunnel stream data header
		if (dataHeader->containerType == 0)
		{
			genericMsg.msgBase.containerType = RSSL_DT_MSG;
		}
		else
		{
			genericMsg.msgBase.containerType = dataHeader->containerType;
		}
		rsslGenericMsgApplyMessageComplete(&genericMsg);
	}
	else // fragmented
	{
		genericMsg.msgBase.containerType = RSSL_DT_OPAQUE;
		if (dataHeader->msgComplete)
		{
			rsslGenericMsgApplyMessageComplete(&genericMsg);
		}
	}
	rsslGenericMsgApplyHasExtendedHdr(&genericMsg);

	rsslGenericMsgApplyHasSeqNum(&genericMsg);
	genericMsg.seqNum = dataHeader->seqNum;

	if ((ret = rsslEncodeMsgInit(pIter, (RsslMsg*)&genericMsg, 0)) != RSSL_RET_ENCODE_EXTENDED_HEADER)
		return ret;

	if ((ret = rsslEncodeNonRWFDataTypeInit(pIter, &tmpBuffer)) != RSSL_RET_SUCCESS)
		return ret;
		
	if (tmpBuffer.length < 2)
		return RSSL_RET_BUFFER_TOO_SMALL;
		
	pData = tmpBuffer.data;

	/* Opcode */
	pData[encodedLen++] = dataHeader->base.opcode;

	if (streamVersion >= COS_CURRENT_STREAM_VERSION)
	{
		// fragmentation flag
		pData[encodedLen++] = dataHeader->flags;
        
		// populate other fragmentation fields if fragmentation flag set
		if (dataHeader->flags & TS_DF_FRAGMENTED)
		{
			if (tmpBuffer.length < 11)
				return RSSL_RET_BUFFER_TOO_SMALL;
            
			encodedLen += bufPut32(&pData[encodedLen], dataHeader->totalMsgLength);
	
			encodedLen += bufPut32(&pData[encodedLen], dataHeader->fragmentNumber);
	
			encodedLen += bufPut16(&pData[encodedLen], dataHeader->messageId);
	
			pData[encodedLen++] = dataHeader->containerType - RSSL_DT_CONTAINER_TYPE_MIN;
		}
	}

	tmpBuffer.length = encodedLen;

	if ((ret = rsslEncodeNonRWFDataTypeComplete(pIter, &tmpBuffer, RSSL_TRUE)) != RSSL_RET_SUCCESS)
		return ret;
				
	if ((ret = rsslEncodeExtendedHeaderComplete(pIter,  RSSL_TRUE)) < RSSL_RET_SUCCESS)
		return ret;

	return RSSL_RET_SUCCESS;
}

RsslRet tunnelStreamDataReplaceOpcode(RsslEncodeIterator *pIter, RsslUInt8 opcode)
{
	if (pIter->_pBuffer->length < 16)
		return RSSL_RET_INCOMPLETE_DATA;

	pIter->_pBuffer->data[15] = opcode;
	return RSSL_RET_SUCCESS;
}

RsslRet tunnelStreamAckEncode(RsslEncodeIterator *pIter, TunnelStreamAck *ackHeader, AckRangeList *ackRangeList, AckRangeList *nakRangeList)
{
	RsslGenericMsg genericMsg;
	RsslBuffer tmpBuffer;
	int ret, encodedLen = 0;
	char *pData = NULL;
	RsslUInt32 i;
	RsslUInt32 *ackRanges, *nakRanges;

	rsslClearGenericMsg(&genericMsg);
	genericMsg.msgBase.msgClass = RSSL_MC_GENERIC;
	genericMsg.msgBase.streamId = ackHeader->base.streamId;
	genericMsg.msgBase.domainType = ackHeader->base.domainType;
	genericMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	rsslGenericMsgApplyHasExtendedHdr(&genericMsg);
	rsslGenericMsgApplyMessageComplete(&genericMsg);

	if ((ret = rsslEncodeMsgInit(pIter, (RsslMsg*)&genericMsg, 0)) != RSSL_RET_ENCODE_EXTENDED_HEADER)
		return ret;

	if ((ret = rsslEncodeNonRWFDataTypeInit(pIter, &tmpBuffer)) != RSSL_RET_SUCCESS)
		return ret;

	if (tmpBuffer.length < 1 /* OpCode */ + 2 /* Flags */ + 4 /* seqNum */ 
			+ 1 + ((nakRangeList != NULL) ? nakRangeList->count * 2 * 4: 0) /* Nak ranges */
			+ 1 + ((ackRangeList != NULL) ? ackRangeList->count * 2 * 4: 0) /* Ack ranges */
			+ 4 /* RecvWindow */)
		return RSSL_RET_BUFFER_TOO_SMALL;
		
	pData = tmpBuffer.data;
		
	/* OpCode */
	pData[encodedLen++] = TS_MC_ACK;

	/* Flags */
	encodedLen += bufPutResBitU15(&pData[encodedLen], ackHeader->flags);
		
	/* SeqNum */
	encodedLen += bufPut32(&pData[encodedLen], ackHeader->seqNum);

	/* Nak Ranges */
	if (nakRangeList == NULL)
		pData[encodedLen++] = 0;
	else
	{
		nakRanges = nakRangeList->rangeArray;

		pData[encodedLen++] = nakRangeList->count;
		for (i = 0; i < nakRangeList->count * 2; ++i)
		{
			encodedLen += bufPut32(&pData[encodedLen], nakRanges[i]);
		}
	}
		
	if (ackRangeList == NULL)
		pData[encodedLen++] = 0;
	else
	{
		ackRanges = ackRangeList->rangeArray;

		pData[encodedLen++] = ackRangeList->count;
		for (i = 0; i < ackRangeList->count * 2; ++i)
		{
			encodedLen += bufPut32(&pData[encodedLen], ackRanges[i]);
		}
	}
		
	/* RecvWindow */
	encodedLen += bufPut32(&pData[encodedLen], ackHeader->recvWindow);

	tmpBuffer.length = encodedLen;

	if ((ret = rsslEncodeNonRWFDataTypeComplete(pIter, &tmpBuffer, RSSL_TRUE)) != RSSL_RET_SUCCESS)
		return ret;
				
	if ((ret = rsslEncodeExtendedHeaderComplete(pIter,  RSSL_TRUE)) < RSSL_RET_SUCCESS)
		return ret;
		
	if ((ret = rsslEncodeMsgComplete(pIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
		return ret;

	return RSSL_RET_SUCCESS;
}

RsslRet tunnelStreamMsgDecode(RsslMsg *pMsg, TunnelStreamMsg *pTunnelMsg, AckRangeList *ackRangeList, AckRangeList *nakRangeList, RsslUInt streamVersion)
{
	RsslBuffer tmpBuffer;
	RsslUInt32 tmpPos;
	char *pData = NULL;
	RsslUInt32 i, *ranges;
	RsslUInt32 count;

	assert(ackRangeList != NULL);
	assert(nakRangeList != NULL);

	tunnelStreamMsgClear(pTunnelMsg);
	
	pTunnelMsg->base.streamId = pMsg->msgBase.streamId;
	pTunnelMsg->base.domainType = pMsg->msgBase.domainType;

	switch(pMsg->msgBase.msgClass)
	{
		case RSSL_MC_GENERIC:
		{
			RsslGenericMsg *pGenericMsg = (RsslGenericMsg*)pMsg;

			if (rsslGenericMsgCheckHasExtendedHdr(pGenericMsg) == RSSL_FALSE
					|| pGenericMsg->extendedHeader.length < 1)
				return RSSL_RET_INCOMPLETE_DATA;

			/* Decode extendedHeader. */
			tmpBuffer = pGenericMsg->extendedHeader;
			tmpPos = 0;
			pData = tmpBuffer.data;

			pTunnelMsg->base.opcode = (TunnelStreamMsgOpcodes)pData[tmpPos++];

			switch(pTunnelMsg->base.opcode)
			{
				case TS_MC_DATA:
				case TS_MC_RETRANS:
					if (streamVersion >= COS_CURRENT_STREAM_VERSION)
					{
						if (tmpPos + 1 /* flags */
								> tmpBuffer.length)
							return RSSL_RET_INCOMPLETE_DATA;

						/* flags */
						pTunnelMsg->dataHeader.flags = pData[tmpPos++];

						// decode other fragmentation fields if fragmentation flag set
						if (pTunnelMsg->dataHeader.flags & TS_DF_FRAGMENTED)
						{
							if (tmpPos + 4 /* totalMsgLength */ + 4 /* fragmentNumber */ + 2 /* messageId */ + 1 /* containerType */
									> tmpBuffer.length)
								return RSSL_RET_INCOMPLETE_DATA;

                    		/* totalMsgLength */
							tmpPos += bufGet32(&pData[tmpPos], &pTunnelMsg->dataHeader.totalMsgLength);
                    		/* fragmentNumber */
							tmpPos += bufGet32(&pData[tmpPos], &pTunnelMsg->dataHeader.fragmentNumber);
                    		/* messageId */
							tmpPos += bufGet16(&pData[tmpPos], &pTunnelMsg->dataHeader.messageId);
							/* containerType */
                    		pTunnelMsg->dataHeader.containerType = pData[tmpPos++] + RSSL_DT_CONTAINER_TYPE_MIN;
						}
					}

					if (rsslGenericMsgCheckHasSeqNum(pGenericMsg) == RSSL_FALSE)
						return RSSL_RET_INCOMPLETE_DATA;

					pTunnelMsg->dataHeader.seqNum = pGenericMsg->seqNum;

					break;

				case TS_MC_ACK:
				{
					if (tmpPos + 1 /* Flags */ + 4 /* seqNum */ + 1 /* Nak count */ + 2 /* ackCount */
							> tmpBuffer.length)
						return RSSL_RET_INCOMPLETE_DATA;

					/* Flags */
					tmpPos += bufGetResBitU15(&pTunnelMsg->ackHeader.flags, &pData[tmpPos]);

					if (tmpPos + 4 /* seqNum */ + 1 /* Nak range count */ > tmpBuffer.length)
						return RSSL_RET_INCOMPLETE_DATA;

					/* SeqNum */
					tmpPos += bufGet32(&pData[tmpPos], &pTunnelMsg->ackHeader.seqNum);

					/* Nak ranges */
					if (tmpPos + 1 > tmpBuffer.length)
						return RSSL_RET_INCOMPLETE_DATA;
					count = (unsigned char)pData[tmpPos++];

					if (tmpPos + 8 * count > tmpBuffer.length)
						return RSSL_RET_INCOMPLETE_DATA;

					nakRangeList->count = count;

					ranges = nakRangeList->rangeArray;
					for (i = 0; i < nakRangeList->count * 2; ++i)
						tmpPos += bufGet32(&pData[tmpPos], &ranges[i]);


					/* Selective ack ranges */
					if (tmpPos + 1 > tmpBuffer.length)
						return RSSL_RET_INCOMPLETE_DATA;
					count = (unsigned char)pData[tmpPos++];
					ackRangeList->count = count;

					if (tmpPos + 8 * count > tmpBuffer.length)
						return RSSL_RET_INCOMPLETE_DATA;

					ranges = ackRangeList->rangeArray;
					for (i = 0; i < ackRangeList->count * 2; ++i)
						tmpPos += bufGet32(&pData[tmpPos], &ranges[i]);

					tmpPos += bufGetI32(&pData[tmpPos], &pTunnelMsg->ackHeader.recvWindow);

					break;
				}

				default:
					return RSSL_RET_INCOMPLETE_DATA;
			}
			break;
		}

		case RSSL_MC_REFRESH:
		{
			pTunnelMsg->base.opcode = TS_MC_REFRESH;
			if (pMsg->refreshMsg.flags & RSSL_RFMF_HAS_MSG_KEY)
			{
				if (pMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME)
				{
					pTunnelMsg->refreshHeader.flags |= TS_RFMF_HAS_NAME;
					pTunnelMsg->refreshHeader.name = pMsg->msgBase.msgKey.name;
				}

				if (pMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID)
				{
					pTunnelMsg->refreshHeader.flags |= TS_RFMF_HAS_SERVICE_ID;
					pTunnelMsg->refreshHeader.serviceId = pMsg->msgBase.msgKey.serviceId;
				}
			}

			pTunnelMsg->refreshHeader.state = pMsg->refreshMsg.state;
			pTunnelMsg->refreshHeader.containerType = pMsg->msgBase.containerType;
			pTunnelMsg->refreshHeader.encDataBody = pMsg->msgBase.encDataBody;
			break;
		}

		case RSSL_MC_STATUS:
		{
			pTunnelMsg->base.opcode = TS_MC_STATUS;

			if (pMsg->statusMsg.flags & RSSL_STMF_HAS_STATE)
			{
				pTunnelMsg->statusHeader.flags |= TS_STMF_HAS_STATE;
				pTunnelMsg->statusHeader.state = pMsg->statusMsg.state;
			}
			break;
		}

		case RSSL_MC_CLOSE:
		{
			pTunnelMsg->base.opcode = TS_MC_CLOSE;
			break;
		}

		default:
			return RSSL_RET_INCOMPLETE_DATA;
	}

	return RSSL_RET_SUCCESS;
}

RsslRet rsslEncodeMsgQueueSubstreamRequestHeader(RsslEncodeIterator *pIter, MsgQueueSubstreamRequestHeader *requestHeader)
{
	RsslRequestMsg requestMsg;
	RsslBuffer tmpBuffer;
	int ret, encodedLen = 0;
	char *pData = NULL;

	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.msgClass = RSSL_MC_REQUEST;
	requestMsg.msgBase.streamId = requestHeader->base.streamId;
	requestMsg.msgBase.domainType = requestHeader->base.domainType;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;

	rsslRequestMsgApplyStreaming(&requestMsg);
	rsslRequestMsgApplyHasExtendedHdr(&requestMsg);

	rsslMsgKeyApplyHasName(&requestMsg.msgBase.msgKey);
	requestMsg.msgBase.msgKey.name = requestHeader->fromQueue;

	if ((ret = rsslEncodeMsgInit(pIter, (RsslMsg*)&requestMsg, 0)) < RSSL_RET_SUCCESS)
		return ret;

	if ((ret = rsslEncodeNonRWFDataTypeInit(pIter, &tmpBuffer)) != RSSL_RET_SUCCESS)
		return ret;
		
	if (tmpBuffer.length < 9)
		return RSSL_RET_BUFFER_TOO_SMALL;
		
	pData = tmpBuffer.data;

	/* OpCode (Request uses DATA) */
	pData[encodedLen++] = MSGQUEUE_SHO_DATA;

	encodedLen += bufPut32(&pData[encodedLen], requestHeader->lastOutSeqNum);
	encodedLen += bufPut32(&pData[encodedLen], requestHeader->lastInSeqNum);

	tmpBuffer.length = encodedLen;

	if ((ret = rsslEncodeNonRWFDataTypeComplete(pIter, &tmpBuffer, RSSL_TRUE)) != RSSL_RET_SUCCESS)
		return ret;
				
	if ((ret = rsslEncodeExtendedHeaderComplete(pIter,  RSSL_TRUE)) != RSSL_RET_SUCCESS)
		return ret;
		
	if ((ret = rsslEncodeMsgComplete(pIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		return ret;

	return RSSL_RET_SUCCESS;
}

RsslRet rsslEncodeMsgQueueSubstreamDataHeader(RsslEncodeIterator *pIter, MsgQueueSubstreamDataHeader *dataHeader)
{
	RsslGenericMsg subGenericMsg;
	RsslBuffer tmpBuffer;
	int ret, encodedLen = 0;

	rsslClearGenericMsg(&subGenericMsg);
	subGenericMsg.msgBase.msgClass = RSSL_MC_GENERIC;
	subGenericMsg.msgBase.streamId = dataHeader->base.streamId;
	subGenericMsg.msgBase.containerType = dataHeader->containerType;
	subGenericMsg.msgBase.domainType = dataHeader->base.domainType;
	rsslGenericMsgApplyHasExtendedHdr(&subGenericMsg);
	rsslGenericMsgApplyMessageComplete(&subGenericMsg);

	rsslGenericMsgApplyHasSeqNum(&subGenericMsg);
	subGenericMsg.seqNum = dataHeader->seqNum;

	rsslGenericMsgApplyHasMsgKey(&subGenericMsg);
	rsslMsgKeyApplyHasName(&subGenericMsg.msgBase.msgKey);
	subGenericMsg.msgBase.msgKey.name = dataHeader->toQueue;

	if ((ret = rsslEncodeMsgInit(pIter, (RsslMsg *)&subGenericMsg, 0)) != RSSL_RET_ENCODE_EXTENDED_HEADER)
		return ret;

	if ((ret = rsslEncodeNonRWFDataTypeInit(pIter, &tmpBuffer)) != RSSL_RET_SUCCESS)
		return ret;

	if ((ret = _msgQueueSubstreamDataEncodeExtendedHeader(&tmpBuffer, dataHeader->base.opcode,
					&dataHeader->fromQueue, dataHeader->identifier, dataHeader->timeout)) != RSSL_RET_SUCCESS)
		return ret;

	if ((ret = rsslEncodeNonRWFDataTypeComplete(pIter, &tmpBuffer, RSSL_TRUE)) != RSSL_RET_SUCCESS)
		return ret;
				
	if ((ret = rsslEncodeExtendedHeaderComplete(pIter,  RSSL_TRUE)) != RSSL_RET_ENCODE_CONTAINER)
		return ret;

	/* Copy data body. */
	if (dataHeader->enclosedBuffer.data != NULL)
	{
		if ((ret = rsslEncodeNonRWFDataTypeInit(pIter, &tmpBuffer)) != RSSL_RET_SUCCESS)
			return ret;

		if (dataHeader->enclosedBuffer.length > tmpBuffer.length)
			return RSSL_RET_BUFFER_TOO_SMALL;

		memcpy(tmpBuffer.data, dataHeader->enclosedBuffer.data,
				dataHeader->enclosedBuffer.length);
		tmpBuffer.length = dataHeader->enclosedBuffer.length;

		if ((ret = rsslEncodeNonRWFDataTypeComplete(pIter, &tmpBuffer, RSSL_TRUE)) != RSSL_RET_SUCCESS)
			return ret;
	}
		
	if ((ret = rsslEncodeMsgComplete(pIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
		return ret;
		
	return RSSL_RET_SUCCESS;
}

RsslRet rsslEncodeMsgQueueSubstreamAckHeader(RsslEncodeIterator *pIter, MsgQueueSubstreamAckHeader *ackHeader)
{
	RsslGenericMsg subGenericMsg;
	RsslBuffer tmpBuffer;
	int ret, encodedLen = 0;

	rsslClearGenericMsg(&subGenericMsg);
	subGenericMsg.msgBase.msgClass = RSSL_MC_GENERIC;
	subGenericMsg.msgBase.streamId = ackHeader->base.streamId;
	subGenericMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	subGenericMsg.msgBase.domainType = ackHeader->base.domainType;
	rsslGenericMsgApplyHasExtendedHdr(&subGenericMsg);
	rsslGenericMsgApplyMessageComplete(&subGenericMsg);

	rsslGenericMsgApplyHasSecondarySeqNum(&subGenericMsg);
	subGenericMsg.secondarySeqNum = ackHeader->seqNum;

	rsslGenericMsgApplyHasMsgKey(&subGenericMsg);

	/* toName */
	rsslMsgKeyApplyHasName(&subGenericMsg.msgBase.msgKey);
	subGenericMsg.msgBase.msgKey.name = ackHeader->toQueue;

	if ((ret = rsslEncodeMsgInit(pIter, (RsslMsg*)&subGenericMsg, 0)) != RSSL_RET_ENCODE_EXTENDED_HEADER)
		return ret;

	if ((ret = rsslEncodeNonRWFDataTypeInit(pIter, &tmpBuffer)) != RSSL_RET_SUCCESS)
		return ret;

	if ((ret = _msgQueueSubstreamAckEncodeExtendedHeader(&tmpBuffer, &ackHeader->fromQueue,
					ackHeader->identifier)) != RSSL_RET_SUCCESS)
		return ret;

	if ((ret = rsslEncodeNonRWFDataTypeComplete(pIter, &tmpBuffer, RSSL_TRUE)) != RSSL_RET_SUCCESS)
		return ret;
				
	if ((ret = rsslEncodeExtendedHeaderComplete(pIter,  RSSL_TRUE)) != RSSL_RET_SUCCESS)
		return ret;
		
	if ((ret = rsslEncodeMsgComplete(pIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
		return ret;

	return RSSL_RET_SUCCESS;
}

RsslRet decodeSubstreamHeader(RsslDecodeIterator *pIter, RsslMsg *pMsg, MsgQueueSubstreamHeader *pSubstreamHeader)
{
	RsslUInt32 msgLength;
	RsslGenericMsg *pGenericMsg = (RsslGenericMsg *)pMsg;
	RsslBuffer tmpBuffer;
	RsslUInt32 tmpPos;
	char *pData = NULL;

	pSubstreamHeader->base.streamId = pMsg->msgBase.streamId;
	pSubstreamHeader->base.domainType = pMsg->msgBase.domainType;

	switch (pMsg->msgBase.msgClass)
	{	
		case RSSL_MC_GENERIC:
		{
			if (rsslGenericMsgCheckHasExtendedHdr(pGenericMsg) == RSSL_FALSE
					|| pGenericMsg->extendedHeader.length < 1)
				return RSSL_RET_INCOMPLETE_DATA;

			/* Decode extended header. */
			tmpBuffer = pMsg->genericMsg.extendedHeader;
			tmpPos = 0;
			pData = tmpBuffer.data;
										
			pSubstreamHeader->base.opcode = (MsgQueueSubstreamHeaderOpcodes)pData[tmpPos++];

			switch(pSubstreamHeader->base.opcode)
			{
				case MSGQUEUE_SHO_DATA:
				case MSGQUEUE_SHO_DEAD_LETTER:
				{					
					if (rsslGenericMsgCheckHasMsgKey(pGenericMsg) == RSSL_FALSE)
						return RSSL_RET_INCOMPLETE_DATA;
							
					if (rsslMsgKeyCheckHasName(&pGenericMsg->msgBase.msgKey) == RSSL_FALSE)
						return RSSL_RET_INCOMPLETE_DATA;
							
					if (rsslGenericMsgCheckHasSeqNum(pGenericMsg) == RSSL_FALSE)
						return RSSL_RET_INCOMPLETE_DATA;

					pSubstreamHeader->dataHeader.containerType = pGenericMsg->msgBase.containerType;

					pSubstreamHeader->dataHeader.toQueue = pGenericMsg->msgBase.msgKey.name;
									
					pSubstreamHeader->dataHeader.enclosedBuffer = pGenericMsg->msgBase.encDataBody;

					if (tmpPos + 1 /* Flags (possibly 2, accounting for it as we go) */ 
							+ 1 /* fromQueue length */ + 1 /* toQueueLength */ 
							> tmpBuffer.length)
						return RSSL_RET_INCOMPLETE_DATA;


					/* Flags */
					if ((msgLength = bufGetResBitU15(&pSubstreamHeader->dataHeader.flags,
									&pData[tmpPos])) < 0)
						return RSSL_RET_INCOMPLETE_DATA;
					tmpPos += msgLength;


					/* SeqNum */
					pSubstreamHeader->dataHeader.seqNum = pGenericMsg->seqNum;
							
					/* fromQueue length */
					if (tmpPos + 1 > tmpBuffer.length)
						return RSSL_RET_INCOMPLETE_DATA;
					msgLength = (unsigned char)pData[tmpPos++];

					/* fromQueue */
					if (tmpPos + msgLength > tmpBuffer.length)
						return RSSL_RET_INCOMPLETE_DATA;
					pSubstreamHeader->dataHeader.fromQueue.data = &pData[tmpPos];
					pSubstreamHeader->dataHeader.fromQueue.length = msgLength;
					tmpPos += msgLength;
							
					/* Timeout (not present on dead letters) */
					if (pSubstreamHeader->base.opcode == MSGQUEUE_SHO_DATA)
					{
						if ((msgLength = bufGetLenSpecI64(&pSubstreamHeader->dataHeader.timeout,
										&pData[tmpPos])) < 0)
							return RSSL_RET_INCOMPLETE_DATA;
						tmpPos += msgLength;
					}
					else
						pSubstreamHeader->dataHeader.timeout = 0;
					
					/* Identifier */
					if ((msgLength = bufGetLenSpecI64(&pSubstreamHeader->dataHeader.identifier,
									&pData[tmpPos])) < 0)
						return RSSL_RET_INCOMPLETE_DATA;

					tmpPos += msgLength;

					if (pSubstreamHeader->base.opcode == MSGQUEUE_SHO_DATA)
						pSubstreamHeader->dataHeader.undeliverableCode = RDM_QMSG_UC_UNSPECIFIED;
					else
					{
						pSubstreamHeader->dataHeader.undeliverableCode = pData[tmpPos];
						++tmpPos;
					}

					/* Queue Depth */
					tmpPos += bufGet16(&pData[tmpPos], &pSubstreamHeader->dataHeader.queueDepth);
							
					return RSSL_RET_SUCCESS;
				}

				case MSGQUEUE_SHO_ACK:
				{
					if (rsslGenericMsgCheckHasSecondarySeqNum(pGenericMsg) == RSSL_FALSE)
						return RSSL_RET_INCOMPLETE_DATA;
							
					/* seqNum */
					pSubstreamHeader->ackHeader.seqNum = pGenericMsg->secondarySeqNum;

					/* toQueue */
					pSubstreamHeader->ackHeader.toQueue = pGenericMsg->msgBase.msgKey.name;

					/* fromQueue length */
					msgLength = (unsigned char)pData[tmpPos++];

					/* fromQueue */
					pSubstreamHeader->ackHeader.fromQueue.data = &pData[tmpPos];
					pSubstreamHeader->ackHeader.fromQueue.length = msgLength;
					tmpPos += msgLength;

					
					/* Identifier */
					if ((msgLength = bufGetLenSpecI64(&pSubstreamHeader->ackHeader.identifier,
									&pData[tmpPos])) < 0)
						return RSSL_RET_INCOMPLETE_DATA;

					return RSSL_RET_SUCCESS;
				}

				default:
					return RSSL_RET_INCOMPLETE_DATA;
			}
		}

		case RSSL_MC_REFRESH:
		{
			RsslRefreshMsg *pRefreshMsg = (RsslRefreshMsg*)pMsg;
			MsgQueueSubstreamRefreshHeader *pSubRefresh = 
				(MsgQueueSubstreamRefreshHeader*) pSubstreamHeader;

			pSubstreamHeader->base.opcode = MSGQUEUE_SHO_REFRESH;

			if (rsslRefreshMsgCheckHasExtendedHdr(pRefreshMsg) == RSSL_FALSE
					|| pRefreshMsg->extendedHeader.length < 8)
				return RSSL_RET_INCOMPLETE_DATA;

			/* Decode extended header. */
			tmpBuffer = pRefreshMsg->extendedHeader;
			tmpPos = 0;
			pData = tmpBuffer.data;

			pSubRefresh->base.opcode = MSGQUEUE_SHO_REFRESH;
			pSubRefresh->base.streamId = pMsg->msgBase.streamId;
			pSubRefresh->base.domainType = pMsg->msgBase.domainType;
										
			tmpPos += bufGet32(&pData[tmpPos], &pSubRefresh->lastOutSeqNum);
			tmpPos += bufGet32(&pData[tmpPos], &pSubRefresh->lastInSeqNum);
			tmpPos += bufGet16(&pData[tmpPos], &pSubRefresh->queueDepth);

			return RSSL_RET_SUCCESS;
		}


		default:
			return RSSL_RET_INCOMPLETE_DATA;
	}
}

RsslRet substreamSetRsslMsgFromQueueDataExpiredMsg( RsslRDMQueueDataExpired *iQueueDataExpired, 
		RsslGenericMsg *oGenericMsg, RsslBuffer *pExtHeaderBuffer)
{
	RsslRet ret;

	/* Populate RsslMsg */
	rsslClearGenericMsg(oGenericMsg);
	oGenericMsg->msgBase.streamId = iQueueDataExpired->rdmMsgBase.streamId;
	oGenericMsg->msgBase.domainType = iQueueDataExpired->rdmMsgBase.domainType;
	oGenericMsg->msgBase.encDataBody = iQueueDataExpired->encDataBody;
	oGenericMsg->msgBase.containerType = iQueueDataExpired->containerType;
	rsslGenericMsgApplyHasExtendedHdr(oGenericMsg);
	rsslGenericMsgApplyMessageComplete(oGenericMsg);

	/* No sequence number -- this message has not been assigned one. */

	rsslGenericMsgApplyHasMsgKey(oGenericMsg);
	rsslMsgKeyApplyHasName(&oGenericMsg->msgBase.msgKey);
	oGenericMsg->msgBase.msgKey.name = iQueueDataExpired->destName;

	/* Encode Ext Header */
	if ((ret = _msgQueueSubstreamDataEncodeExtendedHeader(pExtHeaderBuffer, 
					MSGQUEUE_SHO_DEAD_LETTER, &iQueueDataExpired->sourceName, iQueueDataExpired->identifier, RDM_QMSG_TC_INFINITE)) 
			!= RSSL_RET_SUCCESS)
		return ret;

	oGenericMsg->extendedHeader = *pExtHeaderBuffer;
	return RSSL_RET_SUCCESS;
}

RsslRet substreamSetRsslMsgFromQueueAckMsg( RsslRDMQueueAck *iQueueAck, 
		RsslGenericMsg *oGenericMsg, 
		RsslUInt32 seqNum, RsslBuffer *pExtHeaderBuffer)
{
	RsslRet ret;

	/* Populate RsslMsg */
	rsslClearGenericMsg(oGenericMsg);
	oGenericMsg->msgBase.msgClass = RSSL_MC_GENERIC;
	oGenericMsg->msgBase.streamId = iQueueAck->rdmMsgBase.streamId;
	oGenericMsg->msgBase.containerType = RSSL_DT_NO_DATA;
	oGenericMsg->msgBase.domainType = iQueueAck->rdmMsgBase.domainType;
	rsslGenericMsgApplyHasExtendedHdr(oGenericMsg);
	rsslGenericMsgApplyMessageComplete(oGenericMsg);

	rsslGenericMsgApplyHasSecondarySeqNum(oGenericMsg);
	oGenericMsg->secondarySeqNum = seqNum;

	rsslGenericMsgApplyHasMsgKey(oGenericMsg);

	/* Set key member (toName) */
	oGenericMsg->msgBase.msgKey.name = iQueueAck->destName;

	/* Encode Ext Header */
	if ((ret = _msgQueueSubstreamAckEncodeExtendedHeader(pExtHeaderBuffer, 
					&iQueueAck->sourceName, iQueueAck->identifier)) 
			!= RSSL_RET_SUCCESS)
		return ret;

	oGenericMsg->extendedHeader = *pExtHeaderBuffer;
	return RSSL_RET_SUCCESS;
}

RsslRet decodeSubstreamRequestHeader(RsslDecodeIterator *pIter, RsslMsg *pMsg, MsgQueueSubstreamRequestHeader *pSubstreamRequestHeader)
{
	RsslRequestMsg *pRequestMsg = (RsslRequestMsg *)pMsg;
	RsslBuffer tmpBuffer;
	RsslUInt32 tmpPos;
	char *pData = NULL;

	switch (pMsg->msgBase.msgClass)
	{	
		case RSSL_MC_REQUEST:
			if (rsslRequestMsgCheckHasExtendedHdr(pRequestMsg) == RSSL_FALSE)
				return RSSL_RET_INCOMPLETE_DATA;

			/* Decode extended header. */
			tmpBuffer = pMsg->requestMsg.extendedHeader;
			tmpPos = 0;
			pData = tmpBuffer.data;
										
			pSubstreamRequestHeader->base.opcode = (MsgQueueSubstreamHeaderOpcodes)pData[tmpPos++];

			switch(pSubstreamRequestHeader->base.opcode)
			{
				case MSGQUEUE_SHO_DATA:
					tmpPos += bufGet32(&pData[tmpPos], &pSubstreamRequestHeader->lastOutSeqNum);
					tmpPos += bufGet32(&pData[tmpPos], &pSubstreamRequestHeader->lastInSeqNum);

					return RSSL_RET_SUCCESS;

				default:
					return RSSL_RET_INCOMPLETE_DATA;
			}

	default:
		return RSSL_RET_INCOMPLETE_DATA;

	}
}

RsslRet rsslReplaceQueueDataTimeout(RsslDecodeIterator *pIter, RsslInt64 timeout)
{
	RsslUInt32 msgLength;
	RsslUInt32 tmpPos;
	RsslMsg rsslMsg;
	RsslRet ret;

	if ((ret = rsslDecodeMsg(pIter, &rsslMsg)) != RSSL_RET_SUCCESS)
		return ret;

	switch (rsslMsg.msgBase.msgClass)
	{	
		case RSSL_MC_GENERIC:
		{
			char *pData = NULL;

			if (rsslGenericMsgCheckHasExtendedHdr(&rsslMsg.genericMsg) == RSSL_FALSE)
				return RSSL_RET_INCOMPLETE_DATA;

			/* Decode extended header. */
			tmpPos = 0;
			pData = rsslMsg.genericMsg.extendedHeader.data;
										
			switch((MsgQueueSubstreamHeaderOpcodes)pData[tmpPos++])
			{
				case MSGQUEUE_SHO_DATA:
				{					
					RsslInt64 oldTimeout;
					RsslUInt16 tmpFlags;
					/* Find the position of the existing timeout in the extended header. */

					/* Flags */
					if ((msgLength = bufGetResBitU15(&tmpFlags, &pData[tmpPos])) < 0)
						return RSSL_RET_INCOMPLETE_DATA;
					tmpPos += msgLength;

					/* fromQueue length */
					msgLength = (unsigned char)pData[tmpPos++];

					/* fromQueue */
					tmpPos += msgLength;
							
					/* Replace the timeout. */
					msgLength = bufGetLenSpecI64(&oldTimeout, &pData[tmpPos]);
					if (tmpPos + msgLength > rsslMsg.genericMsg.extendedHeader.length)
						return RSSL_RET_INCOMPLETE_DATA;

					if (timeout >= oldTimeout)
						return RSSL_RET_SUCCESS;

					bufReplaceLenSpecI64(&pData[tmpPos], timeout);

					
					return RSSL_RET_SUCCESS;
				}

				default:
					return RSSL_RET_INCOMPLETE_DATA;
			}
		}

		default:
			return RSSL_RET_INCOMPLETE_DATA;
	}
}

RsslRet rsslAddQueueDataDuplicateFlag(RsslDecodeIterator *pIter)
{
	RsslUInt32 tmpPos;
	RsslMsg rsslMsg;
	RsslRet ret;

	if ((ret = rsslDecodeMsg(pIter, &rsslMsg)) != RSSL_RET_SUCCESS)
		return ret;

	switch (rsslMsg.msgBase.msgClass)
	{	
		case RSSL_MC_GENERIC:
		{
			char *pData = NULL;

			if (rsslGenericMsgCheckHasExtendedHdr(&rsslMsg.genericMsg) == RSSL_FALSE)
				return RSSL_RET_INCOMPLETE_DATA;

			/* Decode extended header. */
			tmpPos = 0;
			pData = rsslMsg.genericMsg.extendedHeader.data;
										
			switch((MsgQueueSubstreamHeaderOpcodes)pData[tmpPos++])
			{
				case MSGQUEUE_SHO_DATA:
				{					
					RsslUInt16 tmpFlags;

					if (bufGetResBitU15(&tmpFlags, &pData[tmpPos]) < 1)
						return RSSL_RET_INCOMPLETE_DATA;

					/* Add Possible-duplicate flag (it's in the first byte, so it shouldn't
					 * increase the encoding length) */
					tmpFlags |= MSGQUEUE_DF_POSSIBLE_DUPLICATE;

					if (bufPutResBitU15(&pData[tmpPos], tmpFlags) < 1)
						return RSSL_RET_INCOMPLETE_DATA;

					return RSSL_RET_SUCCESS;
				}

				default:
					return RSSL_RET_INCOMPLETE_DATA;
			}
		}

		default:
			return RSSL_RET_INCOMPLETE_DATA;
	}
}

static RsslRet _msgQueueSubstreamDataEncodeExtendedHeader(RsslBuffer *pTmpBuffer, 
	RsslUInt8 opcode, RsslBuffer *pFromQueue, RsslInt64 identifier, RsslInt64 timeout)
{
	RsslUInt32 encodedLen = 0;

	assert(opcode == MSGQUEUE_SHO_DATA || opcode == MSGQUEUE_SHO_DEAD_LETTER);
	assert(opcode != MSGQUEUE_SHO_DEAD_LETTER || timeout == RDM_QMSG_TC_INFINITE);

	if (pTmpBuffer->length < 2 + pFromQueue->length + 18)
		return RSSL_RET_BUFFER_TOO_SMALL;

	/* Opcode */
	pTmpBuffer->data[encodedLen++] = opcode;

	/* Flags (None are currently encoded initially) */
	encodedLen += bufPutResBitU15(&pTmpBuffer->data[encodedLen], 0);

	/* From queue */
	pTmpBuffer->data[encodedLen++] = pFromQueue->length;
		
	memcpy(&pTmpBuffer->data[encodedLen], pFromQueue->data, pFromQueue->length);
	encodedLen += pFromQueue->length;

	if (opcode == MSGQUEUE_SHO_DATA)
	{
		/* Timeout */
		encodedLen += bufPutLenSpecI64(&pTmpBuffer->data[encodedLen], timeout);
	}

	/* Identifier */
	encodedLen += bufPutLenSpecI64(&pTmpBuffer->data[encodedLen], identifier);

	/* Queue Depth (Not used by provider right now, so send 0) */
	encodedLen += bufPut16(&pTmpBuffer->data[encodedLen], 0);

	pTmpBuffer->length = encodedLen;

	return RSSL_RET_SUCCESS;
}

static RsslRet _msgQueueSubstreamAckEncodeExtendedHeader(RsslBuffer *pTmpBuffer, 
	RsslBuffer *pFromQueue, RsslInt64 identifier)
{
	char *pData = NULL;
	RsslUInt32 encodedLen = 0;

	if (pTmpBuffer->length < 2 + pFromQueue->length + 9)
		return RSSL_RET_BUFFER_TOO_SMALL;

	pData = pTmpBuffer->data;

	/* Opcode */
	pData[encodedLen++] = MSGQUEUE_SHO_ACK;
	
	/* Write fromName length */
	pData[encodedLen++] = pFromQueue->length;
		
	/* Write fromName */
	memcpy(&pData[encodedLen], pFromQueue->data, pFromQueue->length);
	encodedLen += pFromQueue->length;

	/* Write identifier */
	encodedLen += bufPutLenSpecI64(&pData[encodedLen], identifier);
		
	pTmpBuffer->length = encodedLen;

	return RSSL_RET_SUCCESS;
}
