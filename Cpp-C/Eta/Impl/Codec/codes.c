/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2018-2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "rtr/rsslRetCodes.h"



const char * rsslRetCodeInfo(RsslRet code)
{
	switch (code)
	{
		case	RSSL_RET_CONGESTION_DETECTED:
			return "Warning: Network congestion detected.  Gaps are likely.";
		case	RSSL_RET_SLOW_READER:
			return "Warning: Application is consuming more slowly than data is being provided.  Gaps are likely.";
		case	RSSL_RET_PACKET_GAP_DETECTED:
			return "Warning: An unrecoverable packet gap was detected and some content may have been lost.";
		case	RSSL_RET_VALUE_OUT_OF_RANGE:
			return "Failure: A value being encoded into a set is outside of the valid range of the type given by that set.";
		case	RSSL_RET_ITERATOR_OVERRUN:	
			return "Failure: Iterator is nested too deeply. There is a limit of 16 levels.";
		case	RSSL_RET_DUPLICATE_LOCAL_SET_DEFS:
			return "Failure: A duplicate set definition has been received.";
		case	RSSL_RET_TOO_MANY_LOCAL_SET_DEFS:
			return "Failure: Maximum number of set definitions has been exceeded.";
		case	RSSL_RET_ILLEGAL_LOCAL_SET_DEF:
			return "Failure: Set definition is not valid.";
		case	RSSL_RET_INVALID_DATA:
			return "Failure: Invalid data provided to function.";
		case	RSSL_RET_SET_DEF_NOT_PROVIDED:
			return "Failure: A Database containing the Set Definition for encoding the desired set was not provided.";
		case	RSSL_RET_INCOMPLETE_DATA:
			return "Failure: Not enough data was provided.";
		case	RSSL_RET_UNEXPECTED_ENCODER_CALL:
			return "Failure: An encoder was used in an unexpected sequence.";
		case	RSSL_RET_UNSUPPORTED_DATA_TYPE:
			return "Failure: The data type is unsupported.";
		case	RSSL_RET_ENCODING_UNAVAILABLE:
			return "Failure: No encoder is available for the data type specified.";
		case	RSSL_RET_INVALID_ARGUMENT:
			return "Failure: An invalid argument was provided.";
		case	RSSL_RET_BUFFER_TOO_SMALL:
			return "Failure: The buffer provided does not have sufficient space to perform the operation.";
		case	RSSL_RET_READ_IN_PROGRESS:
			return "Success: Another rsslRead call is currently in progress from another thread.  This can only be detected if per-channel locking is enabled.";
		case	RSSL_RET_READ_FD_CHANGE:
			return "Success: rsslRead received an FD change event.  The application should unregister the oldSocketId and register the socketId with its notifier.";
		case	RSSL_RET_READ_PING:
			return "Success: rsslRead has received a ping message.  There is no buffer in this case.";
		case 	RSSL_RET_READ_WOULD_BLOCK:
			return "Success: Reading was blocked by the OS.  Typically indicates that there are no bytes available to read, returned from rsslRead.";
		case	RSSL_RET_WRITE_CALL_AGAIN: 
			return "Success: rsslWrite is fragmenting the buffer and needs to be called again with the same buffer.  This indicates that rsslWrite was unable to send all fragments with the current call and must continue fragmenting.";
		case	RSSL_RET_WRITE_FLUSH_FAILED:
			return "Success: rsslWrite internally attempted to flush data to the connection but was blocked.  This is not a failure and the user should not release their buffer.";
		case 	RSSL_RET_BUFFER_NO_BUFFERS:
			return "Failure: There are no buffers available from the buffer pool, returned from rsslGetBuffer.  Use rsslIoctl to increase pool size or use rsslFlush to flush data and return buffers to pool.";
		case	RSSL_RET_INIT_NOT_INITIALIZED:
			return "Failure: Not initialized failure code, returned from transport functions when rsslInitialize did not succeed.";
		case 	RSSL_RET_CHAN_INIT_REFUSED:
			return "Failure: Channel initialization failed/connection refused, returned from rsslInitChannel.";
		case	RSSL_RET_FAILURE:
			return "Failure: RSSL general failure return code.";
		case 	RSSL_RET_SUCCESS:
			return "Success: RSSL general success return code.";
		case	RSSL_RET_CHAN_INIT_IN_PROGRESS:
			return "Success: Channel initialization is In progress, returned from rsslInitChannel.";
		case	RSSL_RET_DICT_PART_ENCODED:
			return "Success: Successfully encoded part of a dictionary message, returned from the rssl dictionary processing functions.";
		case	RSSL_RET_ENCODE_MSG_KEY_OPAQUE:
			return "Success: The user should now encode their msgKey opaque data.";
		case	RSSL_RET_ENCODE_EXTENDED_HEADER:
			return "Success: The user should now encode their extended header information.";
		case 	RSSL_RET_ENCODE_CONTAINER:
			return "Success: The user should encode the container type payload";
		case 	RSSL_RET_END_OF_CONTAINER:
			return "Success: The end of the current container has been reached while decoding.";
		case 	RSSL_RET_BLANK_DATA:
			return "Success: Decoded data is a Blank.";
		case	RSSL_RET_NO_DATA:
			return "Success: Container was decoded from an empty payload. The user should not try to decode any entries.";
		case	RSSL_RET_SET_COMPLETE:
			return "Success: The encoded entry completed a FieldList or ElementList set. Subsequent entries will be encoded normally.";
		case	RSSL_RET_SET_SKIPPED:
			return "Success: The FieldList or ElementList contains set data and the necessary definition was not provided. Standard entries may still be decoded.";
		case	RSSL_RET_SET_DEF_DB_EMPTY:
			return "Success: A Set Definition Database decoded successfully but contained no definitions.";
		default:
			return "Unknown.";
	}
}


