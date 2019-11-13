/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

/*
 * This is the UPA Interactive Provider Training series of the UPA Training Suite
 * applications. The purpose of this application is to show step-by-step 
 * training how to build a UPA OMM Interactive Provider using the UPA Transport layer.
 *
 * Main h header file for the UPA Interactive Provider Training application. It is a 
 * single-threaded client application.
 *
 ************************************************************************
 * UPA Interactive Provider Training Module 1a: Establish network communication
 ************************************************************************
 * Summary:
 * An OMM Interactive Provider application opens a listening socket on a well-known 
 * port allowing OMM consumer applications to connect. Once connected, consumers 
 * can request data from the Interactive Provider.
 * 
 * In this module, the OMM Interactive Provider application opens a listening socket 
 * on a well-known port allowing OMM consumer applications to connect.
 *
 * Detailed Descriptions:
 * The first step of any UPA Interactive Provider application is to establish 
 * a listening socket, usually on a well-known port so that consumer applications 
 * can easily connect. The provider uses the rsslBind function to open the port 
 * and listen for incoming connection attempts.
 * Whenever an OMM consumer application attempts to connect, the provider uses 
 * the rsslAccept function to begin the connection initialization process.
 * 
 * For this simple training app, the interactive provider only supports a single client. 
 *
 *
 ************************************************************************
 * UPA Interactive Provider Training Module 1b: Ping (heartbeat) Management
 ************************************************************************
 * Summary:
 * In this module, after establishing a connection, ping messages might 
 * need to be exchanged. The negotiated ping timeout is available via 
 * the RsslChannel. If ping heartbeats are not sent or received within 
 * the expected time frame, the connection can be terminated. Refinitiv 
 * recommends sending ping messages at intervals one-third the 
 * size of the ping timeout.
 *
 * Detailed Descriptions:
 * Once the connection is active, the consumer and provider applications 
 * might need to exchange ping messages. A negotiated ping timeout is available 
 * via RsslChannel corresponding to each connection (this value might differ on
 * a per-connection basis). A connection can be terminated if ping heartbeats 
 * are not sent or received within the expected time frame. Refinitiv 
 * recommends sending ping messages at intervals one-third the size of the ping timeout.
 * Ping or heartbeat messages are used to indicate the continued presence of 
 * an application. These are typically only required when no other information is 
 * being exchanged. Because the provider application is likely sending more frequent 
 * information, providing updates on any streams the consumer has requested, 
 * it may not need to send heartbeats as the other data is sufficient to announce 
 * its continued presence. It is the responsibility of each connection to manage 
 * the sending and receiving of heartbeat messages.
 *
 *
 ************************************************************************
 * UPA Interactive Provider Training Module 1c: Reading and Writing Data
 ************************************************************************
 * Summary:
 * In this module, when a client or server RsslChannel.state is 
 * RSSL_CH_STATE_ACTIVE, it is possible for an application to receive 
 * data from the connection. Similarly, when a client or server 
 * RsslChannel.state is RSSL_CH_STATE_ACTIVE, it is possible for an 
 * application to write data to the connection. Writing involves a several 
 * step process. 
 *
 * Detailed Descriptions:
 * When a client or server RsslChannel.state is RSSL_CH_STATE_ACTIVE, it is 
 * possible for an application to receive data from the connection. The 
 * arrival of this information is often announced by the I/O notification 
 * mechanism that the RsslChannel.socketId is registered with. The UPA 
 * Transport reads information from the network as a byte stream, after 
 * which it determines RsslBuffer boundaries and returns each buffer one by 
 * one.
 *
 * When a client or server RsslChannel.state is RSSL_CH_STATE_ACTIVE, it is 
 * possible for an application to write data to the connection. Writing 
 * involves a several step process. Because the UPA Transport provides 
 * efficient buffer management, the user is required to obtain a buffer 
 * from the UPA Transport buffer pool. This can be the guaranteed output 
 * buffer pool associated with an RsslChannel. After a buffer is acquired, 
 * the user can populate the RsslBuffer.data and set the RsslBuffer.length 
 * to the number of bytes referred to by data. If queued information cannot 
 * be passed to the network, a function is provided to allow the application 
 * to continue attempts to flush data to the connection. An I/O notification
 * mechanism can be used to help with determining when the network is able 
 * to accept additional bytes for writing. The UPA Transport can continue to
 * queue data, even if the network is unable to write. 
 *
 *
 ************************************************************************
 * UPA Interactive Provider Training Module 2: Perform/Handle Login Process
 ************************************************************************
 * Summary:
 * Applications authenticate with one another using the Login domain model. 
 * An OMM Interactive Provider must handle the consumer’s Login request messages 
 * and supply appropriate responses.
 * 
 * In this module, after receiving a Login request, the Interactive Provider 
 * can perform any necessary authentication and permissioning.
 *
 * Detailed Descriptions:
 * After receiving a Login request, the Interactive Provider can perform any 
 * necessary authentication and permissioning.
 *
 * a) If the Interactive Provider grants access, it should send an RsslRefreshMsg 
 * to convey that the user successfully connected. This message should indicate 
 * the feature set supported by the provider application.
 * b) If the Interactive Provider denies access, it should send an RsslStatusMsg, 
 * closing the connection and informing the user of the reason for denial.
 *
 * The login handler for this simple Interactive Provider application only allows
 * one login stream per channel. It provides functions for processing login requests
 * from consumers and sending back the responses. Functions for sending login request
 * reject/close status messages, initializing the login handler, and closing login streams 
 * are also provided.
 *
 * Also please note for simple training app, the interactive provider only supports 
 * one client session from the consumer, that is, only supports one channel/client connection.
 *
 * Content is encoded and decoded using the UPA Message Package and the UPA 
 * Data Package. 
 *
 *
 ************************************************************************
 * UPA Interactive Provider Training Module 3: Provide Source Directory Information
 ************************************************************************
 * Summary:
 * In this module, OMM Interactive Provider application provides Source Directory 
 * information. The Source Directory domain model conveys information about all 
 * available services in the system. An OMM consumer typically requests a Source Directory 
 * to retrieve information about available services and their capabilities. 
 * 
 * Detailed Descriptions:
 * The Source Directory domain model conveys information about all available services 
 * in the system. An OMM consumer typically requests a Source Directory to retrieve 
 * information about available services and their capabilities. This includes information 
 * about supported domain types, the service’s state, the QoS, and any item group 
 * information associated with the service. Refinitiv recommends that at a minimum, 
 * an Interactive Provider supply the Info, State, and Group filters for the Source Directory.
 * 
 * a) The Source Directory Info filter contains the name and serviceId for each 
 * available service. The Interactive Provider should populate the filter with information 
 * specific to the services it provides.
 *
 * b) The Source Directory State filter contains status information for the service informing 
 * the consumer whether the service is Up (available), or Down (unavailable).
 *
 * c) The Source Directory Group filter conveys item group status information, including 
 * information about group states, as well as the merging of groups. If a provider determines 
 * that a group of items is no longer available, it can convey this information by sending 
 * either individual item status messages (for each affected stream) or a Directory message 
 * containing the item group status information. 
 * 
 * Content is encoded and decoded using the UPA Message Package and the UPA 
 * Data Package.
 *
 *
 ************************************************************************
 * UPA Interactive Provider Training Module 4: Provide Necessary Dictionaries
 ************************************************************************
 * Summary:
 * In this module, OMM Interactive Provider application provides Necessary Dictionaries.
 * Some data requires the use of a dictionary for encoding or decoding. The dictionary 
 * typically defines type and formatting information, and tells the application how to 
 * encode or decode information.
 *
 * Detailed Descriptions:
 * Some data requires the use of a dictionary for encoding or decoding. The dictionary 
 * typically defines type and formatting information, and tells the application how to 
 * encode or decode information. Content that uses the RsslFieldList type requires the 
 * use of a field dictionary (usually the Refinitiv RDMFieldDictionary, though it 
 * can instead be a user-defined or modified field dictionary).
 * 
 * The Source Directory message should notify the consumer about dictionaries needed to 
 * decode content sent by the provider. If the consumer needs a dictionary to decode 
 * content, it is ideal that the Interactive Provider application also make this dictionary
 * available to consumers for download. The provider can inform the consumer whether the
 * dictionary is available via the Source Directory.
 * 
 * If loading from a file, UPA offers several utility functions for loading and managing 
 * a properly-formatted field dictionary. There are also utility functions provided to 
 * help the provider encode into an appropriate format for downloading. 
 *
 * Content is encoded and decoded using the UPA Message Package and the UPA 
 * Data Package.
 *
 *
 ************************************************************************
 * UPA Interactive Provider Training Module 5: Handle Item Requests
 ************************************************************************
 * Summary:
 * In this module, OMM Interactive Provider application handles Item Requests. Once 
 * connected, consumers can request data from the Interactive Provider. A provider can 
 * receive a request for any domain, though this should typically be limited to the 
 * domain capabilities indicated in the Source Directory. In this simple example, we 
 * are sending just 1 Market Price item response message to a channel. 
 *
 * Detailed Descriptions:
 * A provider can receive a request for any domain, though this should typically be 
 * limited to the domain capabilities indicated in the Source Directory. When a request 
 * is received, the provider application must determine if it can satisfy the request by:
 * 
 * a) Comparing msgKey identification information
 * b) Determining whether it can provide the requested QoS
 * c) Ensuring that the consumer does not already have a stream open for the requested 
 * information
 * 
 * If a provider can service a request, it should send appropriate responses. However, 
 * if the provider cannot satisfy the request, the provider should send an RsslStatusMsg 
 * to indicate the reason and close the stream. All requests and responses should follow 
 * specific formatting as defined in the domain model specification. The UPA RDM Usage
 * Guide defines all domains provided by Refinitiv.
 *
 * Content is encoded and decoded using the UPA Message Package and the UPA 
 * Data Package.
 *
 */

