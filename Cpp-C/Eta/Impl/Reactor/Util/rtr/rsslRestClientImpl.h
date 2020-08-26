/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
*/

#ifndef __RSSL_REST_CLIENT_IMPL
#define __RSSL_REST_CLIENT_IMPL

#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslTypes.h"
#include "rtr/rsslTransport.h"
#include "rtr/rsslDataPackage.h"
#include "rtr/rsslQueue.h"

#ifdef WIN32
	#define	RSSL_REST_INVALID_SOCKET (~0)
#else
	#define RSSL_REST_INVALID_SOCKET -1
#endif

 /**
 *	@addtogroup RSSL REST CLIENT used internally by the Reactor and EMA C++
 *	@{
 */

extern RsslBuffer rssl_rest_service_discovery_token_scope;
extern RsslBuffer rssl_rest_authorization_text;
extern RsslBuffer rssl_rest_accept_text;
extern RsslBuffer rssl_rest_application_json_text;
extern RsslBuffer rssl_rest_transport_type_tcp_text;
extern RsslBuffer rssl_rest_transport_type_websocket_text;
extern RsslBuffer rssl_rest_dataformat_type_rwf_text;
extern RsslBuffer rssl_rest_dataformat_type_tr_json2_text;
extern RsslBuffer rssl_rest_grant_type_refresh_token_text;
extern RsslBuffer rssl_rest_grant_type_password_text;
extern RsslBuffer rssl_rest_username_text;
extern RsslBuffer rssl_rest_password_text;
extern RsslBuffer rssl_rest_new_password_text;
extern RsslBuffer rssl_rest_client_id_text;
extern RsslBuffer rssl_rest_client_secret_text;
extern RsslBuffer rssl_rest_refresh_token_text;
extern RsslBuffer rssl_rest_scope_text;
extern RsslBuffer rssl_rest_take_exclusive_sign_on_false_text;
extern RsslBuffer rssl_rest_take_exclusive_sign_on_true_text;
extern RsslBuffer rssl_rest_content_type_text;
extern RsslBuffer rssl_rest_application_form_urlencoded_text;
extern RsslBuffer rssl_rest_location_header_text;

#define RSSL_REST_MAX_WRITE_BUF_SIZE 16384
#define RSSL_REST_INIT_TOKEN_BUFFER_SIZE 8192
#define RSSL_REST_INIT_SVC_DIS_BUF_SIZE 9216
#define RSSL_REST_ADDITIONAL_REQ_AUTH_LENGTH 95 /* Support for both password and refresh_token grant types*/
#define RSSL_REST_STORE_HOST_AND_PORT_BUF_SIZE 128 /* Enough size to store max domain name(63) and port(5)*/
    
