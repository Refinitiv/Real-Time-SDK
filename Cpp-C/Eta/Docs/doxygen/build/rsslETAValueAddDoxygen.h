/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

/** 
 *	@defgroup Transport API Value Added Components
 *	\brief The Transport API Value Added Components are offered along side of Transport API to allow for 
 *		   more straight forward, rapid creation of Transport API-based applications.  These 
 *		   optional components utilize the Transport API interfaces and are designed to work alongside
 *		   Transport API.  
 *	@{
 */
 
/**
 *	@defgroup VAReactor Transport API Reactor
 *	\brief The Transport API Reactor is a callback-based connection management and event processing component that will 
 *		   signifigantly reduce the amount of code an application must write for OMM-based applications. 
 *		   The Reactor can be used to create Consumer, Interactive Provider, and Non-Interactive Provider 
 *		   applications.  In addition, the Reactor also manages automatic ping handling and outbound buffer 
 *		   flushing for all application types, and provides automatic Consumer and Non-Interactive Provider start-up 
 *		   processing.
 *  @{
 */
 
/**
 *	@defgroup VAReactorStruct RSSL Reactor Structure
 *	\brief The RSSL Reactor structure is used by the Transport API Reactor to manage RSSL Reactor Channel connections.
 *	@{
 */
 
/**
 *	@defgroup VAReactorEvent RSSL Reactor Channel Event Structure
 *	\brief The RSSL Reactor Channel Event structure is used to alert the application that an event has 
 *		   occurred on an RsslReactorChannel, and allows the user to provide the callbacks for processing the event.
 *	@{
 *	@}
 */
 
/**
 *	@}
 */
 
/**
 *	@defgroup VAReactorChnl RSSL Reactor Channel Structure
 *	\brief The RSSL Reactor Channel structure is used by the Transport API Reactor to represent a connection that can 
 *		   send or receive information across a network.  This structure is used by the user to perform actions 
 *		   on the connection that it represents.
 *	@{
 */
 
/**
 *	@defgroup VAReactorChnlOps RSSL Reactor Channel Operation Functions
 *	\brief The RSSL Reactor Channel operation functions are used to perform channel-specific actions.
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
 *	@defgroup RSSLVARDM RSSL RDM Codec API
 *	\brief The RSSL RDM Codec API is intended to assist developers in encoding and decoding 
 *		   OMM domain representations.
 *	@{
 */
 
/**
 *	@defgroup VARDMCommon RSSL RDM Codec Common Structures
 *	\brief The RSSL RDM Codec Common Structures are used by all of the Value Added RDM components. 
 *	@{
 */
 
/**
 *	@defgroup VARDMCommonBase RSSL RDM Message Base Structure
 *	\brief The RDM Message Base structure is used by all Value Added RDM message structures.
 *	@{
 *	@}
 */
 
/**
 *	@}
 */
 
/**
 *	@defgroup VARDMLogin RSSL RDM Login Codec
 *	\brief The RSSL RDM Login component is intended to assist developers with encoding and decoding messages
 *		   used by the Login Domain Model.  This component provides typed data structures, encoding 
 *		   and decoding functions for Login Domain messages.
 *	@{
 */
 
/**
 *	@defgroup VARDMLoginHelper RDM Login Helper Functions
 *	@{
 *	@}
 */
 
 
/**
 *	@}
 */
 
/**
 *	@defgroup VARDMDirectory RSSL RDM Directory Codec
 *	\brief The RSSL RDM Directory component is intended to assist developers with encoding and decoding messages
 *		   used by the Directory Domain Model.  This component provides typed data structures, encoding 
 *		   and decoding functions for Directory Domain messages.
 *	@{
 */
 
/**
 *	@defgroup VARDMDirectoryHelper RDM Directory Helper Functions
 *	@{
 *	@}
 */
 
 
/**
 *	@}
 */
 
/**
 *	@defgroup VARDMDictionary RSSL RDM Dictionary Codec 
 *	\brief The RSSL RDM Dictionary component is intended to assist developers with encoding and decoding messages
 *		   used by the Dictionary Domain Model.  This component provides typed data structures, encoding 
 *		   and decoding functions for Dictionary Domain messages.
 *	@{
 */
 
/**
 *	@defgroup VARDMDictionaryHelper RDM Dictionary Helper Functions
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
 *	@defgroup RSSLVAUtils RSSL Value Add Utility Components
 *	\brief The Value Add Utility component contains helper structures and funcions used by the entire
 *		   Transport API Value Added package.
 *	@{
 */
 
/**
 *	@}
 */

/**
 * @defgroup RSSLVACacheGroup RSSL Value Add Payload Cache
 * \brief The Value Add Payload Cache component can be used to store and retrieve encoded OMM data containers.
 * Functions are single threaded unless specified otherwise.
 * @{
 */
 
/**
 * @defgroup RSSLVACacheInstance RSSL Value Add Payload Cache Instance
 * \brief An instance of a payload cache is a collection of payload data entries associated with a single
 *        RDM Field Dictionary.
 * @{
 */

/**
 * @}
 */

/**
 * @defgroup RSSLVAPayloadEntry RSSL Value Add Cache Payload Entry
 * \brief A cache Payload Entry is a container for one of the supported OMM payload
 *        types that can be stored in the cache: Field List, Element List, Map,
 *        Series, Vector, Filter List
 * @{
 */

/**
 * @}
 */

/**
 * @defgroup RSSLVAPayloadCursor RSSL Value Add Cache Payload Cursor
 * \brief The cache Payload Cursor is used to support multi-part (fragmented)
 * retrieval of encoded OMM data from the cache entry.
 * @{
 */

/**
 * @}
 */

/**
 * @defgroup RSSLVAPayloadCacheUtil RSSL Value Add Cache Utilities
 * \brief Utility methods and structures used in the payload cache interface
 * @{
 */

/**
 * @}
 */

/**
 * @}
 */
 
 /**
  *	@}
  */
