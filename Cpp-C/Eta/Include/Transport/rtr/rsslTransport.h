/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RTR_RSSL_TRANSPORT_H
#define __RTR_RSSL_TRANSPORT_H


#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslTypes.h"
#include "rtr/rsslRetCodes.h"

/**
 *	@addtogroup RSSLTransportStructs
 *	@{
 */
 
/**
 * @brief Socket Identifier Type
 */
#ifdef _WIN32
	#ifdef _WIN64
		typedef RsslUInt64 RsslSocket;
	#else
		typedef RsslUInt32 RsslSocket;
	#endif
#else
typedef RsslInt32 RsslSocket;
#endif

/**
 * @brief Maximum size flush strategy for RSSL
 * @see rsslWrite
 * @see rsslIoctl
 */
#define RSSL_RSSL_MAX_FLUSH_STRATEGY 32

/**
 * @brief RSSL Channel states
 */
typedef enum {
	RSSL_CH_STATE_CLOSED			= -1, /*!< (-1) Channel has been CLOSED. */
	RSSL_CH_STATE_INACTIVE			= 0,  /*!< (0) Channel is in the INACTIVE state. */
	RSSL_CH_STATE_INITIALIZING		= 1,  /*!< (1) Channel is in the INITIALIZING state. */
	RSSL_CH_STATE_ACTIVE			= 2   /*!< (2) Channel is in the ACTIVE state. */ 
} RsslChannelState;


/**
 * @brief RSSL Supported Compression types
 */
typedef enum {
	RSSL_COMP_NONE	= 0x00,  /*!< (0) No compression will be negotiated. */
	RSSL_COMP_ZLIB	= 0x01,	 /*!< (1) RSSL will attempt to use Zlib compression. */
	RSSL_COMP_LZ4	= 0x02	 /*!< (2) RSSL will attempt to use LZ4 compression */
} RsslCompTypes;

/**
 * @brief RSSL IOCtl codes
 * @see rsslIoctl
 * @see rsslServerIoctl
 */
typedef enum {
	RSSL_MAX_NUM_BUFFERS			= 1, /*!< (1) Channel: Used for changing the max number of buffers for this channel */
	RSSL_NUM_GUARANTEED_BUFFERS 	= 2, /*!< (2) Channel: Used for changing the number of guaranteed buffers for this channel */
	RSSL_HIGH_WATER_MARK			= 3, /*!< (3) Channel: Used to set the upper buffer usage threshold for this channel*/
	RSSL_SYSTEM_READ_BUFFERS		= 4, /*!< (4) Channel: Used to change the number of system read buffers (SO_RCVBUF) for this channel */
	RSSL_SYSTEM_WRITE_BUFFERS		= 5, /*!< (5) Channel: Used to change the number of system write buffers (SO_SNDBUF) for this channel */
	RSSL_DEBUG_FLAGS				= 6, /*!< (6) Channel: Used to turn on debug flags for this channel  */
	RSSL_PRIORITY_FLUSH_ORDER		= 7, /*!< (7) Channel: Used to set the priority flush order for this channel */
	RSSL_SERVER_NUM_POOL_BUFFERS	= 8, /*!< (8) Server: Used to increase or decrease the number of server shared pool buffers */
	RSSL_COMPRESSION_THRESHOLD		= 9, /*!< (9) Channel: When compression is on, this value is the smallest size packet that will be compressed (default is 30 bytes) */
	RSSL_SERVER_PEAK_BUF_RESET      = 10,/*!< (10) Server: Used to reset the peak number of server shared pool buffers */
	RSSL_TRACE			            = 11,/*!< (11) Channel: used to turn RWF message tracing on and off */
										 /*!< (12) Reserved */
										 /*!< (13) Reserved */
	RSSL_REGISTER_HASH_ID			= 14, /*!< (14) Channel: Used with ::RSSL_CONN_TYPE_RELIABLE_MCAST connections. Registers a hash so that a filtering-enabled channel allows it. */
	RSSL_UNREGISTER_HASH_ID			= 15 /*!< (15) Channel: Used with ::RSSL_CONN_TYPE_RELIABLE_MCAST connections. Unregisters a hash so that a filtering-enabled channel no longer allows it. */
} RsslIoctlCodes;

/**
 * @brief RSSL Connection types
 */
typedef enum {
	RSSL_CONN_TYPE_INIT				= -1, /*!< (-1) Channel is not connected */
	RSSL_CONN_TYPE_SOCKET			= 0,  /*!< (0) Channel is a standard TCP socket connection type */
	RSSL_CONN_TYPE_ENCRYPTED		= 1,  /*!< (1) Channel is encrypted */								
	RSSL_CONN_TYPE_HTTP				= 2,  /*!< (2) Channel is an HTTP connection based tunneling type */
	RSSL_CONN_TYPE_UNIDIR_SHMEM		= 3,  /*!< (3) Channel is using a shared memory connection */
	RSSL_CONN_TYPE_RELIABLE_MCAST	= 4,   /*!< (4) Channel is a reliable multicast based connection. This can be on a unified/mesh network where send and receive networks are the same or a segmented network where send and receive networks are different */
	RSSL_CONN_TYPE_EXT_LINE_SOCKET  = 5,   /*!< (5) Channel is using an extended line socket transport */	
	RSSL_CONN_TYPE_SEQ_MCAST		= 6,   /*!< (6) Channel is an unreliable, sequenced multicast connection for reading from an Elektron Direct Feed system. This is a client-only, read-only transport. This transport is supported on Linux only. */
	RSSL_CONN_TYPE_WEBSOCKET		= 7    /*!< (7) Channel is a WebSocket connection type. */
} RsslConnectionTypes;

/**
 *	@}
 */

/**
 * @brief Maximum RSSL Error text length
 * @see RsslError
 */
#define MAX_RSSL_ERROR_TEXT	1200

/**
 *	@addtogroup RSSLChannel RSSL Channel Operations
 *	@{
 */
 
 /**
 * @brief RSSL Channel structure returned via rsslAccept() or rsslConnect() calls.
 */
typedef struct {
	RsslSocket			socketId;		/*!< @brief Socket ID of this RSSL channel. */
	RsslSocket			oldSocketId;	/*!< @brief Old Socket Id of this rssl channel - used in rsslRead FD Change events */
	RsslChannelState	state;			/*!< @brief State of this RSSL channel. */
	RsslConnectionTypes connectionType; /*!< @brief Type that this connection is */
	char				*clientIP;		/*!< @brief When returned through rsslAccept, this contains the IP address of the connected client. */
	char				*clientHostname;/*!< @brief When returned through rsslAccept, this contains the hostname of the connected client. */
	RsslUInt32			pingTimeout;	/*!< @brief Contains the interval of time that a message or ping should be sent. */
	RsslUInt32			majorVersion;	/*!< @brief Contains the major version number of the encoder/decoder that should be used */
	RsslUInt32			minorVersion;	/*!< @brief Contains the minor version number of the encoder/decoder that should be used */
	RsslUInt32			protocolType;	/*!< @brief Contains the protocol type of the encoder/decoder that should be used */
	void				*userSpecPtr;	/*!< @brief A user specified pointer, possibly a closure. */
	char				*hostname;		/*!< @brief When returned through rsslConnect, this contains the hostname or IP address to which we connected */
	RsslUInt16			port;			/*!< @brief Contains the port number that was used to connect to the server (for Consumer, NiProvider). */
} RsslChannel;

/**
 *	@}
 */

/**
 *	@addtogroup RSSLTransportStructs
 *	@{
 */

/**
 * @brief RSSL Error message 
 * @see MAX_RSSL_ERROR_TEXT
 */
typedef struct {
	RsslChannel			*channel;					/*!< @brief The RSSL channel the error occurred on. */
	RsslRet				rsslErrorId;				/*!< @brief The RSSL Error value. */ 
	RsslUInt32			sysError;					/*!< @brief The system error number.  */ 
	char				text[MAX_RSSL_ERROR_TEXT+1];/*!< @brief Detailed text describing the error. */
} RsslError;


/**
 * @brief Gets the IP address of a hostname
 *
 * Typical use:<BR>
 * This function gets the IP address of a hostname in host byte order.
 *
 * @param hostName Hostname to get IP address for
 * @param ipAddr IP address of the hostname
 * @return RsslRet RSSL return value
 */
RSSL_API RsslRet rsslHostByName(RsslBuffer *hostName, RsslUInt32 *ipAddr);

/**
 * @brief Gets the user name
 *
 * Typical use:<BR>
 * This function gets the user name that the owner of the current process is
 * logged in under.  The returned user name is truncated if length of user name
 * from machine is greater than length of userName->length.
 *
 * @param userName User name of user
 * @return RsslRet RSSL return value
 */
RSSL_API RsslRet rsslGetUserName(RsslBuffer *userName);

 
/**
 *	@}
 */
 
/**
 *	@addtogroup RSSLInit
 *	@{
 */
 
/**
 * @brief Options of which locks are enabled in RSSL.
 * @see rsslInitialize
 */
typedef enum
{
	RSSL_LOCK_NONE = 0,					/*!< (0) No locking will be enabled. Use if only one thread is using RSSL. */
	RSSL_LOCK_GLOBAL_AND_CHANNEL = 1,	/*!< (1) Global lock and per-channel locks enabled.  Can access functions from different threads */
	RSSL_LOCK_GLOBAL = 2				/*!< (2) Only the global lock is enabled. RsslChannels can be used by different threads, however each channel should not be used by more than one thread. */
} RsslLockingTypes;


/**
 *	@brief Just-In-Time loaded library configuration options
 */
/* TODO: Better documentation about default behaviors */
typedef struct {
	char*			libsslName;				/*!< Name of the openSSL libssl shared library.  The RSSL API will attempt to dynamically load this library for encrypted connections. */
	char*			libcryptoName;			/*!< Name of the openSSL libcrypto shared library.  The RSSL API will attempt to dynamically load this library for encrypted connections. */
	char*			libcurlName;			/*!< Name of the curl shared library.  The RSSL API will attempt to dynamically load this library for proxy connections. */
} rsslJITOpts;

#define RSSL_INIT_SSL_LIB_JIT_OPTS { NULL, NULL, NULL }

	/* Initialization  & Uninitialization*/
/**
 *	@brief Structure that provides additional initialization information for the RSSL API 
 */
typedef struct
{
	RsslLockingTypes rsslLocking;			/*!< Lock method used for the RSSL API */
	rsslJITOpts		 jitOpts;				/*!< JIT libray options */
	void*			 initConfig;			/*!< private config init */			
	size_t			 initConfigSize;		/*!< private size of config init */			
}RsslInitializeExOpts;

/**
 * @brief Static initializer for RsslInitializeExOpts
 */
#define RSSL_INIT_INITIALIZE_EX_OPTS { RSSL_LOCK_NONE, RSSL_INIT_SSL_LIB_JIT_OPTS, NULL, 0 }

/**
 * @brief Initializes the RSSL API and all internal members
 *
 * Typical use:<BR>
 * This is the first function called when using the RSSL. It 
 * initializes internal data structures and pre-allocates some memory.
 *
 * @param rsslLocking Method of locking used.
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslRet RSSL return value or RsslInitRets value.
 * @see RsslReturnCodes
 * @see RsslLockingTypes
 */
RSSL_API RsslRet rsslInitialize(	RsslLockingTypes rsslLocking,
							 				RsslError *error);
											
