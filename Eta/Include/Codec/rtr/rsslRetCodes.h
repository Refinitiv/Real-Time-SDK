
#ifndef _RTR_RSSL_RETCODES_H
#define _RTR_RSSL_RETCODES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslTypes.h"


/**
 * @addtogroup RSSLRetCodes
 * @{
 */

/** 
 * @brief Return code enumeration for RSSL functions from all packages. 
 */
typedef enum
{
	/* Tunnel Stream Specific return codes */
	RSSL_RET_PERSISTENCE_FULL					= -91,	/*!< (-91) Tunnel Stream Failure: This message could not be sent because no space was available to persist it. */

	/* Multicast Transport Specific Return Codes */
	/* -70 through -61 */
	RSSL_RET_CONGESTION_DETECTED				= -63,	/*!< (-63) Transport Warning: Network congestion detected.  Gaps are likely. */
	RSSL_RET_SLOW_READER						= -62,  /*!< (-62) Transport Warning: Application is consuming more slowly than data is being provided.  Gaps are likely. */
	RSSL_RET_PACKET_GAP_DETECTED				= -61,	/*!< (-61) Transport Warning: An unrecoverable packet gap was detected and some content may have been lost. */

	/* -35 through -60 reserved */

	RSSL_RET_VALUE_OUT_OF_RANGE					= -34,  /*!< (-34) Codec Failure: A value being encoded into a set is outside of the valid range of the type given by that set. */
	RSSL_RET_ITERATOR_OVERRUN					= -33,  /*!< (-33) Codec Failure: Iterator is nested too deeply. There is a limit of 16 levels. */
	RSSL_RET_DUPLICATE_LOCAL_SET_DEFS			= -32,	/*!< (-32) Codec Failure: A duplicate set definition has been received */
	RSSL_RET_TOO_MANY_LOCAL_SET_DEFS			= -31,  /*!< (-31) Codec Failure: Maximum number of set definitions has been exceeded */
	RSSL_RET_ILLEGAL_LOCAL_SET_DEF				= -30,  /*!< (-30) Codec Failure: Set definition is not valid */
	RSSL_RET_INVALID_DATA						= -29,  /*!< (-29) Codec Failure: Invalid data provided to function */
	RSSL_RET_SET_DEF_NOT_PROVIDED				= -27,	/*!< (-27) Codec Failure: A Database containing the Set Definition for encoding the desired set was not provided. */
	RSSL_RET_INCOMPLETE_DATA					= -26,	/*!< (-26) Codec Failure: Not enough data was provided. */
	RSSL_RET_UNEXPECTED_ENCODER_CALL			= -25,	/*!< (-25) Codec Failure: An encoder was used in an unexpected sequence. */
	RSSL_RET_UNSUPPORTED_DATA_TYPE				= -24,	/*!< (-24) Codec Failure: The data type is unsupported, may indicate invalid containerType or primitiveType specified. */
	RSSL_RET_ENCODING_UNAVAILABLE			    = -23,	/*!< (-23) Codec Failure: No encoder is available for the data type specified. */
	RSSL_RET_INVALID_ARGUMENT				    = -22,	/*!< (-22) Codec Failure: An invalid argument was provided. */
	RSSL_RET_BUFFER_TOO_SMALL					= -21,	/*!< (-21) Codec Failure: The buffer provided does not have sufficient space to perform the operation. */

	/* -20 through -16 reserved */

	/* Transport Return Codes */
	/* Because positive values indicate bytes left to read or write */
	/* Some negative transport layer return codes still indicate success */
	RSSL_RET_READ_IN_PROGRESS					= -15, /*!< (-15) Transport Success: Another rsslRead call is currently in progress from another thread.  This can only be detected if per-channel locking is enabled. */
	RSSL_RET_READ_FD_CHANGE				  		= -14, /*!< (-14) Transport Success: rsslRead received an FD change event.  The application should unregister the oldSocketId and register the socketId with its notifier */
	RSSL_RET_READ_PING				  			= -13, /*!< (-13) Transport Success: rsslRead has received a ping message.  There is no buffer in this case. */
	RSSL_RET_READ_WOULD_BLOCK					= -11, /*!< (-11) Transport Success: Reading was blocked by the OS.  Typically indicates that there are no bytes available to read, returned from rsslRead. */ 
	RSSL_RET_WRITE_CALL_AGAIN					= -10, /*!< (-10) Transport Success: rsslWrite is fragmenting the buffer and needs to be called again with the same buffer.  This indicates that rsslWrite was unable to send all fragments with the current call and must continue fragmenting */
	RSSL_RET_WRITE_FLUSH_FAILED					= -9,  /*!< (-9)  Transport Success: rsslWrite internally attempted to flush data to the connection but was blocked.  This is not a failure and the user should not release their buffer */
	/* -8 through -5 reserved */
	RSSL_RET_BUFFER_NO_BUFFERS					= -4,   /*!< (-4) Transport Failure: There are no buffers available from the buffer pool, returned from rsslGetBuffer.  Use rsslIoctl to increase pool size or use rsslFlush to flush data and return buffers to pool.  */
	RSSL_RET_INIT_NOT_INITIALIZED               = -3,   /*!< (-3) Transport Failure: Not initialized failure code, returned from transport functions when rsslInitialize did not succeed */
	RSSL_RET_CHAN_INIT_REFUSED					= -2,	/*!< (-2) Transport Failure: Channel initialization failed/connection refused, returned from rsslInitChannel. */

	/* General Failure and Success Codes */
	RSSL_RET_FAILURE							= -1,	/*!< (-1) General Failure: RSSL general failure return code */
	RSSL_RET_SUCCESS							= 0,	/*!< (0)  General Success: RSSL general success return code */

	/* Transport Return Codes */
	RSSL_RET_CHAN_INIT_IN_PROGRESS				= 2,	/*!< (2)  Transport Success: Channel initialization is In progress, returned from rsslInitChannel. */

	/* 3 through 9 reserved */

	/* Encoder/Decoder Success Return Codes */
	RSSL_RET_DICT_PART_ENCODED					= 10,	/*!< (10) Dictionary Success: Successfully encoded part of a dictionary message, returned from the rssl dictionary processing functions. */
	RSSL_RET_ENCODE_MSG_KEY_OPAQUE			    = 11,	/*!< (11) Codec Success: The user should now encode their msgKey opaque data. */
	RSSL_RET_ENCODE_EXTENDED_HEADER			    = 12,	/*!< (12) Codec Success: The user should now encode their extended header information. */
	RSSL_RET_ENCODE_CONTAINER				    = 13,   /*!< (13) Codec Success: The user should encode the container type payload */
	RSSL_RET_END_OF_CONTAINER					= 14,	/*!< (14) Codec Success: The end of the current container has been reached while decoding */
	RSSL_RET_BLANK_DATA							= 15,	/*!< (15) Codec Success: Decoded data is a Blank */
	RSSL_RET_NO_DATA							= 16,	/*!< (16) Codec Success: Container was decoded from an empty payload. The user should not try to decode any entries. */
	RSSL_RET_SET_COMPLETE						= 17,	/*!< (17) Codec Success: The encoded entry completed a FieldList or ElementList set. Subsequent entries will be encoded normally. */
	RSSL_RET_SET_SKIPPED						= 18,	/*!< (18) Codec Success: The FieldList or ElementList contains set data and the necessary definition was not provided. Standard entries may still be decoded. */
	RSSL_RET_SET_DEF_DB_EMPTY					= 19,	/*!< (19) Codec Success: A Set Definition Database decoded successfully but contained no definitions. */
	RSSL_RET_ENCODE_REQMSG_KEY_ATTRIB			= 20	/*!< (20) Codec Success: The user should now encode their request msgKey attrib data. */
} RsslReturnCodes;



/**
 * @brief Convert an RsslReturnCodes return code enumerated value to a string representation (e.g. 16 to "RSSL_RET_NO_DATA")
 *
 * @param code RsslReturnCodes value to convert to string representation
 * @see RsslRet, RsslReturnCodes
 * @return char* containing string representation of RsslReturnCodes enumerated value
 */
RSSL_API const char* rsslRetCodeToString(RsslRet code);


/**
 * @brief Provide additional information about the meaning of an RsslReturnCodes return code enumerated value.
 *
 * @param code RsslReturnCodes value to obtain extended error information for
 * @see RsslRet, RsslReturnCodes
 * @return char* containing extended error description
 */
RSSL_API const char* rsslRetCodeInfo(RsslRet code);

/**
 * @}
 */



#ifdef __cplusplus
}
#endif



#endif
