

#ifndef __RSSL_STATE_H
#define __RSSL_STATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslTypes.h"

/** 
 * @addtogroup RsslStateType
 * @{
 */



/**
 * @brief State structure, contains information used to convey data and stream health information.
 * @see RsslMsg, RsslDataStates, RsslStreamStates, RsslStateCodes, RSSL_INIT_STATE, rsslClearState, rsslEncodeState, rsslDecodeState
 */
typedef struct
{
 	RsslUInt8	streamState;	/*!< @brief An enumerated value providing information about the state of the stream, populated from \ref RsslStreamStates */
    RsslUInt8	dataState;		/*!< @brief An enumerated value providing information about the state of data, populated from \ref RsslDataStates */
	RsslUInt8	code;			/*!< @brief An enumerated code providing additional state information, populated from \ref RsslStateCodes */
	RsslBuffer	text;			/*!< @brief text describing the state or state code */
} RsslState;


/** 
 * @brief RsslState Stream States provide information on the health of the stream
 * @see RsslState
 */
typedef enum {
	RSSL_STREAM_UNSPECIFIED		= 0,    /*!< (0) Unspecified (Used as a structure initialization value and is not intended to be encoded or decoded) */
	RSSL_STREAM_OPEN			= 1,	/*!< (1) Stream is open (typically implies that information will be streaming, as information changes updated information will be sent on the stream, after final RsslRefreshMsg or RsslStatusMsg) */
	RSSL_STREAM_NON_STREAMING	= 2,	/*!< (2) Request was non-streaming (after final RsslRefreshMsg or RsslStatusMsg is received, the stream will be closed and no updated information will be delivered without subsequent re-request) */
	RSSL_STREAM_CLOSED_RECOVER	= 3,    /*!< (3) Closed, the applications may attempt to re-open the stream later (can occur via either an RsslRefreshMsg or an RsslStatusMsg) */
	RSSL_STREAM_CLOSED			= 4,    /*!< (4) Closed (indicates that the data is not available on this service/connection and is not likely to become available) */
    RSSL_STREAM_REDIRECTED      = 5		/*!< (5) Closed and Redirected (indicates that the current stream has been closed and has new identifying information, the user can issue a new request for the data using the new message key information contained in the redirect message) */
} RsslStreamStates;


/** 
 * @brief RsslState Data States provide information on the health of the data flowing within a stream
 * @see RsslState
 */
typedef enum {
	RSSL_DATA_NO_CHANGE				= 0, 	/*!< (0) No change to the data state. (Typically used when code and text need to be conveyed, for RsslRefreshMsg and RsslStatusMsg, actual state of OK or SUSPECT should be used when available) */
	RSSL_DATA_OK					= 1,    /*!< (1) Data is Ok (indicates that all data associated with the stream is healthy and current) */
	RSSL_DATA_SUSPECT				= 2		/*!< (2) Data is Suspect (similar to a stale data state, indicates that the health of some or all data associated with the stream is out of date or cannot be confirmed that it is current ) */
} RsslDataStates;



/** 
 * @brief RsslState State Codes provide additional information about the current state.  Applications should typically not trigger behavior off of this information.
 * @see RsslState
 */