#ifndef _TR_UPA_Provider_TRAINING_H
#define _TR_UPA_Provider_TRAINING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"

#define RDNDISPLAY_FID 2
#define RDN_EXCHID_FID 4
#define DIVPAYDATE_FID 38
#define TRDPRC_1_FID 6
#define BID_FID 22
#define ASK_FID 25
#define ACVOL_1_FID 32
#define NETCHNG_1_FID 11
#define ASK_TIME_FID 267

/* We set the Update Rate Interval to be 1 second for Interactive Provider application, which 
 * is the Update Interval the provider application sends the Update Message content to client
 */
#define UPDATE_INTERVAL 1

/* EnumType Dictionary now supports fragmenting at a message level -
 * However, some EnumType Dictionary message can be still very large, up to 10K
 */
#define MAX_ENUM_TYPE_DICTIONARY_MSG_SIZE 12800

#ifdef _WIN32
#ifdef _WIN64
#define SOCKET_PRINT_TYPE "%llu"	/* WIN64 */
#else
#define SOCKET_PRINT_TYPE "%u"	/* WIN32 */
#endif
#else
#define SOCKET_PRINT_TYPE "%d"  /* Linux */
#endif

/* ping management information */
typedef struct {
	RsslUInt32	pingTimeoutClient; /* client ping timeout */
	RsslUInt32	pingTimeoutServer; /* server ping timeout */
	time_t		nextReceivePingTime; /* time server should receive next message/ping from client */
	time_t		nextSendPingTime; /* time to send next ping from server */
	time_t		currentTime;	/* current time from system clock */
	RsslBool	receivedClientMsg; /* flag for client message received */
} UpaPingManagementInfo;