/**
 * @brief Initializes the RSSL API and all internal members
 *
 * Typical use:<BR>
 * This is the first function called when using the RSSL. It 
 * initializes internal data structures and pre-allocates some memory.
 *
 * @param rsslInitOpts Initialize options.
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslRet RSSL return value or RsslInitRets value.
 * @see RsslReturnCodes
 * @see RsslInitializeExOpts
 */
RSSL_API RsslRet rsslInitializeEx(	RsslInitializeExOpts *rsslInitOpts,
							 				RsslError *error);

/**
 * @brief Uninitializes the RSSL API and all internal members
 *
 * Typical use:<BR>
 * This is the last function called when using the RSSL.  It 
 * uninitializes internal data structures and deletes any allocated
 * memory.
 *
 * @return RsslRet RSSL return value
 */
RSSL_API RsslRet rsslUninitialize();

/**
 *	@}
 */
 
/**
 * @brief Multicast statistics returned by rsslGetChannelInfo call.
 * @see rsslGetChannelInfo
 * @see RsslChannelInfo
 */
typedef struct {
	RsslUInt64		mcastSent;			/*!< @brief This is the number of multicast packets sent by this channel */
	RsslUInt64		mcastRcvd;			/*!< @brief This is the number of multicast packets received by this channel */
	RsslUInt64		unicastSent;		/*!< @brief This is the number of unicast UDP packets sent by this channel */
	RsslUInt64		unicastRcvd;		/*!< @brief This is the number of unicast UDP packets received by this channel */
	RsslUInt64		gapsDetected;		/*!< @brief This is the number of unrecoverable gaps that have been detected on this channel.  This value includes gaps detected for both multicast and unicast data.  Positive values indicate a possible network problem, more severe as value is larger */
	RsslUInt64		retransReqSent;		/*!< @brief This is the number of retransmission requests sent by this channel, populated only for reliable multicast connection types.  This value includes retransmit requests for both multicast and unicast data.  Positive values indicate a possible network problem, more severe as value is larger */
	RsslUInt64		retransReqRcvd;		/*!< @brief This is the number of retransmission requests received by this channel, populated only for reliable multicast connection types.  This value includes retransmit requests for both multicast and unicast data.  Positive values indicate a possible network problem, more severe as value is larger */
	RsslUInt64		retransPktsSent;	/*!< @brief This is the number of retransmitted packets sent by this channel, populated only for reliable multicast connection types.  This value includes retransmit packets for both multicast and unicast data.  Positive values indicate a possible network problem, more severe as value is larger */
	RsslUInt64		retransPktsRcvd;	/*!< @brief This is the number of retransmitted packets received by this channel, populated only for reliable multicast connection types.  This value includes retransmit packets for both multicast and unicast data.  Positive values indicate a possible network problem, more severe as value is larger */
} RsslMCastStats;


/**
 * @brief Connected Component Information, used to identify components from across the connection
 * @see rsslGetChannelInfo
 * @see RsslChannelInfo
 */
typedef struct {
	RsslBuffer componentVersion;
} RsslComponentInfo;



/**
 * @brief RSSL Channel Info returned by rsslGetChannelInfo call.
 * @see rsslGetChannelInfo
 * @see RsslMCastStats
 * @see RsslComponentInfo
 */
typedef struct {
	RsslUInt32 			maxFragmentSize;		 /*!< @brief This is the max fragment size before fragmentation and reassembly is necessary. */ 
	RsslUInt32			maxOutputBuffers;		 /*!< @brief This is the maximum number of output buffers available to the channel. */
	RsslUInt32			guaranteedOutputBuffers; /*!< @brief This is the guaranteed number of output buffers available to the channel. */
	RsslUInt32			numInputBuffers;		 /*!< @brief This is the number of input buffers available to the channel. */
	RsslUInt32			pingTimeout;			 /*!< @brief This is the value of the negotiated ping timeout */
	RsslBool			clientToServerPings;	 /*!< @brief This is true if the client sends pings to the server */
	RsslBool			serverToClientPings;	 /*!< @brief This is true if the server sends pings to the client */
	RsslUInt32			sysSendBufSize;			 /*!< @brief This is the systems Send Buffer size.  This reports the systems send buffer size respective to the transport type being used (TCP, UDP, etc) */
	RsslUInt32			sysRecvBufSize;			 /*!< @brief This is the systems Receive Buffer size.  This reports the systems receive buffer size respective to the transport type being used (TCP, UDP, etc) */
	RsslUInt32			tcpSendBufSize;			 /*!< @deprecated DEPRECATED: This is the TCP Send Buffer size.  For consistency across all transport types, use RsslChannelInfo::sysSendBufSize */
	RsslUInt32			tcpRecvBufSize;			 /*!< @deprecated DEPRECATED: This is the TCP Receive Buffer size.  For consistency across all transport types, use RsslChannelInfo::sysRecvBufSize */
	RsslCompTypes		compressionType;		 /*!< @brief This is the type of compression being used, if it is enabled */
	RsslUInt32			compressionThreshold;	 /*!< @brief When compression is enabled, any message larger than this threshold will be compressed */
	char				priorityFlushStrategy[RSSL_RSSL_MAX_FLUSH_STRATEGY]; /*!< @brief The currently used flush strategy associated with this channel, H = High, M = Medium, L = Low */
	RsslMCastStats		multicastStats;			 /*!< @brief When using a multicast connection type, this will be populated with information about the multicast protocol */
	RsslUInt32			componentInfoCount;		 /*!< @brief Number of RsslComponentInfo structures contained in the dynamic componentInfo array */
	RsslComponentInfo**	componentInfo;			 /*!< @brief A variable length array that contains product version information for the component(s) that this RsslChannel is connected to. The number of RsslComponentInfo structures present in array is indicated by componentInfoCount.  */
	RsslUInt64			encryptionProtocol;		 /*!< @brief Current encryption protocol used. */
} RsslChannelInfo;

/**
 * @brief Gets information about the channel
 *
 * Typical use:<BR>
 * If information about the RsslChannel is needed, such as maxFragmentSize or 
 * maxOutputBuffers, this function can be called to retrieve this information.
 *
 * @param chnl RSSL Channel to get information about
 * @param info RSSL Channel Info structure to be populated
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslRet RSSL return value
 */
RSSL_API RsslRet rsslGetChannelInfo( RsslChannel *chnl, 
											   RsslChannelInfo *info, 
											   RsslError *error);

											   
/**
 * @brief Allows changing some I/O values programmatically
 * 
 * Typical use:<BR>
 * If an I/O value needs to be changed, this is used
 *
 * @param chnl RSSL Channel to change I/O value for
 * @param code Code of I/O Option to change
 * @param value Value to change Option to
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslRet RSSL return value
 */
RSSL_API RsslRet rsslIoctl(RsslChannel *chnl,
									 RsslIoctlCodes code,
									 void *value,
									 RsslError *error);
									 
/**
 * @brief Returns the total number of used buffers for this channel
 *
 * Typical use: <BR>
 * This function can be called to find out the number of used buffers
 * for the calling channel.  This, in combination with the maxOutputBuffers
 * obtained from the RsslChannelInfo call, can be used to monitor and 
 * potentially throttle buffer usage.
 *
 * @param chnl RSSL Channel to obtain buffer usage for
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslInt32 If less than 0, this is an RsslRet value, otherwise
 *					 it is the total number of buffers in use by this channel
 */
RSSL_API RsslInt32 rsslBufferUsage(RsslChannel *chnl,
											  RsslError *error);
									 
/**
 *	@}
 */
 
/**
 *	@addtogroup RSSLConnectOpts
 *	@{
 */

/**
 * @brief Options used for configuring TCP specific transport options (::RSSL_CONN_TYPE_SOCKET, ::RSSL_CONN_TYPE_ENCRYPTED, ::RSSL_CONN_TYPE_HTTP).
 * @see rsslConnect
 * @see RsslConnectOptions
 */
typedef struct {
	RsslBool			tcp_nodelay;			/*!< @brief Only used with connectionType of ::RSSL_CONN_TYPE_SOCKET.  If RSSL_TRUE, disables Nagle's Algorithm. */
} RsslTcpOpts;

#define RSSL_INIT_TCP_OPTS { RSSL_FALSE }

typedef enum {
	RSSL_MCAST_NO_FLAGS				= 0x00, /*!< @brief None. */
	RSSL_MCAST_FILTERING_ON			= 0x01  /*!< @brief Enables hash-based filtering of incoming messages. */
} RsslMCastFlags;

/**
 * @brief Options used for configuring multicast specific transport options (::RSSL_CONN_TYPE_RELIABLE_MCAST).
 * @see rsslConnect
 * @see RsslConnectOptions
 */
typedef struct {
	RsslUInt16		flags;				/*!< @brief set RsslMCastFlags here */
	RsslBool		disconnectOnGaps;	/*!< @brief Connection will be removed from network on detection of any gap.  @note Enabling this will stop communication with all devices communicating with this connection, even though all may not be affected by detected gaps.  If application can recover only impacted data above transport layer, this is ideal. */
	RsslUInt8		packetTTL;			/*!< @brief The time-to-live for a multicast datagram on the network.  This controls the number of hops content can flow over a network before it will be halted. */
	RsslUInt32		ndata;				/*!< @brief The maximum number of retransmissions that will be attempted for an unacknowledged point-to-point packet. */
	RsslUInt32		nrreq;				/*!< @brief Specifies the maximum number of retransmit requests that will be sent for a missing packet. */
	RsslUInt32		tdata;				/*!< @brief Specifies the time that RRCP must wait before retransmitting an unacknowledged point-to-point packet, in hundreds of millseconds. */
	RsslUInt32		trreq;				/*!< @brief Specifies the minimum time that RRCP will wait before resending a retransmit request for a missed multicast packet, in hundreds of milliseconds. */
	RsslUInt32		twait;				/*!< @brief Specifies the time that RRCP will ignore additional retransmit requests for a data packet that it has already retransmitted, in hundreds of milliseconds. The time period starts with the receipt of the first request for retransmission. */
	RsslUInt32		tbchold;			/*!< @brief Specifies the maximum time that RRCP will hold a transmitted broadcast packet in case the packet needs to be retransmitted, in hundreds of milliseconds. */
	RsslUInt32		tpphold;			/*!< @brief Specifies the maximum time that RRCP will hold a transmitted point-to-point packet in case the packet needs to be retransmitted, in hundreds of milliseconds. */
	RsslUInt16		userQLimit;			/*!< @brief Specifies the maximum backlog of messages allowed on the channel's inbound message queue. Once this limit is exceeded RRCP will begin to discard messages until the the backlog decreases. pktPoolLimitLow should be greater than three times userQLimit. */
	RsslUInt16		nmissing;			/*!< @brief Specifies the maximum number of missed consecutive multicast packets, from a particular node, from which RRCP will attempt to request retransmits. */
	RsslUInt32		pktPoolLimitHigh;	/*!< @brief Specifies the high-water mark for RRCP packet pool. If this limit is reached, no further RRCP packets will be allocated until and unless the usage falls below the low-water mark, the pktPoolLimitLow parameter.  */
	RsslUInt32		pktPoolLimitLow;	/*!< @brief Specifies the low-water mark for RRCP packet pool. Additional RRCP packets will only be allocated if the usage falls from the high-water mark pktPoolLimitHigh to below this low-water mark value. pktPoolLimitLow should be greater than three times userQLimit. */
	char*			hsmInterface;		/*!< @brief Network interface card on which to send host status message(HSM) packets. If NULL, will use default NIC. */
	char*			hsmMultAddress;		/*!< @brief Sets the multicast address over which to send host status message(HSM) packets. */
	char*			hsmPort;			/*!< @brief Sets the multicast port on which to send host status message(HSM) packets. */
	RsslUInt16		hsmInterval;		/*!< @brief The time interval over which HSM packets are sent, in seconds. Set this to 0 to disable sending host status messages.  This setting may be adjusted by the rrdump tool (see the unicastServiceName option). */
	RsslUInt8		reserved;			/* Reserved */
	char*			tcpControlPort;		/*!< @brief Specifies the port to use for the RRCP tcpControlPort, used for troubleshooting RRCP using the rrdump tool.  If this is set to NULL then it will be mapped to the same value as the unicastServiceName.  If this is set to -1 then the tcpControlPort will be disabled. */
	RsslUInt16		portRoamRange;		/*!< @brief Specifies the number of ports to attempt binding on if the unicast base port specified fails to bind.  Set this to 0 to explicitly disable port roaming.  e.g. If this is set to 2 and if the unicast port fails to bind then a bind will be attempted on the unicast port number + 1, and if that port fails to bind then an attempt to bind on unicast port number + 2 will be performed. */
} RsslMCastOpts;

