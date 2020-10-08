/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019-2020 Refinitiv. All rights reserved.
*/

#ifndef _RTR_RSSL_EVENTS_H
#define _RTR_RSSL_EVENTS_H

#include "rtr/rsslReactorChannel.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *	@addtogroup VAReactorEvent
 *	@{
 */

/**
 * @brief Type indicating the type of RsslReactorChannelEvent.
 * @see RsslReactorChannelEvent
 */
typedef enum
{
	RSSL_RC_CET_MIN				= -128,	/*!< Minimum value expected - this enumeration is for range checking */
	RSSL_RC_CET_INIT			= 0,	/*!< Unknown event type. */
	RSSL_RC_CET_CHANNEL_UP		= 1,	/*!< Channel has successfully initialized and can be dispatched. If the application presented any messages for setting
										 * up the session in their RsslReactorChannelRole structure in rsslReactorConnect() or rsslReactorAccept(),
										 * they will now be sent. */
	RSSL_RC_CET_CHANNEL_READY	= 2,	/*!< Channel has sent and received all messages expected for setting up the session. Normal use(such as item requests) can now be done. */
	RSSL_RC_CET_CHANNEL_DOWN	= 3,	/*!< Channel has failed(e.g. the connection was lost or a ping timeout expired) and will no longer be processed.  The application should call rsslReactorCloseChannel() to clean up the channel. */
	RSSL_RC_CET_FD_CHANGE		= 4,	/*!< The file descriptor representing this channel has changed. The new and old Socket ID can be found on the RsslReactorChannel. */
	RSSL_RC_CET_WARNING			= 5,		/*!< An event has occurred that did not result in channel failure, but may require attention by the application. */
	RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING = 6,
	RSSL_RC_CET_CHANNEL_OPENED	= 7		/*!< Channel was opened by the application and can be used (occurs when watchlist is enabled and only appears in the channelOpenCallback). */
} RsslReactorChannelEventType;

/**
 * @brief An event that has occurred on an RsslReactorChannel.
 * @see RsslReactorChannel
 */
typedef struct
{
	RsslReactorChannelEventType	channelEventType;	/*!< The type of the event. Populated by RsslReactorChannelEventType. */
	RsslReactorChannel			*pReactorChannel;	/*!< The channel associated with this event. */
	RsslErrorInfo				*pError; 			/*!< Present on warnings and channel down events. Contains information about the error that occurred. */
} RsslReactorChannelEvent;

/**
 * @brief Clears an RsslReactorChannelEvent.
 * @see RsslReactorChannelEvent
 */
RTR_C_INLINE void rsslClearReactorChannelEvent(RsslReactorChannelEvent *pEvent)
{
	memset(pEvent, 0, sizeof(RsslReactorChannelEvent));
}

/** 
 * @brief Information about the stream associated with a message.
 * Only used when a watchlist is enabled. */
typedef struct
{
	const RsslBuffer	*pServiceName;	/*!< Name of service associated with the stream, if any. */
	void				*pUserSpec;		/*!< User-specified pointer given when the stream was opened. */
} RsslStreamInfo;

/**
 * @brief Base event provided to Message Callback functions.  This structure is provided to defaultMsgCallback functions
 * and is present in structures provided to other Message Callback functions such as loginMsgCallback and directoryMsgCallback. 
 */
typedef struct
{
	RsslBuffer		*pRsslMsgBuffer;	/*!< The raw message that was read and processed. */
	RsslMsg			*pRsslMsg;			/*!< The RsslMsg structure. Present if decoding of the message was successful. */
	RsslStreamInfo	*pStreamInfo;		/*!< Stream information (watchlist enabled only). */
	RsslErrorInfo	*pErrorInfo;		/*!< Error information. Present if a problem was encountered, and provides information about the error and its location in the source code. */
	RsslUInt32		*pSeqNum;			/*!< Sequence number associated with this message. */
	RsslUInt8		*pFTGroupId;		/*!< FTGroupId associated with this message. */
} RsslMsgEvent;

/**
 * @brief Clears an RsslMsgEvent.
 * @see RsslMsgEvent
 */
RTR_C_INLINE void rsslClearMsgEvent(RsslMsgEvent *pEvent)
{
	pEvent->pRsslMsgBuffer = NULL;
	pEvent->pRsslMsg = NULL;
	pEvent->pErrorInfo = NULL;
	pEvent->pStreamInfo = NULL;
	pEvent->pSeqNum = NULL;
	pEvent->pFTGroupId = NULL;
}

typedef enum
{
	RSSL_RDM_LG_LME_NONE = 0x00,	/*!< No flags for this event. */
	RSSL_RDM_LG_LME_RTT_RESPONSE_SENT = 0x01	/*!< The RTT response has already been sent for this event.  No further user reponse is required. */
} RsslRDMLoginRTTEventFlags;

/**
 * @brief Event provided to Login Message Callback functions.  
 * @see RsslMsgEvent
 */
typedef struct
{
	RsslMsgEvent	baseMsgEvent;		/*!< Base Message event. */
	RsslRDMLoginMsg		*pRDMLoginMsg;	/*!< The RDM Representation of the decoded Login message.  If not present, an error was encountered while decoding and information will be in baseMsgEvent.pErrorInfo */
	RsslUInt32		flags;				/*!< Flags indicating any additional behaviors for this Login event. */
} RsslRDMLoginMsgEvent;

/**
 * @brief Clears an RsslRDMLoginMsgEvent
 * @see RsslRDMLoginMsgEvent
 */
RTR_C_INLINE void rsslClearRDMLoginMsgEvent(RsslRDMLoginMsgEvent *pEvent)
{
	rsslClearMsgEvent(&pEvent->baseMsgEvent);
	pEvent->pRDMLoginMsg = NULL;
	pEvent->flags = RSSL_RDM_LG_LME_NONE;
}

/**
 * @brief Event provided to Directory Message Callback functions.  
 * @see RsslMsgEvent
 */
typedef struct
{
	RsslMsgEvent	baseMsgEvent;			/*!< Base Message event. */
	RsslRDMDirectoryMsg	*pRDMDirectoryMsg;	/*!< The RDM Representation of the decoded Directory message.  If not present, an error was encountered while decoding and information will be in baseMsgEvent.pErrorInfo */
} RsslRDMDirectoryMsgEvent;


/**
 * @brief Clears an RsslRDMDirectoryMsgEvent
 * @see RsslRDMDirectoryMsgEvent
 */
RTR_C_INLINE void rsslClearRDMDirectoryMsgEvent(RsslRDMDirectoryMsgEvent *pEvent)
{
	rsslClearMsgEvent(&pEvent->baseMsgEvent);
	pEvent->pRDMDirectoryMsg = NULL;
}

/**
 * @brief Event provided to Dicionary Message Callback functions.  
 * @see RsslMsgEvent
 */
typedef struct
{
	RsslMsgEvent		baseMsgEvent;			/*!< Base Message event. */
	RsslRDMDictionaryMsg	*pRDMDictionaryMsg;	/*!< The RDM Representation of the decoded Dicionary message.  If not present, an error was encountered while decoding and information will be in baseMsgEvent.pErrorInfo */
} RsslRDMDictionaryMsgEvent;


/**
 * @brief Clears an RsslRDMDicionaryMsgEvent
 * @see RsslRDMDicionaryMsgEvent
 */
RTR_C_INLINE void rsslClearRDMDictionaryMsgEvent(RsslRDMDictionaryMsgEvent *pEvent)
{
	rsslClearMsgEvent(&pEvent->baseMsgEvent);
	pEvent->pRDMDictionaryMsg = NULL;
}

/**
* @brief Represents authentication token information
*/
typedef struct
{
	RsslBuffer		accessToken;	/*!<represents access token is used to invoke REST data API calls. */
	RsslBuffer		refreshToken;	/*!<represents refresh token is used for getting next access token. */
	RsslInt			expiresIn;		/*!<represents access token validity time in seconds. */
	RsslBuffer		tokenType;		/*!<represents a token type for specifying in the Authorization header */
	RsslBuffer		scope;			/*!<represents a list of all the scopes this token can be used with. */
} RsslReactorAuthTokenInfo;

/**
 * @brief Clears an RsslReactorAuthTokenInfo.
 * @see RsslReactorAuthTokenInfo
 */
RTR_C_INLINE void rsslClearReactorAuthTokenInfo(RsslReactorAuthTokenInfo *pEvent)
{
	memset(pEvent, 0, sizeof(RsslReactorAuthTokenInfo));
	pEvent->expiresIn = -1;
}

/**
 * @brief An token information event that has occurred on an RsslReactorChannel.
 * @see RsslReactorChannel, RsslReactorAuthTokenInfo
 */
typedef struct
{
	RsslReactorChannel			*pReactorChannel;	/*!< The channel associated with this event. */
	RsslReactorAuthTokenInfo	*pReactorAuthTokenInfo; /*!< The token information associated with this event. */
	RsslErrorInfo				*pError; 			/*!< Contains information about the error that occurred with the token information. */
	RsslUInt32					statusCode;			/*!< Represents HTTP response status code */
} RsslReactorAuthTokenEvent;

/**
 * @brief Clears an RsslReactorAuthTokenEvent.
 * @see RsslReactorAuthTokenEvent
 */
RTR_C_INLINE void rsslClearReactorAuthTokenEvent(RsslReactorAuthTokenEvent *pEvent)
{
	memset(pEvent, 0, sizeof(RsslReactorAuthTokenEvent));
}

/**
* @brief Represents Service Endpoint information
*/
typedef struct
{
	RsslBuffer      *dataFormatList; /*!< A list of data formats. The list indicates the data format used by transport.*/
	RsslUInt32      dataFormatCount; /*!< The number of data formats in dataFormatList. */
	RsslBuffer      endPoint;        /*!< A domain name of the service access endpoint. */
	RsslBuffer      *locationList;   /*!< A list of locations. The list indicates the location of the service. */
	RsslUInt32      locationCount;   /*!< The number of locations in locationList. */
	RsslBuffer      port;            /*!< A port number used to establish connection. */
	RsslBuffer      provider;        /*!< A public Refinitiv Real-Time - Optimized provider. */
	RsslBuffer      transport;       /*!< A transport type used to access service. */
} RsslReactorServiceEndpointInfo;

/**
 * @brief Clears an RsslReactorServiceEndpointInfo.
 * @see RsslReactorServiceEndpointInfo
 */
RTR_C_INLINE void rsslClearReactorServiceEndpointInfo(RsslReactorServiceEndpointInfo *pEvent)
{
	memset(pEvent, 0, sizeof(RsslReactorServiceEndpointInfo));
}

/**
 * @brief A service endpoint information event that has occurred.
 * @see RsslReactorServiceEndpointInfo
 */
typedef struct
{
	RsslReactorServiceEndpointInfo *serviceEndpointInfoList; /*!< The list of service endpoints associated with this event. */
	RsslUInt32                     serviceEndpointInfoCount; /*!< The number of service endpoint information in serviceEndpointInfoList. */
	void                           *userSpecPtr;        /*!< A user-specified pointer associated with this RsslReactorServiceEndpointEvent. */
	RsslErrorInfo                  *pErrorInfo;             /*!< Contains information about the error that occurred with RDP token service and service discovery 
                                                             * which provides information about the error and its location in the source code. */
	RsslUInt32                     statusCode;          /*!< Represents HTTP response status code */
} RsslReactorServiceEndpointEvent;

/**
 * @brief Clears an RsslReactorServiceEndpointEvent.
 * @see RsslReactorServiceEndpointEvent
 */
RTR_C_INLINE void rsslClearReactorServiceEndpointEvent(RsslReactorServiceEndpointEvent *pEvent)
{
	memset(pEvent, 0, sizeof(RsslReactorServiceEndpointEvent));
}

/**
 * @brief Structure representing the OAuth credential for renewal authentication with sensitive information for password and client secret.
 * @see RsslReactorOAuthCredentialEvent
 */
typedef struct
{
	RsslBuffer					userName;					/*!< The user name to authorize with the RDP token service. This is used to get sensitive information
															 *   for the user name in the RsslReactorOAuthCredentialEventCallback. */
	RsslBuffer					password;					/*!< The password for user name used to get an access token and a refresh token. */
	RsslBuffer					newPassword;				/*!< The new password to change the password associated with this user name.
															 *   Both current and new passwords will be required in order to authenticate and change password. Optional.*/
	RsslBuffer					clientId;					/*!< A unique ID defined for an application marking the request. Optional */
	RsslBuffer					clientSecret;				/*!< A secret used by OAuth client to authenticate to the Authorization Server. Optional */
	RsslBuffer					tokenScope;					/*!< A user can optionally limit the scope of generated token. Optional. */
	RsslBool					takeExclusiveSignOnControl;	/*!< The exclusive sign on control to force sign-out of other applications using the same credentials. Optional */
} RsslReactorOAuthCredentialRenewal;

/**
 * @brief Clears an RsslReactorOAuthCredentialRenewal.
 * @see RsslReactorOAuthCredentialRenewal
 */
RTR_C_INLINE void rsslClearReactorOAuthCredentialRenewal(RsslReactorOAuthCredentialRenewal *pOAuthCredentialRenewal)
{
	memset(pOAuthCredentialRenewal, 0, sizeof(RsslReactorOAuthCredentialRenewal));
	pOAuthCredentialRenewal->tokenScope.data = (char *)"trapi.streaming.pricing.read";
	pOAuthCredentialRenewal->tokenScope.length = 28;
	pOAuthCredentialRenewal->takeExclusiveSignOnControl = RSSL_TRUE;
}

/**
 * @brief An OAuth credential event that has occurred for users to specify OAuth credentials(password and/or client secret).
 * This event is dispatched to application whenever the Reactor requires the sensitive information to renew the access and refresh token.
 * @see RsslReactorChannel, RsslReactorOAuthCredentialRenewal
 */

typedef struct
{
	RsslReactorChannel					*pReactorChannel;						/*!< The channel associated with this event. */
	RsslReactorOAuthCredentialRenewal	*pReactorOAuthCredentialRenewal;		/*!< The OAuth credential for renewal authentication with the RDP token service. */
} RsslReactorOAuthCredentialEvent;

/**
 * @brief Clears an RsslReactorOAuthCredentialEvent.
 * @see RsslReactorOAuthCredentialEvent
 */
RTR_C_INLINE void rsslClearReactorOAuthCredentialEvent(RsslReactorOAuthCredentialEvent *pEvent)
{
	memset(pEvent, 0, sizeof(RsslReactorOAuthCredentialEvent));
}

/**
 * @brief This event occurs when the Reactor needs to convert from a service name to a service Id.
 * @see RsslReactor
 */

typedef struct
{
	void				*pUserSpec;		/*!< User-specified pointer given when specifying the callback for this event. */
} RsslReactorServiceNameToIdEvent;

/**
 * @brief Clears an RsslReactorServiceNameToIdEvent.
 * @see RsslReactorServiceNameToIdEvent
 */
RTR_C_INLINE void rsslClearReactorServiceNameToIdEvent(RsslReactorServiceNameToIdEvent *pEvent)
{
	memset(pEvent, 0, sizeof(RsslReactorServiceNameToIdEvent));
}

/**
 * @brief This event occurs when the Reactor fails to convert from JSON to RWF protocol.
 * @see RsslReactor
 */

typedef struct
{
	void				*pUserSpec;		/*!< User-specified pointer given when specifying the callback for this event. */
	RsslErrorInfo		*pError; 		/*!< Contains information about the error that occurred with the JSON conversion. */
} RsslReactorJsonConversionEvent;

/**
 * @brief Clears an RsslReactorJsonConversionEvent.
 * @see RsslReactorJsonConversionEvent
 */
RTR_C_INLINE void rsslClearReactorJsonConversionEvent(RsslReactorJsonConversionEvent *pEvent)
{
	memset(pEvent, 0, sizeof(RsslReactorJsonConversionEvent));
}

/**
 *	@}
 */

#ifdef __cplusplus
}
#endif

#endif