/* login request information */
typedef struct {
	RsslInt32	StreamId;
	char		Username[128];
	char		ApplicationId[128];
	char		ApplicationName[128];
	char		Position[128];
	char		Password[128];
	char		InstanceId[128];
	RsslUInt64	Role;
	RsslBool	IsInUse;
} UpaLoginRequestInfo;

/* source directory request information */
typedef struct {
	RsslInt32	StreamId;
	char		ServiceName[128];  /* service name requested by application */
	RsslUInt64	ServiceId; /* service id associated with the service name requested by application */
	RsslBool	IsInUse;
} UpaSourceDirectoryRequestInfo;

/* dictionary request information */
typedef struct {
	RsslInt32	StreamId;
	char		DictionaryName[128];
	RsslMsgKey	MsgKey;
	RsslBool	IsInUse;
} UpaDictionaryRequestInfo;

/* market price item data */
typedef struct {
	RsslUInt64		RDNDISPLAY;
	RsslEnum		RDN_EXCHID;
	RsslDate		DIVPAYDATE;
	double			TRDPRC_1;
	double			BID;
	double			ASK;
	double			ACVOL_1;
	double			NETCHNG_1;
	RsslDateTime	ASK_TIME;
} UpaMarketPriceItem;

/* market price item request information. */
typedef struct {
	RsslInt32		StreamId;
	RsslBool		IsRefreshComplete;
	char			ItemName[128];
	/* Holds information about the item's data. This data will be different depending on the domain of the item. 
	 * In this simple Interactive Provider example, we only show Market Price domain. */
	void*			ItemData;
	RsslMsgKey		MsgKey;
	RsslBool		IsInUse;
} UpaMarketPriceItemRequestInfo;