#define RSSL_INIT_MCAST_OPTS { RSSL_MCAST_NO_FLAGS, RSSL_FALSE, 5, 7, 3, 1, 4, 3, 3, 3, 65535, 128, 190000, 180000, NULL, NULL, NULL, 0, 0, NULL, 0 }

/**
 * @brief Options used for configuring shared memory specific transport options (::RSSL_CONN_TYPE_UNIDIR_SHMEM).
 * @see rsslConnect
 * @see RsslConnectOptions
 */
typedef struct {
	RsslUInt		maxReaderLag;			/*!<  @brief Maximum number of messages that the client can have waiting to read. If the client "lags" the server by more than this amount, UPA will disconnect the client */
} RsslShmemOpts;

#define RSSL_INIT_SHMEM_OPTS { 0 }

/**
 * @brief Options used for configuring sequenced multicast specific transport options (::RSSL_CONN_TYPE_SEQ_MCAST).
 * see rsslConnect
 * see RsslConnectOptions
 */
typedef struct {
	RsslUInt32		maxMsgSize;			/*!<  @brief Maximum size of messages that the SEQ_MCAST transport will read. */
	RsslUInt16		instanceId;			/*!<  @brief This is used, when combined with the origin IP address and port, to uniquely identify a sequenced multicast channel. */
} RsslSeqMCastOpts;

#define RSSL_INIT_SEQ_MCAST_OPTS { 3000, 0 }
typedef struct {
	char* proxyHostName;				/*!<  @brief Proxy host name. */
	char* proxyPort;					/*!<  @brief Proxy port. */
	char* proxyUserName;				/*!<  @brief User Name for authenticated proxies. */
	char* proxyPasswd;					/*!<  @brief Password for authenticated proxies. */
	char* proxyDomain;					/*!<  @brief Domain for authenticated proxies. */
} RsslProxyOpts;

#define RSSL_INIT_PROXY_OPTS {0, 0, 0, 0, 0}

typedef enum {
	RSSL_ENC_NONE    = 0x00,			/*!< @brief (0x00) No encryption. */
	RSSL_ENC_TLSV1_2 = 0x04				/*!< @brief (0x08) Encryption using TLSv1.2 protocol */
} RsslEncryptionProtocolTypes;


/** @brief Options used for configuring a WebSocket connection (::RSSL_CONN_TYPE_WEBSOCKET).
 * @see rsslConnect
 * @see RsslConnectOptions
 * @see RsslBindOptions
 */
typedef struct {
	char			*protocols;			/*!< @brief The left-to-right priority ordered, white space delineated list of supported/preferred protocols */
	RsslUInt64		maxMsgSize;	 /*!<  @brief Maximum size of messages that the WebSocket transport will read on client side. */
} RsslWSocketOpts;

#define RSSL_INIT_WEBSOCKET_OPTS { 0, 61440}


/** @brief Options used for configuring an encrypted tunneled connection (::RSSL_CONN_TYPE_ENCRYPTED).
 *  see rsslConnect
 *  see RsslConnectOptions
 */
typedef struct {
	RsslUInt32			encryptionProtocolFlags;	/*!< @brief Bitmap flag set defining the TLS version(s) to be used by this connection.  See RsslEncryptionProtocolTypes */
	RsslConnectionTypes encryptedProtocol;			/*!< @brief Defines the protocol used for this connection.<BR>
														RSSL_CONN_TYPE_HTTP will use the legacy WinInet-based protocol for Windows only.<BR>
														RSSL_CONN_TYPE_SOCKET will use the standard TCP transport protocol and OpenSSL for encryption. 
														RSSL_CONN_TYPE_WEBSOCKET will use the WebSocket transport protocol and OpenSSL for encryption. */
	char*				openSSLCAStore;				/*!< Path to the CAStore.  This will be used by any OpenSSL encrypted connections for certificate validation.  <BR>
														A NULL input will result in the following behavior:<BR>
															Windows: RSSL will load Windows Root Certificate store.<BR>
															Linux: Load the default CA Store path based on the OpenSSL library's default behavior. This may be distribution specific, please see vendor documentation for more information */
} RsslEncryptionOpts;

#ifdef _WIN32
#define RSSL_INIT_ENCRYPTION_OPTS { RSSL_ENC_TLSV1_2, RSSL_CONN_TYPE_HTTP, NULL}
#else
#define RSSL_INIT_ENCRYPTION_OPTS { RSSL_ENC_TLSV1_2, RSSL_CONN_TYPE_SOCKET, NULL}
#endif


/*
 *	brief Options used for configuring Extended Line specific transport options (::RSSL_CONN_TYPE_EXTENDED_LINE).
 *	see rsslConnect
 *	see RsslConnectOptions
 */
typedef struct {
	RsslUInt32		numConnections;			/* (not in doxygen) Number of concurrent Extended Line connections to be established */
} RsslELOpts;
//
#define RSSL_INIT_EL_OPTS {20}



/**
 * @brief Options used for configuring a unified/fully connected mesh network.
 * @see rsslConnect
 * @see RsslConnectionInfo
 * @see RsslConnectOptions
 */
typedef struct {
	char*			address;			/*!< @brief Address or hostname to connect to/join for all inbound and outbound data.  All data is exchanged on this hostName:serviceName combination */
	char*			serviceName;		/*!< @brief Port number or service name to connect to/join for all inbound and outbound data.  All data is exchanged on this hostName:serviceName combination */
	char*			unicastServiceName;	/*!< @brief Only used with connectionType of ::RSSL_CONN_TYPE_RELIABLE_MCAST.  Port number or service name for any unicast messages such as ACK/NAK traffic or retransmit requests. Also sets the TCP listening port for use with the rrdump tool. */
	char*			interfaceName;		/*!< @brief Network interface card to bind to for all inbound and outbound data.  If NULL, will use default NIC.*/
} RsslUnifiedNetwork;

/**
 * @brief Options used for configuring a segmented network (separate send and receive networks).
 * @see rsslConnect
 * @see RsslConnectionInfo
 * @see RsslConnectOptions
 */
typedef struct {
	char*			recvAddress;		/*!< @brief Address to connect to/join for inbound data.  Data will only be received on this recvAddress:recvPort combination */
	char*			recvServiceName;	/*!< @brief Port number or service name to connect to/join for inbound data.  Data will only be received on this recvAddress:recvPort combination */
	char*			unicastServiceName;	/*!< @brief Only used with connectionType of ::RSSL_CONN_TYPE_RELIABLE_MCAST.  Port number or service name for any unicast messages such as ACK/NAK traffic or retransmit requests. Also sets the TCP listening port for use with the rrdump tool. */
	char*			interfaceName;		/*!< @brief Network interface card to bind to for send and recv networks.  If NULL, will use default NIC.*/
	char*			sendAddress;		/*!< @brief Address to connect to/join for outbound data.  Data will only be sent on this sendAddress:sendPort combination */
	char*			sendServiceName;	/*!< @brief Port number or service name to connect to/join for outbound data.  Data will only be sent on this sendAddress:sendPort combination */
} RsslSegmentedNetwork;

/**
 * @brief Options used for configuring the network connection.
 * @see rsslConnect
 * @see RsslUnifiedNetwork
 * @see RsslSegmentedNetwork
 * @see RsslConnectOptions
 */
typedef union {
	RsslSegmentedNetwork  segmented;	/*!< Connection parameters when sending and receiving on different networks.  This is typically used with multicast networks that have different groups of senders and receivers (e.g. NIProvider can send network and ADH on receive network) */
	RsslUnifiedNetwork	  unified;	    /*!< Connection parameters when sending and receiving on same network.  This is typically used with ::RSSL_CONN_TYPE_SOCKET, ::RSSL_CONN_TYPE_HTTP, ::RSSL_CONN_TYPE_ENCRYPTED, and fully connected/mesh multicast networks */
} RsslConnectionInfo;

#define RSSL_INIT_CONNECTION_INFO {{ 0, 0, 0, 0, 0, 0}}
 
/**
 * @brief RSSL Connect Options used in the rsslConnect call.
 * @see RSSL_INIT_CONNECT_OPTS
 * @see rsslConnect
 * @see RsslConnectionInfo
 */