typedef enum {
	RSSL_SC_NONE							= 0,	/*!< (0) No state code */
	RSSL_SC_NOT_FOUND						= 1,	/*!< (1) Not found (indicates that requested information was not found, it may become available at a later time or by changing some of the requested parameters) */
	RSSL_SC_TIMEOUT							= 2,	/*!< (2) Timeout (indicates that a timeout has occurred somewhere in the system while processing requested information) */
	RSSL_SC_NOT_ENTITLED					= 3,	/*!< (3) Not entitled (indicates that the request has been denied due to a permissioning issue) */
	RSSL_SC_INVALID_ARGUMENT				= 4,	/*!< (4) Invalid argument (indicates that a parameter on the request was invalid or unrecognized somewhere within the system) */
	RSSL_SC_USAGE_ERROR						= 5,	/*!< (5) Usage Error (indicates an invalid usage within the system) */
	RSSL_SC_PREEMPTED						= 6,	/*!< (6) Preempted (indicates the stream has been pre-empted, possibly by a caching device) */
	RSSL_SC_JIT_CONFLATION_STARTED			= 7,	/*!< (7) Conflation started (indicates that Just-In-Time Conflation has begun on the stream, user should be notified when JIT Conflation ends via an RSSL_SC_REALTIME_RESUMED code) */
	RSSL_SC_REALTIME_RESUMED				= 8,	/*!< (8) Realtime resumed (indicates that Just-In-Time Conflation has completed on the stream) */
	RSSL_SC_FAILOVER_STARTED				= 9,	/*!< (9) Failover started (indicates that a component has begun recovery due to a failover condition, user should be notified when recovery due to failover is completed via an RSSL_SC_FAILOVER_COMPLETED code) */
	RSSL_SC_FAILOVER_COMPLETED				= 10,	/*!< (10) Failover completed (indicates that recovery from failover condition has been completed) */
	RSSL_SC_GAP_DETECTED					= 11,	/*!< (11) Gap detected (indicates that gap has been detected between messages, this may have been detected via an external reliability mechanism (e.g. transport) or may have been detected using the seqNum present on the UPA messages) */
	RSSL_SC_NO_RESOURCES					= 12,	/*!< (12) No resources (indicates that there are no resources available to accommodate the stream) */
	RSSL_SC_TOO_MANY_ITEMS					= 13,	/*!< (13) Too many items open (indicates that a request cannot be processed because there are too many other streams already open) */
	RSSL_SC_ALREADY_OPEN					= 14,	/*!< (14) Item already open (indicates that a stream is already open on the connection for the requested information) */
	RSSL_SC_SOURCE_UNKNOWN					= 15,	/*!< (15) Unknown source (indicates that requested service is not known, service may become available at a later point in time) */
	RSSL_SC_NOT_OPEN						= 16,	/*!< (16) Not open (indicates that the stream is not opened) */
													/*!< (17) Reserved */
													/*!< (18) Reserved */
	RSSL_SC_NON_UPDATING_ITEM				= 19,	/*!< (19) Item was requested as streaming but does not update */
	RSSL_SC_UNSUPPORTED_VIEW_TYPE			= 20,	/*!< (20) View Type requested is not supported for this domain */
	RSSL_SC_INVALID_VIEW					= 21,	/*!< (21) An invalid view was requested */
	RSSL_SC_FULL_VIEW_PROVIDED				= 22,	/*!< (22) Although a view was requested, the full view is being provided */
	RSSL_SC_UNABLE_TO_REQUEST_AS_BATCH		= 23,	/*!< (23) Although a batch of items were requested, the batch was split into individual request messages */
													/*!< (24) Reserved */
													/*!< (25) Reserved */
	RSSL_SC_NO_BATCH_VIEW_SUPPORT_IN_REQ 	= 26, 	/*!< (26) Request does not support batch and view */
	RSSL_SC_EXCEEDED_MAX_MOUNTS_PER_USER 	= 27,	/*!< (27) Login rejected, exceeded maximum number of mounts per user */
	RSSL_SC_ERROR 							= 28, 	/*!< (28) Internal error from sender. */
	RSSL_SC_DACS_DOWN 						= 29,	/*!< (29) A21: Connection to DACS down, users are not allowed to connect" */
	RSSL_SC_USER_UNKNOWN_TO_PERM_SYS 		= 30, 	/*!< (30) User unknown to permissioning system, it could be DACS, AAA or EED */
	RSSL_SC_DACS_MAX_LOGINS_REACHED 		= 31, 	/*!< (31) Maximum logins reached. */
	RSSL_SC_DACS_USER_ACCESS_TO_APP_DENIED 	= 32,	/*!< (32)  The application is denied access to the system */
                                                    /*!< (33) Reserved */	
	RSSL_SC_GAP_FILL						= 34,	/*!< (34) Content is intended to fill a recognized gap */
	RSSL_SC_APP_AUTHORIZATION_FAILED		= 35,	/*!< (35) Application Authorization Failed */
    RSSL_SC_MAX_RESERVED					= 127	/*!< (127) Max reserved value */
} RsslStateCodes;


/**
 * @brief RsslState static initializer
 * @see RsslState, RsslStreamStates, RsslDataStates, RsslStateCodes, rsslClearState
 */
#define RSSL_INIT_STATE { RSSL_STREAM_UNSPECIFIED, RSSL_DATA_SUSPECT, RSSL_SC_NONE, RSSL_INIT_BUFFER }