const char * rsslRetCodeToString(RsslRet code)
{
	switch (code)
	{
		case 	RSSL_RET_CONGESTION_DETECTED:
			return "RSSL_RET_CONGESTION_DETECTED";
		case 	RSSL_RET_SLOW_READER:
			return "RSSL_RET_SLOW_READER";
		case	RSSL_RET_PACKET_GAP_DETECTED:
			return "RSSL_RET_PACKET_GAP_DETECTED";
    	case	RSSL_RET_VALUE_OUT_OF_RANGE:
			return "RSSL_RET_VALUE_OUT_OF_RANGE";
		case	RSSL_RET_ITERATOR_OVERRUN:	
			return "RSSL_RET_ITERATOR_OVERRUN";
		case	RSSL_RET_DUPLICATE_LOCAL_SET_DEFS:
			return "RSSL_RET_DUPLICATE_LOCAL_SET_DEFS";
		case	RSSL_RET_TOO_MANY_LOCAL_SET_DEFS:
			return "RSSL_RET_TOO_MANY_LOCAL_SET_DEFS";
		case	RSSL_RET_ILLEGAL_LOCAL_SET_DEF:
			return "RSSL_RET_ILLEGAL_LOCAL_SET_DEF";
		case	RSSL_RET_INVALID_DATA:
			return "RSSL_RET_INVALID_DATA";
		case	RSSL_RET_SET_DEF_NOT_PROVIDED:
			return "RSSL_RET_SET_DEF_NOT_PROVIDED";
		case	RSSL_RET_INCOMPLETE_DATA:
			return "RSSL_RET_INCOMPLETE_DATA";
		case	RSSL_RET_UNEXPECTED_ENCODER_CALL:
			return "RSSL_RET_UNEXPECTED_ENCODER_CALL";
		case	RSSL_RET_UNSUPPORTED_DATA_TYPE:
			return "RSSL_RET_UNSUPPORTED_DATA_TYPE";
		case	RSSL_RET_ENCODING_UNAVAILABLE:
			return "RSSL_RET_ENCODING_UNAVAILABLE";
		case	RSSL_RET_INVALID_ARGUMENT:
			return "RSSL_RET_INVALID_ARGUMENT";
		case	RSSL_RET_BUFFER_TOO_SMALL:
			return "RSSL_RET_BUFFER_TOO_SMALL";
		case	RSSL_RET_READ_IN_PROGRESS:
			return "RSSL_RET_READ_IN_PROGRESS";
		case	RSSL_RET_READ_FD_CHANGE:
			return "RSSL_RET_READ_FD_CHANGE";
		case	RSSL_RET_READ_PING:
			return "RSSL_RET_READ_PING";
		case 	RSSL_RET_READ_WOULD_BLOCK:
			return "RSSL_RET_READ_WOULD_BLOCK";
		case	RSSL_RET_WRITE_CALL_AGAIN: 
			return "RSSL_RET_WRITE_CALL_AGAIN";
		case	RSSL_RET_WRITE_FLUSH_FAILED:
			return "RSSL_RET_WRITE_FLUSH_FAILED";
		case 	RSSL_RET_BUFFER_NO_BUFFERS:
			return "RSSL_RET_BUFFER_NO_BUFFERS";
		case	RSSL_RET_INIT_NOT_INITIALIZED:
			return "RSSL_RET_INIT_NOT_INITIALIZED";
		case 	RSSL_RET_CHAN_INIT_REFUSED:
			return "RSSL_RET_CHAN_INIT_REFUSED";
		case	RSSL_RET_FAILURE:
			return "RSSL_RET_FAILURE";
		case 	RSSL_RET_SUCCESS:
			return "RSSL_RET_SUCCESS";
		case	RSSL_RET_CHAN_INIT_IN_PROGRESS:
			return "RSSL_RET_CHAN_INIT_IN_PROGRESS";
		case	RSSL_RET_DICT_PART_ENCODED:
			return "RSSL_RET_DICT_PART_ENCODED";
		case	RSSL_RET_ENCODE_MSG_KEY_OPAQUE:
			return "RSSL_RET_ENCODE_MSG_KEY_OPAQUE";
		case	RSSL_RET_ENCODE_EXTENDED_HEADER:
			return "RSSL_RET_ENCODE_EXTENDED_HEADER";
		case 	RSSL_RET_ENCODE_CONTAINER:
			return "RSSL_RET_ENCODE_CONTAINER";
		case 	RSSL_RET_END_OF_CONTAINER:
			return "RSSL_RET_END_OF_CONTAINER";
		case 	RSSL_RET_BLANK_DATA:
			return "RSSL_RET_BLANK_DATA";
		case	RSSL_RET_NO_DATA:
			return "RSSL_RET_NO_DATA";
		case	RSSL_RET_SET_COMPLETE:
			return "RSSL_RET_SET_COMPLETE";
		case	RSSL_RET_SET_SKIPPED:
			return "RSSL_RET_SET_SKIPPED";
		case	RSSL_RET_SET_DEF_DB_EMPTY:
			return "RSSL_RET_SET_DEF_DB_EMPTY";
		default:
			return "Unknown";
	}
}