typedef struct {
	char*				hostName;				/*!< @deprecated DEPRECATED: Hostname to connect to/join. All data is exchanged on this hostName:serviceName combination.  This option will only function for the SOCKET, HTTP, and ENCRYPTED connection types.  Users should migrate to the RsslConnectOptions::connectionInfo::unified::address configuration for the same behavior with current and future connection types */
	char*				serviceName;			/*!< @deprecated DEPRECATED: Port number or service name to connect to/join.  All data is exchanged on this hostName:serviceName combination.  This option will only function for the SOCKET, HTTP, and ENCRYPTED connection types.  Users should migrate to the RsslConnectOptions::connectionInfo::unified::serviceName configuration for the same behavior with current and future connection types */
	char				*objectName;			/*!< @brief When using connection type of ::RSSL_CONN_TYPE_HTTP or ::RSSL_CONN_TYPE_ENCRYPTED, this can be used as an object name to pass with the URL in underlying HTTP connection messages */
	RsslConnectionTypes	connectionType;			/*!< @brief If ::RSSL_CONN_TYPE_ENCRYPTED this will use encryption, if ::RSSL_CONN_TYPE_HTTP this will use unencrypted http tunneling, if ::RSSL_CONN_TYPE_UNIDIR_SHMEM this will use server to client shared memory */
	RsslConnectionInfo	connectionInfo;			/*!< @brief Information about the network hosts/addresses, ports, and network interface cards to leverage during connection.  This configuration offers configuration for various network topologies and can be used for all connection types.  */
	RsslCompTypes		compressionType;		/*!< @brief Which compression type, if any, to attempt to negotiate. Compression is only supported for connectionType of SOCKET, WSOCKET, HTTP, or ENCRYPTED */	
	RsslBool			blocking;				/*!< @brief If RSSL_TRUE, the connection will block. */
	RsslBool			tcp_nodelay;			/*!< @deprecated DEPRECATED: Only used with connectionType of SOCKET.  If RSSL_TRUE, disables Nagle's Algorithm. Users should migrate to the RsslConnectOptions::tcpOpts::tcp_nodelay configuration for the same behavior with current and future connection types */
	RsslUInt32			pingTimeout;			/*!< @brief The desired amount of time to use to timeout the server. */
	RsslUInt32			guaranteedOutputBuffers;/*!< @brief Number of output buffers guaranteed to this RsslChannel. */
	RsslUInt32			numInputBuffers;		/*!< @brief Sets the number of buffers (of maxFragmentSize) used to read in data. */
	RsslUInt8			protocolType;			/*!< @brief The protocol type of the RsslChannel. */
	RsslUInt8			majorVersion;			/*!< @brief The major version number of the RsslChannel. */ 
	RsslUInt8			minorVersion;			/*!< @brief The minor version number of the RsslChannel. */
	RsslUInt32			sysSendBufSize;			/*!< @brief The size (in kilobytes) of the system's send buffer used for this connection, where applicable.  Setting of 0 indicates to use default sizes.  This can also be set or changed via rsslIoctl(), however setting prior to connection may allow for larger sizes to be specified.  */
	RsslUInt32			sysRecvBufSize;			/*!< @brief The size (in kilobytes) of the system's receive buffer used for this connection, where applicable.  Setting of 0 indicates to use default sizes.  This can also be set or changed via rsslIoctl(), however setting prior to connection may allow for larger sizes to be specified. */
	void				*userSpecPtr;			/*!< @brief A user specified pointer, returned as userSpecPtr of the RsslChannel. */
	RsslTcpOpts			tcpOpts;				/*!< @brief TCP transport specific options (used by ::RSSL_CONN_TYPE_SOCKET, ::RSSL_CONN_TYPE_WEBSOCKET, ::RSSL_CONN_TYPE_ENCRYPTED, ::RSSL_CONN_TYPE_HTTP). */
	RsslMCastOpts		multicastOpts;			/*!< @brief Multicast transport specific options (used by ::RSSL_CONN_TYPE_RELIABLE_MCAST). */
	RsslShmemOpts		shmemOpts;				/*!< @brief shmem transport specific options (used by ::RSSL_CONN_TYPE_UNIDIR_SHMEM). */
	RsslSeqMCastOpts	seqMulticastOpts;		/*!< @brief Sequenced Multicast transport specific options (used by ::RSSL_CONN_TYPE_SEQ_MCAST). */
	RsslProxyOpts		proxyOpts;
	char*				componentVersion;		/*!< @brief User defined component version information*/
	RsslEncryptionOpts  encryptionOpts;
	RsslELOpts			extLineOptions;			/* Extended Line specific options */
	RsslWSocketOpts		wsOpts;					/*!< @brief WebSocket specific options (::RSSL_CONN_TYPE_WEBSOCKET) */
} RsslConnectOptions;

/**
 * @brief RSSL Connect Options initialization
 * @see RsslConnectOptions
 */
#define RSSL_INIT_CONNECT_OPTS { 0, 0, 0, RSSL_CONN_TYPE_SOCKET, RSSL_INIT_CONNECTION_INFO, RSSL_COMP_NONE, RSSL_FALSE, RSSL_FALSE, 60, 50, 10, 0, 0, 0, 0, 0, 0, RSSL_INIT_TCP_OPTS, RSSL_INIT_MCAST_OPTS, RSSL_INIT_SHMEM_OPTS, RSSL_INIT_SEQ_MCAST_OPTS, RSSL_INIT_PROXY_OPTS, 0, RSSL_INIT_ENCRYPTION_OPTS, RSSL_INIT_EL_OPTS, RSSL_INIT_WEBSOCKET_OPTS }


/**
 * @brief Clears RSSL Connect Options
 * 
 * This initializes the Rssl Connect Options and is typically
 * used instead of RSSL_INIT_CONNECT_OPTS initializer.
 * @param opts RsslConnectOptions
 * @see RsslConnectOptions
 */
RTR_C_INLINE void rsslClearConnectOpts(RsslConnectOptions *opts)
{
	opts->hostName = 0;
	opts->serviceName = 0;
	opts->connectionInfo.segmented.recvAddress = 0;
	opts->connectionInfo.segmented.recvServiceName = 0;
	opts->connectionInfo.segmented.interfaceName = 0;
	opts->connectionInfo.segmented.sendAddress = 0;
	opts->connectionInfo.segmented.sendServiceName = 0;
	opts->connectionInfo.segmented.unicastServiceName = 0;
	opts->objectName = 0;
	opts->compressionType = RSSL_COMP_NONE;
	opts->blocking = RSSL_FALSE;
	opts->tcp_nodelay = RSSL_FALSE;
	opts->connectionType = RSSL_CONN_TYPE_SOCKET;
	opts->pingTimeout = 60;
	opts->guaranteedOutputBuffers = 50;
	opts->numInputBuffers = 10;
	opts->majorVersion = 0;
	opts->minorVersion = 0;
	opts->protocolType = 0;
	opts->userSpecPtr = 0;
	opts->tcpOpts.tcp_nodelay = RSSL_FALSE;
	opts->multicastOpts.flags = RSSL_MCAST_NO_FLAGS;
	opts->multicastOpts.disconnectOnGaps = RSSL_FALSE;
	opts->multicastOpts.packetTTL = 5;
	opts->multicastOpts.ndata = 7;
	opts->multicastOpts.nrreq = 3;
	opts->multicastOpts.tdata = 1;
	opts->multicastOpts.trreq = 4;
	opts->multicastOpts.twait = 3;	
	opts->multicastOpts.tbchold = 3;
	opts->multicastOpts.tpphold = 3;
	opts->multicastOpts.userQLimit = 65535;
	opts->multicastOpts.nmissing = 128;
	opts->multicastOpts.pktPoolLimitHigh = 190000;
	opts->multicastOpts.pktPoolLimitLow = 180000;
	opts->multicastOpts.hsmInterface = NULL;
	opts->multicastOpts.hsmMultAddress = NULL;
	opts->multicastOpts.hsmPort = NULL;
	opts->multicastOpts.hsmInterval = 0;
	opts->multicastOpts.tcpControlPort = NULL;
	opts->multicastOpts.portRoamRange = 0;
	opts->shmemOpts.maxReaderLag = 0;
	opts->sysSendBufSize = 0;
	opts->sysRecvBufSize = 0;
	opts->seqMulticastOpts.maxMsgSize = 3000;
	opts->seqMulticastOpts.instanceId = 0;
	opts->proxyOpts.proxyHostName = 0;
	opts->proxyOpts.proxyPort = 0;
	opts->componentVersion = NULL;
	opts->encryptionOpts.encryptionProtocolFlags = RSSL_ENC_TLSV1_2;
#ifdef _WIN32
	opts->encryptionOpts.encryptedProtocol = RSSL_CONN_TYPE_HTTP;
#else
	opts->encryptionOpts.encryptedProtocol = RSSL_CONN_TYPE_SOCKET;
#endif
	opts->extLineOptions.numConnections = 20;
	opts->encryptionOpts.openSSLCAStore = NULL;
	opts->proxyOpts.proxyHostName = NULL;
	opts->proxyOpts.proxyPort = NULL;
	opts->proxyOpts.proxyUserName = NULL;
	opts->proxyOpts.proxyPasswd = NULL;
	opts->proxyOpts.proxyDomain = NULL;
	opts->wsOpts.maxMsgSize = 61440;
	opts->wsOpts.protocols = NULL;
}

/**
 *	@}
 */

/**
 *	@addtogroup RSSLChannelConnection
 *	@{
 */
 
/**
 * @brief Connects a client to a listening server
 *
 * Typical use:<BR>
 * 1. Initialize RsslConnectOptions<BR>
 * 2. Set RsslConnectOptions to desired values<BR>
 * 3. Call rsslConnect to create RsslChannel<BR>
 * 4. Read or write with the RsslChannel<BR>
 *
 * @param opts Options used when connecting
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslChannel* Connected RSSL channel or NULL
 */
RSSL_API RsslChannel* rsslConnect(RsslConnectOptions *opts,
											RsslError *error);


/**
 * @brief Reconnects a client to a listening server - used for tunneling connection types
 *
 * Typical use:<BR>
 * 1. Call rsslReconnectClient with already active RsslChannel
 *
 * @note This function only performs a reconnection for client connections and
 * only when tunnelType is enabled.  This function is designed so the application
 * can proactively bridge a connection to keep data flow alive through proxy servers.
 *
 * @param chnl RSSL Channel to reconnect
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslRet of Success or failure
 */
RSSL_API RsslRet rsslReconnectClient(RsslChannel *chnl,
						  					   RsslError *error);


/**
 *	@}
 */ 

 /**
 *	@addtogroup RSSLServer
 *	@{
 */
 
/**
 * @brief RSSL Server structure returned via the rsslBind call.
 */
typedef struct {
	RsslSocket			socketId;		/*!< @brief Socket ID of this RSSL server. */
	RsslChannelState	state;			/*!< @brief State of this RSSL server. */
	RsslUInt16			portNumber;		/*!< @brief Port number this server is bound to. */
	void				*userSpecPtr;	/*!< @brief A user specified pointer, possibly a closure. */ 
} RsslServer;

/**
 * @brief RSSL Server Info returned by rsslGetServerInfo call.
 * @see rsslGetServerInfo
 */
typedef struct {
	RsslUInt32 	currentBufferUsage;  /*!< @brief This is the current buffer usage for the server. */ 
	RsslUInt32 	peakBufferUsage;	 /*!< @brief This is the peak buffer usage for the server. */ 
} RsslServerInfo;

/**
 * @brief Gets information about the server
 *
 * Typical use:<BR>
 * If information about the RsslServer is needed, such as peakBufferUsage,
 * this function can be called to retrieve this information.
 *
 * @param srvr RSSL Server to get information about
 * @param info RSSL Server Info structure to be populated
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslRet RSSL return value
 */
RSSL_API RsslRet rsslGetServerInfo( RsslServer *srvr, 
											  RsslServerInfo *info, 
											  RsslError *error);

/**
 * @brief Allows changing some I/O values programmatically for an Rssl Server
 *
 * Typical use:<BR>
 * If an I/O value needs to be changed for a server, this is 
 * used. Currently this only supports changing the shared pool size.
 *
 * @param srvr RSSL Server to change I/O value for
 * @param code Code of I/O Option to change
 * @param value Value to change Option to
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslRet RSSL return value
 */
RSSL_API RsslRet rsslServerIoctl(RsslServer *srvr,
										   RsslIoctlCodes code,
										   void *value,
										   RsslError *error);
/**
 * @brief Returns the total number of used buffers for the server
 *
 * Typical use: <BR>
 * This function can be called to find out the number of used buffers
 * for the server.  This, in combination with the bufferPoolSize
 * used as input to the rsslBind call, can be used to monitor and 
 * potentially throttle buffer usage.
 *
 * @param srvr RSSL Server to obtain buffer usage for
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslInt32 If less than 0, this is an RsslRet value, otherwise
 *					 it is the total number of buffers in use by the server
 */
RSSL_API RsslInt32 rsslServerBufferUsage(RsslServer *srvr,
												    RsslError *error);

/**
 *	@}
 */
 
/**
 *	@addtogroup RSSLServerBindOpts
 *	@{
 */

 /** @brief Options used for configuring an encrypted tunneled connection (::RSSL_CONN_TYPE_ENCRYPTED).
 *  see rsslConnect
 *  see RsslConnectOptions
 */
