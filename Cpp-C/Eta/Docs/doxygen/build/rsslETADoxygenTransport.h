/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2018-2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

 /**
 *	@addtogroup RSSLTransport RSSL Transport API
 *	@brief The Transport API provides the interface to a reliable transport that is capable of communicating with other OMM-based components.
 *	@{
 */
/**
 *	@defgroup RSSLTransportStructs RSSL Transport Common Data Structures and Enumerations
 *	@brief The RSSL Common Data Structures and Enumerations are used by the RSSL API to store information  
 *	@{
 *	@}
 */

/**
 *	@defgroup RSSLInit RSSL Initialization and Uninitialization
 *	@brief These functions are used to allocate internal memory and boot strap any underlying dependencies used by the RSSL Transport.
 *	@{
 */
 
/**
 *	@}
 */
 
/**
 *	@defgroup RSSLServer RSSL Server Operations
 *	@brief The RSSL Server structure is used to create and maintain a listening socket that allows the application to accept connections created with rsslConnect.
 *	@{
 */
 
/**
 *	@defgroup RSSLServerBind RSSL Bind Operation
 *	@brief The rsslBind function establishes a listening socket connection that can be used for both standard and HTTP rsslConnect connections.
 *	@{
 */

/**
 *	@defgroup RSSLServerBindOpts RSSL Bind Operation Options
 *	@{
 *	@}
 */

/**
 *	@}
 */

 /**
 *	@}
 */
 
/**
 *	@defgroup RSSLChannel RSSL Channel Operations
 *	@brief The RSSL Channel structure is used to represent a connection between two components that can send and receive information across a network.
 *	@{
 */
 
/**
 *	@defgroup RSSLChannelConnection RSSL Client Channel Connection Operation
 *	@brief The rsslConnect function establishes an outbound connection to a server.
 *	@{
 */
 
 /**
 *	@defgroup RSSLConnectOpts RSSL Client Channel Connection Options
 *	@{
 */
 
/**
 *	@}
 */
 
/**
 *	@}
 */
 
/**
 *	@defgroup RSSLServerAccept RSSL Server Channel Accept Operation
 *	@brief The rsslAccept function is used to begin the accepting process for an incoming connection request.
 *	@{
 */
 
/**
 *	@defgroup RSSLServerAcceptOpts RSSL Accept Operation Options
 *	@{
 *	@}
 */
 
/**
 *	@}
 */
 
/**
 *	@defgroup RSSLChannelOps RSSL Client and Server Channel Operations
 *	@brief These functions are used by both the Client and Server to finalize the rsslChannel initialization, read from the channel, write to the channel, and request buffers from the shared pool buffers
 *	@{
 */
 
/**
 *	@defgroup RSSLRead RSSL Read Operations
 *	@brief The rsslRead function is used to read data received from the active rsslChannel. 
 *	@{
 *	@}
 */
 
/**
 *	@defgroup RSSLWrite RSSL Write Operations
 *	@brief The rsslWrite function is used to write data to the active rsslChannel.
 *	@{
 */
 
/**
 *	@defgroup RSSLChannelBuffer RSSL Channel Buffer Operations
 *	@brief The RSSL Channel Buffer functions are used to acquire and release buffers from the guaranteed output buffers associated with an active rsslChannel or the shared buffer pool associated with an rsslServer.
 *	@{
 */
 
/**
 *	@}
 */

/**
 *	@}
 */
 

 
/**
 *	@}
 */
 
/**
 *	@}
 */
 

/**
 *	@}
 */