#ifdef __cplusplus
extern "C" {
#endif

/**
* @brief RsslRestProxyArgs provides users to specify proxy arguments
*/
typedef struct {
	RsslBuffer				proxyHostName;  /*!< specifies proxy hostname. */
	RsslBuffer				proxyPort;      /*!< specifies proxy port. */
	RsslBuffer				proxyUserName;  /*!< specifies username to perform authorization with a proxy server. */
	RsslBuffer				proxyPassword;  /*!< specifies password to perform authorization with a proxy server. */
	RsslBuffer				proxyDomain;	/*!< specifies proxy domain of the user to authenticate.
								Needed for NTLM or for Negotiate/Kerberos or for Kerberos authentication protocols. */
} RsslRestProxyArgs;

#define RSSL_INIT_REST_PROXY_ARGS { {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} }

/**
* @brief Clears the RsslRestProxyArgs structure passed in
* @param rsslRestProxyArgs a pointer to the RsslRestProxyArgs structure to be cleared
*/
RTR_C_INLINE void rsslClearRestProxyArgs(RsslRestProxyArgs *rsslRestProxyArgs)
{
	memset(rsslRestProxyArgs, 0, sizeof(RsslRestProxyArgs));
}

/**
* @brief RsslRestNetworkArgs provides users to specify network arguments
*/
typedef struct {
	RsslBuffer				interfaceName;      /*!< specifies source interface for outgoing traffic. */
	RsslBool				tcpNoDelay;			/*!< specifies true to set the TCP_NODELAY option. */
	RsslRestProxyArgs		proxyArgs;			/*!< specifies proxy arguments if any. */
	RsslBuffer				openSSLCAStore;		/*!<specifies a path to the CAStore file for OpenSSL. */
} RsslRestNetworkArgs;

#define RSSL_INIT_REST_NETWORK_ARGS { {0, 0} , 1, RSSL_INIT_REST_PROXY_ARGS, {0, 0} };

/**
* @brief Clears the RsslRestNetworkArgs structure passed in
* @param rsslRestNetworkArgs a pointer to the RsslRestNetworkArgs structure to be cleared
*/
RTR_C_INLINE void rsslClearRestNetworkArgs(RsslRestNetworkArgs *rsslRestNetworkArgs)
{
	rsslClearBuffer(&rsslRestNetworkArgs->interfaceName);
	rsslRestNetworkArgs->tcpNoDelay = 1;
	rsslClearRestProxyArgs(&rsslRestNetworkArgs->proxyArgs);
	rsslClearBuffer(&rsslRestNetworkArgs->openSSLCAStore);
}

/**
* @brief RsslRestHandle represents a handle for non-blocking requests
*/
typedef struct {
	RsslQueueLink		queueLink;	/*!< Link is used to add to RsslQueue for cleaning outside callback methods.*/
	RsslSocket			socketId;	/*!< @brief Socket ID of this REST handle.*/
} RsslRestHandle;

/**
* @brief Clears the RsslRestNetworkArgs structure passed in
* @param rsslRestNetworkArgs a pointer to the RsslRestNetworkArgs structure to be cleared
*/
RTR_C_INLINE void rsslClearRestHandle(RsslRestHandle *rsslRestHandle)
{
	rsslInitQueueLink(&rsslRestHandle->queueLink);
	rsslRestHandle->socketId = RSSL_REST_INVALID_SOCKET;
}
/**
* @brief RsslRestHeader represents an individual header information
*/
typedef struct {
	RsslQueueLink	queueLink;		/*!<Link for the httpHeaders queue. */
	RsslBuffer		key;			/*!<specifies a header key. */
	RsslBuffer		value;			/*!<specifies a header value. */
} RsslRestHeader;

/**
* @brief Clears the RsslRestHeader structure passed in
* @param rsslRestHeader a pointer to the RsslRestHeader structure to be cleared
*/
RTR_C_INLINE void rsslClearRestHeader(RsslRestHeader *rsslRestHeader)
{
	rsslInitQueueLink(&rsslRestHeader->queueLink);
	rsslClearBuffer(&rsslRestHeader->key);
	rsslClearBuffer(&rsslRestHeader->value);
}

/**
* @brief RSSL HTTP Methods
*/
typedef enum {
	RSSL_REST_HTTP_UNKNOWN = 0,	/*!< (0) Indicates UNKNOW HTTP method. */
	RSSL_REST_HTTP_GET = 1,		/*!< (1) Indicates HTTP GET method. */
	RSSL_REST_HTTP_POST = 2,	/*!< (2) Indicates HTTP POST method. */
	RSSL_REST_HTTP_PUT = 3,		/*!< (3) Indicates HTTP PUT method. */
	RSSL_REST_HTTP_DELETE = 4,	/*!< (4) Indicates HTTP DELETE method. */
	RSSL_REST_HTTP_PATCH = 5	/*!< (5) Indicates HTTP PATCH method. */
} RsslRestHttpMethod;

/**
* @brief RsslRestRequestArgs represents request arguments for a REST request.
*/
typedef struct {
	RsslUInt16				httpMethod;			/*!<specifies HTTP method for the REST request defined in RsslRestHttpMethod. */
	RsslBuffer				url;				/*!<specifies URL for this requires using the <scheme>://<host>:<port> format. */
	RsslQueue				httpHeaders;		/*!<specifies the list of RsslRestHeader for specifying header attributes if any. */
	RsslBuffer				httpBody;			/*!<specifies the body buffer. */
	void*					pUserSpecPtr;		/*!<a user specified pointer, possibly a closure. */
	RsslRestNetworkArgs		networkArgs;		/*!<specifies network arguments if any. */
	RsslUInt32				requestTimeOut;		/*!<specifies maximum time the request is allowed to take, in seconds. */
} RsslRestRequestArgs;

/**
* @brief Clears the RsslRestRequestArgs structure passed in
* @param rsslRestRequestArgs a pointer to the RsslRestRequestArgs structure to be cleared
*/
RTR_C_INLINE void rsslClearRestRequestArgs(RsslRestRequestArgs *rsslRestRequestArgs)
{
	rsslRestRequestArgs->httpMethod = RSSL_REST_HTTP_UNKNOWN;
	rsslClearBuffer(&rsslRestRequestArgs->url);
	rsslInitQueue(&rsslRestRequestArgs->httpHeaders);
	rsslClearBuffer(&rsslRestRequestArgs->httpBody);
	rsslRestRequestArgs->pUserSpecPtr = 0;
	rsslClearRestNetworkArgs(&rsslRestRequestArgs->networkArgs);
	rsslRestRequestArgs->requestTimeOut = 0; /* Never timeout during transfer and waiting for a response */
}

/**
* @brief RsslTokenInformation represents an token information from EDP token service.
*/
typedef struct {
	RsslBuffer		accessToken;		/*!<represents access token is used to invoke REST data API calls. */
	RsslBuffer		refreshToken;		/*!<represents refresh token is used for getting next access token. */
	RsslInt			expiresIn;			/*!<represents access token validity time in seconds. */
	RsslBuffer		tokenType;			/*!<represents a list of all the scopes this token can be used with. */
	RsslBuffer		scope;				/*!<represents a token type for specifying in the Authorization header */
} RsslTokenInformation;

/**
* @brief Clears the RsslTokenInformation structure passed in
* @param rsslTokenInformation a pointer to the RsslTokenInformation structure to be cleared
*/
RTR_C_INLINE void rsslClearTokenInformation(RsslTokenInformation *rsslTokenInformation)
{
	rsslClearBuffer(&rsslTokenInformation->accessToken);
	rsslClearBuffer(&rsslTokenInformation->refreshToken);
	rsslTokenInformation->expiresIn = 0;
	rsslClearBuffer(&rsslTokenInformation->tokenType);
	rsslClearBuffer(&rsslTokenInformation->scope);
}

/**
* @brief RsslRestResponse represents an REST response
*/
typedef struct {
	RsslUInt32		statusCode;			/*!<represents HTTP status code for the request. */
	RsslQueue		headers;			/*!<represents a list of avaliable RsslHeader. */
	RsslQueue		cookies;			/*!<represents a list of avaliable cookie. This feature is not avaliable. */
	RsslBuffer		dataBody;			/*!<represents the data body portion of message. */
	RsslBuffer		protocolVersion;	/*!<represents the protocol version. */
	RsslBool		isMemReallocated;	/*!<indicates users that the RsslRestClient creates memory to hold the entire response. */
	RsslBuffer		reallocatedMem;		/*!<indicates reallocated memory which users is responsible to free the memory if any. */
} RsslRestResponse;

/**
* @brief Clears the RsslRestResponse structure passed in
* @param rsslRestResponse a pointer to the RsslRestResponse structure to be cleared
*/
RTR_C_INLINE void rsslClearRestResponse(RsslRestResponse *rsslRestResponse)
{
	rsslRestResponse->isMemReallocated = RSSL_FALSE;
	rsslRestResponse->statusCode = 0;
	rsslInitQueue(&rsslRestResponse->headers);
	rsslInitQueue(&rsslRestResponse->cookies);
	rsslClearBuffer(&rsslRestResponse->dataBody);
	rsslClearBuffer(&rsslRestResponse->protocolVersion);
	rsslClearBuffer(&rsslRestResponse->reallocatedMem);
}

/**
* @brief RsslRestResponseEvent represents a REST response event
*/
typedef struct {
	RsslRestHandle*		handle;				/*!<represents the handle for this request. */
	void*				closure;			/*!<represents the user's closure for this request if specified. */
	RsslBuffer*			userMemory;			/*!<represents the specified user's memory for this request. */
}RsslRestResponseEvent;

/**
* @brief Clears the RsslRestResponseEvent structure passed in
* @param rsslRestResponseEvent a pointer to the RsslRestResponseEvent structure to be cleared
*/
RTR_C_INLINE void rsslClearRestResponseEvent(RsslRestResponseEvent *rsslRestResponseEvent)
{
	memset(rsslRestResponseEvent, 0, sizeof(RsslRestResponseEvent));
}

/**
* @brief RsslRestClient represents a REST client
*/
typedef struct {
	void		*userSpecPtr;	/*!< A user-specified pointer associated with this RsslRestClient. */
	RsslSocket	eventFd;		/*!< A descriptor that provides notification for events avaliable to be processed by calling rsslRestClientDispatch(). */
}RsslRestClient;

/**
* @brief Configuraion options for creating an RsslRestClient.
* @see RsslRestClient
*/
typedef struct {
	void		*userSpecPtr; 			/*!< user-specified pointer which will be set on the RsslRestClient. */
	RsslUInt32	requestCountHint;		/*!< an estimation number of REST requests the RsslRest expects to maintain. */
	RsslBool	dynamicBufferSize;		/*!< dynamic reallocate the memory buffer if the allocated size is not sufficient to copy the response. */
	RsslUInt32	numberOfBuffers;		/*!< number of the buffer pool for handling HTTP headers. This is used when the dynamicBufferSize is enable.*/
} RsslCreateRestClientOptions;

/**
* @brief Clears the RsslCreateRestClientOptions structure passed in
* @param rsslCreateRestClientOptions a pointer to the RsslCreateRestClientOptions structure to be cleared
*/
RTR_C_INLINE void rsslClearRestClientOptions(RsslCreateRestClientOptions *rsslCreateRestClientOptions)
{
	rsslCreateRestClientOptions->userSpecPtr = 0;
	rsslCreateRestClientOptions->requestCountHint = 5000;
	rsslCreateRestClientOptions->dynamicBufferSize = RSSL_FALSE;
	rsslCreateRestClientOptions->numberOfBuffers = 30;
}

/**
* @brief RsslRestServiceEndpointInfo represents a service endpoint
*/
typedef struct {
	RsslBuffer      *dataFormatList; /*!< A list of data formats. The list indicates the data format used by transport.*/
	RsslUInt32      dataFormatCount; /*!< The number of data formats in dataFormatList. */
	RsslBuffer      endPoint;        /*!< A domain name of the service access endpoint. */
	RsslBuffer      *locationList;   /*!< A list of locations. The list indicates the location of the service. */
	RsslUInt32      locationCount;   /*!< The number of locations in locationList. */
	RsslBuffer      port;            /*!< A port number used to establish connection. */
	RsslBuffer      provider;        /*!< A public Refinitiv Real-Time Optimized provider. */
	RsslBuffer      transport;       /*!< A transport type used to access service. */
} RsslRestServiceEndpointInfo;

/**
* @brief Clears the RsslRestServiceEndpointInfo structure passed in
* @param pRsslRestServiceEndpointInfo a pointer to the RsslRestServiceEndpointInfo structure to be cleared
*/
RTR_C_INLINE void rsslClearRestServiceEndpointInfo(RsslRestServiceEndpointInfo *pRsslRestServiceEndpointInfo)
{
	memset(pRsslRestServiceEndpointInfo, 0, sizeof(RsslRestServiceEndpointInfo));
}

/**
 * @brief A service endpoint information response.
 * @see RsslRestServiceEndpointInfo
 */
typedef struct
{
	RsslRestServiceEndpointInfo    *serviceEndpointInfoList; /*!< The list of service endpoints associated with this event. */
	RsslUInt32                     serviceEndpointInfoCount; /*!< The number of service endpoint information in serviceEndpointInfoList. */
} RsslRestServiceEndpointResp;

/**
 * @brief Clears an RsslRestServiceEndpointResp structure passed in.
 * @see RsslRestServiceEndpointResp
 */
RTR_C_INLINE void rsslClearRestServiceEndpointResp(RsslRestServiceEndpointResp *pRsslRestServiceEndpointResp)
{
	memset(pRsslRestServiceEndpointResp, 0, sizeof(RsslRestServiceEndpointResp));
}

/**
*	@addtogroup REST Callback methods
*	@{
*/

/**
* @brief Callback signature for handling REST responses for the non-blocking requests.
* @see RsslRestResponse, RsslRestResponseEvent
*/
typedef void RsslRestResponseCallback(RsslRestResponse*, RsslRestResponseEvent*);

/**
* @brief Callback signature for handling errors for the non-blocking requests
* @see RsslError, RsslRestResponseEvent
*/
typedef void RsslRestErrorCallback(RsslError*, RsslRestResponseEvent*);

/**
*@}
*/

/** 
* @brief Initializes the REST client. This function must be called before calling others function
* @param libcurlName specifies the shared library name of libcurl.
* @param pError Error structure to be populated in the event of an error.
* @return RSSL_RET_SUCCESS if perform successfully otherwise the error codes.
* @see pRsslRestClient, RsslError
*/
RsslRet rsslRestClientInitialize(char* libcurlName, RsslError *error);

/**
* @brief Uninitializes the REST client. This function must be called when no logger uses the REST client.
* @param pError Error structure to be populated in the event of an error.
* @return RSSL_RET_SUCCESS if perform successfully otherwise the error codes.
* @see pRsslRestClient, RsslError
*/
RsslRet rsslRestClientUninitialize(RsslError *error);

/**
* @brief Creates an RsslRestClient to request non-blocking requests and dispatch messages
* @param pRestClientOpts Configuration options for creating the RsslRestClient.
* @param pError Error structure to be populated in the event of an error.
* @return Pointer to the newly created RsslRestClient. If the pointer is NULL, an error occurred.
* @see RsslCreateRestClientOptions, RsslError
*/
RsslRestClient* rsslCreateRestClient(RsslCreateRestClientOptions *pRestClientOpts, RsslError *pError);

/**
* @brief Cleans up an RsslRestClient. 
* @param pRsslRestClient specifies RsslRestClient to cleanup.
* @param pError Error structure to be populated in the event of an error.
* @return RSSL_RET_SUCCESS if perform successfully otherwise the error codes.
* @see pRsslRestClient, RsslError
*/
RsslRet rsslDestroyRestClient(RsslRestClient *pRsslRestClient, RsslError *pError);

/**
* @brief Relinquish application thread of control to receive callbacks.
* @param pRsslRestClient specifies RsslRestClient to dispatch event from.
* @return 1 if there is more event to dispatch otherwise 0 to indicate there is more event.
* @see RsslRestClient
*/
RsslInt64 rsslRestClientDispatch(RsslRestClient* RsslRestClient);

/**
* @brief Opens a blocking REST request. This method returns after it gets a response or a error occurs.
* @param restClient specifies a RsslRestClient to use user defined configuration.
* @param restRequestArgs specifies request arguments.
* @param restResponse specifies RsslRestResponse for populating response by the REST client.
* @param memorybuffer specifies a buffer for storing response headers and data body
* @param pError Error structure to be populated in the event of an error.
* @return RSSL_RET_SUCCESS if perform successfully otherwise the error codes.
* @see RsslRestRequestArgs, RsslRestResponse
*/
RsslRet rsslRestClientBlockingRequest(RsslRestClient* restClient, RsslRestRequestArgs* restRequestArgs,
	RsslRestResponse* restResponse,
	RsslBuffer*	memorybuffer,
	RsslError *error);

/**
* @brief Opens a non-blocking REST request. The application will responses or errors via the specified callback methods.
* @param restClient specifies a RsslRestClient for dispatching events.
* @param restRequestArgs specifies request arguments.
* @param restResponseCallback specifies a callback function to handle responses.
* @param restErrorCallback specifies a callback function to handle errors.
* @param memorybuffer specifies a buffer for storing response headers and data body
* @param pError Error structure to be populated in the event of an error.
* @return RSSL_RET_SUCCESS if perform successfully otherwise the error codes.
* @see RsslRestClient, RsslRestRequestArgs, RsslRestResponseCallback, RsslRestErrorCallback
*/
RsslRestHandle* rsslRestClientNonBlockingRequest(RsslRestClient* restClient, RsslRestRequestArgs* requestArgs,
	RsslRestResponseCallback* restResponseCallback,
	RsslRestErrorCallback*    restErrorCallback,
	RsslBuffer*	memorybuffer,
	RsslError *error);

/**
* @brief Cleans up the RsslRestHandle.
* @param handle specifies the RsslRestHandle to cleanup all memory associated with it.
* @param pError Error structure to be populated in the event of an error.
* @return RSSL_RET_SUCCESS if perform successfully otherwise the error codes.
* @see RsslRestClient
*/
RsslRet rsslRestCloseHandle(RsslRestHandle* handle, RsslError* pError);

/**
* @brief Parses an endpoint from EDP-RT service discovery that provides auto failover mechanism.
* @param dataBody specifies a HTTP data body of HTTP response from the service
* @param location specifies a region to get an endpoint
* @param hostName specifies a buffer for this function to set hostname/ip address of the endpoint
* @param port specifies a buffer for this function to set port of the endpoint
* @param memorybuffer specifies a memory buffer for storing hostname and port.
* @param pError Error structure to be populated in the event of an error.
* @return RSSL_RET_SUCCESS if perform successfully otherwise the error codes.
*/
RsslRet rsslRestParseEndpoint(RsslBuffer* dataBody, RsslBuffer* location, RsslBuffer* hostName, RsslBuffer* port, RsslBuffer* memoryBuffer, RsslError* pError);

/**
* @brief Parses the entire response from EDP-RT service discovery.
* @param dataBody specifies a HTTP data body of HTTP response from the service
* @param pRestServiceEndpointResp specifies a RsslRestServiceEndpointResp structure to populate the response.
* @param memorybuffer specifies a memory buffer for storing service discovery response.
* @param pError Error structure to be populated in the event of an error.
* @return RSSL_RET_SUCCESS if perform successfully otherwise the error codes.
*/
RsslRet rsslRestParseServiceDiscoveryResp(RsslBuffer* dataBody, RsslRestServiceEndpointResp* pRestServiceEndpointResp, RsslBuffer* memoryBuffer, RsslError* pError);

/**
* @brief Parses an token information from EDP token service
* @param dataBody specifies a HTTP data body of HTTP response from the service
* @param accessToken specifies a buffer for this function to set the access token
* @param refreshToken specifies a buffer for this function to set the refresh token
* @param expiresIn specifies a buffer for this function to set the access token validity
* @param tokenType specifies a buffer for this function to set the tokey type
* @param scope specifies a buffer for this function to set a list of all the scopes this token can be used with
* @param memorybuffer specifies a memory buffer for storing token information
* @param pError Error structure to be populated in the event of an error.
* @return RSSL_RET_SUCCESS if perform successfully otherwise the error codes.
*/
RsslRet rsslRestParseAccessToken(RsslBuffer* dataBody, RsslBuffer *accessToken, RsslBuffer *refreshToken, RsslInt *expiresIn, 
									RsslBuffer *tokenType, RsslBuffer *scope, RsslBuffer* memoryBuffer, RsslError* pError);

/**
* @brief Converts the given input string to URL encoded string
* @param inputBuffer specifies a buffer to convert to URL encoded string
* @param pError Error structure to be populated in the event of an error.
* @return a new RsslBuffer if the convertion occurs otherwise returns the passed in buffer.
*/
RsslBuffer* rsslRestEncodeUrlData(RsslBuffer* inputBuffer, RsslError* pError);

/**
* @brief Print out the given input argument to the output stream
* @param outputStream stream where to print data.
* @param pRestRequest specifies REST request data
* @param pError Error structure to be populated in the event of an error.
* @return RSSL_RET_SUCCESS if perform successfully otherwise the error codes.
*/

RsslRet rsslRestRequestDump(FILE* outputStream,  RsslRestRequestArgs* pRestRequest,  RsslError* pError);

/**
* @brief Print out the given input argument to the output stream
* @param outputStream stream where to print data.
* @param pRestRequestResponse specifies REST response data
* @param pError Error structure to be populated in the event of an error.
* @return RSSL_RET_SUCCESS if perform successfully otherwise the error codes.
*/
RsslRet rsslRestResponseDump(FILE* outputStream,  RsslRestResponse* pRestRequestResponse,  RsslError* pError);

 /**
 *	@}
 */

#ifdef __cplusplus
};
#endif

#endif // __RSSL_REST_CLIENT_IMPL