typedef struct {
	RsslUInt32			encryptionProtocolFlags;	/*!< @brief Bitmap flag set defining the TLS version(s) to be used by this connection.  See RsslEncryptionProtocolTypes */
	char*				serverCert;					/*!< Path to this server's certificate.   */
	char*				serverPrivateKey;			/*!< Path to the server's key file */
	char*				cipherSuite;				/*!< Optional OpenSSL formatted cipher suite string.  ETA's default configuration is OWASP's "B" tier recommendations, which are the following:
														DHE-RSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-RSA-AES128-GCM-SHA256:DHE-RSA-AES256-SHA256:DHE-RSA-AES128-SHA256:ECDHE-RSA-AES256-SHA384:ECDHE-RSA-AES128-SHA256:!aNULL:!eNULL:!LOW:!3DES:!MD5:!EXP:!PSK:!DSS:!RC4:!SEED:!ECDSA:!ADH:!IDEA:!3DES */
	char*				dhParams;					/*!< Optional Diffie-Hellman parameter file.  If this is not present, RSSL will load it's default DH parameters */
} RsslBindEncryptionOpts;


#define RSSL_INIT_BIND_ENCRYPTION_OPTS { RSSL_ENC_TLSV1_2, NULL, NULL, NULL, NULL}
 
/**
 * @brief RSSL Bind Options used in the rsslBind call.
 * @see RSSL_INIT_BIND_OPTS
 * @see rsslBind
 */
typedef struct {
	char			*serviceName;			/*!< @brief Local port number or service name to bind to. */
	char			*interfaceName;			/*!< @brief Network interface card to bind to.  If NULL, will use default NIC */
	RsslUInt32		compressionType;		/*!< @brief Bitmask of RsslCompTypes compression types supported by the server */
	RsslUInt32		compressionLevel;		/*!< @brief Level of compression to use, 1: More speed - 9: More compression. */
	RsslBool		forceCompression;		/*!< @brief Lets the server force the client to use compression */
	RsslBool		serverBlocking;			/*!< @brief If RSSL_TRUE, the server will be allowed to block. */
	RsslBool		channelsBlocking;		/*!< @brief If RSSL_TRUE, the channels will be allowed to block. */
	RsslBool		tcp_nodelay;			/*!< @deprecated DEPRECATED: Only used with connectionType of SOCKET.  If RSSL_TRUE, disables Nagle's Algorithm. Users should migrate to the RsslBindOptions::tcpOpts::tcp_nodelay configuration for the same behavior with current and future connection types */
	RsslBool		serverToClientPings;	/*!< @brief If RSSL_TRUE, pings will be sent from server side to client side */
	RsslBool		clientToServerPings;	/*!< @brief If RSSL_TRUE, pings will be sent from client side to server side */
	RsslConnectionTypes	connectionType;		/*!< @brief If RSSL_CONN_TYPE_UNIDIR_SHMEM this will use server to client shared memory.  Setting to RSSL_CONN_TYPE_SOCKET or RSSL_CONN_TYPE_HTTP will allow for accepting both socket or HTTP connection types.  RSSL_CONN_TYPE_ENCRYPTED is currently not supported for servers  */
	RsslUInt32		pingTimeout;			/*!< @brief Desired amount of time to use as a timeout for a connected channel. */
	RsslUInt32		minPingTimeout;			/*!< @brief Least amount of time to use as a timeout for a connected channel. */
	RsslUInt32		maxFragmentSize;		/*!< @brief Sets the maximum size fragment to be sent without any fragmentation or assembly of messages. */
	RsslUInt32		maxOutputBuffers;		/*!< @brief Sets the maximum number of output buffers for accepted channels. */
	RsslUInt32		guaranteedOutputBuffers;/*!< @brief Sets the guaranteed number of output buffers for accepted channels. */
	RsslUInt32		numInputBuffers;		/*!< @brief Sets the number of input buffers for reading into (of maxFragmentSize) used to read in data. */
	RsslUInt32		sharedPoolSize;			/*!< @brief Sets the maximum size of the shared buffer pool (of sharedPoolSize * maxFragmentSize). */	
	RsslBool		sharedPoolLock;			/*!< @brief Whether to enable mutex locks on the shared buffer pool */
	RsslUInt8		majorVersion;			/*!< @brief The major version number of the RsslServer. */ 
	RsslUInt8		minorVersion;			/*!< @brief The minor version number of the RsslServer. */
	RsslUInt8		protocolType;			/*!< @brief The protocol type of the RsslServer. */
	RsslUInt32		sysSendBufSize;			/*!< @brief The size (in kilobytes) of the system's send buffer used for this connection, where applicable.  Setting of 0 indicates to use default sizes.  This setting will carry through on all connections accepted with rsslAccept().  This can also be set or changed per channel via rsslIoctl(), however setting prior to connection may allow for larger sizes to be specified.  */
	RsslUInt32		sysRecvBufSize;			/*!< @brief The size (in kilobytes) of the system's receive buffer used for this connection, where applicable.  Setting of 0 indicates to use default sizes.  This setting will carry through on all connections accepted with rsslAccept(). This can also be set or changed via rsslIoctl(), however setting prior to connection may allow for larger sizes to be specified. */
	void			*userSpecPtr;			/*!< @brief A user specified pointer, returned as userSpecPtr of the RsslServer. */ 
	RsslTcpOpts		tcpOpts;				/*!< @brief TCP transport specific options (used by RSSL_CONN_TYPE_SOCKET and RSSL_CONN_TYPE_HTTP). */
	char*			componentVersion;		/*!< @brief User defined component version information */
	RsslWSocketOpts	wsOpts;					/*!< @brief WebSocket transport options for RSSL_CONN_TYPE_WEBSOCKET */
	RsslBindEncryptionOpts encryptionOpts;	/*!< @brief Encryption options. */
} RsslBindOptions;


/**
 * @brief RSSL Bind Options initialization
 * @see RsslBindOptions
 */
#define RSSL_INIT_BIND_OPTS { 0, 0, RSSL_COMP_NONE, 0, RSSL_FALSE, RSSL_FALSE, RSSL_FALSE, RSSL_FALSE, RSSL_TRUE, RSSL_TRUE, RSSL_CONN_TYPE_SOCKET, 60, 20, 6144, 50, 50, 10, 0, RSSL_FALSE, 0, 0, 0, 0, 0, 0, RSSL_INIT_TCP_OPTS, 0, RSSL_INIT_WEBSOCKET_OPTS, RSSL_INIT_BIND_ENCRYPTION_OPTS }

/**
 * @brief Clears RSSL Bind Options 
 * 
 * This initializes the RSSL Bind options and is 
 * typically used instead of the RSSL_INIT_BIND_OPTS initializer. 
 * @param opts Rssl Bind Options 
 * @see RsslBindOptions
 */
RTR_C_INLINE void rsslClearBindOpts(RsslBindOptions *opts)
{
	opts->serviceName = 0;
	opts->interfaceName = 0;
	opts->compressionType = RSSL_COMP_NONE;
	opts->compressionLevel = 0;
	opts->forceCompression = RSSL_FALSE;
	opts->serverBlocking = RSSL_FALSE;
	opts->channelsBlocking = RSSL_FALSE;
	opts->tcp_nodelay = RSSL_FALSE;
	opts->serverToClientPings = RSSL_TRUE;
	opts->clientToServerPings = RSSL_TRUE;
	opts->connectionType = RSSL_CONN_TYPE_SOCKET;
	opts->pingTimeout = 60;
	opts->minPingTimeout = 20;
	opts->maxFragmentSize = 6144;
	opts->maxOutputBuffers = 50;
	opts->guaranteedOutputBuffers = 50;
	opts->numInputBuffers = 10;
	opts->sharedPoolSize = 0;
	opts->sharedPoolLock = RSSL_FALSE;
	opts->majorVersion = 0;
	opts->minorVersion = 0;
	opts->protocolType = 0;
	opts->userSpecPtr = 0;
	opts->tcpOpts.tcp_nodelay = RSSL_FALSE;
	opts->sysSendBufSize = 0;
	opts->sysRecvBufSize = 0;
	opts->componentVersion = NULL;
	opts->wsOpts.maxMsgSize = 61440;
;	opts->wsOpts.protocols = NULL;
	opts->encryptionOpts.cipherSuite = NULL;
	opts->encryptionOpts.dhParams = NULL;
	opts->encryptionOpts.encryptionProtocolFlags = RSSL_ENC_TLSV1_2;
	opts->encryptionOpts.serverCert = NULL;
	opts->encryptionOpts.serverPrivateKey = NULL;
}

/**
 *	@}
 */
 
/**
 *	@addtogroup RSSLServerBind
 *	@{
 */

/**
 * @brief Creates an RSSL Server by binding to a port 
 *
 * Typical use:<BR>
 * 1. Initialize RsslBindOptions<BR>
 * 2. Set RsslBindOptions to desired values<BR>
 * 3. Call rsslBind to create RsslServer<BR>
 *
 * @param opts Options used when binding
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslServer* Bound RSSL server or NULL
 */
RSSL_API RsslServer* rsslBind(	RsslBindOptions *opts,
											RsslError *error);

/**
 *	@}
 */
 
/**
 *	@addtogroup RSSLServerAcceptOpts
 *	@{
 */
 
/**
 * @brief RSSL Accept Options used in the rsslAccept call. 
 * @see RSSL_INIT_ACCEPT_OPTS
 * @see rsslAccept
 */
typedef struct {
	RsslBool	nakMount;		/*!< @brief If RSSL_TRUE, rsslAccept will send a NAK - even if the connection request is valid. */
	void		*userSpecPtr;	/*!< @brief A user specified pointer, returned as userSpecPtr of the RsslChannel from rsslAccept(). */
} RsslAcceptOptions;


/**
 * @brief RSSL Accept Options initialization
 * @see RsslAcceptOptions
 */
#define RSSL_INIT_ACCEPT_OPTS { RSSL_FALSE, 0 }

/**
 * @brief Clears RSSL Accept Options
 *
 * This initializes the Rssl Accept Options and is typically
 * used instead of RSSL_INIT_ACCEPT_OPTS initializer.
 * @param opts RsslAcceptOptions
 * @see RsslAcceptOptions
 */
RTR_C_INLINE void rsslClearAcceptOpts(RsslAcceptOptions *opts)
{
	opts->nakMount = RSSL_FALSE;
	opts->userSpecPtr = 0;
}

/**
 *	@}
 */

/**
 *	@addtogroup RSSLServerAcceptOpts
 *	@{
 */

/**
 * @brief Accepts an incoming connection and returns a channel which corresponds to it
 *
 * Typical use:<BR>
 * After a server is created using the rsslBind call, the rsslAccept call can
 * be made.  When the socketId of the server detects something to be read, 
 * this will check for incoming client connections.  When a client
 * successfully connects, a RsslChannel is returned which corresponds to this
 * connection.  This channel can be used to read or write with the connected 
 * client. If a clients connect message is not accepted (nakMount = RSSL_TRUE),
 * a negative acknowledgment can be sent when the rsslInitChannel handshake
 * is completed. 
 *
 * @param srvr RSSL Server to accept incoming connections on
 * @param opts RSSL Accept Options 
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslChannel* Accepted RSSL channel or NULL in failure. 
 */
RSSL_API RsslChannel* rsslAccept(	RsslServer *srvr,
											RsslAcceptOptions *opts,
											RsslError *error);

/**
 * @brief Closes an RSSL Server
 * 
 * Typical use:<BR>
 * When done using a Server, this call closes it.  
 * Active channels connected to this server will not be closed; 
 * this allows them to continue receiving data even if the server
 * is not accepting more connections.  
 *
 * @param srvr RSSL Server to close
 * @param error RSSL Error, to be populated in event of an error 
 * @return RsslRet RSSL return value
 */
RSSL_API RsslRet rsslCloseServer(RsslServer *srvr,
										   RsslError *error);
 
/**
 *	@}
 */ 
 