/* channel management information */
typedef struct {
	RsslChannel* upaChannel;
	RsslChannelInfo upaChannelInfo; /* UPA Channel Info returned by rsslGetChannelInfo call */
	UpaPingManagementInfo pingManagementInfo;
	UpaLoginRequestInfo loginRequestInfo;
	UpaSourceDirectoryRequestInfo sourceDirectoryRequestInfo;
	UpaDictionaryRequestInfo fieldDictionaryRequestInfo;
	UpaDictionaryRequestInfo enumTypeDictionaryRequestInfo;
	UpaMarketPriceItemRequestInfo marketPriceItemRequestInfo;
	/* upaProvider QoS */
	RsslQos upaProviderQos;
} UpaChannelManagementInfo;

/* reasons a login request is rejected */
typedef enum {
	MAX_LOGIN_REQUESTS_REACHED	= 0,
	NO_USER_NAME_IN_REQUEST		= 1
} UpaLoginRejectReason;

/* reasons a source directory request is rejected */
typedef enum {
	MAX_SRCDIR_REQUESTS_REACHED	= 0,
	INCORRECT_FILTER_FLAGS		= 1
} UpaSourceDirectoryRejectReason;

/* dictionary type supported by UPA applications */
typedef enum {
	DICTIONARY_FIELD_DICTIONARY	= 0,
	DICTIONARY_ENUM_TYPE		= 1
} UpaDictionaryType;

/* reasons a dictionary request is rejected */
typedef enum {
	UNKNOWN_DICTIONARY_NAME			= 0,
	MAX_DICTIONARY_REQUESTS_REACHED	= 1
} UpaDictionaryRejectReason;

/* reasons an item request is rejected */
typedef enum {
	ITEM_REJECT_NONE				= 0,
	ITEM_COUNT_REACHED				= 1,
	INVALID_SERVICE_ID				= 2,
	ITEM_ALREADY_OPENED				= 3,
	STREAM_ALREADY_IN_USE			= 4,
	QOS_NOT_SUPPORTED				= 5,
	KEY_ENC_ATTRIB_NOT_SUPPORTED	= 6,
	PRIVATE_STREAM_REDIRECT			= 7,
	PRIVATE_STREAM_MISMATCH			= 8,
	BATCH_ITEM_REISSUE				= 9,
	ITEM_NOT_SUPPORTED              = 10,
	REQUEST_DECODE_ERROR			= 11
} UpaItemRejectReason;

/*
 * Closes channel, closes server, cleans up and exits the application.
 * upaChannel - The channel to be closed
 * upaSrvr - The RsslServer that represents the listening socket connection to the user to be closed
 * code - if exit due to errors/exceptions
 * dataDictionary -  the dictionaries that need to be unloaded to clean up memory
 */