/**
 * @brief Clears an RsslState structure
 * @see RsslState, RSSL_INIT_STATE
 */
#define rsslClearState(pState)        \
               ( void )( (pState)->streamState = RSSL_STREAM_UNSPECIFIED, \
                         (pState)->dataState   = RSSL_DATA_SUSPECT, \
                         (pState)->code        = RSSL_SC_NONE, \
                         rsslClearBuffer( &(pState)->text ) )



/**
 * @}
 */

/**
 *	@addtogroup RsslStateUtils
 *	@{
 */

/**
 * @brief Provide string representation for an RsslState, including all RsslState members
 * @param oBuffer RsslBuffer to populate with string.  RsslBuffer::data should point memory to convert into where RsslBuffer::length indicates the number of bytes available in RsslBuffer::data.
 * @param pState Fully populated RsslState structure
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_FAILURE otherwise
 * @see RsslState
 */
RSSL_API RsslRet rsslStateToString(RsslBuffer *oBuffer, RsslState *pState);



/**
 * @brief Provide string representation for an RsslState::code
 * @param value \ref RsslStateCodes enumeration to convert to string
 * @return const char* representation of corresponding \ref RsslStateCodes
 * @see RsslStateCodes, RsslState
 */
RSSL_API const char* rsslStateCodeToString(RsslUInt8 code);

/**
 * @brief Provide string description for an RsslState::code
 * @param value \ref RsslStateCodes enumeration to provide description for
 * @return const char* description of corresponding \ref RsslStateCodes
 * @see RsslStateCodes, RsslState
 */
RSSL_API const char* rsslStateCodeDescription(RsslUInt8 code);

/**
 * @brief Provide string representation for an RsslState::streamState
 * @param value \ref RsslStreamStates enumeration to convert to string
 * @return const char* representation of corresponding \ref RsslStreamStates
 * @see RsslStreamStates, RsslState
 */
RSSL_API const char* rsslStreamStateToString(RsslUInt8 code);


/**
 * @brief Provide string representation for an RsslState::dataState
 * @param value \ref RsslDataStates enumeration to convert to string
 * @return const char* representation of corresponding \ref RsslDataStates
 * @see RsslDataStates, RsslState
 */
RSSL_API const char* rsslDataStateToString(RsslUInt8 code);


/**
 * @brief Provide string representation of meaning associated with RsslState::code
 * @param code \ref RsslStateCodes enumeration to provide meaning as string
 * @return const char* representation of meaning for corresponding \ref RsslStateCodes
 * @see RsslStateCodes, RsslState
 */
RSSL_API const char* rsslStateCodeInfo(RsslUInt8 code);


/**
 * @brief Provide string representation of meaning associated with RsslState::streamState
 * @param code \ref RsslStreamStates enumeration to provide meaning as string
 * @return const char* representation of meaning for corresponding \ref RsslStreamStates
 * @see RsslStreamStates, RsslState
 */
RSSL_API const char* rsslStreamStateInfo(RsslUInt8 code);


/**
 * @brief Provide string representation of meaning associated with RsslState::dataState
 * @param code \ref RsslDataStates enumeration to provide meaning as string
 * @return const char* representation of meaning for corresponding \ref RsslDataStates
 * @see RsslDataStates, RsslState
 */
RSSL_API const char* rsslDataStateInfo(RsslUInt8 code);



/**
 * @brief Checks is the RsslState is a final state (e.g. ::RSSL_STREAM_CLOSED, ::RSSL_STREAM_CLOSED_RECOVER, or ::RSSL_STREAM_REDIRECTED)
 * @param pState Populated RsslState to check 
 * @return RsslBool RSSL_TRUE if RsslState is final, RSSL_FALSE otherwise
 * @see RsslState
 */
RTR_C_INLINE RsslBool rsslIsFinalState( const RsslState *pState )
{
	return pState == 0 || pState->streamState == RSSL_STREAM_CLOSED_RECOVER ||
				pState->streamState == RSSL_STREAM_CLOSED 	||
				pState->streamState == RSSL_STREAM_REDIRECTED;
}

/** 
 * @}
 */


#ifdef __cplusplus
}
#endif


#endif /* __RSSL_STATE_H */