/**
 *	@addtogroup RSSLChannelOps
 *	@{
 */
 
/** RsslInProgInfo
 * @brief RsslInProgInfo Information for the In Progress Connection State
 * If a backward compatibility reconnection occurs, the file descriptor 
 * may change.  This is how that information is relayed
 * @see RsslInProgFlags, RSSL_INIT_IN_PROG_INFO
 */
typedef struct {
	RsslUInt8			flags;		/*!< @brief Flags indicating status of connection attempt, populated with \ref RsslInProgFlags */
	RsslSocket			oldSocket;	/*!< @brief In event of \ref RsslInProgFlags::RSSL_IP_FD_CHANGE, this contains the old file descriptor allowing for removal from any I/O notifier sets. */
	RsslSocket			newSocket;	/*!< @brief In event of \ref RsslInProgFlags::RSSL_IP_FD_CHANGE, this contains the new file descriptor allowing for addition to any I/O notifier sets.  */
	RsslUInt32			internalConnState; /*! @brief An integer representing the internal connection state, can be used when reporting and troubleshooting connection handshake issues and failures.  */
} RsslInProgInfo;

/** 
 * @brief RsslInProgFlags used when connection state is in progress
 * Flags for the InProgress state
 * @see rsslInitChannel
 */
typedef enum {
	RSSL_IP_NONE		= 0x00, /*!< No change */
	RSSL_IP_FD_CHANGE	= 0x01  /*!< File Descriptor change occurred */
} RsslInProgFlags;



/**
 * @brief RsslInProgInfo static initialization
 * @see RsslInProgInfo, rsslClearInProgInfo, rsslInitChannel()
 */
#define RSSL_INIT_IN_PROG_INFO { RSSL_IP_NONE, 0, 0, 0 }

/**
 * @brief Clears RsslInProgInfo structure for reuse
 * @see RsslInProgInfo, RSSL_INIT_IN_PROG_INFO, rsslInitChannel()
 */
RTR_C_INLINE void rsslClearInProgInfo(RsslInProgInfo *inProgInfo)
{
	inProgInfo->flags = RSSL_IP_NONE;
	inProgInfo->newSocket = 0;
	inProgInfo->oldSocket = 0;
	inProgInfo->internalConnState = 0;
}

 
/** 
 * @brief Continues channel initialization for non-blocking channels
 *
 * Typical use:<BR>
 * 1. Connect using rsslConnect()
 * 2. While Channel state is ::RSSL_CH_STATE_INITIALIZING and the channels socketId
 *    detects data to read,  call rsslInitChannel()
 *
 * @note This is not necessary for blocking channels, which will return in the ::RSSL_CH_STATE_ACTIVE state after the rsslConnect() call.  
 *
 * @param chnl RSSL Channel to continue initialization on
 * @param inProg InProg Info for compatibility mode reconnections
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslRet RSSL return value or RsslReturnCodes value
 * @see RsslReturnCodes, RsslInProgInfo
 */
RSSL_API RsslRet rsslInitChannel(RsslChannel *chnl,
										   RsslInProgInfo *inProg, 
										   RsslError *error);
											 
										   
/**
 * @brief Closes an RSSL Channel
 *
 * Typical use:<BR>
 * When done using a Channel, this call closes it.
 *
 * @param chnl RSSL Channel to close
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslRet RSSL return value
 */
RSSL_API RsslRet rsslCloseChannel(RsslChannel *chnl, 
										    RsslError *error);
/**
 * @brief RSSL readFlags in
 * @see rsslReadEx
 */
typedef enum {
	RSSL_READ_IN_NO_FLAGS		= 0x00	/*!< (0x00) No read flags*/
} RsslReadFlagsIn;


/**
 * @brief RsslReadEx input arguments
 */
typedef struct {
	   RsslUInt32                         readInFlags;   /*!< param readInFlags for reading the buffer */
} RsslReadInArgs;

/**
 * @brief RSSL RsslReadInArgs static initialization
 */
#define RSSL_INIT_READ_IN_ARGS { RSSL_READ_IN_NO_FLAGS }

/**
 * @brief RSSL RsslReadOutFlags
 * @see rsslReadEx
 */
typedef enum {
	RSSL_READ_OUT_NO_FLAGS		= 0x0000,	/*!< (0x00) No read flags*/
	RSSL_READ_OUT_FTGROUP_ID	= 0x0001,	/*!< (0x01) set when a valid FT Group ID is returned */
	RSSL_READ_OUT_NODE_ID		= 0x0002,	/*!< (0x02) set when a valid NODE ID is returned */
	RSSL_READ_OUT_SEQNUM		= 0x0004,	/*!< (0x04) set when a seqnum is returned */
	RSSL_READ_OUT_HASH_ID		= 0x0008,	/*!< (0x08) set when a hash ID is returned */
	RSSL_READ_OUT_UNICAST		= 0x0010,	/*!< (0x10) set when the message was sent unicast to this node */
	RSSL_READ_OUT_INSTANCE_ID	= 0x0020,	/*!< (0x20) set when the message has an instance ID set */
	RSSL_READ_OUT_RETRANSMIT     = 0x0040  	/*!< (0x40) indicates that this message is a retransmission of previous content*/
} RsslReadOutFlags;

typedef struct {
	RsslUInt32	nodeAddr;
	RsslUInt16	port;
} RsslNodeId;

/**
 * @brief RsslReadEx output arguments
 */
typedef struct {
		RsslUInt32              readOutFlags;			/*!< readOutFlags for reading the buffer */
		RsslUInt32              bytesRead;				/*!< bytesRead Returns the number of bytes read on this call to rsslReadEx(). */
		RsslUInt32              uncompressedBytesRead;	/*!< uncompressedBytesRead Returns number of bytes read after decompression. */
		RsslUInt32				hashId;					/*!< The hash ID for the buffer returned */
		RsslNodeId				nodeId;					/*!< The node ID of the node that sent this message */
		RsslUInt32				seqNum;					/*!< The sequence number of the buffer */
		RsslUInt8				FTGroupId;				/*!< The FTGroup of the node that sent this message */
		RsslUInt16				instanceId;				/*!< The instance ID of the sender's channel.  When combined with the sender's IP address and port, contained in the nodeId, 
															 this can be used to identify the specific channel that sent this message. */
} RsslReadOutArgs;

/**
 * @brief RsslReadOutArgs static initialization
 */
#define RSSL_INIT_READ_OUT_ARGS {RSSL_READ_OUT_NO_FLAGS, 0, 0, 0, {0, 0}, 0, 0, 0}

/**
 * @brief Clears the RsslReadInArgs structure passed in
 * @param readInArgs a pointer to the RsslReadInArgs structure to be cleared
 */
RTR_C_INLINE void rsslClearReadInArgs(RsslReadInArgs *readInArgs)
{
	readInArgs->readInFlags = RSSL_READ_IN_NO_FLAGS;
}

/**
 * @brief Clears the RsslReadOutArgs structure passed in
 * @param readOutArgs a pointer to the RsslReadOutArgs structure to be cleared
 */
RTR_C_INLINE void rsslClearReadOutArgs(RsslReadOutArgs *readOutArgs)
{
	readOutArgs->bytesRead = 0;
	readOutArgs->readOutFlags = RSSL_READ_OUT_NO_FLAGS;
	readOutArgs->uncompressedBytesRead = 0;
}

/**
 *	@}
 */
 
/**
 *	@addtogroup RSSLRead
 *	@{
 */
 
/**
 * @brief Reads on a given channel
 *
 * Typical use:<BR>
 * rsslRead is called and returns buffer with any 
 * data read from the channels socketId.  The buffer is only good
 * until the next time rsslRead is called. The buffer used for reading
 * is populated by this call, it is not necessary to use rsslGetBuffer
 * to create a buffer. rsslRead will assign readRet a positive value if there is more
 * data to read, ::RSSL_RET_READ_WOULD_BLOCK if the read call is blocked, or a failure
 * code. 
 *
 * @param chnl RSSL Channel to read from
 * @param readRet RsslRet value or RsslReturnCodes value which is the return value of read.
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslBuffer Buffer that contains data read from network 
 * @see RsslReturnCodes
 */
RSSL_API RsslBuffer* rsslRead(	RsslChannel *chnl,
											RsslRet *readRet,
											RsslError *error);

/**
 * @brief Extended reads on a given channel
 *
 * Typical use:<BR>
 * rsslReadEx is called and returns buffer with any 
 * data read from the channels socketId.  The buffer is only good
 * until the next time rsslReadEx is called. The buffer used for reading
 * is populated by this call, it is not necessary to use rsslGetBuffer
 * to create a buffer. rsslReadEx will assign readRet a positive value if there is more
 * data to read, ::RSSL_RET_READ_WOULD_BLOCK if the read call is blocked, or a failure
 * code. 
 *
 * @param chnl RSSL Channel to read from
 * @param RsslReadInArgs input arguments (RsslReadInArgs)
 * @param RsslReadOutArgs various output values from the read
 * @param readRet RsslRet value or RsslReturnCodes value which is the return value of read
 * @return RsslBuffer Buffer that contains data read from network 
 * @see RsslReturnCodes
 */
RSSL_API RsslBuffer* rsslReadEx(	RsslChannel *chnl,
											RsslReadInArgs *readInArgs,
											RsslReadOutArgs *readOutArgs,
											RsslRet *readRet,
											RsslError *error);

/**
 *	@}
 */

/**
 *	@addtogroup RSSLChannelBuffer RSSL Channel Buffer Operations
 *	@{
 */

/**
 * @brief Retrieves a RsslBuffer for use
 *
 * Typical use: <BR>
 * This is called when a buffer is needed to write data to.  Generally,
 * the user will populate the RsslBuffer structure and then pass it to
 * the rsslWrite function. 
 *
 * @param chnl RSSL Channel who requests the buffer
 * @param size Size of the requested buffer
 * @param packedBuffer Set to RSSL_TRUE if you plan on packing multiple messages into the same buffer
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslBuffer RSSL buffer to be filled in with valid memory
 * @see RsslReturnCodes
 */
RSSL_API RsslBuffer* rsslGetBuffer(	RsslChannel *chnl, 
									RsslUInt32 size, 
									RsslBool  packedBuffer,
									RsslError *error);


/**
 * @brief Releases a RsslBuffer after use
 * 
 * Typical use: <BR>
 * This is called when a buffer is done being used.  
 * The rsslWrite function will release the buffer if it 
 * successfully writes.  The user should only need to use 
 * this function when they get a buffer that they do not need
 * or when rsslWrite fails.
 *
 * @param buffer RSSL buffer to be released
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslRet RSSL return value
 */
RSSL_API RsslRet rsslReleaseBuffer(RsslBuffer *buffer, 
											  RsslError *error);


/**
 * @brief Allows user to pack multiple RSSL encoded messages into the same RSSL Buffer
 *
 * Typical use: <BR>
 * This is called when the application wants to perform message packing.
 * Call rsslGetBuffer to get a new buffer
 * Call necessary encode functions
 * Call rsslPackBuffer to write size of packed buffer and prepare for next packed buffer
 * Repeat above two steps until ready to write
 * Call rsslWrite with buffer
 *
 * @param chnl RsslChannel that the message is being sent on.
 * @param buffer Rssl buffer to be packed (length must be set to encoded size)
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslBuffer Buffer pointing to new position to encode data to
 */
RSSL_API RsslBuffer* rsslPackBuffer(RsslChannel *chnl, 
											   RsslBuffer *buffer,
											   RsslError *error);

/**
 *	@}
 */
 