void closeChannelServerCleanUpAndExit(RsslChannel* upaChannel, RsslServer* upaSrvr, int code, RsslDataDictionary* dataDictionary);

/* 
 * Initializes the ping times for upaChannelManagementInfo.upaChannel. 
 * upaChannelInfo - The channel management information including the ping management information
 */
void initPingManagementHandler(UpaChannelManagementInfo *upaChannelManagementInfo);

/* 
 * Processing ping management handler for upaChannelManagementInfo.upaChannel. 
 * upaChannelInfo - The channel management information including the ping management information
 */
RsslRet processPingManagementHandler(UpaChannelManagementInfo *upaChannelManagementInfo);

/*
 * Sends a message buffer to a channel.  
 * upaChannel - The channel to send the message buffer to
 * msgBuf - The msgBuf to be sent
 */
RsslRet sendMessage(RsslChannel* upaChannel, RsslBuffer* msgBuf);

/*
 * Processes a login request. This consists of decoding the login request and calling
 * sendLoginResponse() to send the login response.
 * upaChannelInfo - The channel management information including the login request information
 * msg - The partially decoded message
 * decodeIter - The decode iterator
 */
RsslRet processLoginRequest(UpaChannelManagementInfo *upaChannelManagementInfo, RsslMsg* msg, RsslDecodeIterator* decodeIter);

/*
 * Sends a Login refresh response to a channel. This consists of getting a message buffer, setting the login response information, 
 * encoding the login response, and sending the login response to the client. If the Interactive Provider grants access, it should 
 * send an RsslRefreshMsg to convey that the user successfully connected. This message should indicate the feature set supported by 
 * the provider application.
 * upaChannelInfo - The channel management information including the login request information and 
 * including the channel to send a login refresh response to
 */
RsslRet sendLoginResponse(UpaChannelManagementInfo *upaChannelManagementInfo);

/*
 * Sends the login request reject status message for a channel.
 * upaChannelInfo - The channel management information including the login request information and 
 * including the channel to send the login request reject status message to
 * streamId - The stream id of the login request reject status
 * reason - The reason for the reject
 */
RsslRet sendLoginRequestRejectStatusMsg(UpaChannelManagementInfo *upaChannelManagementInfo, RsslInt32 streamId, UpaLoginRejectReason reason);

/* 
 * Closes a login stream. 
 * streamId - The stream id to close the login for
 * upaChannelInfo - The channel management information including the login request information
 */
void closeLoginStream(RsslInt32 streamId, UpaChannelManagementInfo *upaChannelManagementInfo);

/*
 * Clears the login request information.
 * loginRequestInfo - The login request information to be cleared
 */
RTR_C_INLINE void clearLoginReqInfo(UpaLoginRequestInfo* loginRequestInfo)
{
	loginRequestInfo->StreamId = 0;
	loginRequestInfo->Username[0] = '\0';
	loginRequestInfo->ApplicationId[0] = '\0';
	loginRequestInfo->ApplicationName[0] = '\0';
	loginRequestInfo->Position[0] = '\0';
	loginRequestInfo->InstanceId[0] = '\0';
	loginRequestInfo->Password[0] = '\0';
	loginRequestInfo->Role = 0;
	loginRequestInfo->IsInUse = RSSL_FALSE;
}

/*
 * Processes a source directory request. This consists of decoding the source directory request and calling
 * sendSourceDirectoryResponse() to send the source directory response.
 * upaChannelInfo - The channel management information including the source directory request information
 * msg - The partially decoded message
 * decodeIter - The decode iterator
 */
RsslRet processSourceDirectoryRequest(UpaChannelManagementInfo *upaChannelManagementInfo, RsslMsg* msg, RsslDecodeIterator* decodeIter);

/* 
 * Send Source Directory response to a channel. This consists of getting a message buffer, setting the source directory 
 * response information, encoding the source directory response, and sending the source directory response to 
 * the consumer. The Source Directory domain model conveys information about all available services in the system. 
 * An OMM consumer typically requests a Source Directory to retrieve information about available services and their capabilities. 
 * upaChannelInfo - The channel management information including the source directory request information
 * serviceName - The service name specified by the OMM interactive provider application (Optional to set) 
 * serviceId - the serviceId specified by the OMM interactive provider  application (Optional to set) 
 */
