

#ifndef __RSSL_MSG_H_
#define __RSSL_MSG_H_


#ifdef __cplusplus
extern "C" {
#endif

/* Message Package Headers */
#include "rtr/rsslRequestMsg.h"
#include "rtr/rsslRefreshMsg.h"
#include "rtr/rsslStatusMsg.h"
#include "rtr/rsslUpdateMsg.h"
#include "rtr/rsslCloseMsg.h"
#include "rtr/rsslAckMsg.h"
#include "rtr/rsslGenericMsg.h"
#include "rtr/rsslPostMsg.h"



/**
 * @addtogroup RSSLWFMessages
 * @{
 */


/** 
 * @brief Enumeration that defines the value associated with the different message classes offered in OMM.  These are populated in the RsslMsgBase::msgClass member. (MC = MsgClass)
 * @see RsslMsgBase, RsslMsg, rsslMsgClassToString
 */
typedef enum {
	RSSL_MC_REQUEST		= 1,     /*!< (1) Request Message */
	RSSL_MC_REFRESH		= 2,	 /*!< (2) Refresh Message */
	RSSL_MC_STATUS		= 3,     /*!< (3) Status Message */
	RSSL_MC_UPDATE		= 4,     /*!< (4) Update Message */
	RSSL_MC_CLOSE		= 5,	 /*!< (5) Close Message */
	RSSL_MC_ACK			= 6,	 /*!< (6) Acknowledgement Message */
	RSSL_MC_GENERIC		= 7,	 /*!< (7) Generic Bidirectional Message */
	RSSL_MC_POST		= 8		 /*!< (8) Post Message */
} RsslMsgClasses;

/** 
 * @brief General OMM strings associated with the different message classes offered in OMM.  
 * @see RsslMsgClasses, rsslMsgClassToOmmString
 */
static const RsslBuffer RSSL_OMMSTR_MC_REQUEST = { 7, (char*)"Request" };
static const RsslBuffer RSSL_OMMSTR_MC_REFRESH = { 7, (char*)"Refresh" };
static const RsslBuffer RSSL_OMMSTR_MC_STATUS = { 6, (char*)"Status" };
static const RsslBuffer RSSL_OMMSTR_MC_UPDATE = { 6, (char*)"Update" };
static const RsslBuffer RSSL_OMMSTR_MC_CLOSE = { 5, (char*)"Close" };
static const RsslBuffer RSSL_OMMSTR_MC_ACK = { 3, (char*)"Ack" };
static const RsslBuffer RSSL_OMMSTR_MC_GENERIC = { 7, (char*)"Generic" };
static const RsslBuffer RSSL_OMMSTR_MC_POST = { 4, (char*)"Post" };

/**
 * @}
 */


/**
 * @addtogroup RsslMsgUnion 
 * @{
 */

/** 
 * @brief The OMM Message Union. All message encoding and decoding functions expect the
 * RsslMsg type. Any specific message class can be cast to the RsslMsg, and an RsslMsg
 * can be cast to any specific message class. When decoding, the RsslMsg.msgBase contains
 * common members that can be used to identify the specific message class or domain type.
 * @see RsslMsgBase, RsslRequestMsg, RsslAckMsg, RsslRefreshMsg, RsslStatusMsg, RsslUpdateMsg, RsslCloseMsg, RsslGenericMsg, RsslPostMsg, RSSL_INIT_MSG, rsslClearMsg
 */
typedef union {
	RsslMsgBase		msgBase;	    /*!< @brief The message base; members that tend to be common across messages */
	RsslRequestMsg	requestMsg;     /*!< @brief Request Message */
	RsslAckMsg		ackMsg;			/*!< @brief Acknowledgement Message */
	RsslRefreshMsg	refreshMsg;     /*!< @brief Refresh Message */
	RsslStatusMsg	statusMsg;      /*!< @brief Status Message */
	RsslUpdateMsg	updateMsg;      /*!< @brief Update Message */
	RsslCloseMsg	closeMsg;		/*!< @brief Close Message */
	RsslGenericMsg	genericMsg;		/*!< @brief Generic Bidirectional Message */
	RsslPostMsg		postMsg;		/*!< @brief Post Message */
} RsslMsg;


/**
 * @brief Static initializer for the RsslMsg.  
 *
 * @warning On larger structures, like messages, the clear functions tend to outperform the static initializer.  It is recommended to use the clear function when initializing any messages.
 *
 * @see RsslMsg, rsslClearMsg
 */
#define RSSL_INIT_MSG { RSSL_INIT_MSG_BASE }

/**
 * @brief Clears an RsslMsg
 * @see RsslMsg, RSSL_INIT_MSG
 */
RTR_C_INLINE void rsslClearMsg(RsslMsg *pMsg)
{
	memset(pMsg, 0, sizeof(RsslMsg));
}

/**
 *	@}
 */


/**
 * @addtogroup MsgEncodeUtilsHelpers
 * @{
 */

/**
 * @brief Validate a populated RsslMsg
 *
 * Validates fully populated RsslMsg structure to ensure that it's flags and data members are properly set, and can be encoded without any issues.
 * @param pMsg Pointer to a fully populated RsslMsg
 * @return \ref RSSL_TRUE if the message is valid, and can be properly encoded. \ref RSSL_FALSE if the message is not valid.
 * @see rsslCopyMsg, rsslReleaseCopiedMsg
 */
RSSL_API RsslBool rsslValidateMsg( const RsslMsg *pMsg );

/**
 *	@}
 */
 
/**
 * @addtogroup MsgDecodeUtilsHelpers
 * @{
 */

/**
 * @brief Checks if this RsslMsg is the final message for a stream.
 *
 * @param pMsg  Pointer to a fully populated RsslMsg
 * @return \ref RSSL_TRUE if the pMsg is a Final Message for the request, RSSL_FALSE otherwise
 */
RTR_C_INLINE RsslBool  rsslIsFinalMsg( const RsslMsg * pMsg )
{
	switch( pMsg->msgBase.msgClass )
	{
		case RSSL_MC_REFRESH:
			return (rsslIsFinalState( &pMsg->refreshMsg.state) ||
				((pMsg->refreshMsg.flags & RSSL_RFMF_REFRESH_COMPLETE) &&
					pMsg->refreshMsg.state.streamState == RSSL_STREAM_NON_STREAMING));

		case RSSL_MC_STATUS:
			return ( pMsg->statusMsg.flags & RSSL_STMF_HAS_STATE ) ?
				rsslIsFinalState( & pMsg->statusMsg.state) : 
				RSSL_FALSE;

		case RSSL_MC_CLOSE:
			return RSSL_TRUE;
	}
	return RSSL_FALSE;
}

/**
 * @brief Retrieve the sequence number from the provided decoded message structure
 *
 * @param pMsg              fully populated RsslMsg
 * @return Sequence number contained in the RsslMsg.
 */
RSSL_API const RsslUInt32* rsslGetSeqNum( const RsslMsg * pMsg );



/**
 * @brief Retrieve the RsslMsgKey structure from the provided decoded message structure
 *
 * @param pMsg             fully populated RsslMsg
 * @return Pointer to the RsslMsgKey structure contained in the RsslMsg.
 */
RSSL_API const RsslMsgKey* rsslGetMsgKey( const RsslMsg * pMsg );

/**
 * @brief Retrieve the request RsslMsgKey structure from the provided decoded message structure
 *
 * @param pMsg             fully populated RsslMsg
 * @return Pointer to the RsslMsgKey structure contained in the RsslMsg.
 */
RSSL_API const RsslMsgKey* rsslGetReqMsgKey( const RsslMsg * pMsg );


/**
 * @brief Retrieve flags from the provided decoded message structure
 *
 * @param pMsg				fully populated RsslMsg
 * @return Flag values contained in the RsslMsg.
 */
RSSL_API const RsslUInt16* rsslGetFlags( const RsslMsg * pMsg );


/**
 * @brief Retrieve extended header from the provided decoded message structure
 *
 * @param pMsg             fully populated RsslMsg
 * @return Pointer to the Extended Header buffer contained in the RsslMsg.
 */
RSSL_API const RsslBuffer* rsslGetExtendedHeader( const RsslMsg * pMsg );

/**
 * @brief Retrieve state from the provided decoded message structure
 *
 * @param pMsg             fully populated RsslMsg
 * @return Pointer to the RsslState structure contained in the RsslMsg.
 */
RSSL_API const RsslState* rsslGetState( const RsslMsg * pMsg );

/**
 * @brief Retrieve permission expression from the provided decoded message structure
 *
 * @param pMsg             fully populated RsslMsg
 * @return Pointer to the permission data buffer contained in the RsslMsg.
 */
RSSL_API const RsslBuffer* rsslGetPermData( const RsslMsg * pMsg );


/**
 * @brief Retrieve group Id from the provided decoded message structure
 *
 * @param pMsg             fully populated RsslMsg
 * @return Pointer to the GroupId buffer contained in the RsslMsg. 
 */
RSSL_API const RsslBuffer* rsslGetGroupId( const RsslMsg * pMsg );


/** 
 * @brief Used to add a two byte identifier to the group ID buffer.
 *
 * @warning <ul>
 *			<li> This should not be run on a group ID buffer contained in a decoded message, as this can cause data corruption.
 *			<li> The user must have allocated enough free space to the buffer to add another two byte identifier.
 *			</ul>
 *			
 * @param[in] groupIdToAdd 	two byte ID to append to the groupID contained in pGroupId buffer
 * @param[out] pGroupId 	RsslBuffer with any existing groupID information. 
 * @return On success, returns RSSL_RET_SUCCESS.
 */
RSSL_API RsslRet rsslAddGroupId( RsslBuffer *pGroupId,
						RsslUInt16 groupIdToAdd);

/**
 *	@}
 */


/**
 * @addtogroup MsgCopyUtils
 * @{
 */
 
/** 
 * @brief The Copy Message flags (CMF = CopyMsg Flags)
 *		  These flags are used by \ref rsslCopyMsg to control what is copied.
 * @see rsslCopyMsg
 */
typedef enum {
	RSSL_CMF_NONE			    = 0x00000,		/*!< (0x00000) No Flags set, no sub-structs will be copied */
    RSSL_CMF_STATE_TEXT         = 0x00001,      /*!< (0x00001) State text will be copied */
    RSSL_CMF_PERM_DATA          = 0x00002,      /*!< (0x00002) Perm exp will be copied */
    RSSL_CMF_KEY_NAME           = 0x00004,      /*!< (0x00004) Key name will be copied */           
    RSSL_CMF_KEY_ATTRIB         = 0x00008,      /*!< (0x00008) Key attrib will be copied */
    RSSL_CMF_KEY                = 0x0000C,      /*!< (0x0000C) Entire key will be copied */
    RSSL_CMF_EXTENDED_HEADER    = 0x00010,		/*!< (0x00010) Extended header will be copied */
	RSSL_CMF_DATA_BODY		    = 0x00020,      /*!< (0x00020) Data body will be copied */
    RSSL_CMF_MSG_BUFFER		    = 0x00040,      /*!< (0x00040) Encoded message buffer will be copied */
	RSSL_CMF_GROUP_ID			= 0x00080,		/*!< (0x00080) Group Id will be copied */
	RSSL_CMF_NAK_TEXT			= 0x00100,		/*!< (0x00100) Nak Text will be copied */
    RSSL_CMF_REQ_KEY_NAME       = 0x00200,      /*!< (0x00200) req Key name will be copied */           
    RSSL_CMF_REQ_KEY_ATTRIB     = 0x00400,      /*!< (0x00400) req Key attrib will be copied */
    RSSL_CMF_REQ_KEY            = 0x00600,      /*!< (0x00600) Entire req key will be copied */
    RSSL_CMF_ALL_FLAGS          = 0x00FFF,		/*!< (0x00FFF) Everything will be copied */
	RSSL_CMF_NO_EMPTY_ALLOC		= 0x10000		/*!< (0x10000) Do not allocate if all blocks are filtered out */ 
} RsslCopyMsgFlags;



/**
 * @brief Copy RsslMsg to a user-allocated or dynamically allocated buffer.
 *
 * Copies the fully populated RsslMsg structure according to the provided \ref RsslCopyMsgFlags. Depending on the flag 
 * combination provided, this will be either a deep copy if \ref RsslCopyFlags::RSSL_CMF_ALL_FLAGS is used, or a shallow 
 * copy if no flags are present. If a buffer is provided through the pCopyBuffer parameter, this function will copy the 
 * source RsslMsg into the user provided buffer. If a buffer is not provided, the function will dynamically allocate the memory 
 * required to copy the RsslMsg.
 *
 * @note If a user allocated buffer is not provided, the memory allocated for the new copy will not be automatically freed.
 *		    The user must call rsslReleaseCopiedMsg to manually free the data once they are finished with the copy.
 * 
 * @param pSrcMsg          Source RsslMsg
 * @param copyMsgFlags     Copy Message Flags, control how message is copied.  See \ref RsslCopyMsgFlags for more information.
 * @param filterBlocks     Block Mask of Blocks to be included in the final msg
 * @param pCopyBuffer      Optional buffer to copy msg into - in this case user needs to allocate and free memory
 * @return On success, returns a pointer to the copied RsslMsg. On failure, returns 0.
 * @see rsslReleaseCopiedMsg, RsslCopyMsgFlags, rsslValidateMsg, rsslSizeOfMsg
 */
RSSL_API RsslMsg* rsslCopyMsg(   const RsslMsg *pSrcMsg, 
									   RsslUInt32 copyMsgFlags, 
									   RsslUInt32 filterBlocks,
									   RsslBuffer *pCopyBuffer);


/**
 * @brief Release allocated RsslMsg memory created by \ref rsslCopyMsg.
 *
 * @warning This function should only be used if rsslCopyMsg allocated memory for the copied RsslMsg. If the user allocated memory
 *			for the copy(using the pCopyBuffer parameter), this function does not need to be called.
 *
 * @param pMsg       RsslMsg to be freed 
 * @see rsslCopyMsg, rsslValidateMsg
 */
RSSL_API void     rsslReleaseCopiedMsg( RsslMsg * pMsg );


/**
 * @brief This function calculates a deep SizeOf on RsslMsg structure.
 * 
 * This function is intended to assist users who wish to cache or copy the RsslMsg structures by calculating the 
 * fully allocated size of an RsslMsg. In addition to the size of the structure, it also will add in the 
 * size of allocated buffers in the message structure(e.g. \ref RsslMsgKey::name, \ref RsslRefreshMsg::extendedHeader) 
 * according to the \ref RsslCopyMsgFlags options set in the copyMsgFlags parameter.
 *
 * @warning This function will not give an accurate size of an encoded RsslMsg sent on the wire.
 *
 * @param pSrcMsg          	Source RsslMsg
 * @param copyMsgFlags 		Copy Message Flags to control which parts of the message are sized
 * @return Size(in bytes) of the RsslMsg's allocated memory.
 * @see rsslCopyMsg, RsslCopyMsgFlags
 */
RSSL_API RsslUInt32 rsslSizeOfMsg( const RsslMsg *pSrcMsg, RsslUInt32 copyMsgFlags); 
                      

/**
 *	@}
 */





#ifdef __cplusplus
}
#endif


#endif