/**
 *	@addtogroup RSSLWrite
 *	@{
 */
 
/**
 * @brief RsslWriteFlags Flags passed into the rsslWrite function call
 * @see rsslWrite
 */
typedef enum {
	RSSL_WRITE_NO_FLAGS				= 0x00,	/*!< (0x00) No Write Flags */
	RSSL_WRITE_DO_NOT_COMPRESS		= 0x01, /*!< (0x01) indicates that this message should not be compressed even if compression is enabled for this connection */
	RSSL_WRITE_DIRECT_SOCKET_WRITE  = 0x02	/*!< (0x02) Write will attempt to pass the data directly to the transport, avoiding the queuing.  If anything is currently queued, data will be queued.  This option will increase CPU use but may decrease latency */
} RsslWriteFlags;


/**
 * @brief RSSL Write Priorities
 * @see rsslWrite
 */
typedef enum {
	RSSL_HIGH_PRIORITY		= 0, /*!< (0) Assigns message to the high priority flush */
	RSSL_MEDIUM_PRIORITY	= 1, /*!< (1) Assigns message to the medium priority flush */
	RSSL_LOW_PRIORITY		= 2  /*!< (2) Assigns message to the low priority flush */
} RsslWritePriorities;

/**
 * @brief RsslWriteFlagsIn Flags passed into the rsslWriteEx function call
 * @see rsslWriteEx
 */
typedef enum {
	RSSL_WRITE_IN_NO_FLAGS				 = 0x00,	/*!< (0x00) No Write Flags */
	RSSL_WRITE_IN_DO_NOT_COMPRESS		 = 0x01,	/*!< (0x01) indicates that this message should not be compressed even if compression is enabled for this connection */
	RSSL_WRITE_IN_DIRECT_SOCKET_WRITEN    = 0x02,	/*!< @deprecated DEPRECATED: (0x02) Write will attempt to pass the data directly to the transport, avoiding the queuing.  If anything is currently queued, data will be queued.  This option will increase CPU use but may decrease latency */
	RSSL_WRITE_IN_DIRECT_SOCKET_WRITE    = 0x02,	/*!< (0x02) Write will attempt to pass the data directly to the transport, avoiding the queuing.  If anything is currently queued, data will be queued.  This option will increase CPU use but may decrease latency */
	RSSL_WRITE_IN_SEQNUM				 = 0x04,	/*!< (0x04) indicates that the writer wants to attach a sequence number to this message */
	RSSL_WRITE_IN_RETRANSMIT             = 0x10  	/*!< (0x10) indicates that this message is a retransmission of previous content. This requires a user supplied sequence number to indicate which packet is being retransmitted*/
} RsslWriteFlagsIn;

/**
 * @brief RsslWriteEx input arguments
 */
typedef struct {
	   RsslUInt32                        writeInFlags;   /*!< writeInFlags Flags for writing the buffer (RsslWriteInFlags) */
	   RsslWritePriorities				 rsslPriority;   /*!< rsslPriority Priority to flush the message (high, medium, or low) */
	   RsslUInt32						 seqNum;		 /*!< specifies the sequence number of the message  */
} RsslWriteInArgs;

#define RSSL_INIT_WRITE_IN_ARGS {RSSL_WRITE_IN_NO_FLAGS, RSSL_HIGH_PRIORITY, 0}

/**
 * @brief RsslWriteFlags passed into the rsslWriteEx function call
 * @see rsslWriteEx
 */
typedef enum {
	RSSL_WRITE_OUT_NO_FLAGS			= 0x00	/*!< (0x00) No Write Flags */
} RsslWriteOutFlags;

/**
 * @brief RsslWriteEx output arguments
 */
typedef struct {
	   RsslUInt32                        writeOutFlags;	            /*!< writeOutFlags Flags for returning flag information about the outcome of a write */
       RsslUInt32                        bytesWritten;              /*!< bytesWritten Returns total number of bytes written. */
       RsslUInt32                        uncompressedBytesWritten;  /*!< uncompressedBytesWritten Returns number of bytes written before compression. */
} RsslWriteOutArgs;

#define RSSL_INIT_WRITE_OUT_ARGS {RSSL_WRITE_OUT_NO_FLAGS, 0, 0}

/**
 * @brief Clears the RsslWriteInArgs structure passed in
 * @param writeInArgs a pointer to the RsslWriteInArgs structure to be cleared
 */
RTR_C_INLINE void rsslClearWriteInArgs(RsslWriteInArgs *writeInArgs)
{
	writeInArgs->rsslPriority = RSSL_HIGH_PRIORITY;
	writeInArgs->writeInFlags = RSSL_WRITE_IN_NO_FLAGS;
}

/**
 * @brief Clears the RsslWriteOutArgs structure passed in
 * @param writeOutArgs a pointer to the RsslWriteOutArgs structure to be cleared
 */
RTR_C_INLINE void rsslClearWriteOutArgs(RsslWriteOutArgs *writeOutArgs)
{
	writeOutArgs->bytesWritten = 0;
	writeOutArgs->uncompressedBytesWritten = 0;
	writeOutArgs->writeOutFlags = RSSL_WRITE_OUT_NO_FLAGS;
}

/**
 * @brief Writes on a given channel
 *
 * Typical use:<BR>
 * rsslWrite is called after buffer is populated with a message.
 * This message is then written to the channels internal write buffer.  If write
 * is successful, the passed in buffer will be released automatically.  In the
 * event of a failure the user needs to call rsslReleaseBuffer.  In the 
 * success case, rsslWrite will return the number of bytes pending flush.  
 *
 * @note Data is not written across the network until rsslFlush is called. 
 *
 * @param chnl RSSL Channel to write to
 * @param buffer Buffer to write to the network
 * @param rsslPriority Priority to flush the message (high, medium, or low) 
 * @param writeFlags Flags for writing the buffer (RsslWriteFlags)
 * @param bytesWritten Returns total number of bytes written.
 * @param uncompressedBytesWritten Returns number of bytes written before compression.
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslRet RSSL return value, RsslReturnCodes value, or the number of bytes pending flush 
 * @see RsslReturnCodes, RsslWriteFlags
 */
RSSL_API RsslRet rsslWrite(RsslChannel *chnl,
								 	 RsslBuffer *buffer,
									 RsslWritePriorities rsslPriority,
									 RsslUInt8	writeFlags, 
									 RsslUInt32 *bytesWritten,
									 RsslUInt32 *uncompressedBytesWritten,
									 RsslError	*error);

/**
 * @brief Extended writes on a given channel
 *
 * Typical use:<BR>
 * rsslWrite is called after buffer is populated with a message.
 * This message is then written to the channels internal write buffer.  If write
 * is successful, the passed in buffer will be released automatically.  In the
 * event of a failure the user needs to call rsslReleaseBuffer.  In the 
 * success case, rsslWrite will return the number of bytes pending flush.  
 *
 * @note Data is not written across the network until rsslFlush is called. 
 *
 * @param chnl RSSL Channel to write to
 * @param buffer Buffer to write to the network
 * @param writeInArgs input arguments to the function (RsslWriteInArgs)
 * @param writeOutArgs various output values following the write(RsslWriteFlags)
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslRet RSSL return value, RsslReturnCodes value, or the number of bytes pending flush 
 * @see RsslReturnCodes, RsslWriteFlags
 */
RSSL_API RsslRet rsslWriteEx(RsslChannel *chnl,
								 	 RsslBuffer *buffer,
									 RsslWriteInArgs *writeInArgs,
									 RsslWriteOutArgs *writeOutArgs,
									 RsslError	*error);


/**
 * @brief Flushes data waiting to be written on a given channel
 *
 * Typical use:<BR>
 * rsslFlush pushes the data in the write buffer out to the network.  
 * This should be called when write returns that there is data to flush.
 * Under certain circumstances, write will automatically flush data.
 *
 * @param chnl RSSL Channel to attempt flush on
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslRet RSSL return value or the number of bytes pending flush
 */
RSSL_API RsslRet rsslFlush(RsslChannel *chnl,
									  RsslError *error);

/**
 * @brief Sends a heartbeat message
 *
 * Typical use:<BR>
 * rsslPing is called to send some type of ping or heartbeat message.
 * rsslPing will send only the message header across the network.  
 * This helps reduce overhead on the network, and does not incur any
 * cost of parsing or assembling a special ping message type.  
 * It is the user's responsibility to send the ping message  
 * in the correct time frame.  Since it is assumed 
 * a ping or heartbeat is only sent when other writing is not taking place,
 * rsslFlush is automatically called once.  The return value will be the number
 * of bytes left to flush.
 *
 * @param chnl RSSL Channel to send ping or heartbeat on
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslRet RSSL return value or the number of bytes pending flush
 */
RSSL_API RsslRet rsslPing(	RsslChannel *chnl, 
										RsslError *error);
 
/**
 *	@}
 */ 

/**
 *	@addtogroup RSSLTransport
 *	@{
 */
 
/** 
 * @brief Programmatically extracts library and product version information that is compiled into this library
 *
 * User can call this function to programmatically extract version information, or <BR>
 * query version information externally (via 'strings' command or something similar<BR>
 * and grep for the following tags:<BR>
 * 'VERSION' - contains internal library version information such as node number (e.g. rssl1.4.F2)<BR>
 * 'PRODUCT' - contains product information such as load/package naming (e.g. upa7.0.0.L1)<BR>
 * @param pVerInfo RsslLibraryVersionInfo structure to populate with library version information
 * @see RsslLibraryVersionInfo
 */
RSSL_API void rsslQueryTransportLibraryVersion(RsslLibraryVersionInfo *pVerInfo);

/**
 *	@brief RSSL Trace codes used for setting RsslTraceOptions flags
 *	@see RsslTraceOptions
 */
typedef enum {
	RSSL_TRACE_READ					= 0x00000001, /*< (0x00000001) Trace incoming data */
	RSSL_TRACE_WRITE				= 0x00000002, /*< (0x00000002) Trace outgoing data */
	RSSL_TRACE_PING					= 0x00000004, /*< (0x00000004) Trace Pings */
	RSSL_TRACE_HEX					= 0x00000008, /*< (0x00000008) Display hex values of all messages */
	RSSL_TRACE_TO_FILE_ENABLE		= 0x00000010, /*< (0x00000010) Enables tracing to a file*/ 
	RSSL_TRACE_TO_MULTIPLE_FILES    = 0x00000020, /*< (0x00000020) If set, starts writing to a new file if traceMsgMaxFileSize is reached. If disabled, file writing stops when traceMsgMaxFileSize is reached*/
	RSSL_TRACE_TO_STDOUT			= 0x00000040, /*< (0x00000040) Writes the xml trace to stdout. If a non-null value is also provided for traceMsgFileName, writing will be done to stdout and the specified file*/
	RSSL_TRACE_DUMP					= 0x00000080  /*< (0x00000080) Trace dump to enable the rsslDumpBuffer() method to dump RWF or JSON messages. */
} RsslTraceCodes;

/**
 * @brief RSSL Trace Options structure used to configure XML Tracing
 * @see RsslIoctlCodes
 * @see rsslIoctl
 */
typedef struct {
	char*		traceMsgFileName;		/*!< @brief Indicates the name of file to be written to */
	RsslInt64	traceMsgMaxFileSize;	/*!< @brief Indicates maximum size in bytes of file to be written to. Default is 100000000 */
	RsslUInt64	traceFlags;				/*!< @brief  Flags for tracing */
} RsslTraceOptions;