RsslRet sendSourceDirectoryResponse(UpaChannelManagementInfo *upaChannelManagementInfo, char serviceName[128], RsslUInt64 serviceId);

/*
 * Sends the source directory request reject status message for a channel.
 * upaChannelInfo - The channel management information including the source directory request information and 
 * including the channel to send the source directory request reject status message to
 * streamId - The stream id of the source directory request reject status
 * reason - The reason for the reject
 */
RsslRet sendSrcDirectoryRequestRejectStatusMsg(UpaChannelManagementInfo *upaChannelManagementInfo, RsslInt32 streamId, UpaSourceDirectoryRejectReason reason);

/* 
 * Closes a source directory stream. 
 * streamId - The stream id to close the source directory for
 * upaChannelInfo - The channel management information including the source directory request information
 */
void closeSourceDirectoryStream(RsslInt32 streamId, UpaChannelManagementInfo *upaChannelManagementInfo);

/*
 * Clears the source directory request information.
 * srcDirReqInfo - The source directory request information to be cleared
 */
RTR_C_INLINE void clearSourceDirectoryReqInfo(UpaSourceDirectoryRequestInfo* srcDirReqInfo)
{
	srcDirReqInfo->StreamId = 0;
	srcDirReqInfo->ServiceName[0] = '\0';
	srcDirReqInfo->ServiceId = 0;
	srcDirReqInfo->IsInUse = RSSL_FALSE;
}

/*
 * Processes a dictionary request. This consists of decoding the dictionary request and calling the corresponding flavors
 * of the sendDictionaryResponse() functions to send the dictionary response.
 * upaChannelInfo - The channel management information including the dictionary request information
 * msg - The partially decoded message
 * decodeIter - The decode iterator
 * dataDictionary - The dictionary to encode field information or enumerated type information from
 */
RsslRet processDictionaryRequest(UpaChannelManagementInfo *upaChannelManagementInfo, RsslMsg* msg, RsslDecodeIterator* decodeIter, RsslDataDictionary* dataDictionary);

/*
 * Sends the field dictionary or enumType dictionary response to a channel. This consists of getting a message buffer, 
 * encoding the field dictionary or enumType dictionary response, and sending the field dictionary or enumType dictionary response to the server. 
 * upaChannelInfo - The channel management information including the dictionary request information and 
 * including the channel to send the field dictionary or enumType dictionary response to
 * dataDictionary - The dictionary to encode field information or enumerated type information from
 */
RsslRet sendDictionaryResponse(UpaChannelManagementInfo *upaChannelManagementInfo, RsslDataDictionary* dataDictionary, UpaDictionaryType dictionaryType);

/*
 * Sends the dictionary close status message for a channel.
 * upaChannelInfo - The channel management information including the dictionary request information and 
 * including the channel to send the dictionary close status message to
 */
RsslRet sendDictionaryCloseStatusMsg(UpaChannelManagementInfo *upaChannelManagementInfo);

/*
 * Sends the dictionary request reject status message for a channel.
 * upaChannelInfo - The channel management information including the dictionary request information and 
 * including the channel to send the dictionary request reject status message to
 * streamId - The stream id of the dictionary request reject status
 * reason - The reason for the reject
 */
RsslRet sendDictionaryRequestRejectStatusMsg(UpaChannelManagementInfo *upaChannelManagementInfo, RsslInt32 streamId, UpaDictionaryRejectReason reason);

/* 
 * Closes a dictionary stream. 
 * streamId - The stream id to close the dictionary for
 * upaChannelInfo - The channel management information including the dictionary request information
 */
void closeDictionaryStream(RsslInt32 streamId, UpaChannelManagementInfo *upaChannelManagementInfo);

/*
 * Clears the dictionary request information.
 * srcDirReqInfo - The dictionary request information to be cleared
 */