/**
 * @brief Trace Options dynamic initialization
 *
 * This initializes the RsslTraceOptions 
 *
 * @param opts traceOptions
 * @see RsslTraceOptions
 */
RTR_C_INLINE void rsslClearTraceOptions(RsslTraceOptions *traceOptions)
{
	traceOptions->traceMsgFileName = NULL;
	traceOptions->traceMsgMaxFileSize = 100000000;
	traceOptions->traceFlags = 0;
}

/**
 * @brief Calculates necessary buffer size to encrypt content into
 *
 * Typical use:<BR>
 * User populates bufferToEncrypt with content they want to encrypt.
 * Calling this method will return the size of the buffer needed to contain the
 * encrypted output.  User will then typically get or create a buffer of this 
 * size.  Once buffer is obtained, bufferToEncrypt and the buffer for encrypted
 * output can be passed to rsslEncryptBuffer().  
 * 
 * @param bufferToEncrypt RsslBuffer populated with content the user wants to calculate encrypted size of.  buffer->length should represent the number of bytes contained in buffer->data. 
 * @return RsslUInt32 Number of bytes needed in an RsslBuffer to encrypt contents into
 */
RSSL_API RsslUInt32 rsslCalculateEncryptedSize(const RsslBuffer *bufferToEncrypt);


/**
 * @brief If possible, will perform weak encryption of passed in contents
 *
 * Typical use:<BR>
 * User populates bufferToEncrypt with content they want to encrypt.
 * User calculates necessary size for encrypted output buffer by calling rsslCalculateEncryptedSize()
 * User will then typically get or create a buffer of this 
 * size.  Once buffer is obtained, bufferToEncrypt and the buffer for encrypted
 * output can be passed to rsslEncryptBuffer(), which will attempt encryption.  
 * If the RsslChannel has a valid encryption key and the passed in buffers are valid, contents will be encrypted.
 * If the RsslChannel does not have a valid encryption key, most likely due to the connection not having the ability
 * to perform proper key exchange, contents will not be encrypted and an error will be returned.  
 *
 * @param chnl RsslChannel to encrypt contents for transmission on.  Each RsslChannel may have a different encryption key, including no encryption key.
 * @param bufferToEncrypt RsslBuffer populated with content the user wants to encrypt.  buffer->length should represent the number of bytes contained in buffer->data and buffer->data should hold unencrypted content.
 * @param encryptedOutput RsslBuffer with appropriate space to encrypt into.  buffer->length should represent the number of bytes available in buffer->data.
 * @param error RsslError, to be populated in event of an error
 * @return RsslRet RsslRet return value indicating success or failure type.
 */
RSSL_API RsslRet rsslEncryptBuffer(const RsslChannel *chnl, const RsslBuffer* bufferToEncrypt, RsslBuffer* encryptedOutput, RsslError *error);

/**
 * @brief If possible, will perform decryption of passed in encrypted contents.
 *
 * Typical use:<BR>
 * User populates bufferToDecrypt with encrypted content they want to decrypt.
 * User will populated a buffer to decrypt into with sufficient space to properly decrypt,
 * generally bufferToDecrypt->length bytes will be sufficient space for decryptedOutput buffer to 
 * perform proper decryption.
 * bufferToDecrypt and the buffer for decrypted output 
 * can be passed to rsslDecryptBuffer(), which will attempt decryption.  
 * If the RsslChannel has a valid decryption key and the passed in buffers are valid, contents will be decrypted.
 * If the RsslChannel does not have a valid decryption key, most likely due to the connection not having the ability
 * to perform proper key exchange, contents will not be decrypted and an error will be returned.  
 *
 * @param chnl RsslChannel to decrypt contents for transmission on.  Each RsslChannel may have a different decryption key, including no decryption key.
 * @param bufferToDecrypt RsslBuffer populated with content the user wants to decrypt.  buffer->length should represent the number of bytes contained in buffer->data and buffer->data should contain encrypted content.
 * @param decryptedOutput RsslBuffer with appropriate space to decrypt into.  buffer->length should represent the number of bytes available in buffer->data.
 * @param error RsslError, to be populated in event of an error
 * @return RsslRet RsslRet return value indicating success or failure type.
 */
RSSL_API RsslRet rsslDecryptBuffer(const RsslChannel *chnl, const RsslBuffer* bufferToDecrypt, RsslBuffer* decryptedOutput, RsslError *error);

 

/**
 * @brief Calculates necessary buffer size to convert the buffer to a hex dump output
 *
 * Typical use:<BR>
 * User populates bufferToHexDump with content they want to dump as hex.
 * Calling this method will return the size of the buffer needed to contain the
 * hex dump output.  User will then typically get or create a buffer of this 
 * size.  Once buffer is obtained, bufferToHexDump and the buffer for the hex dump
 * output can be passed to rsslBufferToHexDump().  
 * 
 * @param bufferToHexDump RsslBuffer populated with content the user wants to calculate hex dump size of.  buffer->length should represent the number of bytes contained in buffer->data. 
 * @param valuesPerLine Numeric value indicating how many hex values to represent per line in the formatted hex dump output.
 * @return RsslUInt32 Number of bytes needed in an RsslBuffer to successfully dump hex formatted contents into
 */
RSSL_API RsslUInt32 rsslCalculateHexDumpOutputSize(const RsslBuffer *bufferToHexDump, RsslUInt32 valuesPerLine);


/**
 * @brief Will convert buffer's contents to a formatted hex dump with appended string representation
 *
 * Typical use:<BR>
 * User populates bufferToHexDump with content they want to encrypt.
 * User calculates necessary size for hex dump output buffer by calling rsslCalculateHexDumpOutputSize()
 * User will then typically get or create a buffer of this 
 * size.  Once buffer is obtained, bufferToHexDump and the buffer for the hex dump formatted
 * output can be passed to rsslBufferToHexDump(), which will dump the formatted hex.  
 *
 * @param bufferToHexDump RsslBuffer populated with content the user wants to dump as formatted hex.  buffer->length should represent the number of bytes contained in buffer->data and buffer->data should hold unencrypted content.
 * @param hexDumpOutput RsslBuffer with appropriate space to dump formatted hex into.  buffer->length should represent the number of bytes available in buffer->data.
 * @param valuesPerLine Numeric value indicating how many hex values to represent per line in the formatted hex dump output.
 * @param error RsslError, to be populated in event of an error
 * @return RsslRet RsslRet return value indicating success or failure type.
 */
RSSL_API RsslRet rsslBufferToHexDump(const RsslBuffer* bufferToHexDump, RsslBuffer* hexDumpOutput, RsslUInt32 valuesPerLine, RsslError *error);

/**
* @brief Will convert buffer's contents to a formatted hex dump without append string representation
*
* Typical use:<BR>
* User populates bufferToHexDump with content they want to encrypt.
* User calculates necessary size for hex dump output buffer by calling rsslCalculateHexDumpOutputSize()
* User will then typically get or create a buffer of this
* size.  Once buffer is obtained, bufferToHexDump and the buffer for the hex dump formatted
* output can be passed to rsslBufferToHexDump(), which will dump the formatted hex.
*
* @param bufferToHexDump RsslBuffer populated with content the user wants to dump as formatted hex.  buffer->length should represent the number of bytes contained in buffer->data and buffer->data should hold unencrypted content.
* @param hexDumpOutput RsslBuffer with appropriate space to dump formatted hex into.  buffer->length should represent the number of bytes available in buffer->data.
* @param valuesPerLine Numeric value indicating how many hex values to represent per line in the formatted hex dump output.
* @param error RsslError, to be populated in event of an error
* @return RsslRet RsslRet return value indicating success or failure type.
*/
RSSL_API RsslRet rsslBufferToRawHexDump(const RsslBuffer* bufferToHexDump, RsslBuffer* hexDumpOutput, RsslUInt32 valuesPerLine, RsslError *error);

/**
* @brief RSSL debug flags used for setting RsslDebugFlags Flags for the RSSL_DEBUG_FLAGS IOCtl code
* @see RsslIoctlCodes
*/

typedef enum {
	RSSL_DEBUG_IPC_DUMP_IN = 0x0001, /*!<(0x0001) Dump incoming IPC messages as they are received from the network */
	RSSL_DEBUG_IPC_DUMP_OUT = 0x0002, /*!<(0x0002) Dump outgoing IPC messages as they are written to the network */
	RSSL_DEBUG_IPC_DUMP_COMP = 0x0004, /*!<(0x0004) If compression is enabled, dump compressed IPC message */
	RSSL_DEBUG_IPC_DUMP_INIT = 0x0008, /*!<(0x0008) Dump channel IPC initialization messages */
	RSSL_DEBUG_RSSL_DUMP_IN = 0x0010, /*!<(0x0010) Dump incoming RSSL messages as they are received from the transport */
	RSSL_DEBUG_RSSL_DUMP_OUT = 0x0020, /*!<(0x0020) Dump outgoing RSSL messages as they are passed to the transport */
} RsslDebugFlags;

/**
* @brief Sets the debug functions for RSSL
*
* Typical use:<BR>
* This function takes function pointers as arguments.  Internally,
* when receiving or sending data, we call these functions and pass
* them the message as a char*.  This allows the application to perform
* whatever type of tracing or debugging is desired.
* After setting the functions with this call, the debug flags must be
* turned on via the rsslIoctl interface (RSSL_DEBUG_FLAGS).
* The dump functions are global, the debug flags are per RsslChannel.
* @param *dumpIpcIn Function pointer to the incoming IPC message debug function
* @param *dumpIpcOut Function pointer to the outgoing IPC message debug function
* @param *dumpRsslIn Function pointer to the incoming RSSL message debug function
* @param *dumpRsslOut Function pointer to the outgoing RSSL message debug function
* @param error	Rssl Error, to be populated in event of an error
* @return RsslRet RSSL return value
* @see RsslDebugFlags
* @see rsslIoctl
*/
RSSL_API RsslRet rsslSetDebugFunctions(
	void(*dumpIpcIn)(const char *functionName, char *buffer, RsslUInt32 length, RsslUInt64 opaque),
	void(*dumpIpcOut)(const char *functionName, char *buffer, RsslUInt32 length, RsslUInt64 opaque),
	void(*dumpRsslIn)(const char *functionName, char *buffer, RsslUInt32 length, RsslSocket socketId),
	void(*dumpRsslOut)(const char *functionName, char *buffer, RsslUInt32 length, RsslSocket socketId),
	RsslError *error);

/**
* @brief Dump the passed in RWF or JSON messages according to the RsslTraceCodes option for a particular channel.
* The RSSL_TRACE_DUMP flag must be set for this function to take effect.
*
* @param channel RSSL Channel to dump messages.
* @param protocolType The protocol type either RSSL_RWF_PROTOCOL_TYPE(0) or RSSL_JSON_PROTOCOL_TYPE(2).
* @param buffer RsslBuffer contains messages according to the protocol type.
* @param error RsslError, to be populated in event of an error
* @return RsslRet return value indicating success or failure type.
*/
RSSL_API RsslRet rsslDumpBuffer(RsslChannel *channel, RsslUInt32 protocolType, RsslBuffer* buffer, RsslError *error);

/**
 *	@}
 */ 

#define RSSL_IGNORE_CERT_REVOCATION 254 /* (254) Channel: must be used prior to any calls to InitChannel.  
										 	     If value of 0 is passed in, this will unset the 
												 SECURITYH_FLAG_IGNORE_REVOCATION flag for WinInet; 
												 other values passed in will set/enable this flag 
												 to ignore certificate revocation */


#ifdef __cplusplus
};
#endif


#endif