RTR_C_INLINE void clearDictionaryReqInfo(UpaDictionaryRequestInfo* dictionaryReqInfo)
{
	dictionaryReqInfo->StreamId = 0;
	dictionaryReqInfo->DictionaryName[0] = '\0';
	rsslClearMsgKey(&dictionaryReqInfo->MsgKey); /* Clears an Rssl message key */
	dictionaryReqInfo->MsgKey.name.data = dictionaryReqInfo->DictionaryName;
	dictionaryReqInfo->MsgKey.name.length = 128;
	/*!< (0x07) "Normal" Verbosity, e.g. all but description */
	dictionaryReqInfo->MsgKey.filter = RDM_DICTIONARY_NORMAL;
	dictionaryReqInfo->IsInUse = RSSL_FALSE;
}

/*
 * Processes a market price item request. This consists of storing the market price item request information, 
 * decoding the market price item request and then calling sendMarketPriceItemResponse() function to send the market price item response.
 * upaChannelInfo - The channel management information including the market price item request information
 * msg - The partially decoded message
 * decodeIter - The decode iterator
 * dataDictionary - The dictionary used for encoding
 */
RsslRet processMarketPriceItemRequest(UpaChannelManagementInfo *upaChannelManagementInfo, RsslMsg* msg, RsslDecodeIterator* decodeIter, RsslDataDictionary* dataDictionary);

/* 
 * Send just 1 Market Price item response message to a channel. This consists of getting a message buffer, encoding the 
 * Market Price item response, and sending the item response to the channel. 
 * upaChannelInfo - The channel management information including the market price item request information and 
 * including the channel to send the Market Price item response message buffer to
 * dataDictionary - The dictionary used for encoding
 */

RsslRet sendMarketPriceItemResponse(UpaChannelManagementInfo *upaChannelManagementInfo, RsslDataDictionary* dataDictionary);

/*
 * Sends the market price item close status message for a channel.
 * upaChannelInfo - The channel management information including the Market Price item request information and 
 * including the channel to send the Market Price item close status message to
 */
RsslRet sendMarketPriceItemCloseStatusMsg(UpaChannelManagementInfo *upaChannelManagementInfo);

/*
 * Sends the market price item request reject status message for a channel.
 * upaChannelInfo - The channel management information including the market price item request information and 
 * including the channel to send the market price item request reject status message to
 * streamId - The stream id of the market price item request reject status
 * reason - The reason for the reject
 */
RsslRet sendMarketPriceItemRequestRejectStatusMsg(UpaChannelManagementInfo *upaChannelManagementInfo, RsslInt32 streamId, UpaItemRejectReason reason);

/* 
 * Closes a market price item stream. 
 * streamId - The stream id to close the market price item for
 * upaChannelInfo - The channel management information including the market price item request information
 */
void closeMarketPriceItemStream(RsslInt32 streamId, UpaChannelManagementInfo *upaChannelManagementInfo);

/*
 * Clears the market price item request information.
 * marketPriceItemReqInfo - The market price item request information to be cleared
 */
RTR_C_INLINE void clearMarketPriceItemReqInfo(UpaMarketPriceItemRequestInfo* marketPriceItemReqInfo)
{
	marketPriceItemReqInfo->StreamId = 0;
	marketPriceItemReqInfo->ItemName[0] = '\0';
	marketPriceItemReqInfo->IsInUse = RSSL_FALSE;
	marketPriceItemReqInfo->IsRefreshComplete = RSSL_FALSE;
	marketPriceItemReqInfo->ItemData = 0;
	rsslClearMsgKey(&marketPriceItemReqInfo->MsgKey); /* Clears an Rssl message key */
	marketPriceItemReqInfo->MsgKey.name.data = marketPriceItemReqInfo->ItemName;
	marketPriceItemReqInfo->MsgKey.name.length = 128;
}

/* 
 * upaGetBuffer() is the utility function that does 2-pass (more robust) getting non-packable buffer.
 * Also, it simplies the example codes and make the codes more readable.
 */
RsslBuffer* upaGetBuffer(RsslChannel *upaChannel, RsslUInt32 size, RsslError *rsslError);

#ifdef __cplusplus
};
#endif

#endif

